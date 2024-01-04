#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  globalTimer                       = (uint16_t*)  &_emu->_highMem[0x0000];
  frameType                         = (uint8_t*)   &_emu->_baseMem[0x007C];
  lagFrame                          = (uint8_t*)   &_emu->_baseMem[0x01F8];
  cameraPosX1                       = (uint8_t*)   &_emu->_baseMem[0x00D5];
  cameraPosX2                       = (uint8_t*)   &_emu->_baseMem[0x00D4];
  cameraPosY1                       = (uint8_t*)   &_emu->_baseMem[0x00D7];
  cameraPosY2                       = (uint8_t*)   &_emu->_baseMem[0x00D6];

  player1PosX1                      = (uint8_t*)   &_emu->_baseMem[0x03DE];
  player1PosX2                      = (uint8_t*)   &_emu->_baseMem[0x03DA];
  player1PosY1                      = (uint8_t*)   &_emu->_baseMem[0x03EA];
  player1PosY2                      = (uint8_t*)   &_emu->_baseMem[0x03E6];
  player1PosY3                      = (uint8_t*)   &_emu->_baseMem[0x03EE];
  player1Accel                      = (int8_t*)    &_emu->_baseMem[0x0386];
  player1AccelTimer1                = (uint8_t*)   &_emu->_baseMem[0x0102];
  player1AccelTimer2                = (uint8_t*)   &_emu->_baseMem[0x0103];
  player1AccelTimer3                = (uint8_t*)   &_emu->_baseMem[0x010E];
  player1Inertia1                   = (uint8_t*)   &_emu->_baseMem[0x00B0];
  player1Inertia2                   = (uint8_t*)   &_emu->_baseMem[0x00B2];
  player1Inertia3                   = (uint8_t*)   &_emu->_baseMem[0x00B4];
  player1Inertia4                   = (uint8_t*)   &_emu->_baseMem[0x00B6];
  player1Angle1                     = (uint8_t*)   &_emu->_baseMem[0x04B2];
  player1Angle2                     = (uint8_t*)   &_emu->_baseMem[0x040A];
  player1Angle3                     = (uint8_t*)   &_emu->_baseMem[0x04CA];
  player1LapsRemaining              = (uint8_t*)   &_emu->_baseMem[0x04B6];
  player1LapsRemainingPrev          = (uint8_t*)   &_emu->_baseMem[0x07FF];
  player1Checkpoint                 = (uint8_t*)   &_emu->_baseMem[0x04CE];
  player1Input1                     = (uint8_t*)   &_emu->_baseMem[0x009B];
  player1Input2                     = (uint8_t*)   &_emu->_baseMem[0x00CF];
  player1Input3                     = (uint8_t*)   &_emu->_baseMem[0x0352];

  player1RecoveryMode               = (uint8_t*)   &_emu->_baseMem[0x0416];
  player1RecoveryTimer              = (uint8_t*)   &_emu->_baseMem[0x041A];
  player1ResumeTimer                = (uint8_t*)   &_emu->_baseMem[0x00DA];
  player1CanControlCar              = (uint8_t*)   &_emu->_baseMem[0x01BF];

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

  hash.Update(*cameraPosX1);
  hash.Update(*cameraPosX2);
  hash.Update(*cameraPosY1);
  hash.Update(*cameraPosY2);

  hash.Update(*frameType);
  hash.Update(*lagFrame);
  hash.Update(*preRaceTimer);
  hash.Update(*player1PosX1);
  hash.Update(*player1PosX2);
  hash.Update(*player1PosY1);
  hash.Update(*player1PosY2);
  // hash.Update(*player1PosY3);
  hash.Update(*player1Accel);
  hash.Update(*player1AccelTimer1);
  hash.Update(*player1AccelTimer2);
  hash.Update(*player1AccelTimer3);
  hash.Update(*player1Inertia1);
  hash.Update(*player1Inertia2);
  hash.Update(*player1Inertia3);
  hash.Update(*player1Inertia4);
  // hash.Update(*player1Angle1);
  hash.Update(*player1Angle2);
  // hash.Update(*player1Angle3);
  hash.Update(*player1LapsRemaining);
  hash.Update(*player1LapsRemainingPrev);
  hash.Update(*player1Checkpoint);

  hash.Update(*player1RecoveryMode);
  hash.Update(*player1RecoveryTimer);
  hash.Update(*player1CanControlCar);
  hash.Update(*player1ResumeTimer);

  // hash.Update(*activeFrame1);
  // hash.Update(*activeFrame2);
  // hash.Update(*activeFrame3);
  // hash.Update(*activeFrame4);

  hash.Update(_emu->_baseMem[0x039E]);
  
  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 *player1LapsRemainingPrev = *player1LapsRemaining;
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

 cameraPosX = (uint16_t)*cameraPosX1 * 256 + (uint16_t)*cameraPosX2;
 cameraPosY = (uint16_t)*cameraPosY1 * 256 + (uint16_t)*cameraPosY2;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
//  if (*frameType == 0x00) return { "." };
//  if (*frameType == 0x01) return { ".", "A", "R", "L", "RA", "LA" };
  
  return { "A", "R", "L", "RA", "LA" };

// if (*frameType == 0x00) return { ".", "A", "B" };
// if (*frameType == 0x01) return { ".", "A", "B", "R", "L", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA" };
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
  reward += magnets.playerAccelMagnet * ( std::abs((float)*player1Accel) - 0.1*(float)*player1AccelTimer2);

  // Distance to point magnet
  {
   float distX = std::abs(magnets.pointMagnet.x - player1PosX);
   float distY = std::abs(magnets.pointMagnet.y - player1PosY);
   float distanceToPoint = sqrt(distX*distX + distY*distY);
   reward += magnets.pointMagnet.intensity * -distanceToPoint;
  }

  // Distance to camera
  {
   float distX = std::abs(cameraPosX - player1PosX);
   float distY = std::abs(cameraPosY - player1PosY);
   float distanceToPoint = sqrt(distX*distX + distY*distY);
   reward += magnets.cameraDistanceMagnet * -distanceToPoint;
  }

  // Evaluating player health  magnet
  reward += magnets.recoveryTimerMagnet * (float)(*player1RecoveryTimer);

  // Calculating angle magnet
  float distanceToAngle = std::abs(*player1Angle1 - magnets.car1AngleMagnet.angle);
  reward += (255.0 - distanceToAngle) * magnets.car1AngleMagnet.intensity;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);

 LOG("[Jaffar]  + Global Timer:                       %03u\n", *globalTimer);
 LOG("[Jaffar]  + Pre Race Timer:                     %03u\n", *preRaceTimer);
 LOG("[Jaffar]  + Frame Type / Lag:                   %03u %03u\n", *frameType, *lagFrame);

 LOG("[Jaffar]  + Active Frames:                      %03u %03u %03u %03u\n", *activeFrame1, *activeFrame2, *activeFrame3, *activeFrame4 );

 LOG("[Jaffar]  + Last Input / Frame                  %03u\n",  *player1Input1);
 LOG("[Jaffar]  + Laps Remaining:                     %1u (Prev: %1u)\n", *player1LapsRemaining, *player1LapsRemainingPrev);
 LOG("[Jaffar]  + Checkpoint Id:                      %03u\n", *player1Checkpoint);

 LOG("[Jaffar]  + Player Accel (Timers):              %03d (%03u, %03u, %03u)\n", *player1Accel, *player1AccelTimer1, *player1AccelTimer2, *player1AccelTimer3);
 LOG("[Jaffar]  + Player Inertia:                     %03u, %03u, %03u, %03u\n", *player1Inertia1, *player1Inertia2, *player1Inertia3, *player1Inertia4);
 LOG("[Jaffar]  + Player Angle:                       %03u %03u\n", *player1Angle1, *player1Angle2);

 LOG("[Jaffar]  + Camera Pos X:                       %05u\n", cameraPosX);
 LOG("[Jaffar]  + Camera Pos Y:                       %05u\n", cameraPosY);

 LOG("[Jaffar]  + Player Pos X:                       %05u\n", player1PosX);
 LOG("[Jaffar]  + Player Pos Y:                       %05u\n", player1PosY);
 LOG("[Jaffar]  + Player Recovery:                    %03u (%03u)\n", *player1RecoveryMode, *player1RecoveryTimer);
 LOG("[Jaffar]  + Player Resume Timer:                %03u\n", *player1ResumeTimer);
 LOG("[Jaffar]  + Player Can Control Car:             %03u\n", *player1CanControlCar);


 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 float distX = std::abs(magnets.pointMagnet.x - player1PosX);
 float distY = std::abs(magnets.pointMagnet.y - player1PosY);
 float distanceToPoint = sqrt(distX*distX + distY*distY);

 distX = std::abs(cameraPosX - player1PosX);
 distY = std::abs(cameraPosY - player1PosY);
 float cameraDistance = sqrt(distX*distX + distY*distY);

 float distanceToAngle = std::abs(*player1Angle1 - magnets.car1AngleMagnet.angle);

 if (std::abs(magnets.pointMagnet.intensity) > 0.0f)                 LOG("[Jaffar]  + Point Magnet                     - Intensity: %.5f, X: %3.3f, Y: %3.3f, Dist: %3.3f\n", magnets.pointMagnet.intensity, magnets.pointMagnet.x, magnets.pointMagnet.y, distanceToPoint);
 if (std::abs(magnets.cameraDistanceMagnet) > 0.0f)                  LOG("[Jaffar]  + Camera Distance Magnet           - Intensity: %.5f, Dist: %3.3f\n", magnets.cameraDistanceMagnet, cameraDistance);
 if (std::abs(magnets.recoveryTimerMagnet) > 0.0f)                   LOG("[Jaffar]  + Recovery Timer Magnet            - Intensity: %.5f\n", magnets.recoveryTimerMagnet);
 if (std::abs(magnets.playerCurrentLapMagnet) > 0.0f)                LOG("[Jaffar]  + Player Current Lap Magnet        - Intensity: %.5f\n", magnets.playerCurrentLapMagnet);
 if (std::abs(magnets.playerLapProgressMagnet) > 0.0f)               LOG("[Jaffar]  + Player Lap Progress Magnet       - Intensity: %.5f\n", magnets.playerLapProgressMagnet);
 if (std::abs(magnets.playerAccelMagnet) > 0.0f)                     LOG("[Jaffar]  + Player Accel Magnet              - Intensity: %.5f\n", magnets.playerAccelMagnet);
 if (std::abs(magnets.car1AngleMagnet.intensity) > 0.0f)             LOG("[Jaffar]  + Angle Magnet                     - Intensity: %.5f, Angle: %3.0f, Dist: %3.0f\n", magnets.car1AngleMagnet.intensity, magnets.car1AngleMagnet.angle, distanceToAngle);
}

