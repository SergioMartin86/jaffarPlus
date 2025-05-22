#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class BestOfTheBest final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Best Of The Best"; }

  BestOfTheBest(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {

    _lowMem = _emulator->getProperty("LRAM").pointer;

    registerGameProperty("Global Timer"               , &_lowMem[0x0026], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 HP"                , &_lowMem[0x045C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 2 HP"                , &_lowMem[0x045D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Input (Buttons)"     , &_lowMem[0x0300], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs (Pad)"        , &_lowMem[0x0302], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Key Frame"                  , &_lowMem[0x0304], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action 1"          , &_lowMem[0x0465], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action 2"          , &_lowMem[0x0458], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action Frame"      , &_lowMem[0x045A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X"             , &_lowMem[0x046F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Next Key Frame"             , &_nextKeyFrame, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting emulator's low memory pointer
    _globalTimer         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer"           )]->getPointer();
    _player1HP           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 HP"            )]->getPointer();
    _player2HP           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 2 HP"            )]->getPointer();
    _playerInputButtons  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Input (Buttons)" )]->getPointer();
    _playerInputsPad     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs (Pad)"    )]->getPointer();
    _keyFrame            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Key Frame"              )]->getPointer();
    _player1Action1      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action 1"      )]->getPointer();
    _player1Action2      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action 2"      )]->getPointer();
    _player1ActionFrame  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action Frame"  )]->getPointer();
    _player1PosX         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos X"         )]->getPointer();  
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Advancing to check next key frame
    uint8_t buffer[65535];
    jaffarCommon::serializer::Contiguous s(buffer);
    _emulator->serializeState(s);
    _emulator->advanceState(input);
    _nextKeyFrame = *_keyFrame;
    jaffarCommon::deserializer::Contiguous d(buffer);
    _emulator->deserializeState(d);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x400], 0x200);

    if (_nextKeyFrame != 1) hashEngine.Update(*_globalTimer);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _player2HPMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override 
  {
    serializer.push(&_nextKeyFrame, sizeof(_nextKeyFrame));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializer.pop(&_nextKeyFrame, sizeof(_nextKeyFrame));
  }

  __INLINE__ float calculateGameSpecificReward() const override
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += 1.0 * _player2HPMagnet * (float)*_player2HP;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_player2HPMagnet) > 0.0f)  jaffarCommon::logger::log("[J+]  + Player 2 HP Magnet                             Intensity: %.5f\n", _player2HPMagnet);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Player 2 HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_player2HPMagnet = intensity;  });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }


  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  float _player2HPMagnet;

  uint8_t* _globalTimer       ;
  uint8_t* _player1HP          ;
  uint8_t* _player2HP          ;
  uint8_t* _playerInputButtons ;
  uint8_t* _playerInputsPad    ;
  uint8_t* _frameType          ;
  uint8_t* _keyFrame           ;
  uint8_t* _player1Action1     ;
  uint8_t* _player1Action2     ;
  uint8_t* _player1ActionFrame ;
  uint8_t* _player1PosX        ;

  uint8_t _nextKeyFrame;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus