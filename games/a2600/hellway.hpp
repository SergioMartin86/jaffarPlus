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

class Hellway final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "A2600 / Hellway"; }

  Hellway(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Score", &_lowMem[0x34], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("SubDistance", &_lowMem[0x11], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Car State", &_lowMem[0x4B], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _score       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score")]->getPointer();
    _subDistance = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("SubDistance")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const std::string& input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override { hashEngine.Update(_lowMem, 0x80); }

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

    // Distance to point magnet
    reward += 256.0 * *_score + *_subDistance;

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

  uint8_t* _score;
  uint8_t* _subDistance;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
};

} // namespace a2600

} // namespace games

} // namespace jaffarPlus