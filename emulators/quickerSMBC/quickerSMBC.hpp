#pragma once

#include <SMBCInstance.hpp>
#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <cstring>
#include <mutex>

namespace jaffarPlus
{

namespace emulator
{

class QuickerSMBC final : public Emulator
{
public:
  static std::string getName() { return "QuickerSMBC"; }

  // Constructor must only do configuration parsing
  QuickerSMBC(const nlohmann::json& config) : Emulator(config)
  {
    // Getting initial state file from the configuration
    _initialStateFilePath = jaffarCommon::json::popString(_emulatorConfigRemaining, "Initial State File Path");

    // Getting optional initial RAM-data file. When set, its bytes overwrite the 2KB work RAM after
    // initialization, letting a run start from an arbitrary RAM image (e.g. another emulator's work
    // RAM, for cross-validation with aligned starting states). Optional: defaults to none.
    _initialRAMDataFilePath = _emulatorConfigRemaining.contains("Initial RAM Data File Path")
                                  ? jaffarCommon::json::popString(_emulatorConfigRemaining, "Initial RAM Data File Path")
                                  : "";

    // The ROM file path/SHA1 are only used by the player build, but they are always popped here so the
    // strict-key check below accounts for them regardless of build configuration.
    _romFilePath = jaffarCommon::json::popString(_emulatorConfigRemaining, "Rom File Path");
    _romFileSHA1 = jaffarCommon::json::popString(_emulatorConfigRemaining, "Rom File SHA1");

// Only load rom file if using player
#ifdef _JAFFAR_PLAYER

    // For testing purposes, the rom file path can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERSMBC_OVERRIDE_ROM_FILE_PATH")) _romFilePath = std::string(value);

    // For testing purposes, the rom file SHA1 can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERSMBC_OVERRIDE_ROM_FILE_SHA1")) _romFileSHA1 = std::string(value);

#endif

    // Creating internal emulator instance
    _quickerSMBC = std::make_unique<smbc::EmuInstance>(config);

    // All recognized emulator-configuration keys have now been consumed; reject any leftover (unrecognized) key.
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _quickerSMBC->initialize();

// Only load rom file if using player
#ifdef _JAFFAR_PLAYER

    // Reading from ROM file
    std::string romFileData;
    bool        status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = jaffarCommon::hash::getSHA1String(romFileData);
    if (_romFileSHA1 != actualRomSHA1)
      JAFFAR_THROW_LOGIC("ROM file: '%s' expected SHA1 ('%s') does not concide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());

#endif

    // Loading rom into emulator
    _quickerSMBC->loadROM(_romFilePath);

    // If initial state file defined, load it
    if (_initialStateFilePath.empty() == false)
    {
      // Reading from initial state file
      std::string initialState;
      bool        success = jaffarCommon::file::loadStringFromFile(initialState, _initialStateFilePath);
      if (success == false) JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial state file: %s\n", _initialStateFilePath.c_str());

      // Deserializing initial state into the emulator. Initial-state (.state) files hold only the core
      // engine state, so restore that directly (NOT the wrapper's deserializeState, which also expects
      // the appended lag-frame flag) and reset the flag -- a clean starting state is never mid-lag. This
      // keeps existing .state files loadable while the flag still travels with search/state-DB states.
      jaffarCommon::deserializer::Contiguous d(initialState.data(), initialState.size());
      _quickerSMBC->deserializeState(d);
      _owedLagFrame = false;
    }

    // Pushing initial RAM data, if provided (overwrites the 2KB work RAM after state load)
    if (_initialRAMDataFilePath.empty() == false)
    {
      std::string initialRAMDataString;
      if (jaffarCommon::file::loadStringFromFile(initialRAMDataString, _initialRAMDataFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from RAM Data file: %s\n", _initialRAMDataFilePath.c_str());

      memcpy(_quickerSMBC->getRamPointer(), initialRAMDataString.data(), 0x800);
    }
  }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override
  {
    // SMB lag-frame fidelity: on the real NES, every area load "loses" one frame -- the load runs with
    // the NMI disabled while the screen is rebuilt, so that frame the game state is frozen (the NMI's
    // FrameCounter increment and pseudo-random LSFR churn don't happen). The decompiled core collapses
    // the load into a single frame, so it has no equivalent lost frame; the result is that after every
    // level transition its FrameCounter and LSFR run one step ahead of the NES. The player never reads
    // those, so player physics stay in sync, but FrameCounter parity gates enemy movement (e.g. the
    // every-other-frame piranha-plant rise/fall) and the LSFR drives enemy behaviour, so long runs
    // eventually desync. Reproduce the lost frame: detect the area-load (GameEngineSubroutine drops
    // non-zero->0 with the screen disabled in game mode) and, on the following step, hold the state
    // (consume the input without advancing) -- a frozen frame matching the NES's, which re-aligns
    // FrameCounter, the LSFR and hence enemy timing.
    if (_owedLagFrame)
    {
      _owedLagFrame = false;
      return; // frozen NES lag frame: consume this (null transition) input without advancing
    }

    uint8_t* ram     = _quickerSMBC->getRamPointer();
    uint8_t  prevGES = ram[0x000E]; // GameEngineSubroutine before the frame

    _quickerSMBC->advanceState(input);

    _owedLagFrame = ram[0x0770] == 1     // OperMode == game mode
                 && ram[0x0772] == 1     // OperMode_Task == area-load
                 && ram[0x000E] == 0     // GameEngineSubroutine now idle
                 && ram[0x0774] == 1     // DisableScreenFlag set
                 && prevGES != 0;        // ...and it just dropped from non-zero (load start)
  }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override
  {
    _quickerSMBC->serializeState(serializer);
    // The pending-lag-frame flag (see advanceStateImpl) spans two steps, so it must travel with the
    // serialized state for a state-saving best-first search to reproduce the held lag frame correctly
    // after restoring a just-entered-area state. It is appended after the core engine state so that
    // legacy initial-state (.state) files -- which hold only the engine state -- remain loadable via
    // the engine-only path in initializeImpl().
    serializer.push(&_owedLagFrame, sizeof(_owedLagFrame));
  };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override
  {
    _quickerSMBC->deserializeState(deserializer);
    deserializer.pop(&_owedLagFrame, sizeof(_owedLagFrame));
  };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string& propertyName) const override
  {
    if (propertyName == "LRAM") return property_t(_quickerSMBC->getRamPointer(), 0x800);

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override { _quickerSMBC->initializeVideoOutput(); }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerSMBC->finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerSMBC->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerSMBC->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.push(_quickerSMBC->getVideoBufferPointer(), _quickerSMBC->getVideoBufferSize());
    serializeState(serializer);
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializer.pop(_quickerSMBC->getVideoBufferPointer(), _quickerSMBC->getVideoBufferSize());
    deserializeState(deserializer);
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerSMBC->updateRenderer(); }

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override { return _quickerSMBC->getInputParser(); }

private:
  std::unique_ptr<smbc::EmuInstance> _quickerSMBC;

  std::string _romFilePath = "";
  std::string _romFileSHA1;

  std::string _initialStateFilePath;
  std::string _initialRAMDataFilePath;

  // True when the previous step started an area load, so the next step must reproduce the NES's frozen
  // lag frame (see advanceStateImpl). Part of the serialized state (serializeState) so it survives the
  // state save/restore a best-first search performs.
  bool _owedLagFrame = false;
};

} // namespace emulator

} // namespace jaffarPlus