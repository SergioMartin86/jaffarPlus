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

class Sonic final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "Genesis / Sonic"; }

  Sonic(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Sonic Pos X",  &_workRAM[0x00D008], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Sonic Pos Y",  &_workRAM[0x00D00C], Property::datatype_t::dt_uint16, Property::endianness_t::little);

    registerGameProperty("Ring Count",   &_workRAM[0x00FE21], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 1",      &_workRAM[0x00FE29], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 2",      &_workRAM[0x00FE28], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 3",      &_workRAM[0x00FE27], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 4",      &_workRAM[0x00FE26], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Sonic Accel", &_workRAM[0x00F766], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Checkpoint",  &_workRAM[0x00FE30], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _sonicPosX  = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Sonic Pos X")]->getPointer();
    _sonicPosY  = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Sonic Pos Y")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override 
  {
     hashEngine.Update(&_workRAM[0xC800], 0xD300 - 0xC800); // Sonic properties
     hashEngine.Update(&_workRAM[0xF100], 0xF100 - 0xF002);
     //hashEngine.Update(&_workRAM[0xF600], 0xF900 - 0xF600);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointXMagnet.intensity = 0.0;
    _pointXMagnet.pos       = 0.0;
    _pointYMagnet.intensity = 0.0;
    _pointYMagnet.pos       = 0.0;
    _point2DMagnet.intensity = 0.0;
    _point2DMagnet.posX      = 0.0;
    _point2DMagnet.posY      = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _sonic1DistanceToPointX = std::abs((float)_pointXMagnet.pos - (float)*_sonicPosX);
    _sonic1DistanceToPointY = std::abs((float)_pointYMagnet.pos - (float)*_sonicPosY);

    // Calculating 2D distance
    float distX = std::abs((float)_point2DMagnet.posX - (float)*_sonicPosX);
    float distY = std::abs((float)_point2DMagnet.posY - (float)*_sonicPosY);
    _sonic1DistanceToPoint2D = std::sqrt(distX*distX + distY*distY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) {}

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnets
    reward += -1.0 * _pointXMagnet.intensity * _sonic1DistanceToPointX;
    reward += -1.0 * _pointYMagnet.intensity * _sonic1DistanceToPointY;
    reward += -1.0 * _point2DMagnet.intensity * _sonic1DistanceToPoint2D;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_pointXMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point X Magnet                           Intensity: %.5f, Pos: %3.3f\n", _pointXMagnet.intensity, _pointXMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _sonic1DistanceToPointX);
    }

    if (std::abs(_pointYMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Y Magnet                           Intensity: %.5f, Pos: %3.3f\n", _pointYMagnet.intensity, _pointYMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _sonic1DistanceToPointY);
    }

    if (std::abs(_point2DMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point 2D Magnet                          Intensity: %.5f, Pos: %3.3f, %3.3f\n", _point2DMagnet.intensity, _point2DMagnet.posX, _point2DMagnet.posY);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _sonic1DistanceToPoint2D);
    }
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_pointXMagnet = point1DMagnet_t{.intensity = intensity, .pos = pos }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Point Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_pointYMagnet = point1DMagnet_t{.intensity = intensity, .pos = pos }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Point 2D Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto posX      = jaffarCommon::json::getNumber<float>(actionJs, "Pos X");
      auto posY      = jaffarCommon::json::getNumber<float>(actionJs, "Pos Y");
      rule.addAction([=, this]() { this->_point2DMagnet = point2DMagnet_t{ .intensity = intensity, .posX = posX, .posY = posY }; });
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
  struct point1DMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float pos       = 0.0; // What is the point of attraction
  };

  // Datatype to describe a 2D point magnet
  struct point2DMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float posX      = 0.0; // What is the point X of attraction
    float posY      = 0.0; // What is the point Y of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  point1DMagnet_t _pointXMagnet;
  point1DMagnet_t _pointYMagnet;
  point2DMagnet_t _point2DMagnet;

  uint16_t *_sonicPosX;
  uint16_t *_sonicPosY;

  // Game-Specific values
  float _sonic1DistanceToPointX;
  float _sonic1DistanceToPointY;
  float _sonic1DistanceToPoint2D;

  // Pointer to emulator's low memory storage
  uint8_t *_workRAM;
};

} // namespace genesis

} // namespace games

} // namespace jaffarPlus