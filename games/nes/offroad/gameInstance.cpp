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

  p1Cash0                 = (uint8_t*)   &_emu->_baseMem[0x0306];
  p1Cash1                 = (uint8_t*)   &_emu->_baseMem[0x0307];
  p1Cash2                 = (uint8_t*)   &_emu->_baseMem[0x0308];
  p1Cash3                 = (uint8_t*)   &_emu->_baseMem[0x0309];
  p1CashGrabbed           = (uint8_t*)   &_emu->_baseMem[0x033C];

  trackType               = (uint8_t*)   &_emu->_baseMem[0x0723];
  menuState1              = (uint8_t*)   &_emu->_baseMem[0x0747];
  menuState2              = (uint8_t*)   &_emu->_baseMem[0x0748];
  menuState3              = (uint8_t*)   &_emu->_baseMem[0x0749];

  menuSelectorX           = (uint8_t*)   &_emu->_baseMem[0x0038];
  menuSelectorY           = (uint8_t*)   &_emu->_baseMem[0x0039];
  menuRaceStartTimer      = (uint8_t*)   &_emu->_baseMem[0x0515];
  menuRaceState           = (uint8_t*)   &_emu->_baseMem[0x054A];

  globalTimer            = (uint16_t*)   &_emu->_baseMem[0x0440];
  playerLastInputKey     = (uint8_t*)   &_emu->_baseMem[0x0026];
  playerLastInputFrame   = (uint16_t*)   &_emu->_baseMem[0x07FE];


  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  if (isDefined(config, "Is Skip Run") == true)
    isSkipRun = config["Is Skip Run"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Is Skip Run' was not defined\n");

  // Checkpoint info
  maxCheckpointId = 0;
  if (isDefined(config, "Lap Checkpoints") == true)
   for (size_t r = 0; r < config["Lap Checkpoints"].size(); r++)
    for (size_t i = 0; i < config["Lap Checkpoints"][r].size(); i++)
    {
     auto& value = config["Lap Checkpoints"][r][i];
     auto id = value[0].get<uint8_t>();
     maxCheckpointId = std::max(maxCheckpointId, id);
     auto posX = value[1].get<uint8_t>();
     auto posY = value[2].get<uint8_t>();
     checkpoints[r][id] = std::make_pair(posX, posY);
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

  if (isSkipRun)
  {
   hash.Update(*gameTimer);
   hash.Update(*p1Cash0);
   hash.Update(*p1Cash1);
   hash.Update(*p1Cash2);
   hash.Update(*p1Cash3);
   hash.Update(*menuState1);
   hash.Update(*menuState2);
   hash.Update(*menuState3);
   hash.Update(*menuSelectorX);
   hash.Update(*menuSelectorY);
   hash.Update(*menuRaceStartTimer);
   hash.Update(*menuRaceState);

//   hash.Update(_emu->_baseMem[0x026]); // Inputs
   hash.Update(_emu->_baseMem[0x027]);
   hash.Update(_emu->_baseMem[0x566]);
   hash.Update(_emu->_baseMem[0x567]); // Menu
  }

  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));

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
 *globalTimer = *globalTimer+1;
 updateDerivedValues();
 return moves;
}

void GameInstance::updateDerivedValues()
{
 totalCash = 1000*(uint16_t)*p1Cash0 + 100*(uint16_t)*p1Cash1 + 10*(uint16_t)*p1Cash2 + *p1Cash3;
 invertedTrack = *trackType >= 0x08;

 if (*playerLastInputKey != 0) *playerLastInputFrame = *globalTimer;

 if (isSkipRun) return;

 if (checkpoints[*trackType].size() == 0) EXIT_WITH_ERROR("[Error] Checkpoints for track type %03u not defined.\n", *trackType);

 if (checkpoints[*trackType].contains(*p1LapProgress) == false)
 {
  checkpointId = -1;
  checkpointPosX = 0;
  checkpointPosY = 0;
  checkpointDistance = 9999.0;
 }
 else
 {
  checkpointId = *p1LapProgress;
  checkpointPosX = checkpoints[*trackType][*p1LapProgress].first;
  checkpointPosY = checkpoints[*trackType][*p1LapProgress].second;

  float distX = checkpointPosX - (float)*p1PosX;
  float distY = checkpointPosY - (float)*p1PosY;
  checkpointDistance = sqrt(distX*distX + distY*distY);
 }

 float checkpointProgress = (362.0 - checkpointDistance) / 362.0;
 if (invertedTrack == true) lapProgress = ((float)*p1LapProgress + 4.0*checkpointProgress) / (float)maxCheckpointId;
 else                       lapProgress = ((float)maxCheckpointId - (float)*p1LapProgress + 4.0*checkpointProgress) / (float)maxCheckpointId;

}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
// return { ".", "A", "B", "R", "L", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA" };
 if (isSkipRun) return { ".", "A", "B", "D", "U", "L", "R", "S", "DA", "DB", "LA", "LB", "UA", "UB", "RA", "RB", "DRA", "DRB", "DLA", "DLB", "ULA", "ULB", "URA", "URB" };
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

// if (stateType == f_win) return -1.0f * (float)*playerLastInputFrame;
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
  float excessTurbo = 99.0f;
  float baseTurboCount = (float) *p1TurboCounter;
  float cappedTurboCount = (float)std::min(baseTurboCount, excessTurbo);
  float excessTurboCount = (float)std::max(0.0f, baseTurboCount - excessTurbo);
  reward += magnets.playerTurboCounterMagnet * (cappedTurboCount + 0.25f * excessTurboCount);

  // Evaluating player health  magnet
  reward += magnets.playerAccelMagnet * (float)*p1Accel;

  // Evaluating player health  magnet
  reward += magnets.playerCashMagnet * (float)*p1CashGrabbed;

  if (isSkipRun)
  {
   if (*menuSelectorY != 112) reward += (float)*menuSelectorX * 0.01 + (float)*menuSelectorY * 0.01;
   reward -= (float)*preRaceTimer * 0.01;
  }

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:                         %03u\n", *gameTimer);

 LOG("[Jaffar]  + Global Timer:                       %03u\n", *globalTimer);
 LOG("[Jaffar]  + Last Input / Frame                  %03u / %03u\n",  *playerLastInputKey, *playerLastInputFrame);

 LOG("[Jaffar]  + Pre Race Timer:                     %03u\n", *preRaceTimer);
 LOG("[Jaffar]  + Race Number: %2u / 99\n", *raceNumber);
 LOG("[Jaffar]  + Current Lap: %1u / 4\n", *p1CurrentLap);

 LOG("[Jaffar]  + Race Type:                          %03u\n", *raceType);
 LOG("[Jaffar]  + Track Type:                         %03u\n", *trackType);
 LOG("[Jaffar]  + Menu State:                         %03u %03u %03u\n", *menuState1, *menuState2, *menuState3);

 LOG("[Jaffar]  + Current Lap:                        %03u\n", *p1CurrentLap);
 LOG("[Jaffar]  + Lap Progress:                       %0f\n", lapProgress);
 LOG("[Jaffar]  + Checkpoint Id / Max (PosX, PosY):   %03u / %03u (%03u, %03u)\n", checkpointId, maxCheckpointId, checkpointPosX, checkpointPosY);
 LOG("[Jaffar]  + Checkpoint Distance:                %03f\n", checkpointDistance);

 LOG("[Jaffar]  + Player Turbo Counter:               %03u\n", *p1TurboCounter);
 LOG("[Jaffar]  + Player Cash:                        %u (%01u %01u %01u %01u)\n", totalCash, *p1Cash0, *p1Cash1, *p1Cash2, *p1Cash3);

 if (isSkipRun)
 {
  LOG("[Jaffar]  + Menu Selector X:                   %03u\n", *menuSelectorX);
  LOG("[Jaffar]  + Menu Selector Y:                   %03u\n", *menuSelectorY);
 }
 else
 {
  LOG("[Jaffar]  + Player Accel:                       %03u\n", *p1Accel);
  LOG("[Jaffar]  + Player Angle:                       %03u\n", *p1Angle);
  LOG("[Jaffar]  + Player Pos X:                       %03u\n", *p1PosX);
  LOG("[Jaffar]  + Player Pos Y:                       %03u\n", *p1PosY);
  LOG("[Jaffar]  + Player Grabbed:                     %03u\n", *p1CashGrabbed);
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.playerCurrentLapMagnet) > 0.0f)                LOG("[Jaffar]  + Player Current Lap Magnet        - Intensity: %.5f\n", magnets.playerCurrentLapMagnet);
 if (std::abs(magnets.playerLapProgressMagnet) > 0.0f)               LOG("[Jaffar]  + Player Lap Progress Magnet       - Intensity: %.5f\n", magnets.playerLapProgressMagnet);
 if (std::abs(magnets.playerTurboCounterMagnet) > 0.0f)              LOG("[Jaffar]  + Player Turbo Counter Magnet      - Intensity: %.5f\n", magnets.playerTurboCounterMagnet);
 if (std::abs(magnets.playerCashMagnet) > 0.0f)                      LOG("[Jaffar]  + Player Cash Magnet Magnet        - Intensity: %.5f\n", magnets.playerCashMagnet);
 if (std::abs(magnets.playerAccelMagnet) > 0.0f)                     LOG("[Jaffar]  + Player Accel Magnet              - Intensity: %.5f\n", magnets.playerAccelMagnet);
}

