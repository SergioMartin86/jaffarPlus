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

// If we use NCurses, we need to use the appropriate printing function
#ifdef NCURSES
 #include <ncurses.h>
 #define LOG printw
#else
 #define LOG printf
#endif

struct gameData_t
{
 // Game specific values
 uint16_t* screenScroll;
 uint8_t* marioAnimation;
 uint8_t* marioState;

 uint8_t* marioBasePosX;
 uint8_t* marioRelPosX;
 uint8_t* marioSubpixelPosX;

 uint8_t* marioPosY;
 uint8_t* marioSubpixelPosY;

 uint8_t* marioMovingDirection;
 uint8_t* marioFacingDirection;
 uint8_t* marioFloatingMode;
 uint8_t* marioWalkingMode;
 uint8_t* marioWalkingDelay;
 uint8_t* marioWalkingFrame;
 int8_t* marioMaxVelLeft;
 int8_t* marioMaxVelRight;
 int8_t* marioVelX;
 int8_t* marioXMoveForce;
 int8_t* marioVelY;
 int8_t* marioFracVelY;
 uint8_t* marioGravity;
 uint8_t* marioFriction;
 uint8_t* timeLeft100;
 uint8_t* timeLeft10;
 uint8_t* timeLeft1;

 uint8_t* screenBasePosX;
 uint8_t* screenRelPosX;

 uint8_t* currentWorldRaw;
 uint8_t* currentStageRaw;
 uint8_t* levelEntryFlag;
 uint8_t* gameMode;

 uint8_t* enemy1Active;
 uint8_t* enemy2Active;
 uint8_t* enemy3Active;
 uint8_t* enemy4Active;
 uint8_t* enemy5Active;

 uint8_t* enemy1State;
 uint8_t* enemy2State;
 uint8_t* enemy3State;
 uint8_t* enemy4State;
 uint8_t* enemy5State;

 uint8_t* enemy1Type;
 uint8_t* enemy2Type;
 uint8_t* enemy3Type;
 uint8_t* enemy4Type;
 uint8_t* enemy5Type;

 uint8_t* marioCollision;
 uint8_t* enemyCollision;
 uint8_t* hitDetectionFlag;

 uint8_t* powerUpActive;

 uint8_t* animationTimer;
 uint8_t* jumpSwimTimer;
 uint8_t* runningTimer;
 uint8_t* blockBounceTimer;
 uint8_t* sideCollisionTimer;
 uint8_t* jumpspringTimer;
 uint8_t* climbSideTimer;
 uint8_t* gameControlTimer;
 uint8_t* enemyFrameTimer;
 uint8_t* frenzyEnemyTimer;
 uint8_t* bowserFireTimer;
 uint8_t* stompTimer;
 uint8_t* airBubbleTimer;
 uint8_t* fallPitTimer;
 uint8_t* multiCoinBlockTimer;
 uint8_t* invincibleTimer;
 uint8_t* starTimer;

 uint8_t* player1Input;
 uint8_t* player1Buttons;
 uint8_t* player1GamePad1;
 uint8_t* player1GamePad2;

 uint16_t* warpAreaOffset;

 // Derivative values
 uint16_t marioPosX;
 uint16_t screenPosX;
 int16_t marioScreenOffset;
 uint8_t currentWorld;
 uint8_t currentStage;
};

class State
{
 public:

  // Container for game-specific values
  gameData_t _gameData;

  // Constructor for the underlying emulator using a rom file and a state
  State(const std::string romFile, const std::string stateFile)
  {
    // Creating quickNES instance
    _emu = new EmuInstance(romFile);
    _emu->loadStateFile(stateFile);

    // Setting game-specific value pointers
    setGameValuePointers();

    // Updating initial derived values
    updateDerivedValues();
  }

  // Constructor using an already existing emulator
  State(EmuInstance* emu)
  {
    // Copying emu instance pointer
    _emu = emu;

    // Setting game-specific value pointers
    setGameValuePointers();

    // Updating initial derived values
    updateDerivedValues();
  }

  // Rule parser
  void parseRules(const nlohmann::json rulesConfig)
  {
   // Processing rules
   for (size_t ruleId = 0; ruleId < rulesConfig.size(); ruleId++)
    _rules.push_back(new Rule(rulesConfig[ruleId], this));

   // Setting global rule count
   _ruleCount = _rules.size();

   if (_ruleCount > _MAX_RULE_COUNT) EXIT_WITH_ERROR("[ERROR] Configured Jaffar to run %lu rules, but the specified script contains %lu. Modify frame.h and rebuild to run this script.\n", _MAX_RULE_COUNT, _ruleCount);

   // Checking for repeated rule labels
   std::set<size_t> ruleLabelSet;
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
   {
    size_t label = _rules[ruleId]->_label;
    ruleLabelSet.insert(label);
    if (ruleLabelSet.size() < ruleId + 1)
     EXIT_WITH_ERROR("[ERROR] Rule label %lu is repeated in the configuration file.\n", label);
   }

   // Looking for rule satisfied sub-rules indexes that match their labels
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
    for (size_t satisfiedId = 0; satisfiedId < _rules[ruleId]->_satisfiesLabels.size(); satisfiedId++)
    {
     bool foundLabel = false;
     size_t label = _rules[ruleId]->_satisfiesLabels[satisfiedId];
     if (label == _rules[ruleId]->_label) EXIT_WITH_ERROR("[ERROR] Rule %lu references itself in satisfied vector.\n", label);

     for (size_t subRuleId = 0; subRuleId < _ruleCount; subRuleId++)
      if (_rules[subRuleId]->_label == label)
      {
       _rules[ruleId]->_satisfiesIndexes[satisfiedId] = subRuleId;
       foundLabel = true;
       break;
      }
     if (foundLabel == false) EXIT_WITH_ERROR("[ERROR] Could not find rule label %lu, specified as satisfied by rule %lu.\n", label, satisfiedId);
    }
  }

  std::vector<Rule *> _rules;
  size_t _ruleCount;
  EmuInstance *_emu;

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
    hash.Update(*_gameData.screenScroll);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.marioAnimation);
    hash.Update(*_gameData.marioState);

    hash.Update(*_gameData.marioBasePosX);
    hash.Update(*_gameData.marioRelPosX);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.marioSubpixelPosX);

    hash.Update(*_gameData.marioPosY);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.marioSubpixelPosY);

    hash.Update(*_gameData.marioXMoveForce);
    hash.Update(*_gameData.marioFacingDirection);
    hash.Update(*_gameData.marioMovingDirection);
    hash.Update(*_gameData.marioFloatingMode);
    hash.Update(*_gameData.marioWalkingMode);
    if (!USE_LIGHT_HASH)  hash.Update(*_gameData.marioWalkingDelay);
    if (!USE_LIGHT_HASH)  hash.Update(*_gameData.marioWalkingFrame);
    hash.Update(*_gameData.marioMaxVelLeft);
    hash.Update(*_gameData.marioMaxVelRight);
    hash.Update(*_gameData.marioVelX);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.marioVelY);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.marioFracVelY);
    hash.Update(*_gameData.marioGravity);
    hash.Update(*_gameData.marioFriction);

    hash.Update(*_gameData.screenBasePosX);
    hash.Update(*_gameData.screenRelPosX);

    hash.Update(*_gameData.currentWorldRaw);
    hash.Update(*_gameData.currentStageRaw);
    hash.Update(*_gameData.levelEntryFlag);
    hash.Update(*_gameData.gameMode);

    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy1Active);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy2Active);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy3Active);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy4Active);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy5Active);

    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy1State);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy2State);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy3State);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy4State);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemy5State);

    hash.Update(*_gameData.enemy1Type);
    hash.Update(*_gameData.enemy2Type);
    hash.Update(*_gameData.enemy3Type);
    hash.Update(*_gameData.enemy4Type);
    hash.Update(*_gameData.enemy5Type);

    if (!USE_LIGHT_HASH) hash.Update(*_gameData.marioCollision);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemyCollision);
    hash.Update(*_gameData.hitDetectionFlag);

    // To Reduce timer pressure on hash, have 0, 1, and >1 as possibilities only
    if (!USE_LIGHT_HASH)  hash.Update(*_gameData.animationTimer < 2 ? *_gameData.animationTimer : (uint8_t)2);
    hash.Update(*_gameData.jumpSwimTimer < 2 ? *_gameData.jumpSwimTimer : (uint8_t)2);
    hash.Update(*_gameData.runningTimer < 2 ? *_gameData.runningTimer : (uint8_t)2);
    hash.Update(*_gameData.blockBounceTimer < 2 ? *_gameData.blockBounceTimer : (uint8_t)2);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.sideCollisionTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.jumpspringTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.gameControlTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.climbSideTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.enemyFrameTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.frenzyEnemyTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.bowserFireTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.stompTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.airBubbleTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.fallPitTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.multiCoinBlockTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.invincibleTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.starTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_gameData.powerUpActive);


    hash.Update(*_gameData.warpAreaOffset);

  //    hash.Update(*_gameData.player1Input);
  //    hash.Update(*_gameData.player1Buttons);
  //    hash.Update(*_gameData.player1GamePad1);
  //    hash.Update(*_gameData.player1GamePad2);

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
   _gameData.screenScroll         = (uint16_t*) &_emu->_baseMem[0x071B];
   _gameData.marioAnimation       = (uint8_t*)  &_emu->_baseMem[0x0001];
   _gameData.marioState           = (uint8_t*)  &_emu->_baseMem[0x000E];

   _gameData.marioBasePosX        = (uint8_t*)  &_emu->_baseMem[0x006D];
   _gameData.marioRelPosX         = (uint8_t*)  &_emu->_baseMem[0x0086];
   _gameData.marioSubpixelPosX    = (uint8_t*)  &_emu->_baseMem[0x0400];

   _gameData.marioPosY            = (uint8_t*)  &_emu->_baseMem[0x00CE];
   _gameData.marioSubpixelPosY    = (uint8_t*)  &_emu->_baseMem[0x0416];

   _gameData.marioMovingDirection = (uint8_t*)  &_emu->_baseMem[0x0045];
   _gameData.marioFacingDirection = (uint8_t*)  &_emu->_baseMem[0x0033];
   _gameData.marioFloatingMode    = (uint8_t*)  &_emu->_baseMem[0x001D];
   _gameData.marioWalkingMode     = (uint8_t*)  &_emu->_baseMem[0x0702];
   _gameData.marioWalkingDelay    = (uint8_t*)  &_emu->_baseMem[0x070C];
   _gameData.marioWalkingFrame    = (uint8_t*)  &_emu->_baseMem[0x070D];
   _gameData.marioMaxVelLeft      = (int8_t*)   &_emu->_baseMem[0x0450];
   _gameData.marioMaxVelRight     = (int8_t*)   &_emu->_baseMem[0x0456];
   _gameData.marioVelX            = (int8_t*)   &_emu->_baseMem[0x0057];
   _gameData.marioXMoveForce      = (int8_t*)   &_emu->_baseMem[0x0705];
   _gameData.marioVelY            = (int8_t*)   &_emu->_baseMem[0x009F];
   _gameData.marioFracVelY        = (int8_t*)   &_emu->_baseMem[0x0433];
   _gameData.marioGravity         = (uint8_t*)  &_emu->_baseMem[0x0709];
   _gameData.marioFriction        = (uint8_t*)  &_emu->_baseMem[0x0701];
   _gameData.timeLeft100          = (uint8_t*)  &_emu->_baseMem[0x07F8];
   _gameData.timeLeft10           = (uint8_t*)  &_emu->_baseMem[0x07F9];
   _gameData.timeLeft1            = (uint8_t*)  &_emu->_baseMem[0x07FA];

   _gameData.screenBasePosX       = (uint8_t*)  &_emu->_baseMem[0x071A];
   _gameData.screenRelPosX        = (uint8_t*)  &_emu->_baseMem[0x071C];

   _gameData.currentWorldRaw      = (uint8_t*)  &_emu->_baseMem[0x075F];
   _gameData.currentStageRaw      = (uint8_t*)  &_emu->_baseMem[0x075C];
   _gameData.levelEntryFlag       = (uint8_t*)  &_emu->_baseMem[0x0752];
   _gameData.gameMode             = (uint8_t*)  &_emu->_baseMem[0x0770];

   _gameData.enemy1Active         = (uint8_t*)  &_emu->_baseMem[0x000F];
   _gameData.enemy2Active         = (uint8_t*)  &_emu->_baseMem[0x0010];
   _gameData.enemy3Active         = (uint8_t*)  &_emu->_baseMem[0x0011];
   _gameData.enemy4Active         = (uint8_t*)  &_emu->_baseMem[0x0012];
   _gameData.enemy5Active         = (uint8_t*)  &_emu->_baseMem[0x0013];

   _gameData.enemy1State          = (uint8_t*)  &_emu->_baseMem[0x001E];
   _gameData.enemy2State          = (uint8_t*)  &_emu->_baseMem[0x001F];
   _gameData.enemy3State          = (uint8_t*)  &_emu->_baseMem[0x0020];
   _gameData.enemy4State          = (uint8_t*)  &_emu->_baseMem[0x0021];
   _gameData.enemy5State          = (uint8_t*)  &_emu->_baseMem[0x0022];

   _gameData.enemy1Type           = (uint8_t*)  &_emu->_baseMem[0x0016];
   _gameData.enemy2Type           = (uint8_t*)  &_emu->_baseMem[0x0017];
   _gameData.enemy3Type           = (uint8_t*)  &_emu->_baseMem[0x0018];
   _gameData.enemy4Type           = (uint8_t*)  &_emu->_baseMem[0x0019];
   _gameData.enemy5Type           = (uint8_t*)  &_emu->_baseMem[0x001A];

   _gameData.marioCollision       = (uint8_t*)  &_emu->_baseMem[0x0490];
   _gameData.enemyCollision       = (uint8_t*)  &_emu->_baseMem[0x0491];
   _gameData.hitDetectionFlag     = (uint8_t*)  &_emu->_baseMem[0x0722];

   _gameData.powerUpActive        = (uint8_t*)  &_emu->_baseMem[0x0014];

   _gameData.animationTimer       = (uint8_t*)  &_emu->_baseMem[0x0781];
   _gameData.jumpSwimTimer        = (uint8_t*)  &_emu->_baseMem[0x0782];
   _gameData.runningTimer         = (uint8_t*)  &_emu->_baseMem[0x0783];
   _gameData.blockBounceTimer     = (uint8_t*)  &_emu->_baseMem[0x0784];
   _gameData.sideCollisionTimer   = (uint8_t*)  &_emu->_baseMem[0x0785];
   _gameData.jumpspringTimer      = (uint8_t*)  &_emu->_baseMem[0x0786];
   _gameData.gameControlTimer     = (uint8_t*)  &_emu->_baseMem[0x0787];
   _gameData.climbSideTimer       = (uint8_t*)  &_emu->_baseMem[0x0789];
   _gameData.enemyFrameTimer      = (uint8_t*)  &_emu->_baseMem[0x078A];
   _gameData.frenzyEnemyTimer     = (uint8_t*)  &_emu->_baseMem[0x078F];
   _gameData.bowserFireTimer      = (uint8_t*)  &_emu->_baseMem[0x0790];
   _gameData.stompTimer           = (uint8_t*)  &_emu->_baseMem[0x0791];
   _gameData.airBubbleTimer       = (uint8_t*)  &_emu->_baseMem[0x0792];
   _gameData.fallPitTimer         = (uint8_t*)  &_emu->_baseMem[0x0795];
   _gameData.multiCoinBlockTimer  = (uint8_t*)  &_emu->_baseMem[0x079D];
   _gameData.invincibleTimer      = (uint8_t*)  &_emu->_baseMem[0x079E];
   _gameData.starTimer            = (uint8_t*)  &_emu->_baseMem[0x079F];

   _gameData.player1Input         = (uint8_t*)  &_emu->_baseMem[0x06FC];
   _gameData.player1Buttons       = (uint8_t*)  &_emu->_baseMem[0x074A];
   _gameData.player1GamePad1      = (uint8_t*)  &_emu->_baseMem[0x000A];
   _gameData.player1GamePad2      = (uint8_t*)  &_emu->_baseMem[0x000D];

   _gameData.warpAreaOffset       = (uint16_t*) &_emu->_baseMem[0x0750];
  }

  inline void updateDerivedValues()
  {
   // Recalculating derived values
   _gameData.marioPosX = (uint16_t)*_gameData.marioBasePosX * 256 + (uint16_t)*_gameData.marioRelPosX;
   _gameData.screenPosX = (uint16_t)*_gameData.screenBasePosX * 256 + (uint16_t)*_gameData.screenRelPosX;
   _gameData.marioScreenOffset = _gameData.marioPosX - _gameData.screenPosX;
   _gameData.currentWorld = *_gameData.currentWorldRaw + 1;
   _gameData.currentStage = *_gameData.currentStageRaw + 1;
  }

  // Function to determine the current possible moves
  inline std::vector<uint8_t> getPossibleMoveIds() const
  {
   // Possible moves
   // Move Ids =        0    1    2    3    4    5     6     7     8    9     10    11      12    13     14      15
   //_possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "LAB", "RAB", "LRAB" };

   // If Mario's state is not normal (!= 8), there's nothing to do except wait
   if (*_gameData.marioState != 8) return { 0 };

   // If floating, B, D have no effect
   if (*_gameData.marioFloatingMode == 1) return { 0, 1, 2, 4, 6, 7, 10, 11 };

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
    reward += magnetSet.screenHorizontalMagnet.intensity * std::min((float)_gameData.screenPosX, magnetSet.screenHorizontalMagnet.max);

    // Evaluating mario / screen offset magnet value
    reward += magnetSet.marioScreenOffsetMagnet.intensity * std::min((float)_gameData.marioScreenOffset, magnetSet.marioScreenOffsetMagnet.max);

    // Evaluating mario magnet's reward on position X
    reward += magnetSet.marioHorizontalMagnet.intensity * std::min((float)_gameData.marioPosX, magnetSet.marioHorizontalMagnet.max);

    // Evaluating mario magnet's reward on position Y
    reward += magnetSet.marioVerticalMagnet.intensity * std::min((float)*_gameData.marioPosY, magnetSet.marioVerticalMagnet.max);

    // If mario is getting into the tube, reward timer going down
    if (*_gameData.marioState == 3) reward += 1000.0f * (24.0f - (float)*_gameData.climbSideTimer);

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

  void printRuleStatus(const bool* rulesStatus)
  {
   printf("[Jaffar]  + Rule Status: ");
   for (size_t i = 0; i < _ruleCount; i++)
   {
     if (i > 0 && i % 60 == 0) printf("\n                         ");
     printf("%d", rulesStatus[i] ? 1 : 0);
   }
   printf("\n");

   // Getting magnet values
   auto magnets = getMagnetValues(rulesStatus);

   LOG("[Jaffar]  + Screen Horizontal Magnet   - Intensity: %.1f, Max: %f\n", magnets.screenHorizontalMagnet.intensity, magnets.screenHorizontalMagnet.max);
   LOG("[Jaffar]  + Mario Screen Offset Magnet - Intensity: %.1f, Max: %f\n", magnets.marioScreenOffsetMagnet.intensity, magnets.marioScreenOffsetMagnet.max);
   LOG("[Jaffar]  + Mario Horizontal Magnet    - Intensity: %.1f, Max: %f\n", magnets.marioHorizontalMagnet.intensity, magnets.marioHorizontalMagnet.max);
   LOG("[Jaffar]  + Mario Vertical Magnet      - Intensity: %.1f, Max: %f\n", magnets.marioVerticalMagnet.intensity, magnets.marioVerticalMagnet.max);
  }

  void printStateInfo() const
  {
    LOG("[Jaffar]  + Current World-Stage:    %1u-%1u\n", _gameData.currentWorld, _gameData.currentStage);
    LOG("[Jaffar]  + Time Left:              %1u%1u%1u\n", *_gameData.timeLeft100, *_gameData.timeLeft10, *_gameData.timeLeft1);
    LOG("[Jaffar]  + Mario Animation:        %02u\n", *_gameData.marioAnimation);
    LOG("[Jaffar]  + Mario State:            %02u\n", *_gameData.marioState);
    LOG("[Jaffar]  + Screen Pos X:           %04u (%02u * 256 = %04u + %02u)\n", _gameData.screenPosX, *_gameData.screenBasePosX, (uint16_t)*_gameData.screenBasePosX * 255, *_gameData.screenRelPosX);
    LOG("[Jaffar]  + Mario Pos X:            %04u (%02u * 256 = %04u + %02u)\n", _gameData.marioPosX, *_gameData.marioBasePosX, (uint16_t)*_gameData.marioBasePosX * 255, *_gameData.marioRelPosX);
    LOG("[Jaffar]  + Mario / Screen Offset:  %04d\n", _gameData.marioScreenOffset);
    LOG("[Jaffar]  + Mario Pos Y:            %02u\n", *_gameData.marioPosY);
    LOG("[Jaffar]  + Mario SubPixel X/Y:     %02u / %02u\n", *_gameData.marioSubpixelPosX, *_gameData.marioSubpixelPosY);
    LOG("[Jaffar]  + Mario Vel X:            %02d (Force: %02d, MaxL: %02d, MaxR: %02d)\n", *_gameData.marioVelX, *_gameData.marioXMoveForce, *_gameData.marioMaxVelLeft, *_gameData.marioMaxVelRight);
    LOG("[Jaffar]  + Mario Vel Y:            %02d (%02d)\n", *_gameData.marioVelY, *_gameData.marioFracVelY);
    LOG("[Jaffar]  + Mario Gravity:          %02u\n", *_gameData.marioGravity);
    LOG("[Jaffar]  + Mario Friction:         %02u\n", *_gameData.marioFriction);
    LOG("[Jaffar]  + Mario Moving Direction: %s\n", *_gameData.marioMovingDirection == 1 ? "Right" : "Left");
    LOG("[Jaffar]  + Mario Facing Direction: %s\n", *_gameData.marioFacingDirection == 1 ? "Right" : "Left");
    LOG("[Jaffar]  + Mario Floating Mode:    %02u\n", *_gameData.marioFloatingMode);
    LOG("[Jaffar]  + Mario Walking:          %02u %02u %02u\n", *_gameData.marioWalkingMode, *_gameData.marioWalkingDelay, *_gameData.marioWalkingFrame);
    LOG("[Jaffar]  + Player 1 Inputs:        %02u %02u %02u %02u\n", *_gameData.player1Input, *_gameData.player1Buttons, *_gameData.player1GamePad1, *_gameData.player1GamePad2);
    LOG("[Jaffar]  + Powerup Active:         %1u\n", *_gameData.powerUpActive);
    LOG("[Jaffar]  + Enemy Active:           %1u%1u%1u%1u%1u\n", *_gameData.enemy1Active, *_gameData.enemy2Active, *_gameData.enemy3Active, *_gameData.enemy4Active, *_gameData.enemy5Active);
    LOG("[Jaffar]  + Enemy State:            %02u %02u %02u %02u %02u\n", *_gameData.enemy1State, *_gameData.enemy2State, *_gameData.enemy3State, *_gameData.enemy4State, *_gameData.enemy5State);
    LOG("[Jaffar]  + Enemy Type:             %02u %02u %02u %02u %02u\n", *_gameData.enemy1Type, *_gameData.enemy2Type, *_gameData.enemy3Type, *_gameData.enemy4Type, *_gameData.enemy5Type);
    LOG("[Jaffar]  + Hit Detection Flags:    %02u %02u %02u\n", *_gameData.marioCollision, *_gameData.enemyCollision, *_gameData.hitDetectionFlag);
    LOG("[Jaffar]  + LevelEntry / GameMode:  %02u / %02u\n", *_gameData.levelEntryFlag, *_gameData.gameMode);
    LOG("[Jaffar]  + Warp Area Offset:       %04u\n", *_gameData.warpAreaOffset);
    LOG("[Jaffar]  + Timers:                 %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n", *_gameData.animationTimer, *_gameData.jumpSwimTimer, *_gameData.runningTimer, *_gameData.blockBounceTimer, *_gameData.sideCollisionTimer, *_gameData.jumpspringTimer, *_gameData.gameControlTimer, *_gameData.climbSideTimer, *_gameData.enemyFrameTimer, *_gameData.frenzyEnemyTimer, *_gameData.bowserFireTimer, *_gameData.stompTimer, *_gameData.airBubbleTimer, *_gameData.multiCoinBlockTimer, *_gameData.invincibleTimer, *_gameData.starTimer);
  }
};
