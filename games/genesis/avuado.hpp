#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace genesis
{

class Avuado final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "Genesis / Avuado"; }

  Avuado(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Score", &_workRAM[0x0725C], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Shot Pos Y", &_workRAM[0x02C46], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_workRAM[0x02B0C], Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _score      = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Score")]->getPointer();
    _playerPosX = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _shotPosY   = (uint16_t *)_propertyMap[jaffarCommon::hash::hashString("Shot Pos Y")]->getPointer();
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

  uint16_t *_score;
  uint16_t *_playerPosX;
  uint16_t *_shotPosY;

  // Pointer to emulator's low memory storage
  uint8_t *_workRAM;
};

} // namespace genesis

} // namespace games

} // namespace jaffarPlus