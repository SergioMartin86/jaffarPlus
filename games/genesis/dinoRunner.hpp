#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace genesis
{

class DinoRunner final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "Genesis / Dino Runner"; }

  DinoRunner(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Game Mode",    &_workRAM[0x083E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global Timer", &_workRAM[0x0AAA], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Score",        &_workRAM[0x0842], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_workRAM[0x000C], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Vel Y", &_workRAM[0x001A], Property::datatype_t::dt_int16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _gameMode        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game Mode")]->getPointer();
    _globalTimer     = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _score           = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Score")]->getPointer();
    _playerPosY      = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _playerVelY      = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Player Vel Y")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const std::string &input) override
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
    _pointMagnet.y         = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPoint = std::abs((float)_pointMagnet.y - (float)*_playerPosY);
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
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _player1DistanceToPoint);
    }
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .y = y}; });
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
    float y         = 0.0; // What is the y point of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _pointMagnet;

  uint16_t *_globalTimer;
  uint16_t *_playerPosY;
  uint16_t *_score;
  int16_t  *_playerVelY;
  uint8_t  *_gameMode;

  // Game-Specific values
  float _player1DistanceToPoint;

  // Pointer to emulator's low memory storage
  uint8_t *_workRAM;
};

} // namespace SNES

} // namespace games

} // namespace jaffarPlus