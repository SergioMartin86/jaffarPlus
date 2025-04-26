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

class LunarBall final : public jaffarPlus::Game
{
  public:

  enum ballState_t
  {
    none     = 0,
    inactive = 161,
    active   = 162,
    entering = 131,
    entered  = 3
  };

  static __INLINE__ std::string getName() { return "NES / Lunar Ball"; }

  LunarBall(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem  = _emulator->getProperty("LRAM").pointer;
    _palette = _emulator->getProperty("PALR").pointer;

    // Registering native game properties
    registerGameProperty("Prev Player Input", &_lowMem[0x0000], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Input", &_lowMem[0x0180], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Lag Frame", &_lowMem[0x0007], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Speed Increment", &_lowMem[0x03B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pending Balls", &_pendingBalls, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Moving Balls", &_movingBalls, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Current Stage", &_lowMem[0x0187], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Rate", &_lowMem[0x01C4], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ball 0 State", &_lowMem[0x0300], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 State", &_lowMem[0x0301], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 State", &_lowMem[0x0302], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 State", &_lowMem[0x0303], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 State", &_lowMem[0x0304], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 State", &_lowMem[0x0305], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 State", &_lowMem[0x0306], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 State", &_lowMem[0x0307], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ball 0 Angle", &_lowMem[0x03C0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Angle", &_lowMem[0x03C1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Angle", &_lowMem[0x03C2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Angle", &_lowMem[0x03C3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Angle", &_lowMem[0x03C4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Angle", &_lowMem[0x03C5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Angle", &_lowMem[0x03C6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Angle", &_lowMem[0x03C7], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ball 0 Speed", &_lowMem[0x03A0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Speed", &_lowMem[0x03A1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Speed", &_lowMem[0x03A2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Speed", &_lowMem[0x03A3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Speed", &_lowMem[0x03A4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Speed", &_lowMem[0x03A5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Speed", &_lowMem[0x03A6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Speed", &_lowMem[0x03A7], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ball 0 Pos X", &_lowMem[0x0370], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Pos X", &_lowMem[0x0371], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Pos X", &_lowMem[0x0372], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Pos X", &_lowMem[0x0373], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Pos X", &_lowMem[0x0374], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Pos X", &_lowMem[0x0375], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Pos X", &_lowMem[0x0376], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Pos X", &_lowMem[0x0377], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ball 0 Pos Y", &_lowMem[0x0330], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Pos Y", &_lowMem[0x0331], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Pos Y", &_lowMem[0x0332], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Pos Y", &_lowMem[0x0333], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Pos Y", &_lowMem[0x0334], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Pos Y", &_lowMem[0x0335], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Pos Y", &_lowMem[0x0336], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Pos Y", &_lowMem[0x0337], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later

    _speedIncrement = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Speed Increment")]->getPointer();
    _currentStage   = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Current Stage")]->getPointer();
    _rate           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Rate")]->getPointer();

    _ball0State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 0 State")]->getPointer();
    _ball1State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 State")]->getPointer();
    _ball2State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 State")]->getPointer();
    _ball3State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 State")]->getPointer();
    _ball4State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 State")]->getPointer();
    _ball5State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 State")]->getPointer();
    _ball6State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 State")]->getPointer();
    _ball7State = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 State")]->getPointer();

    _ball0Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 0 Angle")]->getPointer();
    _ball1Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Angle")]->getPointer();
    _ball2Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Angle")]->getPointer();
    _ball3Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Angle")]->getPointer();
    _ball4Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Angle")]->getPointer();
    _ball5Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Angle")]->getPointer();
    _ball6Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Angle")]->getPointer();
    _ball7Angle = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Angle")]->getPointer();

    _ball0Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 0 Speed")]->getPointer();
    _ball1Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Speed")]->getPointer();
    _ball2Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Speed")]->getPointer();
    _ball3Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Speed")]->getPointer();
    _ball4Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Speed")]->getPointer();
    _ball5Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Speed")]->getPointer();
    _ball6Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Speed")]->getPointer();
    _ball7Speed = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Speed")]->getPointer();

    _ball0PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 0 Pos X")]->getPointer();
    _ball1PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Pos X")]->getPointer();
    _ball2PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Pos X")]->getPointer();
    _ball3PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Pos X")]->getPointer();
    _ball4PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Pos X")]->getPointer();
    _ball5PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Pos X")]->getPointer();
    _ball6PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Pos X")]->getPointer();
    _ball7PosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Pos X")]->getPointer();

    _ball0PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 0 Pos Y")]->getPointer();
    _ball1PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Pos Y")]->getPointer();
    _ball2PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Pos Y")]->getPointer();
    _ball3PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Pos Y")]->getPointer();
    _ball4PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Pos Y")]->getPointer();
    _ball5PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Pos Y")]->getPointer();
    _ball6PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Pos Y")]->getPointer();
    _ball7PosY = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Pos Y")]->getPointer();

    _firstPaletteByte = (uint8_t *)&_palette[0];

    _nullInputIdx   = _emulator->registerInput("|..|........|");
    _leftInputIdx   = _emulator->registerInput("|..|..L.....|");
    _rightInputIdx  = _emulator->registerInput("|..|...R....|");
    _buttonInputIdx = _emulator->registerInput("|..|......B.|");

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
    hashEngine.Update(&_lowMem[0x0300], 0x0100);
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t> &allowedInputSet) override
  {
    if (*_ball0State == ballState_t::inactive)
      {
        allowedInputSet.push_back(_buttonInputIdx);

        if (*_ball0Angle == 0)
          {
            allowedInputSet.push_back(_leftInputIdx);
            allowedInputSet.push_back(_rightInputIdx);
        }

        if (*_ball0Angle > 0 && *_ball0Angle < 128) allowedInputSet.push_back(_rightInputIdx);
        if (*_ball0Angle > 128 && *_ball0Angle < 254) allowedInputSet.push_back(_leftInputIdx);
    }
  };

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _movingBalls = 0;
    if (*_ball0Speed > 0 && *_ball0State == ballState_t::active) _movingBalls++;
    if (*_ball1Speed > 0 && *_ball1State == ballState_t::active) _movingBalls++;
    if (*_ball2Speed > 0 && *_ball2State == ballState_t::active) _movingBalls++;
    if (*_ball3Speed > 0 && *_ball3State == ballState_t::active) _movingBalls++;
    if (*_ball4Speed > 0 && *_ball4State == ballState_t::active) _movingBalls++;
    if (*_ball5Speed > 0 && *_ball5State == ballState_t::active) _movingBalls++;
    if (*_ball6Speed > 0 && *_ball6State == ballState_t::active) _movingBalls++;
    if (*_ball7Speed > 0 && *_ball7State == ballState_t::active) _movingBalls++;

    _pendingBalls = 7;
    if (*_ball1State == ballState_t::entering || *_ball1State == ballState_t::entered || *_ball1State == ballState_t::none) _pendingBalls--;
    if (*_ball2State == ballState_t::entering || *_ball2State == ballState_t::entered || *_ball2State == ballState_t::none) _pendingBalls--;
    if (*_ball3State == ballState_t::entering || *_ball3State == ballState_t::entered || *_ball3State == ballState_t::none) _pendingBalls--;
    if (*_ball4State == ballState_t::entering || *_ball4State == ballState_t::entered || *_ball4State == ballState_t::none) _pendingBalls--;
    if (*_ball5State == ballState_t::entering || *_ball5State == ballState_t::entered || *_ball5State == ballState_t::none) _pendingBalls--;
    if (*_ball6State == ballState_t::entering || *_ball6State == ballState_t::entered || *_ball6State == ballState_t::none) _pendingBalls--;
    if (*_ball7State == ballState_t::entering || *_ball7State == ballState_t::entered || *_ball7State == ballState_t::none) _pendingBalls--;
  }

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

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

    // Reward starting early
    if (*_ball0State != ballState_t::inactive) reward += 1.0;

    // Reward scoring balls
    if (*_ball1State == ballState_t::entering || *_ball1State == ballState_t::entered || *_ball1State == ballState_t::none) reward += 10.0;
    if (*_ball2State == ballState_t::entering || *_ball2State == ballState_t::entered || *_ball2State == ballState_t::none) reward += 10.0;
    if (*_ball3State == ballState_t::entering || *_ball3State == ballState_t::entered || *_ball3State == ballState_t::none) reward += 10.0;
    if (*_ball4State == ballState_t::entering || *_ball4State == ballState_t::entered || *_ball4State == ballState_t::none) reward += 10.0;
    if (*_ball5State == ballState_t::entering || *_ball5State == ballState_t::entered || *_ball5State == ballState_t::none) reward += 10.0;
    if (*_ball6State == ballState_t::entering || *_ball6State == ballState_t::entered || *_ball6State == ballState_t::none) reward += 10.0;
    if (*_ball7State == ballState_t::entering || *_ball7State == ballState_t::entered || *_ball7State == ballState_t::none) reward += 10.0;

    // Reward stationary cue ball
    if (*_ball0State != ballState_t::inactive && *_ball0Speed == 0) reward += 1000.0;

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

    jaffarCommon::logger::log("[J+]  + Ball 1 Angle: %u\n", *_ball1Angle);
    jaffarCommon::logger::log("[J+]  + Ball 2 Angle: %u\n", *_ball2Angle);
    jaffarCommon::logger::log("[J+]  + Ball 3 Angle: %u\n", *_ball3Angle);
    jaffarCommon::logger::log("[J+]  + Ball 4 Angle: %u\n", *_ball4Angle);
    jaffarCommon::logger::log("[J+]  + Ball 5 Angle: %u\n", *_ball5Angle);
    jaffarCommon::logger::log("[J+]  + Ball 6 Angle: %u\n", *_ball6Angle);
    jaffarCommon::logger::log("[J+]  + Ball 7 Angle: %u\n", *_ball7Angle);

    jaffarCommon::logger::log("[J+]  + Ball 1 Speed: %u\n", *_ball1Speed);
    jaffarCommon::logger::log("[J+]  + Ball 2 Speed: %u\n", *_ball2Speed);
    jaffarCommon::logger::log("[J+]  + Ball 3 Speed: %u\n", *_ball3Speed);
    jaffarCommon::logger::log("[J+]  + Ball 4 Speed: %u\n", *_ball4Speed);
    jaffarCommon::logger::log("[J+]  + Ball 5 Speed: %u\n", *_ball5Speed);
    jaffarCommon::logger::log("[J+]  + Ball 6 Speed: %u\n", *_ball6Speed);
    jaffarCommon::logger::log("[J+]  + Ball 7 Speed: %u\n", *_ball7Speed);

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

  uint8_t *_lowMem;
  uint8_t  _pendingBalls;
  uint8_t *_currentStage;
  uint8_t *_rate;

  uint8_t *_ball0State;
  uint8_t *_ball1State;
  uint8_t *_ball2State;
  uint8_t *_ball3State;
  uint8_t *_ball4State;
  uint8_t *_ball5State;
  uint8_t *_ball6State;
  uint8_t *_ball7State;

  uint8_t *_ball0Angle;
  uint8_t *_ball1Angle;
  uint8_t *_ball2Angle;
  uint8_t *_ball3Angle;
  uint8_t *_ball4Angle;
  uint8_t *_ball5Angle;
  uint8_t *_ball6Angle;
  uint8_t *_ball7Angle;

  uint8_t *_ball0Speed;
  uint8_t *_ball1Speed;
  uint8_t *_ball2Speed;
  uint8_t *_ball3Speed;
  uint8_t *_ball4Speed;
  uint8_t *_ball5Speed;
  uint8_t *_ball6Speed;
  uint8_t *_ball7Speed;

  uint8_t *_ball0PosX;
  uint8_t *_ball1PosX;
  uint8_t *_ball2PosX;
  uint8_t *_ball3PosX;
  uint8_t *_ball4PosX;
  uint8_t *_ball5PosX;
  uint8_t *_ball6PosX;
  uint8_t *_ball7PosX;

  uint8_t *_ball0PosY;
  uint8_t *_ball1PosY;
  uint8_t *_ball2PosY;
  uint8_t *_ball3PosY;
  uint8_t *_ball4PosY;
  uint8_t *_ball5PosY;
  uint8_t *_ball6PosY;
  uint8_t *_ball7PosY;

  uint8_t *_speedIncrement;

  uint8_t *_palette;
  uint8_t *_firstPaletteByte;

  uint8_t _movingBalls;

  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _leftInputIdx;
  InputSet::inputIndex_t _rightInputIdx;
  InputSet::inputIndex_t _buttonInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus