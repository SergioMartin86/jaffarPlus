#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameTimer               = (uint8_t*)   &_emu->_baseMem[0x043C];
  isLagFrame              = (uint8_t*)   &_emu->_baseMem[0x00A8];
  isRaceActive            = (uint8_t*)   &_emu->_baseMem[0x0397];
  trafficLightTimer       = (uint8_t*)   &_emu->_baseMem[0x06E2];

  playerSpeed1           = (uint8_t*)   &_emu->_baseMem[0x0596 + _PLAYER_POS];
  playerSpeed2           = (uint8_t*)   &_emu->_baseMem[0x0592 + _PLAYER_POS];
  playerMomentum1        = (uint8_t*)   &_emu->_baseMem[0x05A2 + _PLAYER_POS];
  playerMomentum2        = (uint8_t*)   &_emu->_baseMem[0x05A6 + _PLAYER_POS];
  playerZipperBoost1     = (uint8_t*)   &_emu->_baseMem[0x05B2 + _PLAYER_POS];
  playerZipperBoost2     = (uint8_t*)   &_emu->_baseMem[0x05B6 + _PLAYER_POS];
  playerAngle            = (uint8_t*)   &_emu->_baseMem[0x059E + _PLAYER_POS];
  playerStanding         = (uint8_t*)   &_emu->_baseMem[0x057A + _PLAYER_POS];
  playerPosX1            = (uint8_t*)   &_emu->_baseMem[0x0531 + _PLAYER_POS];
  playerPosX2            = (uint8_t*)   &_emu->_baseMem[0x052C + _PLAYER_POS];
  playerPosY1            = (uint8_t*)   &_emu->_baseMem[0x053B + _PLAYER_POS];
  playerPosY2            = (uint8_t*)   &_emu->_baseMem[0x0536 + _PLAYER_POS];
  playerCurrentLap       = (uint8_t*)   &_emu->_baseMem[0x055E + _PLAYER_POS];
  playerLapProgress1     = (uint8_t*)   &_emu->_baseMem[0x0559 + _PLAYER_POS];
  playerLapProgress2     = (uint8_t*)   &_emu->_baseMem[0x056D + _PLAYER_POS];
  playerFinishPosition   = (uint8_t*)   &_emu->_baseMem[0x076D + _PLAYER_POS];
  playerMoney1           = (uint8_t*)   &_emu->_baseMem[0x05F2 + _PLAYER_POS];
  playerMoney2           = (uint8_t*)   &_emu->_baseMem[0x05F6 + _PLAYER_POS];
  playerPosZ             = (uint8_t*)   &_emu->_baseMem[0x0572 + _PLAYER_POS];
  playerFloorZ           = (uint8_t*)   &_emu->_baseMem[0x058E + _PLAYER_POS];
  playerNitroCount       = (uint8_t*)   &_emu->_baseMem[0x0748 + _PLAYER_POS];
  playerNitroBoost       = (uint8_t*)   &_emu->_baseMem[0x0626 + _PLAYER_POS];
  playerNitroPhase       = (uint8_t*)   &_emu->_baseMem[0x061E + _PLAYER_POS];
  playerNitroCounter     = (uint8_t*)   &_emu->_baseMem[0x0622 + _PLAYER_POS];
  playerNitroPickedUp    = (uint8_t*)   &_emu->_baseMem[0x07FF];
  playerNitroPrevCount   = (uint8_t*)   &_emu->_baseMem[0x07FE];
  playerLastInputKey     = (uint8_t*)   &_emu->_baseMem[0x06E8];
  playerLastInputFrame   = (uint8_t*)   &_emu->_baseMem[0x07FD];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0)
  {
   int timerPhase = *gameTimer % timerTolerance;
   hash.Update(&timerPhase);
  }

  hash.Update(*isLagFrame);
  hash.Update(*trafficLightTimer);
  hash.Update(*isRaceActive);

  hash.Update(*playerSpeed1);
//  hash.Update(*playerSpeed2);
//  hash.Update(*playerMomentum1);
//  hash.Update(*playerMomentum2);
  hash.Update(*playerZipperBoost1);
//  hash.Update(*playerZipperBoost2);

  hash.Update(*playerAngle);
  hash.Update(*playerStanding);
  hash.Update(*playerPosX1);
//  hash.Update(*playerPosX2);
  hash.Update(*playerPosY1);
//  hash.Update(*playerPosY2);
  hash.Update(*playerCurrentLap);
  hash.Update(*playerLapProgress1);
  hash.Update(*playerLapProgress2);
//  hash.Update(*playerFinishPosition);
//  hash.Update(*playerMoney1);
//  hash.Update(*playerMoney2);
  hash.Update(*playerPosZ);
  hash.Update(*playerFloorZ);

  hash.Update(*playerNitroCount);
  hash.Update(*playerNitroBoost > 0);
  hash.Update(*playerNitroPhase);
  hash.Update(*playerNitroCounter > 0);

  hash.Update(_emu->_nes->high_mem()[0x203D]);

  hash.Update(*playerNitroPrevCount);
  hash.Update(*playerNitroPickedUp);
  hash.Update(*playerLastInputFrame);

  uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 playerPosX = (float)*playerPosX1 * 256.0f + (float)*playerPosX2;
 playerPosY = (float)*playerPosY1 * 256.0f + (float)*playerPosY2;
 playerLapProgress = (float)*playerCurrentLap * 256.0f + (float)*playerLapProgress1 + (float)*playerLapProgress2 / 256.0f;
 playerMoney = (float)*playerMoney1 * 2560.0f + (float)*playerMoney2 * 10.0f;
 isAirborne = *playerPosZ != *playerFloorZ;

 playerSpeed = (float)*playerSpeed1 + (float)*playerSpeed2 / 256.0f + (float)*playerZipperBoost1 + (float)*playerZipperBoost2 / 256.0f;

 if (*playerNitroCount > *playerNitroPrevCount) *playerNitroPickedUp += 1;
 *playerNitroPrevCount = *playerNitroCount;

 if (*playerLastInputKey != 0) *playerLastInputFrame = *gameTimer;

 isNitroActive = 0;
 if ( *playerNitroBoost > 0 || *playerNitroPhase > 0 || *playerNitroCounter > 0) isNitroActive = 1;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (isAirborne) return moveList;
 moveList.insert(moveList.end(), { "......B.", ".L......", "R.......", ".L....B.", "R.....B."});
 if (*playerNitroCount > 0 && *playerNitroBoost == 0)
 {
  bool useBoost = false;
  if (playerLapProgress > 3.0 && playerLapProgress < 4.0) useBoost = true;
  if (playerLapProgress > 6.5 && playerLapProgress < 7.5) useBoost = true;
  if (playerLapProgress > 15.4 && playerLapProgress < 16.2) useBoost = true;
  if (playerLapProgress > (256.0 + 3.5) && playerLapProgress < (256.0 + 4.5)) useBoost = true;
  if (playerLapProgress > (256.0 + 6.5) && playerLapProgress < (256.0 + 7.5)) useBoost = true;
  if (playerLapProgress > (256.0 + 15.4) && playerLapProgress < (256.0 + 16.2)) useBoost = true;

  if (useBoost == true) moveList.insert(moveList.end(), { ".......A", "......BA", ".L.....A", "R......A", ".L....BA", "R.....BA"});
 }

 return moveList;
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
  reward += magnets.playerSpeedMagnet * playerSpeed;

  // Evaluating player health  magnet
  reward += magnets.playerStandingMagnet * (float)*playerStanding;

  // Evaluating player health  magnet
  reward += magnets.playerLapProgressMagnet * playerLapProgress;

  // Evaluating player health  magnet
  reward += magnets.playerMoneyMagnet * playerMoney;

  // Evaluating player health  magnet
  reward += magnets.playerNitroActiveMagnet * isNitroActive;

  // Evaluating player health  magnet
  reward += magnets.playerNitroGrabMagnet * (float)*playerNitroPickedUp;

  // Evaluating player health  magnet
  reward += magnets.playerLastInputMagnet * (float)*playerLastInputFrame;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:                         %03u\n", *gameTimer);
 LOG("[Jaffar]  + Traffic Light Timer:                %03u\n", *trafficLightTimer);
 LOG("[Jaffar]  + Is Race Active:                     %03u\n", *isRaceActive);
 LOG("[Jaffar]  + Is Lag Frame:                       %03u\n", *isLagFrame);

 LOG("[Jaffar]  + Player Current Lap:                 %03u\n", *playerCurrentLap);
 LOG("[Jaffar]  + Player Lap Progress:                %f\n", playerLapProgress);
 LOG("[Jaffar]  + Player Is Airborne:                 %s\n", isAirborne ? "True" : "False");
 LOG("[Jaffar]  + Player Standing:                    %01u / 4\n", *playerStanding);
 LOG("[Jaffar]  + Player Pos X:                       %f (%03u, %03u)\n", playerPosX, *playerPosX1, *playerPosX2);
 LOG("[Jaffar]  + Player Pos Y:                       %f (%03u, %03u)\n", playerPosY, *playerPosY1, *playerPosY2);
 LOG("[Jaffar]  + Player Pos Z:                       %03u / %03u\n", *playerPosZ, *playerFloorZ);
 LOG("[Jaffar]  + Player Speed:                       %f (%03u/%03u + %03u/%03u)\n", playerSpeed, *playerSpeed1, *playerSpeed2, *playerZipperBoost1, *playerZipperBoost2);
 LOG("[Jaffar]  + Player Angle:                       %03u\n", *playerAngle);
 LOG("[Jaffar]  + Player Money:                       %f\n", playerMoney);
 LOG("[Jaffar]  + Player Boost:                       %03u, %03u, %03u, %03u, Active: %01u\n", *playerNitroCount, *playerNitroBoost, *playerNitroPhase, *playerNitroCounter, isNitroActive);
 LOG("[Jaffar]  + Player Momentum:                    %03u, %03u\n", *playerMomentum1, *playerMomentum2);
 LOG("[Jaffar]  + SRAM Value:                         %03u\n",  _emu->_nes->high_mem()[0x203D]);
 LOG("[Jaffar]  + Boost Prev / Picked Up              %02u / %02u\n",  *playerNitroPrevCount, *playerNitroPickedUp);
 LOG("[Jaffar]  + Last Input / Frame                  %03u / %03u\n",  *playerLastInputKey, *playerLastInputFrame);
 //for (size_t i = 0; i < 0x20; i++) LOG("%03u ", _emu->_nes->high_mem()[0x203D + i]);


 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.playerSpeedMagnet) > 0.0f)                     LOG("[Jaffar]  + Player Speed Magnet              - Intensity: %.5f\n", magnets.playerSpeedMagnet);
 if (std::abs(magnets.playerStandingMagnet) > 0.0f)                  LOG("[Jaffar]  + Player Standing Magnet           - Intensity: %.5f\n", magnets.playerStandingMagnet);
 if (std::abs(magnets.playerLapProgressMagnet) > 0.0f)               LOG("[Jaffar]  + Player Lap Progress Magnet       - Intensity: %.5f\n", magnets.playerLapProgressMagnet);
 if (std::abs(magnets.playerNitroActiveMagnet) > 0.0f)               LOG("[Jaffar]  + Player Nitro Active Magnet       - Intensity: %.5f\n", magnets.playerNitroActiveMagnet);
 if (std::abs(magnets.playerNitroGrabMagnet) > 0.0f)                 LOG("[Jaffar]  + Player Nitro Grab Magnet         - Intensity: %.5f\n", magnets.playerNitroGrabMagnet);
 if (std::abs(magnets.playerLastInputMagnet) > 0.0f)                 LOG("[Jaffar]  + Player Last Input Magnet         - Intensity: %.5f\n", magnets.playerLastInputMagnet);
}

