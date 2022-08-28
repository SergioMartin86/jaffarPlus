#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting relevant SMB pointers

  // Thanks to https://datacrystal.romhacking.net/wiki/Super_Mario_Bros.:RAM_map and https://tasvideos.org/GameResources/NES/SuperMarioBros for helping me find some of these items
  screenScroll         = (uint16_t*) &_emu->_nes->ram[0x071B];
  marioAnimation       = (uint8_t*)  &_emu->_nes->ram[0x0001];
  marioState           = (uint8_t*)  &_emu->_nes->ram[0x000E];

  marioBasePosX        = (uint8_t*)  &_emu->_nes->ram[0x006D];
  marioRelPosX         = (uint8_t*)  &_emu->_nes->ram[0x0086];
  marioSubpixelPosX    = (uint8_t*)  &_emu->_nes->ram[0x0400];

  marioPosY            = (uint8_t*)  &_emu->_nes->ram[0x00CE];
  marioSubpixelPosY    = (uint8_t*)  &_emu->_nes->ram[0x0416];

  marioMovingDirection = (uint8_t*)  &_emu->_nes->ram[0x0045];
  marioFacingDirection = (uint8_t*)  &_emu->_nes->ram[0x0033];
  marioFloatingMode    = (uint8_t*)  &_emu->_nes->ram[0x001D];
  marioWalkingMode     = (uint8_t*)  &_emu->_nes->ram[0x0702];
  marioWalkingDelay    = (uint8_t*)  &_emu->_nes->ram[0x070C];
  marioWalkingFrame    = (uint8_t*)  &_emu->_nes->ram[0x070D];
  marioMaxVelLeft      = (int8_t*)   &_emu->_nes->ram[0x0450];
  marioMaxVelRight     = (int8_t*)   &_emu->_nes->ram[0x0456];
  marioVelX            = (int8_t*)   &_emu->_nes->ram[0x0057];
  marioXMoveForce      = (int8_t*)   &_emu->_nes->ram[0x0705];
  marioVelY            = (int8_t*)   &_emu->_nes->ram[0x009F];
  marioFracVelY        = (int8_t*)   &_emu->_nes->ram[0x0433];
  marioGravity         = (uint8_t*)  &_emu->_nes->ram[0x0709];
  marioFriction        = (uint8_t*)  &_emu->_nes->ram[0x0701];
  timeLeft100          = (uint8_t*)  &_emu->_nes->ram[0x07F8];
  timeLeft10           = (uint8_t*)  &_emu->_nes->ram[0x07F9];
  timeLeft1            = (uint8_t*)  &_emu->_nes->ram[0x07FA];

  screenBasePosX       = (uint8_t*)  &_emu->_nes->ram[0x071A];
  screenRelPosX        = (uint8_t*)  &_emu->_nes->ram[0x071C];

  currentWorldRaw      = (uint8_t*)  &_emu->_nes->ram[0x075F];
  currentStageRaw      = (uint8_t*)  &_emu->_nes->ram[0x075C];
  levelEntryFlag       = (uint8_t*)  &_emu->_nes->ram[0x0752];
  gameMode             = (uint8_t*)  &_emu->_nes->ram[0x0770];

  enemy1Active         = (uint8_t*)  &_emu->_nes->ram[0x000F];
  enemy2Active         = (uint8_t*)  &_emu->_nes->ram[0x0010];
  enemy3Active         = (uint8_t*)  &_emu->_nes->ram[0x0011];
  enemy4Active         = (uint8_t*)  &_emu->_nes->ram[0x0012];
  enemy5Active         = (uint8_t*)  &_emu->_nes->ram[0x0013];

  enemy1State          = (uint8_t*)  &_emu->_nes->ram[0x001E];
  enemy2State          = (uint8_t*)  &_emu->_nes->ram[0x001F];
  enemy3State          = (uint8_t*)  &_emu->_nes->ram[0x0020];
  enemy4State          = (uint8_t*)  &_emu->_nes->ram[0x0021];
  enemy5State          = (uint8_t*)  &_emu->_nes->ram[0x0022];

  enemy1Type           = (uint8_t*)  &_emu->_nes->ram[0x0016];
  enemy2Type           = (uint8_t*)  &_emu->_nes->ram[0x0017];
  enemy3Type           = (uint8_t*)  &_emu->_nes->ram[0x0018];
  enemy4Type           = (uint8_t*)  &_emu->_nes->ram[0x0019];
  enemy5Type           = (uint8_t*)  &_emu->_nes->ram[0x001A];

  marioCollision       = (uint8_t*)  &_emu->_nes->ram[0x0490];
  enemyCollision       = (uint8_t*)  &_emu->_nes->ram[0x0491];
  hitDetectionFlag     = (uint8_t*)  &_emu->_nes->ram[0x0722];

  powerUpActive        = (uint8_t*)  &_emu->_nes->ram[0x0014];

  animationTimer       = (uint8_t*)  &_emu->_nes->ram[0x0781];
  jumpSwimTimer        = (uint8_t*)  &_emu->_nes->ram[0x0782];
  runningTimer         = (uint8_t*)  &_emu->_nes->ram[0x0783];
  blockBounceTimer     = (uint8_t*)  &_emu->_nes->ram[0x0784];
  sideCollisionTimer   = (uint8_t*)  &_emu->_nes->ram[0x0785];
  jumpspringTimer      = (uint8_t*)  &_emu->_nes->ram[0x0786];
  gameControlTimer     = (uint8_t*)  &_emu->_nes->ram[0x0787];
  climbSideTimer       = (uint8_t*)  &_emu->_nes->ram[0x0789];
  enemyFrameTimer      = (uint8_t*)  &_emu->_nes->ram[0x078A];
  frenzyEnemyTimer     = (uint8_t*)  &_emu->_nes->ram[0x078F];
  bowserFireTimer      = (uint8_t*)  &_emu->_nes->ram[0x0790];
  stompTimer           = (uint8_t*)  &_emu->_nes->ram[0x0791];
  airBubbleTimer       = (uint8_t*)  &_emu->_nes->ram[0x0792];
  fallPitTimer         = (uint8_t*)  &_emu->_nes->ram[0x0795];
  multiCoinBlockTimer  = (uint8_t*)  &_emu->_nes->ram[0x079D];
  invincibleTimer      = (uint8_t*)  &_emu->_nes->ram[0x079E];
  starTimer            = (uint8_t*)  &_emu->_nes->ram[0x079F];

  player1Input         = (uint8_t*)  &_emu->_nes->ram[0x06FC];
  player1Buttons       = (uint8_t*)  &_emu->_nes->ram[0x074A];
  player1GamePad1      = (uint8_t*)  &_emu->_nes->ram[0x000A];
  player1GamePad2      = (uint8_t*)  &_emu->_nes->ram[0x000D];

  warpAreaOffset       = (uint16_t*) &_emu->_nes->ram[0x0750];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();

  for (size_t i = 0; i < 100; i++)
  {
   printf("STEP: %lu\n", i);
   fflush(stdout);
   _emu->advanceState(0);
   printStateInfo(NULL);
  }
  exit(0);
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  // Adding fixed hash elements
  hash.Update(*screenScroll);
  hash.Update(*marioAnimation);
  hash.Update(*marioState);

  hash.Update(*marioBasePosX);
  hash.Update(*marioRelPosX);
//    hash.Update(*marioSubpixelPosX);

  hash.Update(*marioPosY);
//    hash.Update(*marioSubpixelPosY);

  hash.Update(*marioXMoveForce);
  hash.Update(*marioFacingDirection);
  hash.Update(*marioMovingDirection);
  hash.Update(*marioFloatingMode);
  hash.Update(*marioWalkingMode);
//    hash.Update(*marioWalkingDelay);
//    hash.Update(*marioWalkingFrame);
//    hash.Update(*marioMaxVelLeft);
//    hash.Update(*marioMaxVelRight);
  hash.Update(*marioVelX);
  hash.Update(*marioVelY);
  hash.Update(*marioFracVelY);
//    hash.Update(*marioGravity);
//    hash.Update(*marioFriction);

  hash.Update(*screenBasePosX);
  hash.Update(*screenRelPosX);

  hash.Update(*currentWorldRaw);
  hash.Update(*currentStageRaw);
  hash.Update(*levelEntryFlag);
  hash.Update(*gameMode);

  // hash.Update(*enemy1Active);
  // hash.Update(*enemy2Active);
  // hash.Update(*enemy3Active);
  // hash.Update(*enemy4Active);
  // hash.Update(*enemy5Active);

  // hash.Update(*enemy1State);
  // hash.Update(*enemy2State);
  // hash.Update(*enemy3State);
  // hash.Update(*enemy4State);
  // hash.Update(*enemy5State);

  hash.Update(*enemy1Type);
  hash.Update(*enemy2Type);
  hash.Update(*enemy3Type);
  hash.Update(*enemy4Type);
  hash.Update(*enemy5Type);

  // hash.Update(*marioCollision);
  // hash.Update(*enemyCollision);
  hash.Update(*hitDetectionFlag);

  // To Reduce timer pressure on hash, have 0, 1, and >1 as possibilities only
  hash.Update(*animationTimer < 2 ? *animationTimer : (uint8_t)2);
  hash.Update(*jumpSwimTimer < 2 ? *jumpSwimTimer : (uint8_t)2);
  hash.Update(*runningTimer < 2 ? *runningTimer : (uint8_t)2);
  hash.Update(*blockBounceTimer < 2 ? *blockBounceTimer : (uint8_t)2);
  // hash.Update(*sideCollisionTimer);
  // hash.Update(*jumpspringTimer);
  // hash.Update(*gameControlTimer);
  // hash.Update(*climbSideTimer);
  // hash.Update(*enemyFrameTimer);
  // hash.Update(*frenzyEnemyTimer);
  // hash.Update(*bowserFireTimer);
  // hash.Update(*stompTimer);
  // hash.Update(*airBubbleTimer);
  // hash.Update(*fallPitTimer);
  // hash.Update(*multiCoinBlockTimer);
  // hash.Update(*invincibleTimer);
  // hash.Update(*starTimer);
  // hash.Update(*powerUpActive);


  hash.Update(*warpAreaOffset);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 // Recalculating derived values
  marioPosX = (uint16_t)*marioBasePosX * 256 + (uint16_t)*marioRelPosX;
  screenPosX = (uint16_t)*screenBasePosX * 256 + (uint16_t)*screenRelPosX;
  marioScreenOffset = marioPosX - screenPosX;
  currentWorld = *currentWorldRaw + 1;
  currentStage = *currentStageRaw + 1;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 // If Mario's state is not normal (!= 8), there's nothing to do except wait
  if (*marioState != 8) return { "." };

  // If floating, B, D have no effect
  if (*marioFloatingMode == 1) return { ".", "L", "R", "A", "LA", "RA", "LR", "LRA" };

  // On the floor, try all possible combinations, prioritize jumping and right direction
  return { ".", "R", "D", "LR", "A", "RA", "RB", "L", "LA", "LB", "LRA", "LRB" };
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
  magnetSet_t magnets;

  magnets.marioScreenOffsetMagnet.intensity = 0.0f;
  magnets.marioScreenOffsetMagnet.max = 0.0f;

  magnets.marioHorizontalMagnet.intensity = 0.0f;
  magnets.marioHorizontalMagnet.max = 0.0f;

  magnets.marioVerticalMagnet.intensity = 0.0f;
  magnets.marioVerticalMagnet.max = 0.0f;

  for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++) if (rulesStatus[ruleId] == true) magnets = _rules[ruleId]->_magnets;

  return magnets;
}

// Obtains the score of a given frame
float GameInstance::getStateReward(const bool* rulesStatus) const
{
 // Getting rewards from rules
  float reward = 0.0;
  for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
   if (rulesStatus[ruleId] == true)
    reward += _rules[ruleId]->_reward;

  // Getting magnet values for the kid
  auto magnets = getMagnetValues(rulesStatus);

  // Container for bounding calculations
  float boundedValue = 0.0;

  // Evaluating mario / screen offset magnet value
  boundedValue = (float)marioScreenOffset;
  boundedValue = std::min(boundedValue, magnets.marioScreenOffsetMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marioScreenOffsetMagnet.min);
  reward += magnets.marioScreenOffsetMagnet.intensity * boundedValue;

  // Evaluating mario magnet's reward on position X
  boundedValue = (float)marioPosX;
  boundedValue = std::min(boundedValue, magnets.marioHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marioHorizontalMagnet.min);
  reward += magnets.marioHorizontalMagnet.intensity * boundedValue;

  // Evaluating mario magnet's reward on position Y
  boundedValue = (float)*marioPosY;
  boundedValue = std::min(boundedValue, magnets.marioVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marioVerticalMagnet.min);
  reward += magnets.marioVerticalMagnet.intensity * boundedValue;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Current World-Stage:    %1u-%1u\n", currentWorld, currentStage);
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
 LOG("[Jaffar]  + Time Left:              %1u%1u%1u\n", *timeLeft100, *timeLeft10, *timeLeft1);
 LOG("[Jaffar]  + Mario Animation:        %02u\n", *marioAnimation);
 LOG("[Jaffar]  + Mario State:            %02u\n", *marioState);
 LOG("[Jaffar]  + Screen Pos X:           %04u (%02u * 256 = %04u + %02u)\n", screenPosX, *screenBasePosX, (uint16_t)*screenBasePosX * 255, *screenRelPosX);
 LOG("[Jaffar]  + Mario Pos X:            %04u (%02u * 256 = %04u + %02u)\n", marioPosX, *marioBasePosX, (uint16_t)*marioBasePosX * 255, *marioRelPosX);
 LOG("[Jaffar]  + Mario / Screen Offset:  %04d\n", marioScreenOffset);
 LOG("[Jaffar]  + Mario Pos Y:            %02u\n", *marioPosY);
 LOG("[Jaffar]  + Mario SubPixel X/Y:     %02u / %02u\n", *marioSubpixelPosX, *marioSubpixelPosY);
 LOG("[Jaffar]  + Mario Vel X:            %02d (Force: %02d, MaxL: %02d, MaxR: %02d)\n", *marioVelX, *marioXMoveForce, *marioMaxVelLeft, *marioMaxVelRight);
 LOG("[Jaffar]  + Mario Vel Y:            %02d (%02d)\n", *marioVelY, *marioFracVelY);
 LOG("[Jaffar]  + Mario Gravity:          %02u\n", *marioGravity);
 LOG("[Jaffar]  + Mario Friction:         %02u\n", *marioFriction);
 LOG("[Jaffar]  + Mario Moving Direction: %s\n", *marioMovingDirection == 1 ? "Right" : "Left");
 LOG("[Jaffar]  + Mario Facing Direction: %s\n", *marioFacingDirection == 1 ? "Right" : "Left");
 LOG("[Jaffar]  + Mario Floating Mode:    %02u\n", *marioFloatingMode);
 LOG("[Jaffar]  + Mario Walking:          %02u %02u %02u\n", *marioWalkingMode, *marioWalkingDelay, *marioWalkingFrame);
 LOG("[Jaffar]  + Player 1 Inputs:        %02u %02u %02u %02u\n", *player1Input, *player1Buttons, *player1GamePad1, *player1GamePad2);
 LOG("[Jaffar]  + Powerup Active:         %1u\n", *powerUpActive);
 LOG("[Jaffar]  + Enemy Active:           %1u%1u%1u%1u%1u\n", *enemy1Active, *enemy2Active, *enemy3Active, *enemy4Active, *enemy5Active);
 LOG("[Jaffar]  + Enemy State:            %02u %02u %02u %02u %02u\n", *enemy1State, *enemy2State, *enemy3State, *enemy4State, *enemy5State);
 LOG("[Jaffar]  + Enemy Type:             %02u %02u %02u %02u %02u\n", *enemy1Type, *enemy2Type, *enemy3Type, *enemy4Type, *enemy5Type);
 LOG("[Jaffar]  + Hit Detection Flags:    %02u %02u %02u\n", *marioCollision, *enemyCollision, *hitDetectionFlag);
 LOG("[Jaffar]  + LevelEntry / GameMode:  %02u / %02u\n", *levelEntryFlag, *gameMode);
 LOG("[Jaffar]  + Warp Area Offset:       %04u\n", *warpAreaOffset);
 LOG("[Jaffar]  + Timers:                 %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n", *animationTimer, *jumpSwimTimer, *runningTimer, *blockBounceTimer, *sideCollisionTimer, *jumpspringTimer, *gameControlTimer, *climbSideTimer, *enemyFrameTimer, *frenzyEnemyTimer, *bowserFireTimer, *stompTimer, *airBubbleTimer, *multiCoinBlockTimer, *invincibleTimer, *starTimer);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 LOG("[Jaffar]  + Mario Screen Offset Magnet - Intensity: %.1f, Max: %f\n", magnets.marioScreenOffsetMagnet.intensity, magnets.marioScreenOffsetMagnet.max);
 LOG("[Jaffar]  + Mario Horizontal Magnet    - Intensity: %.1f, Min: %f, Max: %f\n", magnets.marioHorizontalMagnet.intensity, magnets.marioHorizontalMagnet.min, magnets.marioHorizontalMagnet.max);
 LOG("[Jaffar]  + Mario Vertical Magnet      - Intensity: %.1f, Min: %f, Max: %f\n", magnets.marioVerticalMagnet.intensity, magnets.marioVerticalMagnet.min, magnets.marioVerticalMagnet.max);
}

