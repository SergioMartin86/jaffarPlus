#include "gameInstance.hpp"
#include "gameRule.hpp"

#define BLOCK_SECTION_START 0x03B0
#define BLOCK_SECTION_END 0x04B0

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  currentLevel              = (uint8_t*)   &_emu->_baseMem[0x001A];
  gameMode                  = (uint8_t*)   &_emu->_baseMem[0x000A];
  frameType                 = (uint8_t*)   &_emu->_baseMem[0x0000];
  remainingBlocks           = (uint8_t*)   &_emu->_baseMem[0x000F];
  ball1X                    = (uint8_t*)   &_emu->_baseMem[0x0038];
  ball1Y                    = (uint8_t*)   &_emu->_baseMem[0x0037];
  ball2X                    = (uint8_t*)   &_emu->_baseMem[0x0052];
  ball2Y                    = (uint8_t*)   &_emu->_baseMem[0x0051];
  ball3X                    = (uint8_t*)   &_emu->_baseMem[0x006C];
  ball3Y                    = (uint8_t*)   &_emu->_baseMem[0x006B];
  paddleState               = (uint8_t*)   &_emu->_baseMem[0x0081];
  fallingPowerUpType        = (uint8_t*)   &_emu->_baseMem[0x008C];
  fallingPowerUpPosY        = (uint8_t*)   &_emu->_baseMem[0x0091];
  paddlePosX                = (uint8_t*)   &_emu->_baseMem[0x011A];
  paddlePowerUp1            = (uint8_t*)   &_emu->_baseMem[0x0129];
  paddlePowerUp2            = (uint8_t*)   &_emu->_baseMem[0x012D];
  warpIsActive              = (uint8_t*)   &_emu->_baseMem[0x0124];

  enemy1PosX                = (uint8_t*)   &_emu->_baseMem[0x00B7];
  enemy2PosX                = (uint8_t*)   &_emu->_baseMem[0x00B8];
  enemy3PosX                = (uint8_t*)   &_emu->_baseMem[0x00B9];

  enemy1PosY                = (uint8_t*)   &_emu->_baseMem[0x00AE];
  enemy2PosY                = (uint8_t*)   &_emu->_baseMem[0x00AF];
  enemy3PosY                = (uint8_t*)   &_emu->_baseMem[0x00B0];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  #ifndef _JAFFAR_PLAY
  _emu->_nes->_doRendering = false;
  #endif

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));
  if (*remainingBlocks == 0) hash.Update(currentStep);

  hash.Update(*gameMode);
  hash.Update(*currentLevel);
  hash.Update(*frameType);
  hash.Update(*remainingBlocks);
  hash.Update(*ball1X);
  hash.Update(*ball1Y);
  hash.Update(*ball2X);
  hash.Update(*ball2Y);
  hash.Update(*ball3X);
  hash.Update(*ball3Y);
  hash.Update(*paddleState);
  hash.Update(*fallingPowerUpType);
  hash.Update(*fallingPowerUpPosY);
  hash.Update(*paddlePosX);
  hash.Update(*paddlePowerUp1);
  hash.Update(*paddlePowerUp2);
  hash.Update(*warpIsActive);

  hash.Update(*enemy1PosX);
  hash.Update(*enemy2PosX);
  hash.Update(*enemy3PosX);

  hash.Update(*enemy1PosY);
  hash.Update(*enemy2PosY);
  hash.Update(*enemy3PosY);

  uint16_t blockCount = BLOCK_SECTION_END - BLOCK_SECTION_START;
  hash.Update(&_emu->_baseMem[BLOCK_SECTION_START], blockCount);

  hash.Update(&_emu->_baseMem[0x100], 0x50);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 _emu->advanceState(move); moves.push_back(move);
 updateDerivedValues();
 return moves;
}

void GameInstance::updateDerivedValues()
{
 lowestBallPosY = *ball1Y; lowestBallPosX = *ball1X;
 if (*ball2Y != 240 && *ball2Y > lowestBallPosY) { lowestBallPosY = *ball2Y; lowestBallPosX = *ball2X; }
 if (*ball3Y != 240 && *ball3Y > lowestBallPosY) { lowestBallPosY = *ball3Y; lowestBallPosX = *ball3X; }

 horizontalDistanceToLowestBall = std::abs((float)*paddlePosX - (float)lowestBallPosX);

 ballHitsRemaining = 0;
 uint16_t blockCount = BLOCK_SECTION_END - BLOCK_SECTION_START;
 for (size_t i = 0; i < blockCount; i++)
 {
  switch(_emu->_baseMem[BLOCK_SECTION_START + i])
  {
    case 0xE5: ballHitsRemaining += 5; break;
    case 0xE4: ballHitsRemaining += 4; break;
    case 0xE3: ballHitsRemaining += 3; break;
    case 0xE2: ballHitsRemaining += 2; break;
    case 0xF0: break; // Gold Brick
    case 0x00: break;
    default: ballHitsRemaining++;
  }
 }

}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
  if (*ball1Y == 204) return { ".", "A", "L", "R" };
  return { "L", "R" };
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
    magnets = _rules[ruleId]->_magnets;

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

  reward += magnets.remainingBlocksMagnet * (float)(*remainingBlocks);
  reward += magnets.remainingHitsMagnet * (float)ballHitsRemaining;
  reward += magnets.ball1PosYMagnet * (float)(*ball1Y);
  reward += magnets.fallingPowerUpPosYMagnet * (float)(*fallingPowerUpPosY);

  float horizontalDistanceToLowestBall = std::abs((float)*paddlePosX - (float)lowestBallPosX);
  reward += magnets.horizontalDistaceToLowestBallMagnet * horizontalDistanceToLowestBall;
  reward += magnets.lowestBallPosYMagnet * (float)lowestBallPosY;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);

 LOG("[Jaffar]  + Current Level:                      %03u\n", *currentLevel);
 LOG("[Jaffar]  + Game Mode:                          %03u\n", *gameMode);
 LOG("[Jaffar]  + Frame Type:                         %03u\n", *frameType);
 LOG("[Jaffar]  + Remaining Blocks:                   %03u\n", *remainingBlocks);
 LOG("[Jaffar]  + Remaining Ball Hits:                %03u\n", ballHitsRemaining);
 LOG("[Jaffar]  + Balls Pos X:                        %03u %03u %03u (Lowest: %03u)\n", *ball1X, *ball2X, *ball3X, lowestBallPosX);
 LOG("[Jaffar]  + Balls Pos Y:                        %03u %03u %03u (Lowest: %03u)\n", *ball1Y, *ball2Y, *ball3Y, lowestBallPosY);
 LOG("[Jaffar]  + Distance to Lowest Ball:            %.3f\n", horizontalDistanceToLowestBall);
 LOG("[Jaffar]  + Paddle State:                       %03u\n", *paddleState);
 LOG("[Jaffar]  + Paddle Pos X:                       %03u\n", *paddlePosX);
 LOG("[Jaffar]  + Paddle Power Up:                    %03u %03u\n", *paddlePowerUp1, *paddlePowerUp2);
 LOG("[Jaffar]  + Falling Power Up Type:              %03u\n", *fallingPowerUpType);
 LOG("[Jaffar]  + Falling Power Up Pos Y:             %03u\n", *fallingPowerUpPosY);
 LOG("[Jaffar]  + Warp Is Active:                     %03u\n", *warpIsActive);
 LOG("[Jaffar]  + Enemy Pos X:                        %03u %03u %03u\n", *enemy1PosX, *enemy2PosX, *enemy3PosX);
 LOG("[Jaffar]  + Enemy Pos Y:                        %03u %03u %03u\n", *enemy1PosY, *enemy2PosY, *enemy3PosY);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.remainingBlocksMagnet) > 0.0f)               LOG("[Jaffar]  + Remaining Blocks Magnet                    - Intensity: %.5f\n", magnets.remainingBlocksMagnet);
 if (std::abs(magnets.remainingHitsMagnet) > 0.0f)                 LOG("[Jaffar]  + Remaining Hits Magnet                      - Intensity: %.5f\n", magnets.remainingHitsMagnet);
 if (std::abs(magnets.ball1PosYMagnet) > 0.0f)                     LOG("[Jaffar]  + Ball 1 Pos Y Magnet                        - Intensity: %.5f\n", magnets.ball1PosYMagnet);
 if (std::abs(magnets.fallingPowerUpPosYMagnet) > 0.0f)            LOG("[Jaffar]  + Falling Power Up Pos Y Magnet              - Intensity: %.5f\n", magnets.fallingPowerUpPosYMagnet);
 if (std::abs(magnets.horizontalDistaceToLowestBallMagnet) > 0.0f) LOG("[Jaffar]  + Horizontal Distance To Lowest Ball Magnet  - Intensity: %.5f\n", magnets.horizontalDistaceToLowestBallMagnet);
 if (std::abs(magnets.lowestBallPosYMagnet) > 0.0f)                LOG("[Jaffar]  + Lowest Ball Pos Y Magnet                   - Intensity: %.5f\n", magnets.lowestBallPosYMagnet);

}

