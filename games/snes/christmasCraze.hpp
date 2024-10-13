#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace snes
{

class ChristmasCraze final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "SNES / Christmas Craze"; }

  ChristmasCraze(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Global Timer", &_lowMem[0x000000], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_lowMem[0x004091], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x004095], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Speed X", &_lowMem[0x004099], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Speed Y", &_lowMem[0x00409D], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Direction", &_lowMem[0x0040AD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game State", &_lowMem[0x001FD6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Presents Grabbed", &_lowMem[0x0040AD], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _globalTimer     = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _playerPosX      = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY      = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _playerDirection = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Direction")]->getPointer();
    _playerSpeedX    = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Player Speed X")]->getPointer();
    _playerSpeedY    = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Player Speed Y")]->getPointer();
    _gameState       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game State")]->getPointer();
    _presentsGrabbed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Presents Grabbed")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override {}

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs((float)_pointMagnet.x - (float)*_playerPosX);
    _player1DistanceToPointY = std::abs((float)_pointMagnet.y - (float)*_playerPosY);
    _player1DistanceToPoint  = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) {}

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_pointMagnet.intensity) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
        jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
        jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
        jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
        auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
        rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
        recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the x point of attraction
    float y         = 0.0; // What is the y point of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _pointMagnet;

  uint16_t *_globalTimer;
  uint16_t *_playerPosX;
  uint16_t *_playerPosY;
  uint16_t *_playerSpeedX;
  uint16_t *_playerSpeedY;
  uint8_t  *_playerDirection;
  uint8_t  *_gameState;
  uint8_t  *_presentsGrabbed;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;

  // Pointer to emulator's low memory storage
  uint8_t *_lowMem;
};

} // namespace snes

} // namespace games

} // namespace jaffarPlus