#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>

namespace jaffarPlus
{

namespace games
{

namespace snes
{

class BestOfTheBest final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "SNES / Best Of The Best"; }

  BestOfTheBest(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {

    _lowMem = _emulator->getProperty("RAM").pointer;

    registerGameProperty("Global Timer"               , &_lowMem[0x000072], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 HP"                , &_lowMem[0x000221], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 2 HP"                , &_lowMem[0x000222], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Input (Buttons)"     , &_lowMem[0x000064], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs (Pad)"        , &_lowMem[0x000065], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Type"                 , &_lowMem[0x000945], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Key Frame"                  , &_lowMem[0x000957], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action 1"          , &_lowMem[0x00022A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action 2"          , &_lowMem[0x00021D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Action Frame"      , &_lowMem[0x00021F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X"             , &_lowMem[0x00027B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Fight Active"               , &_lowMem[0x000240], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Training Hits"              , &_lowMem[0x001ACE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Training Hits"         , &_prevTrainingHits, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Reflexes"                   , &_lowMem[0x000459], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Reflex Hits"                , &_lowMem[0x001ABB], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Initial Reflexes"           , &_initialReflexes, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Reflexes Gained"            , &_reflexesGained, Property::datatype_t::dt_float32, Property::endianness_t::little);

    // Getting emulator's low memory pointer
    _globalTimer         = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer"           )]->getPointer();
    _player1HP           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 HP"            )]->getPointer();
    _player2HP           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 2 HP"            )]->getPointer();
    _playerInputButtons  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Input (Buttons)" )]->getPointer();
    _playerInputsPad     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs (Pad)"    )]->getPointer();
    _frameType           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Type"             )]->getPointer();
    _keyFrame            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Key Frame"              )]->getPointer();
    _player1Action1      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action 1"      )]->getPointer();
    _player1Action2      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action 2"      )]->getPointer();
    _player1ActionFrame  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Action Frame"  )]->getPointer();
    _player1PosX         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos X"         )]->getPointer();  
    _trainingHits        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Training Hits"         )]->getPointer();
    _reflexes            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Reflexes"              )]->getPointer();
    _reflexHits          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Reflex Hits"              )]->getPointer();

    _prevTrainingHits = *_trainingHits;
    _initialReflexes = *_reflexes;
    _reflexesGained = 0.0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _prevTrainingHits = *_trainingHits;

    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x200], 0x100);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _reflexesGained = (float)*_reflexes - (float)_initialReflexes + 0.25 * (float) *_reflexHits;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _player2HPMagnet = 0.0;
    _trainingHitsMagnet = 0.0;
    _reflexesMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.push(&_prevTrainingHits, sizeof(_prevTrainingHits));
    serializer.push(&_initialReflexes, sizeof(_initialReflexes));
    serializer.push(&_reflexesGained, sizeof(_reflexesGained));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializer.pop(&_prevTrainingHits, sizeof(_prevTrainingHits));
    deserializer.pop(&_initialReflexes, sizeof(_initialReflexes));
    deserializer.pop(&_reflexesGained, sizeof(_reflexesGained));
  }

  __INLINE__ float calculateGameSpecificReward() const override
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += 1.0 * _player2HPMagnet * (float)*_player2HP;
    reward += 1.0 * _trainingHitsMagnet * (float)*_trainingHits;
    reward += 1.0 * _reflexesMagnet * _reflexesGained;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_player2HPMagnet) > 0.0f)     jaffarCommon::logger::log("[J+]  + Player 2 HP Magnet                             Intensity: %.5f\n", _player2HPMagnet);
    if (std::abs(_trainingHitsMagnet) > 0.0f)  jaffarCommon::logger::log("[J+]  + Training Hits Magnet                           Intensity: %.5f\n", _trainingHitsMagnet);
    if (std::abs(_reflexesMagnet) > 0.0f)      jaffarCommon::logger::log("[J+]  + Reflexes Magnet                                Intensity: %.5f\n", _reflexesMagnet);
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

    if (actionType == "Set Training Hits Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_trainingHitsMagnet = intensity;  });
      recognizedActionType = true;
    }

    if (actionType == "Set Reflexes Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_reflexesMagnet = intensity;  });
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
  float _trainingHitsMagnet;
  float _reflexesMagnet;

  uint16_t* _globalTimer       ;
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
  uint8_t* _trainingHits        ;
  uint8_t* _reflexes        ;
  uint8_t* _reflexHits        ;
  uint8_t _initialReflexes;
  float _reflexesGained;
  uint8_t _prevTrainingHits        ;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
};

} // namespace snes

} // namespace games

} // namespace jaffarPlus