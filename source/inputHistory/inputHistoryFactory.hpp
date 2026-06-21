#pragma once

/**
 * @file inputHistoryFactory.hpp
 * @brief Builds the configured @ref InputHistory strategy from the "Store Input History" config object.
 *        The shared backing (the trie, for the "Trie" strategy) is created once per search via
 *        @ref createSharedBacking and then bound into each runner's instance via @ref create. None/Raw
 *        have no shared backing. Default type when unspecified: "Trie".
 */

#include "inputHistory.hpp"
#include "inputHistoryNone.hpp"
#include "inputHistoryRaw.hpp"
#include "inputHistoryTrie.hpp"
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{

namespace inputHistory
{

/// @brief The configured strategy name ("None" | "Raw" | "Trie"). Required -- JaffarPlus configs must set
/// every option explicitly, so an absent "Type" is an error (thrown by the strict accessor).
__INLINE__ std::string getType(const nlohmann::json& config) { return jaffarCommon::json::getString(config, "Type"); }

/**
 * @brief Creates the shared backing for the configured strategy (the trie for "Trie"; nullptr otherwise).
 * @param numShards Number of contention-free free-list shards (one per worker thread + one for the
 *                  StateDb/driver manager operations).
 * @return An opaque shared handle held by the caller for the lifetime of the search; pass it to @ref create.
 */
__INLINE__ std::shared_ptr<void> createSharedBacking(const nlohmann::json& config, const uint32_t numShards)
{
  if (getType(config) == "Trie") return std::make_shared<SequenceInputTrie>(numShards);
  return nullptr;
}

/**
 * @brief Creates one runner's input-history instance bound to @p backing.
 * @param maxInputIndex One past the highest input index (per-step bit width for raw/snapshot encoding).
 * @param numShards     Shard count the backing was created with (the last shard is the manager shard).
 * @param shardId       This runner's free-list shard (its worker thread id; 0 for standalone runners).
 * @param backing       The shared handle from @ref createSharedBacking (ignored by None/Raw).
 */
__INLINE__ std::unique_ptr<InputHistory> create(const nlohmann::json& config, const uint32_t maxInputIndex, const uint32_t numShards, const uint32_t shardId,
                                                const std::shared_ptr<void>& backing)
{
  // Strictly consume the config: pop the recognized keys, then reject any leftover (unknown) key.
  auto                          cfg = config;
  const auto                    type = jaffarCommon::json::popString(cfg, "Type");
  std::unique_ptr<InputHistory> result;
  if (type == "None") result = std::make_unique<InputHistoryNone>();
  else if (type == "Raw") result = std::make_unique<InputHistoryRaw>(maxInputIndex, jaffarCommon::json::popNumber<uint32_t>(cfg, "Max Size"));
  else if (type == "Trie")
    result = std::make_unique<InputHistoryTrie>(static_cast<SequenceInputTrie*>(backing.get()), shardId, numShards - 1, maxInputIndex, jaffarCommon::json::popNumber<uint32_t>(cfg, "Max Size"));
  else
    JAFFAR_THROW_LOGIC("Unrecognized 'Store Input History' Type: '%s' (expected None, Raw or Trie)", type.c_str());
  jaffarCommon::json::checkEmpty(cfg, "Runner Configuration > Store Input History");
  return result;
}

} // namespace inputHistory

} // namespace jaffarPlus
