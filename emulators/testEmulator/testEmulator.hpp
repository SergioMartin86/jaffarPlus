#pragma once

// A minimal, fully deterministic emulator used exclusively for testing JaffarPlus
// itself (engine, state DB, hash DB, serialization, rules, reproduction). It models
// a cursor moving on a bounded W x H grid; there is no ROM and no real console core.

#include "inputParser.hpp"
#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <memory>

namespace jaffarPlus
{

namespace emulator
{

class TestEmulator final : public Emulator
{
public:
  static std::string getName() { return "TestEmulator"; }

  // The constructor must only parse configuration so that dry runs are possible.
  TestEmulator(const nlohmann::json& config) : Emulator(config)
  {
    _width  = jaffarCommon::json::getNumber<uint8_t>(config, "Grid Width");
    _height = jaffarCommon::json::getNumber<uint8_t>(config, "Grid Height");
    _startX = jaffarCommon::json::getNumber<uint8_t>(config, "Start X");
    _startY = jaffarCommon::json::getNumber<uint8_t>(config, "Start Y");

    if (_width == 0 || _height == 0) JAFFAR_THROW_LOGIC("Grid dimensions must be non-zero (got %u x %u)", _width, _height);
    if (_startX >= _width || _startY >= _height) JAFFAR_THROW_LOGIC("Start position (%u, %u) lies outside the grid", _startX, _startY);

    _inputParser = std::make_unique<jaffar::InputParser>(config);

    _state[0] = _startX;
    _state[1] = _startY;
  }

  ~TestEmulator() = default;

  void initializeImpl() override
  {
    _state[0] = _startX;
    _state[1] = _startY;
  }

  jaffar::InputParser* getInputParser() const override { return _inputParser.get(); }

  // Applies a cursor move, clamping to the grid boundaries (moves off the edge are no-ops).
  void advanceStateImpl(const jaffar::input_t& input) override
  {
    switch (input.move)
    {
      case 1: // up
        if (_state[1] > 0) _state[1]--;
        break;
      case 2: // down
        if (_state[1] < _height - 1) _state[1]++;
        break;
      case 3: // left
        if (_state[0] > 0) _state[0]--;
        break;
      case 4: // right
        if (_state[0] < _width - 1) _state[0]++;
        break;
      default: break; // none
    }
  }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override { serializer.push(_state, sizeof(_state)); }

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override { deserializer.pop(_state, sizeof(_state)); }

  property_t getProperty(const std::string& propertyName) const override
  {
    if (propertyName == "Position") return property_t((uint8_t*)_state, sizeof(_state));

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void printInfo() const override
  {
    jaffarCommon::logger::log("[J+]  + Grid Size:   %u x %u\n", _width, _height);
    jaffarCommon::logger::log("[J+]  + Cursor:      (%u, %u)\n", _state[0], _state[1]);
  }

  // Render-related functions: no-ops, this emulator has no video output
  void            initializeVideoOutput() override {}
  void            finalizeVideoOutput() override {}
  __INLINE__ void enableRendering() override {}
  __INLINE__ void disableRendering() override {}
  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}
  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base& serializer) const override { serializeState(serializer); }
  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override { deserializeState(deserializer); }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override {}

private:
  // Cursor position: _state[0] = x (column), _state[1] = y (row)
  uint8_t _state[2] = {0, 0};

  uint8_t _width  = 1;
  uint8_t _height = 1;
  uint8_t _startX = 0;
  uint8_t _startY = 0;

  std::unique_ptr<jaffar::InputParser> _inputParser;
};

} // namespace emulator

} // namespace jaffarPlus
