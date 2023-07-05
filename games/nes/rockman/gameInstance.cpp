#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  gameTimer                   = (uint8_t*)   &_emu->_baseMem[0x0023];
  player1Input                = (uint8_t*)   &_emu->_baseMem[0x0014];
  player1Hold                 = (uint8_t*)   &_emu->_baseMem[0x0018];
  cameraPosX1                 = (uint8_t*)   &_emu->_baseMem[0x001B];
  cameraPosX2                 = (uint8_t*)   &_emu->_baseMem[0x001A];
  cameraState                 = (uint8_t*)   &_emu->_baseMem[0x001C];
  cameraPosY                  = (uint8_t*)   &_emu->_baseMem[0x001E];
  playerPosX1                 = (uint8_t*)   &_emu->_baseMem[0x0020];
  playerPosX2                 = (uint8_t*)   &_emu->_baseMem[0x0022];
  playerPosX3                 = (uint8_t*)   &_emu->_baseMem[0x0021];
  playerPosY1                 = (uint8_t*)   &_emu->_baseMem[0x0025];
  playerPosY2                 = (uint8_t*)   &_emu->_baseMem[0x0024];
  playerVelX                  = (uint8_t*)   &_emu->_baseMem[0x0098];
  playerHP                    = (uint8_t*)   &_emu->_baseMem[0x006C];
  groundState                 = (uint8_t*)   &_emu->_baseMem[0x002C];
  playerInvincibility         = (uint8_t*)   &_emu->_baseMem[0x0055];
  bulletOnScreen              = (uint8_t*)   &_emu->_baseMem[0x0060];
  sideObject                  = (uint8_t*)   &_emu->_baseMem[0x0094];
  bullet1PosX                 = (uint8_t*)   &_emu->_baseMem[0x0482];
  bullet2PosX                 = (uint8_t*)   &_emu->_baseMem[0x0483];
  bullet3PosX                 = (uint8_t*)   &_emu->_baseMem[0x0484];
  bullet1PosY                 = (uint8_t*)   &_emu->_baseMem[0x0602];
  bullet2PosY                 = (uint8_t*)   &_emu->_baseMem[0x0603];
  bullet3PosY                 = (uint8_t*)   &_emu->_baseMem[0x0604];
  lagFrameState               = (uint8_t*)   &_emu->_baseMem[0x001F];
  lagFrameType                = (uint8_t*)   &_emu->_baseMem[0x01FE];
  object0                     = (uint8_t*)   &_emu->_baseMem[0x0600];
  gameMode                    = (uint8_t*)   &_emu->_baseMem[0x0031];
  pauseMode                   = (uint8_t*)   &_emu->_baseMem[0x0041];
  playerDirection             = (uint8_t*)   &_emu->_baseMem[0x008D];
  specialFrame                = (uint8_t*)   &_emu->_baseMem[0x00F3];
  prevTimer                   = (uint8_t*)   &_emu->_highMem[0x0000];
  // Initialize derivative values
  updateDerivedValues();

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0)  hash.Update(currentStep % (timerTolerance+1));
//  if (timerTolerance > 0)  hash.Update(*gameTimer % (timerTolerance+1));

  hash.Update(*prevTimer);
  hash.Update(*gameTimer           );
//  hash.Update(*player1Input        );
//  hash.Update(*player1Hold         );
  hash.Update(*cameraPosX1         );
  hash.Update(*cameraPosX2         );
  hash.Update(*cameraState         );
  hash.Update(*cameraPosY          );
  hash.Update(*playerPosX1         );
  hash.Update(*playerPosX2         );
  hash.Update(*playerPosX3         );
  hash.Update(*playerPosY1         );
  hash.Update(*playerPosY2         );
  hash.Update(*playerHP         );
  hash.Update(*playerVelX          );
  hash.Update(*groundState         );
//  hash.Update(*playerInvincibility % 4);
  hash.Update(*bulletOnScreen      );
  hash.Update(*sideObject          );
  hash.Update(*bullet1PosX         % 2);
  hash.Update(*bullet2PosX         % 2);
  hash.Update(*bullet3PosX         % 2);
  hash.Update(*bullet1PosY         == 0xF8);
  hash.Update(*bullet2PosY         == 0xF8);
  hash.Update(*bullet3PosY         == 0xF8);
  hash.Update(*lagFrameState       );
  hash.Update(*lagFrameType        );
  hash.Update(*object0             );
  hash.Update(*gameMode            );
  hash.Update(*pauseMode);
  hash.Update(*playerDirection);
  hash.Update(*specialFrame);

  hash.Update(&_emu->_baseMem[0x0000], 0x012);

  hash.Update(&_emu->_baseMem[0x0400], 0x040);
  hash.Update(&_emu->_baseMem[0x0440], 0x040);
  hash.Update(&_emu->_baseMem[0x0480], 0x080);
  hash.Update(&_emu->_baseMem[0x0500], 0x100);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 playerPosX = (float)*cameraPosX1 * 256.0f + (float)*cameraPosX2;
 playerPosY = (float)*playerPosY1 + (float)*playerPosY2 / 256.0;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 if (*gameTimer == 85) return { "DLS" };
 if (*lagFrameState != 0) return { "." };
 if (*pauseMode != 0) return {".", "s"};

 return { ".", "s", "A", "B", "R", "L", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA" };
}


std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 *prevTimer = *gameTimer;
 _emu->advanceState(move); moves.push_back(move);
 updateDerivedValues();

 return moves;
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

  // Container for bounded value and difference with center
  float diff = 0.0;

  // Evaluating samus magnet's reward on position X and Y
  diff = std::abs(magnets.playerHorizontalMagnet.center - playerPosX);
  float weightedDiffX = magnets.playerHorizontalMagnet.intensity * diff;
  reward -= weightedDiffX;

  diff = std::abs(magnets.playerVerticalMagnet.center - playerPosY);
  float weightedDiffY = magnets.playerVerticalMagnet.intensity * diff;
  reward -= weightedDiffY;

  diff = std::abs(magnets.gameTimerMagnet.center - (float)*gameTimer);
  float weightedDiffTimer = magnets.gameTimerMagnet.intensity * diff;
  reward -= weightedDiffTimer;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second );
  LOG("[Jaffar]  + Game Timer:             %02u (Prev: %02u)\n", *gameTimer, *prevTimer);
  LOG("[Jaffar]  + Pause Mode:             %02u\n", *pauseMode);
  LOG("[Jaffar]  + Game Mode:              %02u\n", *gameMode);
  LOG("[Jaffar]  + Player HP:              %02u\n", *playerHP);
  LOG("[Jaffar]  + Player Direction:       %02u\n", *playerDirection);
  LOG("[Jaffar]  + Player Pos X:           %.2f (%02u %02u)\n", playerPosX, *cameraPosX1, *cameraPosX2);
  LOG("[Jaffar]  + Player Pos Y:           %.2f (%02u %02u)\n", playerPosY, *playerPosY1, *playerPosY2);
  LOG("[Jaffar]  + Bullet Pos X:           %02u %02u %02u\n", *bullet1PosX, *bullet2PosX, *bullet3PosX);
  LOG("[Jaffar]  + Bullet Pos Y:           %02u %02u %02u\n", *bullet1PosY, *bullet2PosY, *bullet3PosY);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.playerHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Player Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.playerHorizontalMagnet.intensity, magnets.playerHorizontalMagnet.center);
  if (std::abs(magnets.playerVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Player Vertical Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.playerVerticalMagnet.intensity, magnets.playerVerticalMagnet.center);
  if (std::abs(magnets.gameTimerMagnet.intensity) > 0.0f)            LOG("[Jaffar]  + Game Timer Magnet               - Intensity: %.5f, Center: %3.3f\n", magnets.gameTimerMagnet.intensity, magnets.gameTimerMagnet.center);
}
