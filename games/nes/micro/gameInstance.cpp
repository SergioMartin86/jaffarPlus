#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  globalTimer                       = (uint16_t*)  &_emu->_highMem[0x0000];
  frameType                         = (uint8_t*)   &_emu->_baseMem[0x007C];
  cameraPosY2                       = (uint8_t*)   &_emu->_baseMem[0x00D6];
  player1PosX1                      = (uint8_t*)   &_emu->_baseMem[0x03DE];
  player1PosX2                      = (uint8_t*)   &_emu->_baseMem[0x03DA];
  player1PosY1                      = (uint8_t*)   &_emu->_baseMem[0x03EA];
  player1PosY2                      = (uint8_t*)   &_emu->_baseMem[0x03E6];
  player1Accel                      = (int8_t*)    &_emu->_baseMem[0x0386];
  player1Angle1                     = (uint8_t*)   &_emu->_baseMem[0x04B2];
  player1Angle2                     = (uint8_t*)   &_emu->_baseMem[0x040A];
  player1LapsRemaining              = (uint8_t*)   &_emu->_baseMem[0x04B6];
  player1Checkpoint                 = (uint8_t*)   &_emu->_baseMem[0x04CE];
  player1Input1                     = (uint8_t*)   &_emu->_baseMem[0x009B];
  player1Input2                     = (uint8_t*)   &_emu->_baseMem[0x00CF];
  player1Input3                     = (uint8_t*)   &_emu->_baseMem[0x0352];
  preRaceTimer                      = (uint8_t*)   &_emu->_baseMem[0x00DD];

  activeFrame1                      = (uint8_t*)   &_emu->_baseMem[0x00B0];
  activeFrame2                      = (uint8_t*)   &_emu->_baseMem[0x00B2];
  activeFrame3                      = (uint8_t*)   &_emu->_baseMem[0x00B4];
  activeFrame4                      = (uint8_t*)   &_emu->_baseMem[0x00B6];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");


  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));

  hash.Update(*cameraPosY2);
  hash.Update(*frameType);
  hash.Update(*preRaceTimer);
  hash.Update(*player1PosX1);
  hash.Update(*player1PosX2);
  hash.Update(*player1PosY1);
  hash.Update(*player1PosY2);
  hash.Update(*player1Accel);
  hash.Update(*player1Angle1);
  hash.Update(*player1Angle2);
  hash.Update(*player1LapsRemaining);
  hash.Update(*player1Checkpoint);

  hash.Update(*activeFrame1);
  hash.Update(*activeFrame2);
  hash.Update(*activeFrame3);
  hash.Update(*activeFrame4);

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
 if (*globalTimer == 65535) *globalTimer = 0;
 else *globalTimer = *globalTimer+1;
 player1PosX = (uint16_t)*player1PosX1 * 256 + (uint16_t)*player1PosX2;
 player1PosY = (uint16_t)*player1PosY1 * 256 + (uint16_t)*player1PosY2;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 if (*frameType == 0x00) return { ".", "A", "B" };
 if (*frameType == 0x01) return { ".", "A", "B", "R", "L", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA" };
 return { "." };
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

  // Evaluating player health  magnet
  reward += magnets.playerCurrentLapMagnet * (float)(*player1LapsRemaining);

  // Evaluating player health  magnet
  reward += magnets.playerLapProgressMagnet * (float)(*player1Checkpoint);

  // Evaluating player health  magnet
  reward += magnets.playerAccelMagnet * std::abs((float)*player1Accel);

  float distX = std::abs(magnets.pointMagnet.x - player1PosX);
  float distY = std::abs(magnets.pointMagnet.y - player1PosY);
  float distanceToPoint = sqrt(distX*distX + distY*distY);
  reward += magnets.pointMagnet.intensity * -distanceToPoint;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);

 LOG("[Jaffar]  + Global Timer:                       %03u\n", *globalTimer);
 LOG("[Jaffar]  + Pre Race Timer:                     %03u\n", *preRaceTimer);

 LOG("[Jaffar]  + Active Frames:                      %03u %03u %03u %03u\n", *activeFrame1, *activeFrame2, *activeFrame3, *activeFrame4 );

 LOG("[Jaffar]  + Last Input / Frame                  %03u\n",  *player1Input1);
 LOG("[Jaffar]  + Laps Remaining:                     %1u\n", *player1LapsRemaining);
 LOG("[Jaffar]  + Checkpoint Id:                      %03u\n", *player1Checkpoint);

 LOG("[Jaffar]  + Player Accel:                       %03d\n", *player1Accel);
 LOG("[Jaffar]  + Player Angle:                       %03u %03u\n", *player1Angle1, *player1Angle2);

 LOG("[Jaffar]  + Player Pos X:                       %05u\n", player1PosX);
 LOG("[Jaffar]  + Player Pos Y:                       %05u\n", player1PosY);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 float distX = std::abs(magnets.pointMagnet.x - player1PosX);
 float distY = std::abs(magnets.pointMagnet.y - player1PosY);
 float distanceToPoint = sqrt(distX*distX + distY*distY);

 if (std::abs(magnets.pointMagnet.intensity) > 0.0f)                 LOG("[Jaffar]  + Point Magnet                     - Intensity: %.5f, X: %3.3f, Y: %3.3f, Dist: %3.3f\n", magnets.pointMagnet.intensity, magnets.pointMagnet.x, magnets.pointMagnet.y, distanceToPoint);
 if (std::abs(magnets.playerCurrentLapMagnet) > 0.0f)                LOG("[Jaffar]  + Player Current Lap Magnet        - Intensity: %.5f\n", magnets.playerCurrentLapMagnet);
 if (std::abs(magnets.playerLapProgressMagnet) > 0.0f)               LOG("[Jaffar]  + Player Lap Progress Magnet       - Intensity: %.5f\n", magnets.playerLapProgressMagnet);
 if (std::abs(magnets.playerAccelMagnet) > 0.0f)                     LOG("[Jaffar]  + Player Accel Magnet              - Intensity: %.5f\n", magnets.playerAccelMagnet);
}

