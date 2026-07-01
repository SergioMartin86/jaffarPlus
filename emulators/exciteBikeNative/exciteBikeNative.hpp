#pragma once
#include <cstring>

/**
 * @file exciteBikeNative.hpp
 * @brief JaffarPlus emulator wrapper around the native Excitebike physics engine (excitebike::Engine).
 *
 * Instead of emulating the NES 6502, this drives the from-scratch, byte-exact native model (see the
 * exciteBot project / docs/2d_engine_spec.md). It is reward-faithful for posX and ~10-100x faster with
 * a TINY hashable state: the engine's packed working set is 64 bytes (vs ~2KB of NES RAM), so with the
 * game's "Bypass Emulator State": false the search dedup key is this small state -- the depth advantage.
 *
 * The native model does NOT emulate the boot/countdown, so the starting state is seeded from a real
 * race-start RAM snapshot via the "Race Start RAM File Path" emulator-config key (the config's
 * "Initial Sequence File Path" must be empty -- there is no boot to replay). The game header
 * games/nes/exciteBike.hpp is reused unchanged: it reads memory via getProperty("LRAM").
 */

// engine.hpp is the single source of truth, pulled from the exciteBot git submodule. Included by
// explicit relative path so the submodule's source/ is NOT on the global include path (its input.hpp
// would shadow jaffarPlus's). engine.hpp's own #include "track_layout.hpp"/"track_sections.hpp" then
// resolve via this directory's include dir (the generated, gitignored track data).
#include "../../extern/exciteBot/source/engine.hpp"
#include "inputParser.hpp"
#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <memory>
#include <string>

namespace jaffarPlus
{

namespace emulator
{

class ExciteBikeNative final : public Emulator
{
public:
  static std::string getName() { return "ExciteBikeNative"; }

  // Constructor: parse configuration only (no heavy init), so dry runs work.
  ExciteBikeNative(const nlohmann::json& config) : Emulator(config)
  {
    // Race-start RAM snapshot (stands in for the un-modeled boot/countdown). 2048 bytes. Optional:
    // if absent, initialize() uses the engine's flat baseline (Engine::reset()) -- e.g. for ROM-free tests.
    if (_emulatorConfigRemaining.contains("Race Start RAM File Path")) _raceStartRamPath = jaffarCommon::json::popString(_emulatorConfigRemaining, "Race Start RAM File Path");

    // Optional mid-run seed: a full engine state (the 68-byte serialized Engine::State written by
    // jaffar-player --saveStateFile). When set, initialize() loads it AFTER the race-start RAM seed, so the
    // search can start from an arbitrary frame (e.g. for isolating a deep region). Pair with the game's
    // "Initial Block Transitions" so the absolute Bike Pos X is correct from the seeded frame.
    if (_emulatorConfigRemaining.contains("Initial State File Path")) _initialStatePath = jaffarCommon::json::popString(_emulatorConfigRemaining, "Initial State File Path");

    // Reuse the NES joypad input parser (identical "|..|UDLRSsBA|" -> controller byte decoding).
    // It reads the controller-type keys without popping, so consume them here too; the base factory
    // always calls finalizeEmulatorConfig(), which rejects any leftover key.
    _inputParser = std::make_unique<jaffar::InputParser>(config);
    jaffarCommon::json::popString(_emulatorConfigRemaining, "Controller 1 Type");
    jaffarCommon::json::popString(_emulatorConfigRemaining, "Controller 2 Type");
  }

  ~ExciteBikeNative() = default;

  void initializeImpl() override
  {
    if (_raceStartRamPath.empty())
    {
      jaffarCommon::logger::log("[J+] WARNING: ExciteBikeNative has no 'Race Start RAM File Path'; using flat baseline (frame 0 will not match the real race start).\n");
      _engine.reset();
    }
    else
    {
      std::string ram;
      if (jaffarCommon::file::loadStringFromFile(ram, _raceStartRamPath) == false) JAFFAR_THROW_LOGIC("Could not read race-start RAM snapshot: '%s'", _raceStartRamPath.c_str());
      if (ram.size() < excitebike::Engine::LRAM_SIZE)
        JAFFAR_THROW_LOGIC("Race-start RAM snapshot '%s' is %zu bytes, need >= %zu", _raceStartRamPath.c_str(), ram.size(), (size_t)excitebike::Engine::LRAM_SIZE);
      _engine.seedFromRam(reinterpret_cast<const uint8_t*>(ram.data()));
    }

    // Mid-run seed: overlay a saved full engine state (the 68-byte Engine::State from --saveStateFile).
    if (!_initialStatePath.empty())
    {
      std::string st;
      if (jaffarCommon::file::loadStringFromFile(st, _initialStatePath) == false) JAFFAR_THROW_LOGIC("Could not read 'Initial State File Path': '%s'", _initialStatePath.c_str());
      if (st.size() < sizeof(excitebike::Engine::State))
        JAFFAR_THROW_LOGIC("Initial state '%s' is %zu bytes, need >= %zu", _initialStatePath.c_str(), st.size(), sizeof(excitebike::Engine::State));
      excitebike::Engine::State s;
      memcpy(&s, st.data(), sizeof(s));
      _engine.deserialize(s);
    }
  }

  jaffar::InputParser* getInputParser() const override { return _inputParser.get(); }

  // The native engine's advance() takes the jaffar joypad code (A=0x01..R=0x80), which is exactly
  // the NES input parser's port1 byte; it bit-reverses internally into RAM 0x5C.
  void advanceStateImpl(const jaffar::input_t& input) override { _engine.advance((uint8_t)input.port1); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override
  {
    excitebike::Engine::State s;
    _engine.serialize(s);
    serializer.push(&s, sizeof(s));
  }
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override
  {
    excitebike::Engine::State s;
    deserializer.pop(&s, sizeof(s));
    _engine.deserialize(s);
  }

  // The game (games/nes/exciteBike.hpp) reads guest memory via getProperty("LRAM").pointer[addr].
  // We expose the engine's modeled 2KB RAM image (reward-critical addresses are byte-exact; the rest
  // is scratch/zero, unused by the game's rules/reward).
  property_t getProperty(const std::string& propertyName) const override
  {
    if (propertyName == "LRAM") return property_t((uint8_t*)_engine.lram(), excitebike::Engine::LRAM_SIZE);
    JAFFAR_THROW_LOGIC("Property '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void printInfo() const override
  {
    jaffarCommon::logger::log("[J+]  + Bike Pos X:    %.3f\n", _engine.bikePosX());
    jaffarCommon::logger::log("[J+]  + Bike Vel X:    %u\n", _engine.velX16());
    jaffarCommon::logger::log("[J+]  + Race Over:     %u\n", _engine.ram(0x52));
    jaffarCommon::logger::log("[J+]  + State Size:    %zu bytes\n", sizeof(excitebike::Engine::State));
  }

  // Render-related: no-ops (the native engine has no video output).
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
  excitebike::Engine                   _engine;
  std::string                          _raceStartRamPath;
  std::string                          _initialStatePath;
  std::unique_ptr<jaffar::InputParser> _inputParser;
};

} // namespace emulator

} // namespace jaffarPlus
