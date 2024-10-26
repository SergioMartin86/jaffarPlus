#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace gba
{

class TollRunner final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "GBA / Toll Runner"; }

  TollRunner(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("WRAM").pointer;

    // Registering native game properties
    registerGameProperty("Score", &_workRAM[0x08AD0], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Mouse Pos X", &_workRAM[0x001E8], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Mouse Pos Y", &_workRAM[0x001EC], Property::datatype_t::dt_int16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _score     = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Score")]->getPointer();
    _mousePosX = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Mouse Pos X")]->getPointer();
    _mousePosY = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Mouse Pos Y")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override { hashEngine.Update(_workRAM, 0x8000); }

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

    // Distance to point magnet
    reward += 100.0 * (double)*_score;

    // Reward advancing always
    reward -= 1.0 * (double)*_mousePosY;

    // Reward going sideways
    // reward += 0.1 * std::abs((double)*_mousePosX);

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

  int16_t *_score;
  int16_t *_mousePosX;
  int16_t *_mousePosY;

  // Pointer to emulator's low memory storage
  uint8_t *_workRAM;
};

} // namespace gba

} // namespace games

} // namespace jaffarPlus