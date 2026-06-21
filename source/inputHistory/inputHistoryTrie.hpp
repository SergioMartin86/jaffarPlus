#pragma once

/**
 * @file inputHistoryTrie.hpp
 * @brief Input-history strategy that stores each state's path as a single node id into a shared,
 *        reference-counted prefix trie (jaffarCommon::sequenceTrie). Sibling states share their common
 *        prefix, so per-state cold storage is just 4 bytes and total path memory is far smaller than the
 *        raw bit-packed history. Selected with `{"Type": "Trie", "Max Size": N}` ("Max Size" bounds only
 *        the reconstructed snapshot/solution buffer; the live search path is unbounded).
 *
 * Cold form (StateDb slot): [node id (4)][count (4)]. Full form (self-contained snapshot): the path
 * reconstructed into a bit-packed buffer + count, so snapshots need no reference back into the trie.
 */

#include "inputHistory.hpp"
#include <cstring>
#include <jaffarCommon/bitwise.hpp>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/sequenceTrie.hpp>
#include <vector>

namespace jaffarPlus
{

/// @brief The shared trie type backing every InputHistoryTrie instance of one search.
using SequenceInputTrie = jaffarCommon::sequenceTrie::SequenceTrie<InputSet::inputIndex_t>;

/// @brief Stores each state's path as one node id into a shared, reference-counted prefix trie; sibling
/// states share their common prefix, so per-state cold storage is just a node id + count. `{"Type":"Trie"}`.
class InputHistoryTrie final : public InputHistory
{
public:
  using nodeId_t = SequenceInputTrie::nodeId_t; ///< Trie node identifier type.

  /// @brief Binds this instance to the shared trie and sizes the bit-packed snapshot (full) form.
  /// @param trie          The shared trie (created once per search; outlives all instances).
  /// @param shardId       This runner's contention-free free-list shard (its worker thread id).
  /// @param managerShard  A shard reserved for StateDb/driver-thread operations (captureColdToFull).
  /// @param maxInputIndex One past the highest input index (sets the snapshot's per-step bit width).
  /// @param maxSize       Maximum steps held in the reconstructed snapshot (full) form.
  InputHistoryTrie(SequenceInputTrie* trie, const uint32_t shardId, const uint32_t managerShard, const uint32_t maxInputIndex, const uint32_t maxSize)
      : _trie(trie), _shardId(shardId), _managerShard(managerShard), _maxSize(maxSize)
  {
    _bits         = jaffarCommon::bitwise::getEncodingBitsForElementCount(maxInputIndex);
    _bitpackBytes = (_maxSize * _bits + 7) / 8;
    _scratch.resize(_bitpackBytes, 0);
  }

  void reset() override
  {
    if (_ownNode) _trie->release(_node, _shardId);
    _node    = SequenceInputTrie::ROOT;
    _ownNode = false;
    _count   = 0;
  }

  void pushInput(const InputSet::inputIndex_t input) override
  {
    // Append as a new owned node; the prior owned node (e.g. a frameskip sub-frame) is kept alive by the
    // new node's child edge, so drop this runner's transient reference to it. The trie is unbounded.
    const nodeId_t next = _trie->extend(_node, input, _shardId);
    if (_ownNode) _trie->release(_node, _shardId);
    _node    = next;
    _ownNode = true;
    _count++;
  }

  size_t getInputCount() const override { return _count; }

  void serializeCold(jaffarCommon::serializer::Base& s) const override
  {
    _trie->acquire(_node); // the destination slot becomes a holder of this path node
    s.pushContiguous(&_node, sizeof(_node));
    s.pushContiguous(&_count, sizeof(_count));
  }
  void deserializeCold(jaffarCommon::deserializer::Base& d) override
  {
    if (_ownNode) _trie->release(_node, _shardId); // drop a just-discarded candidate node, if any
    d.popContiguous(&_node, sizeof(_node));
    d.popContiguous(&_count, sizeof(_count));
    _ownNode = false; // borrowed from the slot (kept alive until the slot is freed)
  }

  void serializeFull(jaffarCommon::serializer::Base& s) const override
  {
    reconstructIntoBuffer(_node, _scratch.data()); // self-contained bit-packed copy (no trie reference)
    s.pushContiguous(_scratch.data(), _bitpackBytes);
    s.pushContiguous(&_count, sizeof(_count));
  }
  void deserializeFull(jaffarCommon::deserializer::Base& d) override
  {
    d.popContiguous(_scratch.data(), _bitpackBytes);
    d.popContiguous(&_count, sizeof(_count));
    rebuildNodeFromBuffer(_scratch.data()); // so a restored runner (e.g. the player) can keep extending
  }

  std::string toString(const std::map<InputSet::inputIndex_t, std::string>& inputStringMap) const override
  {
    std::vector<InputSet::inputIndex_t> seq;
    _trie->reconstruct(_node, seq);
    std::string out;
    for (size_t i = 0; i < seq.size() && i < _maxSize; i++)
    {
      if (inputStringMap.contains(seq[i]) == false) JAFFAR_THROW_RUNTIME("Move Index %u not found in runner\n", seq[i]);
      out += inputStringMap.at(seq[i]) + std::string("\n");
    }
    return out;
  }

  size_t getColdSize() const override { return sizeof(_node) + sizeof(_count); }
  size_t getFullSize() const override { return _bitpackBytes + sizeof(_count); }

  void initColdSlot(void* cold) const override { *reinterpret_cast<nodeId_t*>(cold) = SequenceInputTrie::NONE; }

  size_t getApproxMemoryBytes() const override { return _trie->getApproxMemoryBytes(); }

  void releaseColdSlot(void* cold, const size_t shard) const override
  {
    auto* node = reinterpret_cast<nodeId_t*>(cold);
    if (*node != SequenceInputTrie::NONE)
    {
      _trie->release(*node, (uint32_t)shard);
      *node = SequenceInputTrie::NONE;
    }
  }

  void captureColdToFull(const void* cold, void* full) const override
  {
    // Cold = [node id][count]; full = [bit-packed history][count]. Reconstruct the node's path into the
    // full buffer so the snapshot is self-contained (no later reference into the trie). Use memcpy for the
    // node id and count: the count lives at offset _bitpackBytes in `full`, which is not 4-byte aligned, so
    // a direct uint32_t store there would be undefined (misaligned) behavior.
    nodeId_t node;
    uint32_t count;
    std::memcpy(&node, cold, sizeof(node));
    std::memcpy(&count, reinterpret_cast<const uint8_t*>(cold) + sizeof(nodeId_t), sizeof(count));
    reconstructIntoBuffer(node, reinterpret_cast<uint8_t*>(full), /*pin=*/true);
    std::memcpy(reinterpret_cast<uint8_t*>(full) + _bitpackBytes, &count, sizeof(count));
  }

private:
  /// @brief Reconstructs @p node's path into a bit-packed @p buffer. When @p pin, briefly holds a
  /// reference so a worker freeing the source slot cannot free the node mid-walk (best/worst snapshots
  /// run off the search threads).
  void reconstructIntoBuffer(nodeId_t node, uint8_t* buffer, bool pin = false) const
  {
    std::memset(buffer, 0, _bitpackBytes);
    if (node == SequenceInputTrie::NONE) return;
    if (pin) _trie->acquire(node);
    std::vector<InputSet::inputIndex_t> seq;
    _trie->reconstruct(node, seq);
    if (pin) _trie->release(node, _managerShard);
    for (size_t i = 0; i < seq.size() && i < _maxSize; i++) jaffarCommon::bitwise::bitcopy(buffer, _bitpackBytes, i, &seq[i], sizeof(InputSet::inputIndex_t), 0, 1, _bits);
  }

  /// @brief Rebuilds the cursor's owned trie node by re-extending the path stored in a bit-packed @p buffer
  /// (so a restored runner, e.g. the player, can keep extending the path).
  void rebuildNodeFromBuffer(const uint8_t* buffer)
  {
    if (_ownNode) _trie->release(_node, _shardId);
    _node              = SequenceInputTrie::ROOT;
    _ownNode           = false;
    const size_t steps = (_count < _maxSize) ? _count : _maxSize;
    for (size_t i = 0; i < steps; i++)
    {
      InputSet::inputIndex_t idx = 0;
      jaffarCommon::bitwise::bitcopy(&idx, sizeof(InputSet::inputIndex_t), 0, buffer, _bitpackBytes, i, 1, _bits);
      const nodeId_t next = _trie->extend(_node, idx, _shardId);
      if (_ownNode) _trie->release(_node, _shardId);
      _node    = next;
      _ownNode = true;
    }
  }

  SequenceInputTrie* const     _trie;                                   ///< The shared trie backing this instance.
  const uint32_t               _shardId;                                ///< This runner's free-list shard.
  const uint32_t               _managerShard;                           ///< Shard for off-thread manager ops.
  const uint32_t               _maxSize;                                ///< Max steps in the snapshot (full) form.
  size_t                       _bits         = 0;                       ///< Bits per input index in the snapshot.
  size_t                       _bitpackBytes = 0;                       ///< Size of the bit-packed snapshot buffer.
  nodeId_t                     _node         = SequenceInputTrie::ROOT; ///< Cursor's current trie node.
  bool                         _ownNode      = false;                   ///< Whether this cursor holds a reference to _node.
  uint32_t                     _count        = 0;                       ///< Number of inputs applied so far.
  mutable std::vector<uint8_t> _scratch;                                ///< Bit-pack scratch for the full (snapshot) form.
};

} // namespace jaffarPlus
