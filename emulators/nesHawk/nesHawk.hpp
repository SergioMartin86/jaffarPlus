#pragma once

/**
 * @file nesHawk.hpp
 * @brief JaffarPlus emulator wrapper around the C++ translation of BizHawk's NesHawk core.
 *
 * NesHawk is the cycle-accurate ground-truth NES core (BizHawk, C#, MIT), translated 1:1 to C++
 * under extern/quickerNesHawk/source/ and validated byte-exact against the genuine C# implementation
 * (per-frame 2KB RAM over a 55,412-frame full-game movie plus both level-1 solutions; see
 * extern/quickerNesHawk/harness for the golden oracle). Use it when search results must hold on real
 * hardware timing: QuickerNES is instruction-granular and diverges from NesHawk on
 * interrupt-timing-sensitive glitches (e.g. the Prince of Persia U+D action glitch family).
 *
 * ~10-30x slower per frame than QuickerNES -- cycle accuracy is the price of ground truth.
 *
 * The translated core is a single system configuration matching the Prince of Persia (NES) cart:
 * UNROM (mapper 2), 128KB PRG, 8KB CHR-RAM, vertical mirroring, NTSC, one joypad. Other carts
 * need their board translated first (extern/quickerNesHawk/source/nesBoards.hpp).
 *
 * Config keys (subset of QuickerNES's, same meanings): "Rom File Path", "Rom File SHA1",
 * "Initial State File Path", "Initial Sequence File Path", "Initial RAM Data File Path",
 * "Controller 1 Type", "Controller 2 Type".
 */

#include "../../extern/quickerNesHawk/source/nes.hpp"
#include "../../extern/quickerNesHawk/source/nesSerialization.hpp"
#include "inputParser.hpp"
#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <jaffarCommon/string.hpp>
#include <memory>
#include <string>
#include <vector>

namespace jaffarPlus
{

namespace emulator
{

class NesHawk final : public Emulator
{
public:
  static std::string getName() { return "NesHawk"; }

  // Constructor must only do configuration parsing (no heavy init), so dry runs work.
  NesHawk(const nlohmann::json& config) : Emulator(config)
  {
    _romFilePath             = jaffarCommon::json::popString(_emulatorConfigRemaining, "Rom File Path");
    _romFileSHA1             = jaffarCommon::json::popString(_emulatorConfigRemaining, "Rom File SHA1");
    _initialStateFilePath    = jaffarCommon::json::popString(_emulatorConfigRemaining, "Initial State File Path");
    _initialSequenceFilePath = jaffarCommon::json::popString(_emulatorConfigRemaining, "Initial Sequence File Path");
    _initialRAMDataFilePath  = jaffarCommon::json::popString(_emulatorConfigRemaining, "Initial RAM Data File Path");

    // The NES joypad input parser ("|..|UDLRSsBA|" -> port byte A=0x01..R=0x80, which is exactly
    // the core's ButtonBit order). It reads the controller-type keys without popping, so consume
    // them here for the strict leftover-key check.
    _inputParser = std::make_unique<jaffar::InputParser>(config);
    jaffarCommon::json::popString(_emulatorConfigRemaining, "Controller 1 Type");
    jaffarCommon::json::popString(_emulatorConfigRemaining, "Controller 2 Type");
  }

  ~NesHawk() = default;

  void initializeImpl() override
  {
    // Reading from ROM file
    std::string romFileData;
    if (jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str()) == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = jaffarCommon::hash::getSHA1String(romFileData);
    if (_romFileSHA1 != actualRomSHA1)
      JAFFAR_THROW_LOGIC("ROM file: '%s' expected SHA1 ('%s') does not coincide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());

    // Power-on (parses the iNES header, boots the hardcoded NES-UNROM board)
    _nes = std::make_unique<nesHawk::NES>(reinterpret_cast<const uint8_t*>(romFileData.data()), romFileData.size());
    _stateBuffer.resize(nesHawk::nesStateSize(*_nes));

    // If initial state file defined, load it
    if (_initialStateFilePath.empty() == false)
    {
      std::string initialState;
      if (jaffarCommon::file::loadStringFromFile(initialState, _initialStateFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial state file: %s\n", _initialStateFilePath.c_str());

      jaffarCommon::deserializer::Contiguous d(initialState.data(), initialState.size());
      deserializeState(d);
    }

    // Advancing the state using the initial sequence, if provided
    if (_initialSequenceFilePath != "")
    {
      std::string initialSequenceFileString;
      if (jaffarCommon::file::loadStringFromFile(initialSequenceFileString, _initialSequenceFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial sequence file: %s\n", _initialSequenceFilePath.c_str());

      const auto initialSequence = jaffarCommon::string::split(initialSequenceFileString, '\0');
      for (const auto& inputString : initialSequence) advanceStateImpl(_inputParser->parseInputString(inputString));
    }

    // Pushing initial RAM data
    if (_initialRAMDataFilePath != "")
    {
      std::string initialRAMDataString;
      if (jaffarCommon::file::loadStringFromFile(initialRAMDataString, _initialRAMDataFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from RAM Data file: %s\n", _initialRAMDataFilePath.c_str());

      memcpy(_nes->ram, initialRAMDataString.data(), 0x800);
    }
  }

  jaffar::InputParser* getInputParser() const override { return _inputParser.get(); }

  void advanceStateImpl(const jaffar::input_t& input) override
  {
    // Lag-frame sibling cache: when a frame is lag (the game never latched the controller,
    // nes->islag), its outcome is provably input-independent -- every sibling input expanded from
    // the same parent produces the byte-identical child. The search engine's expansion pattern is
    // deserialize(parent) -> advance(input_k) for each candidate input, so on a lag frame the
    // first sibling's result can be served to the rest as a state load (~100x cheaper than a
    // frame). Exact: keyed on the full parent state bytes, valid only in the
    // deserialize-then-advance pattern, and the cached child is loaded into the core so every
    // later observer (serialize, memory properties) sees the real state.
    if (_parentFresh && _lagChildValid && _stateBuffer == _lagParent)
    {
      nesHawk::StateLoader l(_lagChild.data());
      nesHawk::syncNesState(*_nes, l);
      updateHaltLatch();
      _parentFresh = false;
      return;
    }

    const bool parentWasFresh = _parentFresh;
    _parentFresh              = false;

    _nes->FrameAdvance((uint8_t)input.port1);
    updateHaltLatch();

    if (parentWasFresh && _nes->islag)
    {
      // _stateBuffer still holds the parent bytes (advance does not touch it)
      _lagParent = _stateBuffer;
      _lagChild.resize(_stateBuffer.size());
      nesHawk::StateSaver s(_lagChild.data());
      nesHawk::syncNesState(*_nes, s);
      _lagChildValid = true;
    }
  }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override
  {
    nesHawk::StateSaver s(_stateBuffer.data());
    nesHawk::syncNesState(*_nes, s);
    serializer.push(_stateBuffer.data(), _stateBuffer.size());
  }

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializer.pop(_stateBuffer.data(), _stateBuffer.size());
    nesHawk::StateLoader l(_stateBuffer.data());
    nesHawk::syncNesState(*_nes, l);
    updateHaltLatch();
    _parentFresh = true; // enables the lag-frame sibling cache for the next advance
  }

  property_t getProperty(const std::string& propertyName) const override
  {
    // Same names as the QuickerNES wrapper, so game headers work unchanged. All pointers are
    // stable after initialize (the core never reallocates its state objects mid-run).
    if (propertyName == "LRAM") return property_t(_nes->ram, sizeof(_nes->ram));
    if (propertyName == "NTAB") return property_t(_nes->CIRAM, sizeof(_nes->CIRAM));
    if (propertyName == "CHRR") return property_t(_nes->board->Vram.data(), _nes->board->Vram.size());
    if (propertyName == "SPRT") return property_t(_nes->ppu->OAM, sizeof(_nes->ppu->OAM));
    if (propertyName == "PALR") return property_t(_nes->ppu->PALRAM, sizeof(_nes->ppu->PALRAM));
    // CPU halt detector: 1 when the CPU is executing a KIL/JAM opcode. NesHawk's Jam micro-op
    // freezes the microcode step forever (NMI is never serviced), so a KIL opcode observed at a
    // frame boundary means the CPU is jammed for good -- games should register this and FAIL the
    // state, matching real-hardware freeze semantics. Derived from cpu.opcode (serialized), so it
    // is recomputed after every advance/deserialize and needs no storage of its own.
    if (propertyName == "CPU Halt Latch") return property_t(const_cast<uint8_t*>(&_cpuHaltLatch), 1);
#ifdef _NESHAWK_DETECT_BAD_ACCESS
    // Per-frame ground-truth bad-access flag (unofficial opcode / RAM execution this frame). The
    // glitch discriminator for run 3: a REAL derail sets this; a normal off-path room transition
    // does not. Points directly at the CPU's latch (stable after initialize).
    if (propertyName == "CPU Bad Access") return property_t(&_nes->cpu->badAccessLatch, 1);
#endif

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void printInfo() const override
  {
    jaffarCommon::logger::log("[J+]  + Initial Sequence File Path:          '%s'\n", _initialSequenceFilePath.c_str());
    jaffarCommon::logger::log("[J+]  + Frame / Lag Count:                   %d / %d\n", _nes->_frame, _nes->_lagcount);
    jaffarCommon::logger::log("[J+]  + State Size:                          %zu bytes\n", _stateBuffer.size());
    jaffarCommon::logger::log("[J+]  + CPU Halted:                          %u\n", _cpuHaltLatch);
  }

  // Render-related: no-ops for now (searches run with --disableRender; for visualization replay
  // the solution on the QuickerNES build or export it to BizHawk).
  void              initializeVideoOutput() override {}
  void              finalizeVideoOutput() override {}
  __INLINE__ void   enableRendering() override {}
  __INLINE__ void   disableRendering() override {}
  __INLINE__ void   updateRendererState(const size_t /*stepIdx*/, const std::string /*input*/) override {}
  __INLINE__ void   serializeRendererState(jaffarCommon::serializer::Base& serializer) const override { serializeState(serializer); }
  __INLINE__ void   deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override { deserializeState(deserializer); }
  __INLINE__ size_t getRendererStateSize() const override
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }
  __INLINE__ void showRender() override {}

private:
  __INLINE__ void updateHaltLatch()
  {
    // KIL/JAM opcodes: x2 for x in {0,1,2,3,4,5,6,7,9,B,D,F}. A jammed CPU never leaves the
    // opcode, so this is exact (and inherently sticky) at frame granularity.
    switch (_nes->cpu->opcode)
    {
      case 0x02:
      case 0x12:
      case 0x22:
      case 0x32:
      case 0x42:
      case 0x52:
      case 0x62:
      case 0x72:
      case 0x92:
      case 0xB2:
      case 0xD2:
      case 0xF2: _cpuHaltLatch = 1; break;
      default: _cpuHaltLatch = 0; break;
    }
  }

  std::unique_ptr<nesHawk::NES>        _nes;
  std::unique_ptr<jaffar::InputParser> _inputParser;
  mutable std::vector<uint8_t>         _stateBuffer;
  uint8_t                              _cpuHaltLatch = 0;

  // lag-frame sibling cache (see advanceStateImpl)
  bool                 _parentFresh   = false;
  bool                 _lagChildValid = false;
  std::vector<uint8_t> _lagParent;
  std::vector<uint8_t> _lagChild;

  std::string _romFilePath;
  std::string _romFileSHA1;
  std::string _initialStateFilePath;
  std::string _initialSequenceFilePath;
  std::string _initialRAMDataFilePath;
};

} // namespace emulator

} // namespace jaffarPlus
