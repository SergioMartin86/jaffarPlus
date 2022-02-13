#pragma once

#include "emuInstance.hpp"
#include "nlohmann/json.hpp"
#include "frame.hpp"
#include "rule.hpp"
#include "metrohash64.h"
#include <cstddef>
#include <string>
#include <vector>
#include <set>


class State
{
  public:

  State(const std::string romFile, const std::string stateFile);
  State(EmuInstance* emu);

  // Rule parser
  void parseRules(const nlohmann::json rulesConfig);

  // Print Rule information
  void printRuleStatus(const bool* rulesStatus);

  std::vector<Rule *> _rules;
  size_t _ruleCount;
  EmuInstance *_emu;

  // Game specific values
  uint16_t* _screenScroll; inline uint16_t getScreenScroll() { return swap_endian<uint16_t>(*_screenScroll); };
  uint8_t* _marioAnimation;
  uint8_t* _marioState;

  uint8_t* _marioBasePosX;
  uint8_t* _marioRelPosX;
  uint8_t* _marioSubpixelPosX;

  uint8_t* _marioPosY;
  uint8_t* _marioSubpixelPosY;

  uint8_t* _marioMovingDirection;
  uint8_t* _marioFacingDirection;
  uint8_t* _marioFloatingMode;
  uint8_t* _marioWalkingMode;
  uint8_t* _marioWalkingDelay;
  uint8_t* _marioWalkingFrame;
  int8_t* _marioMaxVelLeft;
  int8_t* _marioMaxVelRight;
  int8_t* _marioVelX;
  int8_t* _marioXMoveForce;
  int8_t* _marioVelY;
  int8_t* _marioFracVelY;
  uint8_t* _marioGravity;
  uint8_t* _marioFriction;
  uint8_t* _timeLeft100;
  uint8_t* _timeLeft10;
  uint8_t* _timeLeft1;

  uint8_t* _screenBasePosX;
  uint8_t* _screenRelPosX;

  uint8_t* _currentWorldRaw;
  uint8_t* _currentStageRaw;
  uint8_t* _levelEntryFlag;
  uint8_t* _gameMode;

  uint8_t* _enemy1Active;
  uint8_t* _enemy2Active;
  uint8_t* _enemy3Active;
  uint8_t* _enemy4Active;
  uint8_t* _enemy5Active;

  uint8_t* _enemy1State;
  uint8_t* _enemy2State;
  uint8_t* _enemy3State;
  uint8_t* _enemy4State;
  uint8_t* _enemy5State;

  uint8_t* _enemy1Type;
  uint8_t* _enemy2Type;
  uint8_t* _enemy3Type;
  uint8_t* _enemy4Type;
  uint8_t* _enemy5Type;

  uint8_t* _marioCollision;
  uint8_t* _enemyCollision;
  uint8_t* _hitDetectionFlag;

  uint8_t* _powerUpActive;

  uint8_t* _animationTimer;
  uint8_t* _jumpSwimTimer;
  uint8_t* _runningTimer;
  uint8_t* _blockBounceTimer;
  uint8_t* _sideCollisionTimer;
  uint8_t* _jumpspringTimer;
  uint8_t* _climbSideTimer;
  uint8_t* _gameControlTimer;
  uint8_t* _enemyFrameTimer;
  uint8_t* _frenzyEnemyTimer;
  uint8_t* _bowserFireTimer;
  uint8_t* _stompTimer;
  uint8_t* _airBubbleTimer;
  uint8_t* _fallPitTimer;
  uint8_t* _multiCoinBlockTimer;
  uint8_t* _invincibleTimer;
  uint8_t* _starTimer;

  uint8_t* _player1Input;
  uint8_t* _player1Buttons;
  uint8_t* _player1GamePad1;
  uint8_t* _player1GamePad2;

  uint16_t* _warpAreaOffset;

  // Derivative values
  uint16_t _marioPosX;
  uint16_t _screenPosX;
  int16_t _marioScreenOffset;
  uint8_t _currentWorld;
  uint8_t _currentStage;

  #define USE_LIGHT_HASH true

  // Function to advance frame
  void advanceFrame(const uint8_t &move) { _emu->advanceFrame(move); updateDerivedValues(); };
  void advanceFrame(const std::string& move) { _emu->advanceFrame(move); updateDerivedValues(); };

  // This function computes the hash for the current state
  inline uint64_t computeHash() const
  {
    // Storage for hash calculation
    MetroHash64 hash;

    // Adding fixed hash elements
    hash.Update(*_screenScroll);
    if (!USE_LIGHT_HASH) hash.Update(*_marioAnimation);
    hash.Update(*_marioState);

    hash.Update(*_marioBasePosX);
    hash.Update(*_marioRelPosX);
    if (!USE_LIGHT_HASH) hash.Update(*_marioSubpixelPosX);

    hash.Update(*_marioPosY);
    if (!USE_LIGHT_HASH) hash.Update(*_marioSubpixelPosY);

    hash.Update(*_marioXMoveForce);
    hash.Update(*_marioFacingDirection);
    hash.Update(*_marioMovingDirection);
    hash.Update(*_marioFloatingMode);
    hash.Update(*_marioWalkingMode);
    if (!USE_LIGHT_HASH)  hash.Update(*_marioWalkingDelay);
    if (!USE_LIGHT_HASH)  hash.Update(*_marioWalkingFrame);
    hash.Update(*_marioMaxVelLeft);
    hash.Update(*_marioMaxVelRight);
    hash.Update(*_marioVelX);
    if (!USE_LIGHT_HASH) hash.Update(*_marioVelY);
    if (!USE_LIGHT_HASH) hash.Update(*_marioFracVelY);
    hash.Update(*_marioGravity);
    hash.Update(*_marioFriction);

    hash.Update(*_screenBasePosX);
    hash.Update(*_screenRelPosX);

    hash.Update(*_currentWorldRaw);
    hash.Update(*_currentStageRaw);
    hash.Update(*_levelEntryFlag);
    hash.Update(*_gameMode);

    if (!USE_LIGHT_HASH) hash.Update(*_enemy1Active);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy2Active);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy3Active);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy4Active);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy5Active);

    if (!USE_LIGHT_HASH) hash.Update(*_enemy1State);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy2State);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy3State);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy4State);
    if (!USE_LIGHT_HASH) hash.Update(*_enemy5State);

    hash.Update(*_enemy1Type);
    hash.Update(*_enemy2Type);
    hash.Update(*_enemy3Type);
    hash.Update(*_enemy4Type);
    hash.Update(*_enemy5Type);

    if (!USE_LIGHT_HASH) hash.Update(*_marioCollision);
    if (!USE_LIGHT_HASH) hash.Update(*_enemyCollision);
    hash.Update(*_hitDetectionFlag);

    // To Reduce timer pressure on hash, have 0, 1, and >1 as possibilities only
    if (!USE_LIGHT_HASH)  hash.Update(*_animationTimer < 2 ? *_animationTimer : (uint8_t)2);
    hash.Update(*_jumpSwimTimer < 2 ? *_jumpSwimTimer : (uint8_t)2);
    hash.Update(*_runningTimer < 2 ? *_runningTimer : (uint8_t)2);
    hash.Update(*_blockBounceTimer < 2 ? *_blockBounceTimer : (uint8_t)2);
    if (!USE_LIGHT_HASH) hash.Update(*_sideCollisionTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_jumpspringTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameControlTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_climbSideTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_enemyFrameTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_frenzyEnemyTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_bowserFireTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_stompTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_airBubbleTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_fallPitTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_multiCoinBlockTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_invincibleTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_starTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_powerUpActive);


    hash.Update(*_warpAreaOffset);

  //    hash.Update(*_player1Input);
  //    hash.Update(*_player1Buttons);
  //    hash.Update(*_player1GamePad1);
  //    hash.Update(*_player1GamePad2);

    uint64_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  void serializeState(uint8_t* state) const
  {
   _emu->serializeState(state);
  }

  void deserializeState(const uint8_t* state)
  {
   _emu->deserializeState(state);
   updateDerivedValues();
  }

  void setGameValuePointers()
  {
   // Thanks to https://datacrystal.romhacking.net/wiki/Super_Mario_Bros.:RAM_map and https://tasvideos.org/GameResources/NES/SuperMarioBros for helping me find some of these items
   // Game specific values
   _screenScroll         = (uint16_t*) &_emu->_baseMem[0x071B];
   _marioAnimation       = (uint8_t*)  &_emu->_baseMem[0x0001];
   _marioState           = (uint8_t*)  &_emu->_baseMem[0x000E];

   _marioBasePosX        = (uint8_t*)  &_emu->_baseMem[0x006D];
   _marioRelPosX         = (uint8_t*)  &_emu->_baseMem[0x0086];
   _marioSubpixelPosX    = (uint8_t*)  &_emu->_baseMem[0x0400];

   _marioPosY            = (uint8_t*)  &_emu->_baseMem[0x00CE];
   _marioSubpixelPosY    = (uint8_t*)  &_emu->_baseMem[0x0416];

   _marioMovingDirection = (uint8_t*)  &_emu->_baseMem[0x0045];
   _marioFacingDirection = (uint8_t*)  &_emu->_baseMem[0x0033];
   _marioFloatingMode    = (uint8_t*)  &_emu->_baseMem[0x001D];
   _marioWalkingMode     = (uint8_t*)  &_emu->_baseMem[0x0702];
   _marioWalkingDelay    = (uint8_t*)  &_emu->_baseMem[0x070C];
   _marioWalkingFrame    = (uint8_t*)  &_emu->_baseMem[0x070D];
   _marioMaxVelLeft      = (int8_t*)   &_emu->_baseMem[0x0450];
   _marioMaxVelRight     = (int8_t*)   &_emu->_baseMem[0x0456];
   _marioVelX            = (int8_t*)   &_emu->_baseMem[0x0057];
   _marioXMoveForce      = (int8_t*)   &_emu->_baseMem[0x0705];
   _marioVelY            = (int8_t*)   &_emu->_baseMem[0x009F];
   _marioFracVelY        = (int8_t*)   &_emu->_baseMem[0x0433];
   _marioGravity         = (uint8_t*)  &_emu->_baseMem[0x0709];
   _marioFriction        = (uint8_t*)  &_emu->_baseMem[0x0701];
   _timeLeft100          = (uint8_t*)  &_emu->_baseMem[0x07F8];
   _timeLeft10           = (uint8_t*)  &_emu->_baseMem[0x07F9];
   _timeLeft1            = (uint8_t*)  &_emu->_baseMem[0x07FA];

   _screenBasePosX       = (uint8_t*)  &_emu->_baseMem[0x071A];
   _screenRelPosX        = (uint8_t*)  &_emu->_baseMem[0x071C];

   _currentWorldRaw      = (uint8_t*)  &_emu->_baseMem[0x075F];
   _currentStageRaw      = (uint8_t*)  &_emu->_baseMem[0x075C];
   _levelEntryFlag       = (uint8_t*)  &_emu->_baseMem[0x0752];
   _gameMode             = (uint8_t*)  &_emu->_baseMem[0x0770];

   _enemy1Active         = (uint8_t*)  &_emu->_baseMem[0x000F];
   _enemy2Active         = (uint8_t*)  &_emu->_baseMem[0x0010];
   _enemy3Active         = (uint8_t*)  &_emu->_baseMem[0x0011];
   _enemy4Active         = (uint8_t*)  &_emu->_baseMem[0x0012];
   _enemy5Active         = (uint8_t*)  &_emu->_baseMem[0x0013];

   _enemy1State          = (uint8_t*)  &_emu->_baseMem[0x001E];
   _enemy2State          = (uint8_t*)  &_emu->_baseMem[0x001F];
   _enemy3State          = (uint8_t*)  &_emu->_baseMem[0x0020];
   _enemy4State          = (uint8_t*)  &_emu->_baseMem[0x0021];
   _enemy5State          = (uint8_t*)  &_emu->_baseMem[0x0022];

   _enemy1Type           = (uint8_t*)  &_emu->_baseMem[0x0016];
   _enemy2Type           = (uint8_t*)  &_emu->_baseMem[0x0017];
   _enemy3Type           = (uint8_t*)  &_emu->_baseMem[0x0018];
   _enemy4Type           = (uint8_t*)  &_emu->_baseMem[0x0019];
   _enemy5Type           = (uint8_t*)  &_emu->_baseMem[0x001A];

   _marioCollision       = (uint8_t*)  &_emu->_baseMem[0x0490];
   _enemyCollision       = (uint8_t*)  &_emu->_baseMem[0x0491];
   _hitDetectionFlag     = (uint8_t*)  &_emu->_baseMem[0x0722];

   _powerUpActive        = (uint8_t*)  &_emu->_baseMem[0x0014];

   _animationTimer       = (uint8_t*)  &_emu->_baseMem[0x0781];
   _jumpSwimTimer        = (uint8_t*)  &_emu->_baseMem[0x0782];
   _runningTimer         = (uint8_t*)  &_emu->_baseMem[0x0783];
   _blockBounceTimer     = (uint8_t*)  &_emu->_baseMem[0x0784];
   _sideCollisionTimer   = (uint8_t*)  &_emu->_baseMem[0x0785];
   _jumpspringTimer      = (uint8_t*)  &_emu->_baseMem[0x0786];
   _gameControlTimer     = (uint8_t*)  &_emu->_baseMem[0x0787];
   _climbSideTimer       = (uint8_t*)  &_emu->_baseMem[0x0789];
   _enemyFrameTimer      = (uint8_t*)  &_emu->_baseMem[0x078A];
   _frenzyEnemyTimer     = (uint8_t*)  &_emu->_baseMem[0x078F];
   _bowserFireTimer      = (uint8_t*)  &_emu->_baseMem[0x0790];
   _stompTimer           = (uint8_t*)  &_emu->_baseMem[0x0791];
   _airBubbleTimer       = (uint8_t*)  &_emu->_baseMem[0x0792];
   _fallPitTimer         = (uint8_t*)  &_emu->_baseMem[0x0795];
   _multiCoinBlockTimer  = (uint8_t*)  &_emu->_baseMem[0x079D];
   _invincibleTimer      = (uint8_t*)  &_emu->_baseMem[0x079E];
   _starTimer            = (uint8_t*)  &_emu->_baseMem[0x079F];

   _player1Input         = (uint8_t*)  &_emu->_baseMem[0x06FC];
   _player1Buttons       = (uint8_t*)  &_emu->_baseMem[0x074A];
   _player1GamePad1      = (uint8_t*)  &_emu->_baseMem[0x000A];
   _player1GamePad2      = (uint8_t*)  &_emu->_baseMem[0x000D];

   _warpAreaOffset       = (uint16_t*) &_emu->_baseMem[0x0750];
  }

  inline void updateDerivedValues()
  {
   // Recalculating derived values
   _marioPosX = (uint16_t)*_marioBasePosX * 256 + (uint16_t)*_marioRelPosX;
   _screenPosX = (uint16_t)*_screenBasePosX * 256 + (uint16_t)*_screenRelPosX;
   _marioScreenOffset = _marioPosX - _screenPosX;
   _currentWorld = *_currentWorldRaw + 1;
   _currentStage = *_currentStageRaw + 1;
  }

  // Function to determine the current possible moves
  inline std::vector<uint8_t> getPossibleMoveIds() const
  {
   // Possible moves
   // Move Ids =        0    1    2    3    4    5     6     7     8    9     10    11      12    13     14      15
   //_possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "LAB", "RAB", "LRAB" };

   // If Mario's state is not normal (!= 8), there's nothing to do except wait
   if (*_marioState != 8) return { 0 };

   // If floating, B, D have no effect
   if (*_marioFloatingMode == 1) return { 0, 1, 2, 4, 6, 7, 10, 11 };

   // On the floor, try all possible combinations, prioritize jumping and right direction
   return { 0, 2, 3, 10, 4, 7, 9, 1, 6, 8, 11, 12 };
  }

  // Function to get magnet information
  inline magnetSet_t getMagnetValues(const bool* rulesStatus) const
  {
   // Storage for magnet information
   magnetSet_t magnetSet;

   magnetSet.marioScreenOffsetMagnet.intensity = 0.0f;
   magnetSet.marioScreenOffsetMagnet.max = 0.0f;

   magnetSet.screenHorizontalMagnet.intensity = 0.0f;
   magnetSet.screenHorizontalMagnet.max = 0.0f;

   magnetSet.marioHorizontalMagnet.intensity = 0.0f;
   magnetSet.marioHorizontalMagnet.max = 0.0f;

   magnetSet.marioVerticalMagnet.intensity = 0.0f;
   magnetSet.marioVerticalMagnet.max = 0.0f;

   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++) if (rulesStatus[ruleId] == true) magnetSet = _rules[ruleId]->_magnetSet;

   return magnetSet;
  }

  // Obtains the score of a given frame
  inline float getFrameReward(const bool* rulesStatus) const
  {
    // Getting rewards from rules
    float reward = 0.0;
    for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
     if (rulesStatus[ruleId] == true)
      reward += _rules[ruleId]->_reward;

    // Getting magnet values for the kid
    auto magnetSet = getMagnetValues(rulesStatus);

    // Evaluating screen screen magnet value
    reward += magnetSet.screenHorizontalMagnet.intensity * std::min((float)_screenPosX, magnetSet.screenHorizontalMagnet.max);

    // Evaluating mario / screen offset magnet value
    reward += magnetSet.marioScreenOffsetMagnet.intensity * std::min((float)_marioScreenOffset, magnetSet.marioScreenOffsetMagnet.max);

    // Evaluating mario magnet's reward on position X
    reward += magnetSet.marioHorizontalMagnet.intensity * std::min((float)_marioPosX, magnetSet.marioHorizontalMagnet.max);

    // Evaluating mario magnet's reward on position Y
    reward += magnetSet.marioVerticalMagnet.intensity * std::min((float)*_marioPosY, magnetSet.marioVerticalMagnet.max);

    // If mario is getting into the tube, reward timer going down
    if (*_marioState == 3) reward += 1000.0f * (24.0f - (float)*_climbSideTimer);

    // Returning reward
    return reward;
  }

  // Evaluates the rule set on a given frame. Returns true if it is a fail.
  inline void evaluateRules(bool* rulesStatus) const
  {
    for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
    {
      // Evaluate rule only if it's active
      if (rulesStatus[ruleId] == false)
      {
        // Checking if conditions are met
        bool isSatisfied = _rules[ruleId]->evaluate();

        // If it's achieved, update its status and run its actions
        if (isSatisfied) satisfyRule(rulesStatus, ruleId);
      }
    }
  }

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  inline void satisfyRule(bool* rulesStatus, const size_t ruleId) const
  {
   // Recursively run actions for the yet unsatisfied rules that are satisfied by this one and mark them as satisfied
   for (size_t satisfiedIdx = 0; satisfiedIdx < _rules[ruleId]->_satisfiesIndexes.size(); satisfiedIdx++)
   {
    // Obtaining index
    size_t subRuleId = _rules[ruleId]->_satisfiesIndexes[satisfiedIdx];

    // Only activate it if it hasn't been activated before
    if (rulesStatus[subRuleId] == false) satisfyRule(rulesStatus, subRuleId);
   }

   // Setting status to satisfied
   rulesStatus[ruleId] = true;
  }

  // Serialization/Deserialization Routines
  inline void pushState(const uint8_t* __restrict__ inputStateData) const
  {
    _emu->deserializeState(inputStateData);
  }

  inline void popState(uint8_t* __restrict__ outputStateData) const
  {
   _emu->serializeState(outputStateData);
  }

  // Get frame type
  inline frameType getFrameType(const bool* rulesStatus) const
  {
   frameType type = f_regular;

   for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
    if (rulesStatus[ruleId] == true)
    {
     if (_rules[ruleId]->_isFailRule == true) type = f_fail;
     if (_rules[ruleId]->_isWinRule == true) type = f_win;
    }

   return type;
  }

  void printStateInfo() const;
};
