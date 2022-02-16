#pragma once

#include "gameInstanceBase.hpp"

class GameInstance : public GameInstanceBase
{
 public:

  void initialize(const nlohmann::json& config) override
  {
    // Always call the base class initialization
    GameInstanceBase::initialize(config);

    // Any SMB-specific configuration goes here
  };

  #define USE_LIGHT_HASH true

  // This function computes the hash for the current state
  inline uint64_t computeHash() const override
  {
    // Storage for hash calculation
    MetroHash64 hash;

    // Adding fixed hash elements
    hash.Update(*_data.screenScroll);
    if (!USE_LIGHT_HASH) hash.Update(*_data.marioAnimation);
    hash.Update(*_data.marioState);

    hash.Update(*_data.marioBasePosX);
    hash.Update(*_data.marioRelPosX);
    if (!USE_LIGHT_HASH) hash.Update(*_data.marioSubpixelPosX);

    hash.Update(*_data.marioPosY);
    if (!USE_LIGHT_HASH) hash.Update(*_data.marioSubpixelPosY);

    hash.Update(*_data.marioXMoveForce);
    hash.Update(*_data.marioFacingDirection);
    hash.Update(*_data.marioMovingDirection);
    hash.Update(*_data.marioFloatingMode);
    hash.Update(*_data.marioWalkingMode);
    if (!USE_LIGHT_HASH)  hash.Update(*_data.marioWalkingDelay);
    if (!USE_LIGHT_HASH)  hash.Update(*_data.marioWalkingFrame);
    hash.Update(*_data.marioMaxVelLeft);
    hash.Update(*_data.marioMaxVelRight);
    hash.Update(*_data.marioVelX);
    hash.Update(*_data.marioVelY);
    if (!USE_LIGHT_HASH) hash.Update(*_data.marioFracVelY);
    hash.Update(*_data.marioGravity);
    hash.Update(*_data.marioFriction);

    hash.Update(*_data.screenBasePosX);
    hash.Update(*_data.screenRelPosX);

    hash.Update(*_data.currentWorldRaw);
    hash.Update(*_data.currentStageRaw);
    hash.Update(*_data.levelEntryFlag);
    hash.Update(*_data.gameMode);

    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy1Active);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy2Active);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy3Active);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy4Active);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy5Active);

    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy1State);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy2State);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy3State);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy4State);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemy5State);

    hash.Update(*_data.enemy1Type);
    hash.Update(*_data.enemy2Type);
    hash.Update(*_data.enemy3Type);
    hash.Update(*_data.enemy4Type);
    hash.Update(*_data.enemy5Type);

    if (!USE_LIGHT_HASH) hash.Update(*_data.marioCollision);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemyCollision);
    hash.Update(*_data.hitDetectionFlag);

    // To Reduce timer pressure on hash, have 0, 1, and >1 as possibilities only
    if (!USE_LIGHT_HASH)  hash.Update(*_data.animationTimer < 2 ? *_data.animationTimer : (uint8_t)2);
    hash.Update(*_data.jumpSwimTimer < 2 ? *_data.jumpSwimTimer : (uint8_t)2);
    hash.Update(*_data.runningTimer < 2 ? *_data.runningTimer : (uint8_t)2);
    hash.Update(*_data.blockBounceTimer < 2 ? *_data.blockBounceTimer : (uint8_t)2);
    if (!USE_LIGHT_HASH) hash.Update(*_data.sideCollisionTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.jumpspringTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.gameControlTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.climbSideTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.enemyFrameTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.frenzyEnemyTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.bowserFireTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.stompTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.airBubbleTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.fallPitTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.multiCoinBlockTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.invincibleTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.starTimer);
    if (!USE_LIGHT_HASH) hash.Update(*_data.powerUpActive);


    hash.Update(*_data.warpAreaOffset);

  //    hash.Update(*_data.player1Input);
  //    hash.Update(*_data.player1Buttons);
  //    hash.Update(*_data.player1GamePad1);
  //    hash.Update(*_data.player1GamePad2);

    uint64_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  inline void setGameValuePointers() override
  {
   // Thanks to https://datacrystal.romhacking.net/wiki/Super_Mario_Bros.:RAM_map and https://tasvideos.org/GameResources/NES/SuperMarioBros for helping me find some of these items
   // Game specific values
   _data.screenScroll         = (uint16_t*) &_emu->_baseMem[0x071B];
   _data.marioAnimation       = (uint8_t*)  &_emu->_baseMem[0x0001];
   _data.marioState           = (uint8_t*)  &_emu->_baseMem[0x000E];

   _data.marioBasePosX        = (uint8_t*)  &_emu->_baseMem[0x006D];
   _data.marioRelPosX         = (uint8_t*)  &_emu->_baseMem[0x0086];
   _data.marioSubpixelPosX    = (uint8_t*)  &_emu->_baseMem[0x0400];

   _data.marioPosY            = (uint8_t*)  &_emu->_baseMem[0x00CE];
   _data.marioSubpixelPosY    = (uint8_t*)  &_emu->_baseMem[0x0416];

   _data.marioMovingDirection = (uint8_t*)  &_emu->_baseMem[0x0045];
   _data.marioFacingDirection = (uint8_t*)  &_emu->_baseMem[0x0033];
   _data.marioFloatingMode    = (uint8_t*)  &_emu->_baseMem[0x001D];
   _data.marioWalkingMode     = (uint8_t*)  &_emu->_baseMem[0x0702];
   _data.marioWalkingDelay    = (uint8_t*)  &_emu->_baseMem[0x070C];
   _data.marioWalkingFrame    = (uint8_t*)  &_emu->_baseMem[0x070D];
   _data.marioMaxVelLeft      = (int8_t*)   &_emu->_baseMem[0x0450];
   _data.marioMaxVelRight     = (int8_t*)   &_emu->_baseMem[0x0456];
   _data.marioVelX            = (int8_t*)   &_emu->_baseMem[0x0057];
   _data.marioXMoveForce      = (int8_t*)   &_emu->_baseMem[0x0705];
   _data.marioVelY            = (int8_t*)   &_emu->_baseMem[0x009F];
   _data.marioFracVelY        = (int8_t*)   &_emu->_baseMem[0x0433];
   _data.marioGravity         = (uint8_t*)  &_emu->_baseMem[0x0709];
   _data.marioFriction        = (uint8_t*)  &_emu->_baseMem[0x0701];
   _data.timeLeft100          = (uint8_t*)  &_emu->_baseMem[0x07F8];
   _data.timeLeft10           = (uint8_t*)  &_emu->_baseMem[0x07F9];
   _data.timeLeft1            = (uint8_t*)  &_emu->_baseMem[0x07FA];

   _data.screenBasePosX       = (uint8_t*)  &_emu->_baseMem[0x071A];
   _data.screenRelPosX        = (uint8_t*)  &_emu->_baseMem[0x071C];

   _data.currentWorldRaw      = (uint8_t*)  &_emu->_baseMem[0x075F];
   _data.currentStageRaw      = (uint8_t*)  &_emu->_baseMem[0x075C];
   _data.levelEntryFlag       = (uint8_t*)  &_emu->_baseMem[0x0752];
   _data.gameMode             = (uint8_t*)  &_emu->_baseMem[0x0770];

   _data.enemy1Active         = (uint8_t*)  &_emu->_baseMem[0x000F];
   _data.enemy2Active         = (uint8_t*)  &_emu->_baseMem[0x0010];
   _data.enemy3Active         = (uint8_t*)  &_emu->_baseMem[0x0011];
   _data.enemy4Active         = (uint8_t*)  &_emu->_baseMem[0x0012];
   _data.enemy5Active         = (uint8_t*)  &_emu->_baseMem[0x0013];

   _data.enemy1State          = (uint8_t*)  &_emu->_baseMem[0x001E];
   _data.enemy2State          = (uint8_t*)  &_emu->_baseMem[0x001F];
   _data.enemy3State          = (uint8_t*)  &_emu->_baseMem[0x0020];
   _data.enemy4State          = (uint8_t*)  &_emu->_baseMem[0x0021];
   _data.enemy5State          = (uint8_t*)  &_emu->_baseMem[0x0022];

   _data.enemy1Type           = (uint8_t*)  &_emu->_baseMem[0x0016];
   _data.enemy2Type           = (uint8_t*)  &_emu->_baseMem[0x0017];
   _data.enemy3Type           = (uint8_t*)  &_emu->_baseMem[0x0018];
   _data.enemy4Type           = (uint8_t*)  &_emu->_baseMem[0x0019];
   _data.enemy5Type           = (uint8_t*)  &_emu->_baseMem[0x001A];

   _data.marioCollision       = (uint8_t*)  &_emu->_baseMem[0x0490];
   _data.enemyCollision       = (uint8_t*)  &_emu->_baseMem[0x0491];
   _data.hitDetectionFlag     = (uint8_t*)  &_emu->_baseMem[0x0722];

   _data.powerUpActive        = (uint8_t*)  &_emu->_baseMem[0x0014];

   _data.animationTimer       = (uint8_t*)  &_emu->_baseMem[0x0781];
   _data.jumpSwimTimer        = (uint8_t*)  &_emu->_baseMem[0x0782];
   _data.runningTimer         = (uint8_t*)  &_emu->_baseMem[0x0783];
   _data.blockBounceTimer     = (uint8_t*)  &_emu->_baseMem[0x0784];
   _data.sideCollisionTimer   = (uint8_t*)  &_emu->_baseMem[0x0785];
   _data.jumpspringTimer      = (uint8_t*)  &_emu->_baseMem[0x0786];
   _data.gameControlTimer     = (uint8_t*)  &_emu->_baseMem[0x0787];
   _data.climbSideTimer       = (uint8_t*)  &_emu->_baseMem[0x0789];
   _data.enemyFrameTimer      = (uint8_t*)  &_emu->_baseMem[0x078A];
   _data.frenzyEnemyTimer     = (uint8_t*)  &_emu->_baseMem[0x078F];
   _data.bowserFireTimer      = (uint8_t*)  &_emu->_baseMem[0x0790];
   _data.stompTimer           = (uint8_t*)  &_emu->_baseMem[0x0791];
   _data.airBubbleTimer       = (uint8_t*)  &_emu->_baseMem[0x0792];
   _data.fallPitTimer         = (uint8_t*)  &_emu->_baseMem[0x0795];
   _data.multiCoinBlockTimer  = (uint8_t*)  &_emu->_baseMem[0x079D];
   _data.invincibleTimer      = (uint8_t*)  &_emu->_baseMem[0x079E];
   _data.starTimer            = (uint8_t*)  &_emu->_baseMem[0x079F];

   _data.player1Input         = (uint8_t*)  &_emu->_baseMem[0x06FC];
   _data.player1Buttons       = (uint8_t*)  &_emu->_baseMem[0x074A];
   _data.player1GamePad1      = (uint8_t*)  &_emu->_baseMem[0x000A];
   _data.player1GamePad2      = (uint8_t*)  &_emu->_baseMem[0x000D];

   _data.warpAreaOffset       = (uint16_t*) &_emu->_baseMem[0x0750];
  }

  inline void updateDerivedValues() override
  {
   // Recalculating derived values
   _data.marioPosX = (uint16_t)*_data.marioBasePosX * 256 + (uint16_t)*_data.marioRelPosX;
   _data.screenPosX = (uint16_t)*_data.screenBasePosX * 256 + (uint16_t)*_data.screenRelPosX;
   _data.marioScreenOffset = _data.marioPosX - _data.screenPosX;
   _data.currentWorld = *_data.currentWorldRaw + 1;
   _data.currentStage = *_data.currentStageRaw + 1;
  }

  // Function to determine the current possible moves
  inline std::vector<std::string> getPossibleMoves() const override
  {
   // If Mario's state is not normal (!= 8), there's nothing to do except wait
   if (*_data.marioState != 8) return { "." };

   // If floating, B, D have no effect
   if (*_data.marioFloatingMode == 1) return { ".", "L", "R", "A", "LA", "RA", "LR", "LRA" };

   // On the floor, try all possible combinations, prioritize jumping and right direction
   return { ".", "R", "D", "LR", "A", "RA", "RB", "L", "LA", "LB", "LRA", "LRB" };
  }

  // Function to get magnet information
  inline gameRuleData_t getMagnetValues(const bool* rulesStatus) const
  {
   // Storage for magnet information
   gameRuleData_t magnets;

   magnets.marioScreenOffsetMagnet.intensity = 0.0f;
   magnets.marioScreenOffsetMagnet.max = 0.0f;

   magnets.marioHorizontalMagnet.intensity = 0.0f;
   magnets.marioHorizontalMagnet.max = 0.0f;

   magnets.marioVerticalMagnet.intensity = 0.0f;
   magnets.marioVerticalMagnet.max = 0.0f;

   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++) if (rulesStatus[ruleId] == true) magnets = _rules[ruleId]->_data;

   return magnets;
  }

  // Obtains the score of a given frame
  inline float getStateReward(const bool* rulesStatus) const override
  {
    // Getting rewards from rules
    float reward = 0.0;
    for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
     if (rulesStatus[ruleId] == true)
      reward += _rules[ruleId]->_reward;

    // Getting magnet values for the kid
    auto magnets = getMagnetValues(rulesStatus);

    // Evaluating mario / screen offset magnet value
    reward += magnets.marioScreenOffsetMagnet.intensity * std::min((float)_data.marioScreenOffset, magnets.marioScreenOffsetMagnet.max);

    // Evaluating mario magnet's reward on position X
    reward += magnets.marioHorizontalMagnet.intensity * std::min((float)_data.marioPosX, magnets.marioHorizontalMagnet.max);

    // Evaluating mario magnet's reward on position Y
    reward += magnets.marioVerticalMagnet.intensity * std::min((float)*_data.marioPosY, magnets.marioVerticalMagnet.max);

    // Returning reward
    return reward;
  }

  void printStateInfo(const bool* rulesStatus) const override
  {
    LOG("[Jaffar]  + Current World-Stage:    %1u-%1u\n", _data.currentWorld, _data.currentStage);
    LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
    LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
    LOG("[Jaffar]  + Time Left:              %1u%1u%1u\n", *_data.timeLeft100, *_data.timeLeft10, *_data.timeLeft1);
    LOG("[Jaffar]  + Mario Animation:        %02u\n", *_data.marioAnimation);
    LOG("[Jaffar]  + Mario State:            %02u\n", *_data.marioState);
    LOG("[Jaffar]  + Screen Pos X:           %04u (%02u * 256 = %04u + %02u)\n", _data.screenPosX, *_data.screenBasePosX, (uint16_t)*_data.screenBasePosX * 255, *_data.screenRelPosX);
    LOG("[Jaffar]  + Mario Pos X:            %04u (%02u * 256 = %04u + %02u)\n", _data.marioPosX, *_data.marioBasePosX, (uint16_t)*_data.marioBasePosX * 255, *_data.marioRelPosX);
    LOG("[Jaffar]  + Mario / Screen Offset:  %04d\n", _data.marioScreenOffset);
    LOG("[Jaffar]  + Mario Pos Y:            %02u\n", *_data.marioPosY);
    LOG("[Jaffar]  + Mario SubPixel X/Y:     %02u / %02u\n", *_data.marioSubpixelPosX, *_data.marioSubpixelPosY);
    LOG("[Jaffar]  + Mario Vel X:            %02d (Force: %02d, MaxL: %02d, MaxR: %02d)\n", *_data.marioVelX, *_data.marioXMoveForce, *_data.marioMaxVelLeft, *_data.marioMaxVelRight);
    LOG("[Jaffar]  + Mario Vel Y:            %02d (%02d)\n", *_data.marioVelY, *_data.marioFracVelY);
    LOG("[Jaffar]  + Mario Gravity:          %02u\n", *_data.marioGravity);
    LOG("[Jaffar]  + Mario Friction:         %02u\n", *_data.marioFriction);
    LOG("[Jaffar]  + Mario Moving Direction: %s\n", *_data.marioMovingDirection == 1 ? "Right" : "Left");
    LOG("[Jaffar]  + Mario Facing Direction: %s\n", *_data.marioFacingDirection == 1 ? "Right" : "Left");
    LOG("[Jaffar]  + Mario Floating Mode:    %02u\n", *_data.marioFloatingMode);
    LOG("[Jaffar]  + Mario Walking:          %02u %02u %02u\n", *_data.marioWalkingMode, *_data.marioWalkingDelay, *_data.marioWalkingFrame);
    LOG("[Jaffar]  + Player 1 Inputs:        %02u %02u %02u %02u\n", *_data.player1Input, *_data.player1Buttons, *_data.player1GamePad1, *_data.player1GamePad2);
    LOG("[Jaffar]  + Powerup Active:         %1u\n", *_data.powerUpActive);
    LOG("[Jaffar]  + Enemy Active:           %1u%1u%1u%1u%1u\n", *_data.enemy1Active, *_data.enemy2Active, *_data.enemy3Active, *_data.enemy4Active, *_data.enemy5Active);
    LOG("[Jaffar]  + Enemy State:            %02u %02u %02u %02u %02u\n", *_data.enemy1State, *_data.enemy2State, *_data.enemy3State, *_data.enemy4State, *_data.enemy5State);
    LOG("[Jaffar]  + Enemy Type:             %02u %02u %02u %02u %02u\n", *_data.enemy1Type, *_data.enemy2Type, *_data.enemy3Type, *_data.enemy4Type, *_data.enemy5Type);
    LOG("[Jaffar]  + Hit Detection Flags:    %02u %02u %02u\n", *_data.marioCollision, *_data.enemyCollision, *_data.hitDetectionFlag);
    LOG("[Jaffar]  + LevelEntry / GameMode:  %02u / %02u\n", *_data.levelEntryFlag, *_data.gameMode);
    LOG("[Jaffar]  + Warp Area Offset:       %04u\n", *_data.warpAreaOffset);
    LOG("[Jaffar]  + Timers:                 %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n", *_data.animationTimer, *_data.jumpSwimTimer, *_data.runningTimer, *_data.blockBounceTimer, *_data.sideCollisionTimer, *_data.jumpspringTimer, *_data.gameControlTimer, *_data.climbSideTimer, *_data.enemyFrameTimer, *_data.frenzyEnemyTimer, *_data.bowserFireTimer, *_data.stompTimer, *_data.airBubbleTimer, *_data.multiCoinBlockTimer, *_data.invincibleTimer, *_data.starTimer);

    LOG("[Jaffar]  + Rule Status: ");
    for (size_t i = 0; i < _ruleCount; i++)
    {
      if (i > 0 && i % 60 == 0) LOG("\n                         ");
      LOG("%d", rulesStatus[i] ? 1 : 0);
    }
    LOG("\n");

    auto magnets = getMagnetValues(rulesStatus);
    LOG("[Jaffar]  + Mario Screen Offset Magnet - Intensity: %.1f, Max: %f\n", magnets.marioScreenOffsetMagnet.intensity, magnets.marioScreenOffsetMagnet.max);
    LOG("[Jaffar]  + Mario Horizontal Magnet    - Intensity: %.1f, Max: %f\n", magnets.marioHorizontalMagnet.intensity, magnets.marioHorizontalMagnet.max);
    LOG("[Jaffar]  + Mario Vertical Magnet      - Intensity: %.1f, Max: %f\n", magnets.marioVerticalMagnet.intensity, magnets.marioVerticalMagnet.max);
  }
};
