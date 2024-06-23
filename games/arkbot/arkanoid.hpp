#pragma once

#include <memory>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulators/quickerArkBot/quickerArkBot.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace arkbot
{

class Arkanoid final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "ArkBot / Arkanoid"; }

  Arkanoid(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
    _arkState = dynamic_cast<jaffarPlus::emulator::QuickerArkBot *>(_emulator.get())->getGameState();
  }

  private:

  __INLINE__ void registerGameProperties() override {}

  __INLINE__ void advanceStateImpl(const std::string &input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override { hashEngine.Update(*_arkState); }

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

  GameState *_arkState;
};

} // namespace arkbot

} // namespace games

} // namespace jaffarPlus