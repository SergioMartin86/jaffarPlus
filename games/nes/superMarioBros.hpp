#pragma once

#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class SuperMarioBros final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "NES / Super Mario Bros"; }

  SuperMarioBros(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Mario Pos X1", &_lowMem[0x0400], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Mario Pos X2", &_lowMem[0x0086], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Mario Pos X3", &_lowMem[0x006D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Mario Pos Y1", &_lowMem[0x0416], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Mario Pos Y2", &_lowMem[0x00CE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Mario Vel X1", &_lowMem[0x0057], Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Mario Vel Y1", &_lowMem[0x0433], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Mario Vel Y2", &_lowMem[0x009F], Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Mario Walking Frame", &_lowMem[0x070D], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Mario Pos X", &_marioPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Mario Pos Y", &_marioPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _marioPosX1        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Pos X1")]->getPointer();
    _marioPosX2        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Pos X2")]->getPointer();
    _marioPosX3        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Pos X3")]->getPointer();
    _marioPosY1        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Pos Y1")]->getPointer();
    _marioPosY2        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Pos Y2")]->getPointer();
    _marioVelX1        = (int8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Vel X1")]->getPointer();
    _marioVelY1        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Vel Y1")]->getPointer();
    _marioVelY2        = (int8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Vel Y2")]->getPointer();
    _marioWalkingFrame = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Mario Walking Frame")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const std::string &input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x0001], 0x0018);
    hashEngine.Update(&_lowMem[0x001C], 0x0050);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _marioPosX = ((float)*_marioPosX1) / 256.0f + (float)*_marioPosX2 + ((float)*_marioPosX3) * 256.0f;
    _marioPosY = ((float)*_marioPosY1) / 256.0f + (float)*_marioPosY2;
  }

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
    _marioDistanceToPointX = std::abs(_pointMagnet.x - _marioPosX);
    _marioDistanceToPointY = std::abs(_pointMagnet.y - _marioPosY);
    _marioDistanceToPoint  = sqrtf(_marioDistanceToPointX * _marioDistanceToPointX + _marioDistanceToPointY * _marioDistanceToPointY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) {}

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _marioDistanceToPoint;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Mario Position X                           %.5f\n", _marioPosX);
    jaffarCommon::logger::log("[J+]  + Mario Position Y                           %.5f\n", _marioPosY);

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _marioDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _marioDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _marioDistanceToPoint);
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

  uint8_t *_marioPosX1;
  uint8_t *_marioPosX2;
  uint8_t *_marioPosX3;
  uint8_t *_marioPosY1;
  uint8_t *_marioPosY2;

  int8_t  *_marioVelX1;
  uint8_t *_marioVelY1;
  int8_t  *_marioVelY2;

  uint8_t *_marioWalkingFrame;

  // Game-Specific values
  float _marioDistanceToPointX;
  float _marioDistanceToPointY;
  float _marioDistanceToPoint;

  // Game-Specific values
  float _marioPosX;
  float _marioPosY;

  // Pointer to emulator's low memory storage
  uint8_t *_lowMem;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus