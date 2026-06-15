#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>


namespace jaffarPlus
{

namespace games
{

namespace a2600
{

class SpaceInvaders final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "A2600 / Space Invaders"; }

  SpaceInvaders(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Remaining Enemies", &_lowMem[0x11], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Shot 1 Active",     &_lowMem[0x4C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Shot 1 Pos Y",      &_lowMem[0x4E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Shot 2 Pos Y",      &_lowMem[0x56], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Type",        &_lowMem[0x01], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _remainingEnemies   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Remaining Enemies")]->getPointer();
    _shot1Active         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Shot 1 Active")]->getPointer();
    _shot1PosY           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Shot 1 Pos Y")]->getPointer();
    _shot2PosY           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Shot 2 Pos Y")]->getPointer();
    _frameType           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Type")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    //  hashEngine.Update(_lowMem, 0x80);
     for (size_t i = 0; i < 0x80; i++)
      if (i != 0x06) // ?
      if (i != 0x6E) // Player Input
      if (i != 0x4A) // Timer
      if (i != 0x51) // Enemy Bullet
      if (i != 0x52) // Enemy Bullet
      if (i != 0x5A) // Enemy Stuff?
      if (i != 0x6A) // Enemy Stuff?
      if (i < 0x2B || i > 0x42) // Brick statuses
       hashEngine.Update(_lowMem[i]);

      // if (i != 0x1C) // Timer ?
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    if (*_shot1Active > 0)
    {
      reward += 100.0;
      reward += (float)*_shot1PosY;
    }

    if (*_shot2PosY < 90.0)
    {
      reward += 100.0;
      reward += (90.0) - (float)*_shot2PosY;
    }
    
    reward -= (float)*_remainingEnemies * 1000.0;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Frame Type:          %u\n", *_frameType);
    jaffarCommon::logger::log("[J+]  + Remaining Enemies:   %u\n", *_remainingEnemies);
    jaffarCommon::logger::log("[J+]  + Shot 1 Active:       %u\n", *_shot1Active);
    jaffarCommon::logger::log("[J+]  + Shot 1 Pos Y:        %u\n", *_shot1PosY);
    jaffarCommon::logger::log("[J+]  + Shot 2 Pos Y:        %u\n", *_shot2PosY);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }


  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;

  uint8_t* _remainingEnemies;
  uint8_t* _shot1Active;
  uint8_t* _shot1PosY;
  uint8_t* _shot2PosY;
  uint8_t* _frameType;

};

} // namespace a2600

} // namespace games

} // namespace jaffarPlus