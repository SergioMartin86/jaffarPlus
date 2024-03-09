#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{

namespace games
{

namespace NES
{

class Sprilo final : public jaffarPlus::Game
{
  public:
  static inline std::string getName() { return "NES / Sprilo"; }

  Sprilo(std::unique_ptr<Emulator> emulator, const nlohmann::json &config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Parsing configuration
    _lastInputStepReward = jaffarCommon::json::getNumber<float>(config, "Last Input Step Reward");

    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Current Lap", &_lowMem[0x0016], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Timer", &_lowMem[0x001B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_lowMem[0x0002], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x0003], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Angle", &_lowMem[0x0004], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Lap Progress", &_lowMem[0x07FF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Last Input Step", &_lastInputStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Current Step", &_currentStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _currentLap = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Current Lap")]->getPointer();
    _timer = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Timer")]->getPointer();
    _playerPosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _lapProgress = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Lap Progress")]->getPointer();

    // Initializing time since last input counter
    _lastInputStep = 0;
    _currentStep = 0;
  }

  private:
  inline void advanceStateImpl(const std::string &input) override
  {
    // Increasing counter if input is null
    if (input != "|..|........|") _lastInputStep = _currentStep;

    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;
  }

  inline void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x0001], 0x0018);
    hashEngine.Update(&_lowMem[0x001C], 0x0050);
  }

  // Updating derivative values after updating the internal state
  inline void stateUpdatePostHook() override
  {
  }

  inline void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x = 0.0;
    _pointMagnet.y = 0.0;
    _stopProcessingReward = false;
  }

  inline void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs((float)_pointMagnet.x - (float)*_playerPosX);
    _player1DistanceToPointY = std::abs((float)_pointMagnet.y - (float)*_playerPosY);
    _player1DistanceToPoint = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);
  }

  inline void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
  }

  inline void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
  }

  inline float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputStepReward * _lastInputStep;

    // If this is a win state, then evaluate only w.r.t. how long since the last input
    if (_stopProcessingReward) return reward;

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Reward Lap Progress
    reward += *_lapProgress * 100.0f;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J++]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J++]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
      jaffarCommon::logger::log("[J++]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
      jaffarCommon::logger::log("[J++]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Advance Lap Progress")
    {
      rule.addAction([this]() { *_lapProgress = *_lapProgress + 1; });
      recognizedActionType = true;
    }

    if (actionType == "Clear Lap Progress")
    {
      rule.addAction([this]() { *_lapProgress = 0; });
      recognizedActionType = true;
    }

    if (actionType == "Stop Processing Reward")
    {
      rule.addAction([this]() { _stopProcessingReward = true; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x = 0.0;         // What is the x point of attraction
    float y = 0.0;         // What is the y point of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _pointMagnet;

  uint8_t *_currentLap;
  uint8_t *_timer;
  uint8_t *_playerPosX;
  uint8_t *_playerPosY;
  uint8_t *_lapProgress;

  // Additions to make the last input as soon as possible
  uint16_t _lastInputStep;
  uint16_t _currentStep;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;

  // Pointer to emulator's low memory storage
  uint8_t *_lowMem;

  // Reward for the last time an input was made (for early termination)
  float _lastInputStepReward;

  // Specifies whether the reward should continue to be processed (for early termination)
  bool _stopProcessingReward;
};

} // namespace NES

} // namespace games

} // namespace jaffarPlus