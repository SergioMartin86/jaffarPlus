#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace gbc
{

class ASlimeTravel final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "GBC / A Slime Travel"; }

  ASlimeTravel(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("WRAM").pointer;

    // Registering native game properties
    registerGameProperty("Game State",  &_workRAM[0x00C7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Slime Pos X", &_workRAM[0x0001], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _gameState = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game State")]->getPointer();
    _slimePosX = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Slime Pos X")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override { hashEngine.Update(_workRAM, 0x0500); }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) {}

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Reward advancing always
    reward += 1.0 * (double)*_slimePosX;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override {}

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

  uint8_t *_gameState;
  uint8_t *_slimePosX;

  // Pointer to emulator's low memory storage
  uint8_t *_workRAM;
};

} // namespace gbc

} // namespace games

} // namespace jaffarPlus