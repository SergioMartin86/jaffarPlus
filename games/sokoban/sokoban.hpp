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
    registerGameProperty("Is Deadlock", &_isDeadlock, Property::datatype_t::dt_bool, Property::endianness_t::little);

    _pusherPosY = &_quickerBan->getState()[0];
    _pusherPosX = &_quickerBan->getState()[1];

    registerGameProperty("Pusher Pos X", _pusherPosX, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pusher Pos Y", _pusherPosY, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Pusher Prev Pos X", &_pusherPrevPosX, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pusher Prev Pos Y", &_pusherPrevPosY, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    stateUpdatePostHook();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Storing previous pusher position
    _pusherPrevPosY = *_pusherPosY;
    _pusherPrevPosX = *_pusherPosX;

    // Running emulator
    _emulator->advanceState(input);

    // Checking if a box has moved
    _hasMovedBox = _quickerBan->getMovedBox();

    // Check if deadlock
    _isDeadlock = _quickerBan->getIsDeadlock();
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(_quickerBan->getState(), _quickerBan->getStateSize());
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _remainingBoxes = _quickerBan->getGoalCount() - _quickerBan->getBoxesOnGoal();
    
    // Getting pusher 

    // Checking if we can move up
    _canMoveUp = _quickerBan->canMoveUp();
    if (_hasMovedBox == false)
      if (*_pusherPosX == _pusherPrevPosX)
        if ((*_pusherPosY-1) == _pusherPrevPosY) _canMoveUp = false;

    _canMoveDown = _quickerBan->canMoveDown();
    if (_hasMovedBox == false)
      if (*_pusherPosX == _pusherPrevPosX)
        if ((*_pusherPosY+1) == _pusherPrevPosY) _canMoveDown = false;

    _canMoveLeft = _quickerBan->canMoveLeft();
    if (_hasMovedBox == false)
      if ((*_pusherPosX-1) == _pusherPrevPosX)
        if (*_pusherPosY == _pusherPrevPosY) _canMoveLeft = false;

    _canMoveRight = _quickerBan->canMoveRight();
    if (_hasMovedBox == false)
      if ((*_pusherPosX+1) == _pusherPrevPosX)
        if (*_pusherPosY == _pusherPrevPosY) _canMoveRight = false;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.push(&_hasMovedBox, sizeof(_hasMovedBox));
    serializer.push(&_pusherPrevPosX, sizeof(_pusherPrevPosX));
    serializer.push(&_pusherPrevPosY, sizeof(_pusherPrevPosY));
    serializer.push(&_isDeadlock, sizeof(_isDeadlock));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.pop(&_hasMovedBox, sizeof(_hasMovedBox));
    deserializer.pop(&_pusherPrevPosX, sizeof(_pusherPrevPosX));
    deserializer.pop(&_pusherPrevPosY, sizeof(_pusherPrevPosY));
    deserializer.pop(&_isDeadlock, sizeof(_isDeadlock));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    // reward -= _remainingBoxes;

    // Punishing collective distances to goal
    reward -= _quickerBan->getTotalDistance();

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+] Boxes on Goal: %lu / %lu\n", _quickerBan->getBoxesOnGoal(), _quickerBan->getGoalCount());
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

  uint8_t* _pusherPosX;
  uint8_t* _pusherPosY;
  
  bool _isDeadlock;
  uint8_t _pusherPrevPosX;
  uint8_t _pusherPrevPosY;
  bool _hasMovedBox;
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