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

class SpiderMan final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "A2600 / Spider-Man"; }

  SpiderMan(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Web Pos Y",         &_lowMem[0x11], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Spider Pos Y1",     &_lowMem[0x19], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Spider Pos Y2",     &_lowMem[0x1D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game State",        &_lowMem[0x69], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player State",      &_lowMem[0x0E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Web Length",        &_lowMem[0x79], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Falling State",     &_lowMem[0x06], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Type",        &_lowMem[0x01], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    

    // Getting some properties' pointers now for quick access later
    _webPosY           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Web Pos Y")]->getPointer();
    _spiderPosY1       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Spider Pos Y1")]->getPointer();
    _spiderPosY2       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Spider Pos Y2")]->getPointer();
    _gameState         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game State")]->getPointer();
    _playerState       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player State")]->getPointer();
    _webLength         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Web Length")]->getPointer();
    _fallingState      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Falling State")]->getPointer();
    _frameType         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Type")]->getPointer();

    _inputNull    = _emulator->registerInput("|.....|.....|");
    _inputU       = _emulator->registerInput("|.....|U....|");
    _inputB       = _emulator->registerInput("|.....|....B|");
    _inputUB      = _emulator->registerInput("|.....|U...B|");
    _inputULB     = _emulator->registerInput("|.....|U.L.B|");
    _inputURB     = _emulator->registerInput("|.....|U..RB|");

    _remainingDistance = 0;
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
      // if (i != 0x02) // Timer?
      if (i != 0x14) // Timer?
      if (i != 0x5F) // Timer?
      if (i != 0x60) // Timer?
      if (i != 0x61) // Timer?
      if (i != 0x62) // Timer?
      if (i != 0x63) // Timer?
         hashEngine.Update(_lowMem[i]);
  }

    // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _remainingDistance = (uint16_t)*_spiderPosY1 * 30 + (uint16_t)*_spiderPosY2;
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.push(&_remainingDistance, sizeof(_remainingDistance));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.pop(&_remainingDistance, sizeof(_remainingDistance));
  }


  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;
    
    reward -= (float)_remainingDistance * 3.0;
    reward += (float)*_webLength;

    // Returning reward
    return reward;
  }

  __INLINE__ std::set<std::string> getAllPossibleInputs()
  {
     return {
             "|.....|.....|",
             "|.....|U....|",
             "|.....|....B|",
             "|.....|U...B|",
             "|.....|U.L.B|",  
             "|.....|U..RB|",  
            };
  }

   __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    if (*_frameType == 255) { allowedInputSet.insert(allowedInputSet.end(), { _inputNull }); return; }
    if (*_webLength == 0) { allowedInputSet.insert(allowedInputSet.end(), { _inputU, _inputUB, _inputULB, _inputURB }); return; }
    allowedInputSet.insert(allowedInputSet.end(), { _inputU, _inputB });
  }


  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Frame Type:          %u\n", *_frameType);
    jaffarCommon::logger::log("[J+]  + Game State:          %u\n", *_gameState);
    jaffarCommon::logger::log("[J+]  + Player State:        %u\n", *_playerState);
    jaffarCommon::logger::log("[J+]  + Web Post Y:          %u\n", *_webPosY);
    jaffarCommon::logger::log("[J+]  + Web Length:          %u\n", *_webLength);
    jaffarCommon::logger::log("[J+]  + Spider Pos Y:        %u / %u \n", *_spiderPosY1, *_spiderPosY2);
    jaffarCommon::logger::log("[J+]  + Remaining Distance:  %u\n", _remainingDistance);
    jaffarCommon::logger::log("[J+]  + Falling State:       %u\n", *_fallingState);
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

  uint8_t* _webPosY      ;
  uint8_t* _spiderPosY1  ;
  uint8_t* _spiderPosY2  ;
  uint8_t* _playerState  ;
  uint8_t* _gameState  ;
  uint8_t* _webLength  ;
  uint8_t* _fallingState  ;
  uint8_t* _frameType  ;

  uint16_t _remainingDistance;

  InputSet::inputIndex_t  _inputNull;
  InputSet::inputIndex_t  _inputU;
  InputSet::inputIndex_t  _inputB;
  InputSet::inputIndex_t  _inputUB;
  InputSet::inputIndex_t  _inputULB;
  InputSet::inputIndex_t  _inputURB;
};

} // namespace a2600

} // namespace games

} // namespace jaffarPlus