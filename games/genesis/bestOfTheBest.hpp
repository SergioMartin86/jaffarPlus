#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>

namespace jaffarPlus
{

namespace games
{

namespace genesis
{

class BestOfTheBest final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "Genesis / Best Of The Best"; }

  BestOfTheBest(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    _lowMem = _emulator->getProperty("RAM").pointer;

    registerGameProperty("Global Timer"               , &_lowMem[0x000002], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 HP"                , &_lowMem[0x000BB6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 2 HP"                , &_lowMem[0x000BB9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Input"               , &_lowMem[0x00000D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Type"                 , &_lowMem[0x000006], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action 1"          , &_lowMem[0x000BC1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action 2"          , &_lowMem[0x000BB2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action Frame"      , &_lowMem[0x000BB4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X"             , &_lowMem[0x000C31], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Fight Active"               , &_lowMem[0x000BBD], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting emulator's low memory pointer
    _globalTimer         = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer"          )]->getPointer();
    _player1HP           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 HP"            )]->getPointer();
    _player2HP           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 2 HP"            )]->getPointer();
    _playerInputs        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Input"           )]->getPointer();
    _frameType           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Type"             )]->getPointer();
    _player1Action1      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action 1"      )]->getPointer();
    _player1Action2      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action 2"      )]->getPointer();
    _player1ActionFrame  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action Frame"  )]->getPointer();
    _player1PosX         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos X"         )]->getPointer();  
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0xB90], 0xD3F - 0xB90);
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

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) override {}

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

    // for (size_t i = 0xBC0; i < 0xBD0; i++)  jaffarCommon::logger::log("[J+]  + Val 0x%X - %u\n", i, _lowMem[i]);
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

  uint16_t* _globalTimer       ;
  uint8_t* _player1HP          ;
  uint8_t* _player2HP          ;
  uint8_t* _playerInputs ;
  uint8_t* _frameType          ;
  uint8_t* _player1Action1     ;
  uint8_t* _player1Action2     ;
  uint8_t* _player1ActionFrame ;
  uint8_t* _player1PosX        ;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
};

} // namespace genesis

} // namespace games

} // namespace jaffarPlus