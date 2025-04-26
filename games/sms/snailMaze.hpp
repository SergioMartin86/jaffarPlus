#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace sms
{

class SnailMaze final : public jaffarPlus::Game
{
  public:

  enum transitionState_t
  {
    normal,
    exiting,
    entering
  };

  static __INLINE__ std::string getName() { return "SMS / Snail Maze"; }

  SnailMaze(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|...|......|");

    // Registering native game properties
    registerGameProperty("Player Pos X", &_workRAM[0x0300], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_workRAM[0x0380], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Type", &_workRAM[0x041F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game State", &_workRAM[0x0200], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _playerPosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override {}

  __INLINE__ void updateCharActualPos() {}

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _playerPointMagnet.posX = 0;
    _playerPointMagnet.posY = 0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _playerDistanceToPointX = std::abs((float)*_playerPosX - _playerPointMagnet.posX);
    _playerDistanceToPointY = std::abs((float)*_playerPosY - _playerPointMagnet.posY);
    _playerDistanceToPoint  = std::sqrt(_playerDistanceToPointX * _playerDistanceToPointX + _playerDistanceToPointY * _playerDistanceToPointY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.push(&_currentStep, sizeof(_currentStep));
    serializer.push(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.pop(&_currentStep, sizeof(_currentStep));
    deserializer.pop(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    reward -= _playerDistanceToPoint;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Player Point Magnet                      Pos X: %3.3f Pos Y: %3.3f\n", _playerPointMagnet.posX, _playerPointMagnet.posY);
    jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _playerDistanceToPointX);

    if (std::abs(_lastInputMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Last Input Magnet                      Intensity: %.5f\n", _lastInputMagnet);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Player Point Magnet")
      {
        auto posX = jaffarCommon::json::getNumber<float>(actionJs, "Pos X");
        auto posY = jaffarCommon::json::getNumber<float>(actionJs, "Pos Y");
        rule.addAction([=, this]() { this->_playerPointMagnet = pointMagnet_t{.posX = posX, .posY = posY}; });
        recognizedActionType = true;
    }

    if (actionType == "Set Last Input Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto action    = [=, this]() { this->_lastInputMagnet = intensity; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  // Datatype to describe a 1D point magnet
  struct pointMagnet_t
  {
    float posX = 0.0; // What is the point of attraction
    float posY = 0.0; // What is the point of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _playerPointMagnet;
  uint8_t      *_playerPosX;
  uint8_t      *_playerPosY;

  float _playerDistanceToPointX;
  float _playerDistanceToPointY;
  float _playerDistanceToPoint;
  float _lastInputMagnet;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;

  // Pointer to emulator's low memory storage
  uint8_t *_workRAM;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace sms

} // namespace games

} // namespace jaffarPlus