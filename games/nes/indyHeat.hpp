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

class IndyHeat final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "NES / Indy Heat"; }

  IndyHeat(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
    // Parsing configuration
    _lastInputStepReward = jaffarCommon::json::getNumber<float>(config, "Last Input Step Reward");
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Pre-Race Timer",       &_lowMem[0x04D4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 Angle",        &_lowMem[0x0430], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 PosX",         &_lowMem[0x0450], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 PosY",         &_lowMem[0x0470], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 Speed" ,       &_lowMem[0x00E3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 Pit Stop" ,    &_lowMem[0x0569], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 Current Lap",  &_lowMem[0x055A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 Checkpoint",   &_lowMem[0x055F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 Fuel",         &_lowMem[0x0529], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player1 Previous Lap", &_player1PreviousLap, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Last Input Step", &_lastInputStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Current Step", &_currentStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _preRaceTimer         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Pre-Race Timer")]->getPointer();
    _player1Angle         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player1 Angle")]->getPointer();
    _player1PosX          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player1 PosX")]->getPointer();
    _player1PosY          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player1 PosY")]->getPointer();
    _player1Speed         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player1 Speed")]->getPointer();
    _player1CurrentLap    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player1 Current Lap")]->getPointer();
    _player1Checkpoint   = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player1 Checkpoint")]->getPointer();

    // Derivative Values
    _player1PreviousLap = *_player1CurrentLap;

    // Getting index for a non input 
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    _player1PreviousLap = *_player1CurrentLap;

    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    // { size_t start = 0x0400; size_t end = 0x0440; hashEngine.Update(&_lowMem[start], end - start); } 
    // { size_t start = 0x0450; size_t end = 0x0460; hashEngine.Update(&_lowMem[start], end - start); } 
    // { size_t start = 0x0470; size_t end = 0x0480; hashEngine.Update(&_lowMem[start], end - start); } 
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override
  {
    _stopProcessingReward  = false;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_player1PreviousLap, sizeof(_player1PreviousLap));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_player1PreviousLap, sizeof(_player1PreviousLap));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputStepReward * _lastInputStep;

    // If this is a win state, then evaluate only w.r.t. how long since the last input
    if (_stopProcessingReward) return reward;

    // Distance to point magnet
    reward += 1000.0f * (float) *_player1CurrentLap;

    // Reward Lap Progress
    reward += *_player1Checkpoint;

    // Reward Speed
    reward += 0.001 * (float)*_player1Speed;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
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

  uint8_t *_player1CurrentLap;
  uint8_t *_player1Angle;
  uint8_t *_preRaceTimer;
  uint8_t *_player1PosX ;
  uint8_t *_player1PosY ;
  uint8_t *_player1Speed;
  uint8_t *_player1Checkpoint;

  uint8_t _player1PreviousLap;

  // Additions to make the last input as soon as possible
  uint16_t _lastInputStep = 0;
  uint16_t _currentStep   = 0;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;

  // Pointer to emulator's low memory storage
  uint8_t *_lowMem;

  // Reward for the last time an input was made (for early termination)
  float _lastInputStepReward;

  // Specifies whether the reward should continue to be processed (for early termination)
  bool _stopProcessingReward;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus