#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameTimer               = (uint8_t*)   &_emu->_baseMem[0x0736];
  raceNumber              = (uint8_t*)   &_emu->_baseMem[0x07AF];
  raceType                = (uint8_t*)   &_emu->_baseMem[0x07B0];
  preRaceTimer            = (uint8_t*)   &_emu->_baseMem[0x073C];

  p1LapProgress           = (uint8_t*)   &_emu->_baseMem[0x0344];
  p1CurrentLap            = (uint8_t*)   &_emu->_baseMem[0x033F];
  p1TurboCounter          = (uint8_t*)   &_emu->_baseMem[0x0310];
  p1PosX                  = (uint8_t*)   &_emu->_baseMem[0x0083];
  p1PosY                  = (uint8_t*)   &_emu->_baseMem[0x008B];
  p1Accel                 = (uint8_t*)   &_emu->_baseMem[0x0097];
  p1Angle                 = (uint8_t*)   &_emu->_baseMem[0x0093];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Checkpoint info
  maxCheckpointId = 0;
  if (isDefined(config, "Lap Checkpoints") == true)
    for (size_t i = 0; i < config["Lap Checkpoints"].size(); i++)
    {
     auto& value = config["Lap Checkpoints"][i];
     auto id = value[0].get<uint8_t>();
     maxCheckpointId = std::max(maxCheckpointId, id);
     auto posX = value[1].get<uint8_t>();
     auto posY = value[2].get<uint8_t>();
     checkpoints[id] = std::make_pair(posX, posY);
    }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Lap Checkpoints' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(*gameTimer % (timerTolerance+1));

  hash.Update(*preRaceTimer);

  hash.Update(*p1Accel % 4);
  hash.Update(*p1Angle % 7);
  hash.Update(*p1PosX);
  hash.Update(*p1PosY);

  hash.Update(&_emu->_baseMem[0x0300], 0x04A);

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
 if (checkpoints.contains(*p1LapProgress) == false)
 {
  checkpointId = -1;
  checkpointPosX = 0;
  checkpointPosY = 0;
  checkpointDistance = 9999.0;
 }
 else
 {
  checkpointId = *p1LapProgress;
  checkpointPosX = checkpoints[*p1LapProgress].first;
  checkpointPosY = checkpoints[*p1LapProgress].second;

  float distX = checkpointPosX - (float)*p1PosX;
  float distY = checkpointPosY - (float)*p1PosY;
  checkpointDistance = sqrt(distX*distX + distY*distY);
 }

 float checkpointProgress = (362.0 - checkpointDistance) / 362.0;

 float progressScore = ((float)maxCheckpointId - ((float)*p1LapProgress) + 4.0*checkpointProgress) / (float)maxCheckpointId;
 lapProgress = progressScore;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
// return { ".", "A", "B", "R", "L", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA" };
 return { ".", "A", "B", "R", "L", "RA", "LA" };
// return { ".", "A", "R", "L", "RA", "LA" };
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
 // We calculate a different reward if this is a winning frame
 auto stateType = getStateType(rulesStatus);
 if (stateType == f_win) return *p1TurboCounter;

  // Getting rewards from rules
  float reward = 0.0;
  for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
   if (rulesStatus[ruleId] == true)
    reward += _rules[ruleId]->_reward;

  // Getting magnet values for the kid
  auto magnets = getMagnetValues(rulesStatus);

  // Evaluating player health  magnet
  reward += magnets.playerCurrentLapMagnet * (float)(*p1CurrentLap-1);

  // Evaluating player health  magnet
  reward += magnets.playerLapProgressMagnet * lapProgress;

  // Evaluating player health  magnet
  reward += magnets.playerTurboCounterMagnet * (float)*p1TurboCounter;

  // Evaluating player health  magnet
  reward += magnets.playerAccelMagnet * (float)*p1Accel;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:                         %03u\n", *gameTimer);
 LOG("[Jaffar]  + Pre Race Timer:                     %03u\n", *preRaceTimer);
 LOG("[Jaffar]  + Race Number:                        %03u\n", *raceNumber);
 LOG("[Jaffar]  + Race Type:                          %03u\n", *raceType);

 LOG("[Jaffar]  + Current Lap:                        %03u\n", *p1CurrentLap);
 LOG("[Jaffar]  + Lap Progress:                       %0f\n", lapProgress);
 LOG("[Jaffar]  + Checkpoint Id / Max (PosX, PosY):   %03u / %03u (%03u, %03u)\n", checkpointId, maxCheckpointId, checkpointPosX, checkpointPosY);
 LOG("[Jaffar]  + Checkpoint Distance:                %03f\n", checkpointDistance);

 LOG("[Jaffar]  + Player Turbo Counter:               %03u\n", *p1TurboCounter);
 LOG("[Jaffar]  + Player Accel:                       %03u\n", *p1Accel);
 LOG("[Jaffar]  + Player Angle:                       %03u\n", *p1Angle);
 LOG("[Jaffar]  + Player Pos X:                       %03u\n", *p1PosX);
 LOG("[Jaffar]  + Player Pos Y:                       %03u\n", *p1PosX);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.playerCurrentLapMagnet) > 0.0f)                LOG("[Jaffar]  + Player Current Lap Magnet        - Intensity: %.5f\n", magnets.playerCurrentLapMagnet);
 if (std::abs(magnets.playerLapProgressMagnet) > 0.0f)               LOG("[Jaffar]  + Player Lap Progress Magnet       - Intensity: %.5f\n", magnets.playerLapProgressMagnet);
 if (std::abs(magnets.playerTurboCounterMagnet) > 0.0f)              LOG("[Jaffar]  + Player Turbo Counter Magnet      - Intensity: %.5f\n", magnets.playerTurboCounterMagnet);
 if (std::abs(magnets.playerAccelMagnet) > 0.0f)                     LOG("[Jaffar]  + Player Accel Magnet              - Intensity: %.5f\n", magnets.playerAccelMagnet);
}

