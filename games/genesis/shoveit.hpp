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

class ShoveIt final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "Genesis / Shove It!"; }

  ShoveIt(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

  uint8_t getMoveTimer() const { return *_moveTimer; }

  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _leftInputIdx;
  InputSet::inputIndex_t _rightInputIdx;
  InputSet::inputIndex_t _upInputIdx;
  InputSet::inputIndex_t _downInputIdx;
  InputSet::inputIndex_t _buttonInputIdx;

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Move Timer", &_workRAM[0xF71B], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _moveTimer = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Move Timer")]->getPointer();

    _nullInputIdx  = _emulator->registerInput("|..|........|");
    _leftInputIdx  = _emulator->registerInput("|..|..L.....|");
    _rightInputIdx = _emulator->registerInput("|..|...R....|");
    _upInputIdx    = _emulator->registerInput("|..|U.......|");
    _downInputIdx  = _emulator->registerInput("|..|.D......|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override { hashEngine.Update(_workRAM, 0x2000); }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) {}

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override {}

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

  uint8_t* _moveTimer;

  // Pointer to emulator's low memory storage
  uint8_t* _workRAM;
};

} // namespace genesis

} // namespace games

} // namespace jaffarPlus