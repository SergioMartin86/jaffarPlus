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

class SidePocket final : public jaffarPlus::Game
{

  public:

  static __INLINE__ std::string getName() { return "NES / Side Pocket"; }

  SidePocket(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Cue Angle",             &_lowMem[0x026E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Cue Effect Vertical",   &_lowMem[0x03F8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Cue Effect Horizontal", &_lowMem[0x03F9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Cue Power",             &_lowMem[0x0628], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game Status 1",         &_lowMem[0x0006], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game Status 2",         &_lowMem[0x0007], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ball 1 State",          &_lowMem[0x0200], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 State",          &_lowMem[0x0201], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 State",          &_lowMem[0x0202], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 State",          &_lowMem[0x0203], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 State",          &_lowMem[0x0204], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 State",          &_lowMem[0x0205], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 State",          &_lowMem[0x0206], Property::datatype_t::dt_uint8, Property::endianness_t::little);
        
    registerGameProperty("Ball 1 Speed1",         &_lowMem[0x24D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Speed1",         &_lowMem[0x24E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Speed1",         &_lowMem[0x24F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Speed1",         &_lowMem[0x250], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Speed1",         &_lowMem[0x251], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Speed1",         &_lowMem[0x252], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Speed1",         &_lowMem[0x253], Property::datatype_t::dt_uint8, Property::endianness_t::little);
        
    registerGameProperty("Ball 1 Speed2",         &_lowMem[0x0258], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Speed2",         &_lowMem[0x0259], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Speed2",         &_lowMem[0x025A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Speed2",         &_lowMem[0x025B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Speed2",         &_lowMem[0x025C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Speed2",         &_lowMem[0x025D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Speed2",         &_lowMem[0x025E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
        
    registerGameProperty("Ball 1 Pos X",          &_lowMem[0x022C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Pos X",          &_lowMem[0x022D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Pos X",          &_lowMem[0x022E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Pos X",          &_lowMem[0x022F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Pos X",          &_lowMem[0x0230], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Pos X",          &_lowMem[0x0231], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Pos X",          &_lowMem[0x0232], Property::datatype_t::dt_uint8, Property::endianness_t::little);
        
    registerGameProperty("Ball 1 Pos Y",          &_lowMem[0x0216], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Pos Y",          &_lowMem[0x0217], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Pos Y",          &_lowMem[0x0218], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Pos Y",          &_lowMem[0x0219], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Pos Y",          &_lowMem[0x021A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Pos Y",          &_lowMem[0x021B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Pos Y",          &_lowMem[0x021C], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later

    _cueAngle             = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Cue Angle"  )]->getPointer();
    _cueEffectVertical    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Cue Effect Vertical"  )]->getPointer();
    _cueEffectHorizontal  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Cue Effect Horizontal"  )]->getPointer();
    _cuePower             = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Cue Power"  )]->getPointer();
    _gameStatus1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game Status 1"  )]->getPointer();
    _gameStatus2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game Status 2"  )]->getPointer();

    _ball1State           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 State"  )]->getPointer();
    _ball2State           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 State"  )]->getPointer();
    _ball3State           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 State"  )]->getPointer();
    _ball4State           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 State"  )]->getPointer();
    _ball5State           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 State"  )]->getPointer();
    _ball6State           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 State"  )]->getPointer();
    _ball7State           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 State"  )]->getPointer();
         
    _ball1Speed1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Speed1"  )]->getPointer();
    _ball2Speed1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Speed1"  )]->getPointer();
    _ball3Speed1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Speed1"  )]->getPointer();
    _ball4Speed1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Speed1"  )]->getPointer();
    _ball5Speed1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Speed1"  )]->getPointer();
    _ball6Speed1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Speed1"  )]->getPointer();
    _ball7Speed1          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Speed1"  )]->getPointer();
         
    _ball1Speed2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Speed2"  )]->getPointer();
    _ball2Speed2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Speed2"  )]->getPointer();
    _ball3Speed2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Speed2"  )]->getPointer();
    _ball4Speed2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Speed2"  )]->getPointer();
    _ball5Speed2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Speed2"  )]->getPointer();
    _ball6Speed2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Speed2"  )]->getPointer();
    _ball7Speed2          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Speed2"  )]->getPointer();
         
    _ball1PosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Pos X"  )]->getPointer();
    _ball2PosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Pos X"  )]->getPointer();
    _ball3PosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Pos X"  )]->getPointer();
    _ball4PosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Pos X"  )]->getPointer();
    _ball5PosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Pos X"  )]->getPointer();
    _ball6PosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Pos X"  )]->getPointer();
    _ball7PosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Pos X"  )]->getPointer();
          
    _ball1PosY            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Pos Y"  )]->getPointer();
    _ball2PosY            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Pos Y"  )]->getPointer();
    _ball3PosY            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Pos Y"  )]->getPointer();
    _ball4PosY            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Pos Y"  )]->getPointer();
    _ball5PosY            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Pos Y"  )]->getPointer();
    _ball6PosY            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Pos Y"  )]->getPointer();
    _ball7PosY            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Pos Y"  )]->getPointer();

    _nullInputIdx   = _emulator->registerInput("|..|........|");
    _leftInputIdx   = _emulator->registerInput("|..|..L.....|");
    _rightInputIdx  = _emulator->registerInput("|..|...R....|");
    _upInputIdx     = _emulator->registerInput("|..|U.......|");
    _downInputIdx   = _emulator->registerInput("|..|.D......|");
    _aInputIdx      = _emulator->registerInput("|..|.......A|");
    _bInputIdx      = _emulator->registerInput("|..|......B.|");

    _movingBalls = 0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    // hashEngine.Update(_lowMem[0x0000]);
    // hashEngine.Update(_lowMem[0x0007]);
    // hashEngine.Update(_lowMem[0x0180]);
    hashEngine.Update(*_gameStatus1);
    hashEngine.Update(*_gameStatus2);
    hashEngine.Update(&_lowMem[0x0200], 0x0280);
  }

 __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t> &allowedInputSet) override
  {
  };

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _movingBalls = 0;
    if (*_ball1Speed1 > 0 && *_ball1Speed2 > 0 && *_ball1State == 0x01) _movingBalls++;
    if (*_ball2Speed1 > 0 && *_ball2Speed2 > 0 && *_ball2State == 0x02) _movingBalls++;
    if (*_ball3Speed1 > 0 && *_ball3Speed2 > 0 && *_ball3State == 0x03) _movingBalls++;
    if (*_ball4Speed1 > 0 && *_ball4Speed2 > 0 && *_ball4State == 0x04) _movingBalls++;
    if (*_ball5Speed1 > 0 && *_ball5Speed2 > 0 && *_ball5State == 0x05) _movingBalls++;
    if (*_ball6Speed1 > 0 && *_ball6Speed2 > 0 && *_ball6State == 0x06) _movingBalls++;
    if (*_ball7Speed1 > 0 && *_ball7Speed2 > 0 && *_ball7State == 0x07) _movingBalls++;

    _pendingBalls = 6;
    if (*_ball2State == 0x02 ) _pendingBalls--;
    if (*_ball3State == 0x03 ) _pendingBalls--;
    if (*_ball4State == 0x04 ) _pendingBalls--;
    if (*_ball5State == 0x05 ) _pendingBalls--;
    if (*_ball6State == 0x06 ) _pendingBalls--;
    if (*_ball7State == 0x07 ) _pendingBalls--;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.pushContiguous(&_movingBalls, sizeof(_movingBalls));
    serializer.pushContiguous(&_pendingBalls, sizeof(_pendingBalls));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.popContiguous(&_movingBalls, sizeof(_movingBalls));
    deserializer.popContiguous(&_pendingBalls, sizeof(_pendingBalls));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
     jaffarCommon::logger::log("[J+]  + Ball 1 State: %u\n", *_ball1State);
     jaffarCommon::logger::log("[J+]  + Ball 2 State: %u\n", *_ball2State);
     jaffarCommon::logger::log("[J+]  + Ball 3 State: %u\n", *_ball3State);
     jaffarCommon::logger::log("[J+]  + Ball 4 State: %u\n", *_ball4State);
     jaffarCommon::logger::log("[J+]  + Ball 5 State: %u\n", *_ball5State);
     jaffarCommon::logger::log("[J+]  + Ball 6 State: %u\n", *_ball6State);
     jaffarCommon::logger::log("[J+]  + Ball 7 State: %u\n", *_ball7State);
     
     jaffarCommon::logger::log("[J+]  + Ball 1 Speed: %u ( + %u / 256)\n", *_ball1Speed1, *_ball1Speed2);
     jaffarCommon::logger::log("[J+]  + Ball 2 Speed: %u ( + %u / 256)\n", *_ball2Speed1, *_ball2Speed2);
     jaffarCommon::logger::log("[J+]  + Ball 3 Speed: %u ( + %u / 256)\n", *_ball3Speed1, *_ball3Speed2);
     jaffarCommon::logger::log("[J+]  + Ball 4 Speed: %u ( + %u / 256)\n", *_ball4Speed1, *_ball4Speed2);
     jaffarCommon::logger::log("[J+]  + Ball 5 Speed: %u ( + %u / 256)\n", *_ball5Speed1, *_ball5Speed2);
     jaffarCommon::logger::log("[J+]  + Ball 6 Speed: %u ( + %u / 256)\n", *_ball6Speed1, *_ball6Speed2);
     jaffarCommon::logger::log("[J+]  + Ball 7 Speed: %u ( + %u / 256)\n", *_ball7Speed1, *_ball7Speed2);

     jaffarCommon::logger::log("[J+]  + Ball 1 Pos:   (%u, %u)\n", *_ball1PosX, *_ball1PosY);
     jaffarCommon::logger::log("[J+]  + Ball 2 Pos:   (%u, %u)\n", *_ball2PosX, *_ball2PosY);
     jaffarCommon::logger::log("[J+]  + Ball 3 Pos:   (%u, %u)\n", *_ball3PosX, *_ball3PosY);
     jaffarCommon::logger::log("[J+]  + Ball 4 Pos:   (%u, %u)\n", *_ball4PosX, *_ball4PosY);
     jaffarCommon::logger::log("[J+]  + Ball 5 Pos:   (%u, %u)\n", *_ball5PosX, *_ball5PosY);
     jaffarCommon::logger::log("[J+]  + Ball 6 Pos:   (%u, %u)\n", *_ball6PosX, *_ball6PosY);
     jaffarCommon::logger::log("[J+]  + Ball 7 Pos:   (%u, %u)\n", *_ball7PosX, *_ball7PosY);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  public:

  uint8_t* _lowMem;

  uint8_t _pendingBalls;

  uint8_t* _cueAngle             ;
  uint8_t* _cueEffectVertical    ;
  uint8_t* _cueEffectHorizontal  ;
  uint8_t* _cuePower             ;
  uint8_t* _gameStatus1          ;
  uint8_t* _gameStatus2          ;

  uint8_t *_ball1State;
  uint8_t *_ball2State;
  uint8_t *_ball3State;
  uint8_t *_ball4State;
  uint8_t *_ball5State;
  uint8_t *_ball6State;
  uint8_t *_ball7State;

  uint8_t *_ball1Speed1;
  uint8_t *_ball2Speed1;
  uint8_t *_ball3Speed1;
  uint8_t *_ball4Speed1;
  uint8_t *_ball5Speed1;
  uint8_t *_ball6Speed1;
  uint8_t *_ball7Speed1;

  uint8_t *_ball1Speed2;
  uint8_t *_ball2Speed2;
  uint8_t *_ball3Speed2;
  uint8_t *_ball4Speed2;
  uint8_t *_ball5Speed2;
  uint8_t *_ball6Speed2;
  uint8_t *_ball7Speed2;

  uint8_t *_ball1PosX;
  uint8_t *_ball2PosX;
  uint8_t *_ball3PosX;
  uint8_t *_ball4PosX;
  uint8_t *_ball5PosX;
  uint8_t *_ball6PosX;
  uint8_t *_ball7PosX;

  uint8_t *_ball1PosY;
  uint8_t *_ball2PosY;
  uint8_t *_ball3PosY;
  uint8_t *_ball4PosY;
  uint8_t *_ball5PosY;
  uint8_t *_ball6PosY;
  uint8_t *_ball7PosY;

  uint8_t _movingBalls;

  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _leftInputIdx;
  InputSet::inputIndex_t _rightInputIdx;
  InputSet::inputIndex_t _upInputIdx;
  InputSet::inputIndex_t _downInputIdx;
  InputSet::inputIndex_t _aInputIdx;
  InputSet::inputIndex_t _bInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus