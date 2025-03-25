#pragma once

#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace sokoban
{

class Sokoban final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "Sokoban"; }

  Sokoban(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    _quickerBan = ((jaffarPlus::emulator::QuickerBan*)_emulator.get())->getEmulator();

    registerGameProperty("Can Move Up", &_canMoveUp, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Can Move Down", &_canMoveDown, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Can Move Left", &_canMoveLeft, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Can Move Right", &_canMoveRight, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Remaining Boxes", &_remainingBoxes, Property::datatype_t::dt_uint16, Property::endianness_t::little);
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(_quickerBan->getState(), _quickerBan->getStateSize());
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _remainingBoxes = _quickerBan->getGoalCount() - _quickerBan->getBoxesOnGoal();
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    _canMoveUp = _quickerBan->canMoveUp();
    _canMoveDown = _quickerBan->canMoveDown();
    _canMoveLeft = _quickerBan->canMoveLeft();
    _canMoveRight = _quickerBan->canMoveRight();
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward -= _remainingBoxes;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    _quickerBan->printInfo();
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


  bool _canMoveUp;
  bool _canMoveDown;
  bool _canMoveLeft;
  bool _canMoveRight;
  uint16_t _remainingBoxes;
  jaffar::EmuInstance* _quickerBan;
};

} // namespace sokoban

} // namespace games

} // namespace jaffarPlus