#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameTimer              = (uint8_t*)   &_emu->_baseMem[0x005D];
  currentStage           = (uint8_t*)   &_emu->_baseMem[0x003B];
  prevStage              = (uint8_t*)   &_emu->_baseMem[0x003C];
  screenScrollX1         = (uint8_t*)   &_emu->_baseMem[0x004D];
  screenScrollX2         = (uint8_t*)   &_emu->_baseMem[0x004C];
  screenScrollY1         = (uint8_t*)   &_emu->_baseMem[0x004F];
  screenScrollY2         = (uint8_t*)   &_emu->_baseMem[0x004E];
  shipCarriedObject      = (uint8_t*)   &_emu->_baseMem[0x0066];
  shipHealth1            = (uint8_t*)   &_emu->_baseMem[0x00C7];
  shipHealth2            = (uint8_t*)   &_emu->_baseMem[0x00C8];
  shipMaxHealth1         = (uint8_t*)   &_emu->_baseMem[0x00C9];
  shipMaxHealth2         = (uint8_t*)   &_emu->_baseMem[0x00C8];
  shipUpgrades           = (uint8_t*)   &_emu->_baseMem[0x00DC];
  shipPosX1              = (uint8_t*)   &_emu->_baseMem[0x021F];
  shipPosX2              = (uint8_t*)   &_emu->_baseMem[0x0200];
  shipPosX3              = (uint8_t*)   &_emu->_baseMem[0x0197];
  shipPosY1              = (uint8_t*)   &_emu->_baseMem[0x027C];
  shipPosY2              = (uint8_t*)   &_emu->_baseMem[0x025D];
  shipPosY3              = (uint8_t*)   &_emu->_baseMem[0x023E];
  shipVelX1              = (uint8_t*)   &_emu->_baseMem[0x02BA];
  shipVelX2              = (uint8_t*)   &_emu->_baseMem[0x029B];
  shipVelXS              = (uint8_t*)   &_emu->_baseMem[0x0336];
  shipVelY1              = (uint8_t*)   &_emu->_baseMem[0x02F8];
  shipVelY2              = (uint8_t*)   &_emu->_baseMem[0x02D9];
  shipVelYS              = (uint8_t*)   &_emu->_baseMem[0x0355];
  shipAngle              = (uint8_t*)   &_emu->_baseMem[0x0374];
  shipFuel1              = (uint8_t*)   &_emu->_baseMem[0x00C7];
  shipFuel2              = (uint8_t*)   &_emu->_baseMem[0x00C6];
  score1                 = (uint8_t*)   &_emu->_baseMem[0x00CB];
  score2                 = (uint8_t*)   &_emu->_baseMem[0x00CC];
  score3                 = (uint8_t*)   &_emu->_baseMem[0x00CD];
  score4                 = (uint8_t*)   &_emu->_baseMem[0x00CE];
  score5                 = (uint8_t*)   &_emu->_baseMem[0x00CF];
  score6                 = (uint8_t*)   &_emu->_baseMem[0x00D0];
  shotActive             = (uint8_t*)   &_emu->_baseMem[0x027F];
  shipShields            = (uint8_t*)   &_emu->_baseMem[0x0065];
  objectType             = (uint8_t*)   &_emu->_baseMem[0x0317];
  objectData             = (uint8_t*)   &_emu->_baseMem[0x0393];
  fuelDelivered          = (uint8_t*)   &_emu->_baseMem[0x0513];

  eye0State              = (uint8_t*)   &_emu->_baseMem[0x0461];
  eye1State              = (uint8_t*)   &_emu->_baseMem[0x0462];
  eye2State              = (uint8_t*)   &_emu->_baseMem[0x0463];
  eye3State              = (uint8_t*)   &_emu->_baseMem[0x0464];
  eye4State              = (uint8_t*)   &_emu->_baseMem[0x0460];

  eye1Health             = (uint8_t*)   &_emu->_baseMem[0x0459];
  eye2Health             = (uint8_t*)   &_emu->_baseMem[0x0458];
  eye3Health             = (uint8_t*)   &_emu->_baseMem[0x045A];
  eye4Health             = (uint8_t*)   &_emu->_baseMem[0x045B];

  eye1Aperture           = (uint8_t*)   &_emu->_baseMem[0x038C];
  eye2Aperture           = (uint8_t*)   &_emu->_baseMem[0x038B];
  eye3Aperture           = (uint8_t*)   &_emu->_baseMem[0x038D];
  eye4Aperture           = (uint8_t*)   &_emu->_baseMem[0x038E];

  eye1Timer           = (uint8_t*)   &_emu->_baseMem[0x041B];
  eye2Timer           = (uint8_t*)   &_emu->_baseMem[0x041A];
  eye3Timer           = (uint8_t*)   &_emu->_baseMem[0x041C];
  eye4Timer           = (uint8_t*)   &_emu->_baseMem[0x041D];

  lastInputKey     = (uint8_t*)   &_emu->_baseMem[0x0018];
  lastInputFrame   = (uint8_t*)   &_emu->_baseMem[0x0150];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");


  // Last Input Frame Tolerance
  if (isDefined(config, "Last Input Key Accepted") == true)
   lastInputKeyAccepted = config["Last Input Key Accepted"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Last Input Key Accepted' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(*gameTimer % timerTolerance);
//  hash.Update(*gameTimer);
//  hash.Update(*currentStage);
//  hash.Update(*prevStage);
  hash.Update(*screenScrollX1);
  hash.Update(*screenScrollX2);
  hash.Update(*screenScrollY1);
  hash.Update(*screenScrollY2);
//  hash.Update(*shipCarriedObject);
//  hash.Update(*shipHealth1);
//  hash.Update(*shipHealth2);
//  hash.Update(*shipMaxHealth1);
//  hash.Update(*shipMaxHealth2);
//  hash.Update(*shipUpgrades);
  hash.Update(*shipPosX1);
  hash.Update(*shipPosX2);
//  hash.Update(*shipPosX3);
  hash.Update(*shipPosY1);
  hash.Update(*shipPosY2);
//  hash.Update(*shipPosY3);
//  hash.Update(*shipVelX1);
  hash.Update(*shipVelX2);
//  hash.Update(*shipVelXS);
//  hash.Update(*shipVelY1);
  hash.Update(*shipVelY2);
//  hash.Update(*shipVelYS);
//  hash.Update(*shipAngle);
//  hash.Update(*shipFuel1);
//  hash.Update(*shipFuel2);
//  hash.Update(*shipShields);
//  hash.Update(*score1);
//  hash.Update(*score2);
//  hash.Update(*score3);
//  hash.Update(*score4);
//  hash.Update(*score5);
//  hash.Update(*score6);
  hash.Update(shotActive, SHOT_COUNT);
  hash.Update(objectType, OBJECT_COUNT);
  hash.Update(*eye0State);
  hash.Update(*eye1State);
  hash.Update(*eye2State);
  hash.Update(*eye3State);
  hash.Update(*eye4State);
  hash.Update(*eye1Health);
  hash.Update(*eye2Health);
  hash.Update(*eye3Health);
  hash.Update(*eye4Health);
  hash.Update(*eye1Aperture);
  hash.Update(*eye2Aperture);
  hash.Update(*eye3Aperture);
  hash.Update(*eye4Aperture);

  hash.Update(*eye1Timer % 2);
  hash.Update(*eye2Timer % 2);
  hash.Update(*eye3Timer % 2);
  hash.Update(*eye4Timer % 2);

  hash.Update(*lastInputFrame);

//  for (size_t i = 0; i < OBJECT_COUNT; i++)
////   if (*(objectType+i) == 10)
//   {
//    hash.Update(*(objectData+i));
//    hash.Update(*(shipPosX1+i));
//    hash.Update(*(shipPosX2+i));
//    hash.Update(*(shipPosY1+i));
//    hash.Update(*(shipPosY2+i));
//   }
//
//  for (size_t i = 0; i < OBJECT_COUNT; i++)
////   if (*(objectType+i) == 184)
//   {
//    hash.Update(*(objectData+i));
//    hash.Update(*(shipPosX1+i));
//    hash.Update(*(shipPosX2+i));
//    hash.Update(*(shipPosY1+i));
//    hash.Update(*(shipPosY2+i));
//   }

  // For fuel delivery
  //hash.Update(&_emu->_baseMem[0x0500], 0x0100);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 screenScroll = (float)*screenScrollX1 * 256.0f + (float)*screenScrollX2;
 shipPosX = (float)*shipPosX1 * 256.0f + (float)*shipPosX2 + (float)*shipPosX3 / 256.0f;
 shipPosY = (float)*shipPosY1 * 256.0f + (float)*shipPosY2 + (float)*shipPosY3 / 256.0f;

 shipVelX = (float)*shipVelX1 + (float)*shipVelX2 / 256.0f; if (*shipVelXS) shipVelX *= -1.0f;
 shipVelY = (float)*shipVelY1 + (float)*shipVelY2 / 256.0f; if (*shipVelYS) shipVelY *= -1.0f;

 shipFuel = (float)*shipFuel1 + (float)*shipFuel2 / 256.0f;
 shipHealth = (float)*shipHealth1 + (float)*shipHealth2 / 256.0f;
 shipMaxHealth = (float)*shipMaxHealth1 + (float)*shipMaxHealth2 / 256.0f;

 score = (float)*score6 * 100000.0f + (float)*score5 * 10000.0f + (float)*score4 * 1000.0f + (float)*score3 * 100.0f + (float)*score2 * 10.0f + (float)*score1 * 1.0f;

 for (size_t i = 0; i < OBJECT_COUNT; i++)
 {
  objectPosX[i] = (float)*(shipPosX1+i) * 256.0f + (float)*(shipPosX2+i) + (float)*(shipPosX3+i) / 256.0f;
  objectPosY[i] = (float)*(shipPosY1+i) * 256.0f + (float)*(shipPosY2+i) + (float)*(shipPosY3+i) / 256.0f;
 }

 warpCounter = 0.0f;
 for (size_t i = 0; i < OBJECT_COUNT; i++) if (*(objectType+i) == 10) warpCounter += 1.0f;
 for (size_t i = 0; i < OBJECT_COUNT; i++) if (*(objectType+i) == 184) warpCounter += 0.5f;

 maxWarp = 0;
 for (size_t i = 0; i < OBJECT_COUNT; i++)
  if (*(objectType+i) == 10 && *(objectData+i) > maxWarp) maxWarp = *(objectData+i);

 eyeCount = 0;
 if (*eye0State == 94) eyeCount++;
 if (*eye1State == 94) eyeCount++;
 if (*eye2State == 94) eyeCount++;
 if (*eye3State == 94) eyeCount++;
 if (*eye4State == 94) eyeCount++;

 eye1Readiness = *eye1Aperture == 180 ? ((float)*eye1Timer - 32.0f) : 1.0f;
 eye2Readiness = *eye2Aperture == 180 ? ((float)*eye2Timer - 32.0f) : 1.0f;
 eye3Readiness = *eye3Aperture == 180 ? ((float)*eye3Timer - 32.0f) : 1.0f;
 eye4Readiness = *eye4Aperture == 180 ? ((float)*eye4Timer - 32.0f) : 1.0f;

 shotsActive = 0;
 for (size_t i = 0; i < SHOT_COUNT; i++)
  if (*(shotActive+i) > 0) shotsActive++;

 if (*lastInputKey != 0) *lastInputFrame = *gameTimer;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*gameTimer % 2 == 1) return {"."};
 if (lastInputKeyAccepted != 0) if (*gameTimer > lastInputKeyAccepted && *gameTimer < lastInputKeyAccepted + 60) return {"."};

 // Stage 00
// moveList.insert(moveList.end(), { "R", "L", "LA", "RA", "RB", "LB", "RLA", "RLB", "B", "A", "BA", "D" });

 // Stage 12
// moveList.insert(moveList.end(), { "R", "L", "B", "D", "RL", "RB", "RD", "LB", "LD", "BD", "RLB", "RLD", "LBD", "RLBD"  });

 // Stage scroller
 moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U...A", "..D....A", ".L.....A", ".L.U....", ".LD.....", "R.D.....", "R......A", "R..U....", "R.D....A", "R..U...A", ".LD....A", ".L.U...A"});

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

  // Container for bounded value and difference with center
  float boundedValue = 0.0;
  float diff = 0.0;

  // Evaluating screen scroll magnet
  boundedValue = screenScroll;
  boundedValue = std::min(boundedValue, magnets.screenScrollMagnet.max);
  boundedValue = std::max(boundedValue, magnets.screenScrollMagnet.min);
  diff = std::abs(magnets.screenScrollMagnet.center - boundedValue);
  reward += magnets.screenScrollMagnet.intensity * -diff;

  // Evaluating ship magnet's reward on position X
  boundedValue = shipPosX;
  boundedValue = std::min(boundedValue, magnets.shipHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.shipHorizontalMagnet.min);
  diff = std::abs(magnets.shipHorizontalMagnet.center - boundedValue);
  reward += magnets.shipHorizontalMagnet.intensity * -diff;

  // Evaluating ship magnet's reward on position Y
  boundedValue = shipPosY;
  boundedValue = std::min(boundedValue, magnets.shipVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.shipVerticalMagnet.min);
  diff = std::abs(magnets.shipVerticalMagnet.center - boundedValue);
  reward += magnets.shipVerticalMagnet.intensity * -diff;

  // Evaluating ship magnet's reward on vel X
  boundedValue = shipVelX;
  boundedValue = std::min(boundedValue, magnets.shipVelXMagnet.max);
  boundedValue = std::max(boundedValue, magnets.shipVelXMagnet.min);
  diff = std::abs(magnets.shipVelXMagnet.center - boundedValue);
  reward += magnets.shipVelXMagnet.intensity * -diff;

  // Evaluating ship magnet's reward on vel Y
  boundedValue = shipVelY;
  boundedValue = std::min(boundedValue, magnets.shipVelYMagnet.max);
  boundedValue = std::max(boundedValue, magnets.shipVelYMagnet.min);
  diff = std::abs(magnets.shipVelYMagnet.center - boundedValue);
  reward += magnets.shipVelYMagnet.intensity * -diff;

  // Evaluating ship health  magnet
  reward += magnets.shipHealthMagnet * shipHealth;

  // Evaluating ship health  magnet
  reward += magnets.scoreMagnet * score;

  // Evaluating ship health  magnet
  reward += magnets.warpCounterMagnet * warpCounter;

  // Evaluating ship health  magnet
  reward += magnets.maxWarpMagnet * maxWarp;

  // Evaluating ship health  magnet
  reward += magnets.eye1HealthMagnet * (float)*eye1Health;
  reward += magnets.eye2HealthMagnet * (float)*eye2Health;
  reward += magnets.eye3HealthMagnet * (float)*eye3Health;
  reward += magnets.eye4HealthMagnet * (float)*eye4Health;
  reward += magnets.eye1ReadinessMagnet * eye1Readiness;
  reward += magnets.eye2ReadinessMagnet * eye2Readiness;
  reward += magnets.eye3ReadinessMagnet * eye3Readiness;
  reward += magnets.eye4ReadinessMagnet * eye4Readiness;
  reward += magnets.shotsActiveMagnet * (float)shotsActive;


  // Evaluating carrying magnet
  int isCarrying = 0;
  if (*shipCarriedObject != 0) isCarrying = 1;
  reward += magnets.carryMagnet * isCarrying;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Game Timer:                       %03u\n", *gameTimer);
 LOG("[Jaffar]  + Stage:                            %03u (Prev: %03u)\n", *currentStage, *prevStage);
 LOG("[Jaffar]  + Last Input Time / Key:            %02u (Max: %02u) / %02u\n", *lastInputFrame, lastInputKeyAccepted, *lastInputKey);
 LOG("[Jaffar]  + Reward:                           %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Score:                            %f\n", score);
 LOG("[Jaffar]  + Ship Health:                      %f / %f\n", shipHealth, shipMaxHealth);
 LOG("[Jaffar]  + Ship Fuel:                        %f\n", shipFuel);
// LOG("[Jaffar]  + Ship Angle:                       %03u\n", *shipAngle);
// LOG("[Jaffar]  + Ship Upgrades:                    %03u\n", *shipUpgrades);
// LOG("[Jaffar]  + Ship Carried Object:              %03u\n", *shipCarriedObject);
 LOG("[Jaffar]  + Ship Pos X:                       %f (%03u, %03u, %03u)\n", shipPosX, *shipPosX1, *shipPosX2, *shipPosX3);
 LOG("[Jaffar]  + Ship Pos Y:                       %f (%03u, %03u, %03u)\n", shipPosY, *shipPosY1, *shipPosY2, *shipPosY3);
 LOG("[Jaffar]  + Ship Vel X:                       %f\n", shipVelX);
 LOG("[Jaffar]  + Ship Vel Y:                       %f\n", shipVelY);
 LOG("[Jaffar]  + Ship Shields:                     %03u\n", *shipShields);
// LOG("[Jaffar]  + Warp Counter:                     %f\n", warpCounter);
// LOG("[Jaffar]  + Max Warp:                         %02u\n", maxWarp);
// LOG("[Jaffar]  + Fuel Delivered:                   %02u\n", *fuelDelivered);
 LOG("[Jaffar]  + Screen Scroll X:                  %f, %03u %03u\n", screenScroll, *screenScrollX1, *screenScrollX2);
 LOG("[Jaffar]  + Screen Scroll Y:                  %03u %03u\n", *screenScrollY1, *screenScrollY2);

// LOG("[Jaffar]  + Shots (Active):                   (%03u)\n", shotsActive);
// for (size_t i = 0; i < SHOT_COUNT; i++)
//  LOG("[Jaffar]  + Shot %lu:                          State: %03u\n", i, *(shotActive+i));

// LOG("[Jaffar]  + Warps:\n");
// for (size_t i = 0; i < OBJECT_COUNT; i++)
//  if (*(objectType+i) == 10)
//   LOG("[Jaffar]    + Warp %02lu:                       Type: %03u, D: %03u, X: %3.3f (%03u, %03u, %03u), Y: %3.3f (%03u, %03u, %03u)\n", i, *(objectType+i), *(objectData+i), objectPosX[i], *(shipPosX1+i), *(shipPosX2+i), *(shipPosX3+i),  objectPosY[i], *(shipPosY1+i), *(shipPosY2+i), *(shipPosY3+i));

// LOG("[Jaffar]  + Eyes:\n");
// for (size_t i = 0; i < OBJECT_COUNT; i++)
//  if (*(objectType+i) == 94)
//   LOG("[Jaffar]    + Eye %02lu:                        Type: %03u, D: %03u, X: %3.3f (%03u, %03u, %03u), Y: %3.3f (%03u, %03u, %03u)\n", i, *(objectType+i), *(objectData+i), objectPosX[i], *(shipPosX1+i), *(shipPosX2+i), *(shipPosX3+i),  objectPosY[i], *(shipPosY1+i), *(shipPosY2+i), *(shipPosY3+i));

//  LOG("[Jaffar]  + Objects:\n");
//  for (size_t i = 0; i < OBJECT_COUNT; i++)
//    LOG("[Jaffar]    + Obj %02lu:                        Type: %03u, D: %03u, X: %3.3f (%03u, %03u, %03u), Y: %3.3f (%03u, %03u, %03u)\n", i, *(objectType+i), *(objectData+i), objectPosX[i], *(shipPosX1+i), *(shipPosX2+i), *(shipPosX3+i),  objectPosY[i], *(shipPosY1+i), *(shipPosY2+i), *(shipPosY3+i));

  LOG("[Jaffar]  + Eyes Count:                            %02u\n", eyeCount);
  LOG("[Jaffar]  + Eye 0 S:                               %03u\n", *eye0State);
  LOG("[Jaffar]  + Eye 1 S / H / A / T / R:               %03u / %03u / %03u / %03u / %f\n", *eye1State, *eye1Health, *eye1Aperture, *eye1Timer, eye1Readiness);
  LOG("[Jaffar]  + Eye 2 S / H / A / T / R:               %03u / %03u / %03u / %03u / %f\n", *eye2State, *eye2Health, *eye2Aperture, *eye2Timer, eye2Readiness);
  LOG("[Jaffar]  + Eye 3 S / H / A / T / R:               %03u / %03u / %03u / %03u / %f\n", *eye3State, *eye3Health, *eye3Aperture, *eye3Timer, eye3Readiness);
  LOG("[Jaffar]  + Eye 4 S / H / A / T / R:               %03u / %03u / %03u / %03u / %f\n", *eye4State, *eye4Health, *eye4Aperture, *eye4Timer, eye4Readiness);

// for (size_t i = 0; i < OBJECT_COUNT; i++)
//  if (*(objectType+i) == 184)
//   LOG("[Jaffar]    + Green Ship %02lu:                 Type: %03u, D: %03u, X: %3.3f (%03u, %03u, %03u), Y: %3.3f (%03u, %03u, %03u)\n", i, *(objectType+i), *(objectData+i), objectPosX[i], *(shipPosX1+i), *(shipPosX2+i), *(shipPosX3+i),  objectPosY[i], *(shipPosY1+i), *(shipPosY2+i), *(shipPosY3+i));

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.screenScrollMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Screen Scroll Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.screenScrollMagnet.intensity, magnets.screenScrollMagnet.center, magnets.screenScrollMagnet.min, magnets.screenScrollMagnet.max);
 if (std::abs(magnets.shipHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Ship Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.shipHorizontalMagnet.intensity, magnets.shipHorizontalMagnet.center, magnets.shipHorizontalMagnet.min, magnets.shipHorizontalMagnet.max);
 if (std::abs(magnets.shipVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Ship Vertical Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.shipVerticalMagnet.intensity, magnets.shipVerticalMagnet.center, magnets.shipVerticalMagnet.min, magnets.shipVerticalMagnet.max);
 if (std::abs(magnets.shipVelXMagnet.intensity) > 0.0f)           LOG("[Jaffar]  + Ship Vel X Magnet             - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.shipVelXMagnet.intensity, magnets.shipVelXMagnet.center, magnets.shipVelXMagnet.min, magnets.shipVelXMagnet.max);
 if (std::abs(magnets.shipVelYMagnet.intensity) > 0.0f)           LOG("[Jaffar]  + Ship Vel Y Magnet             - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.shipVelYMagnet.intensity, magnets.shipVelYMagnet.center, magnets.shipVelYMagnet.min, magnets.shipVelYMagnet.max);
 if (std::abs(magnets.shipHealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Ship Health Magnet            - Intensity: %.5f\n", magnets.shipHealthMagnet);
 if (std::abs(magnets.scoreMagnet) > 0.0f)                        LOG("[Jaffar]  + Score Magnet                  - Intensity: %.5f\n", magnets.scoreMagnet);
 if (std::abs(magnets.warpCounterMagnet) > 0.0f)                  LOG("[Jaffar]  + Warp Counter Magnet           - Intensity: %.5f\n", magnets.warpCounterMagnet);
 if (std::abs(magnets.carryMagnet) > 0.0f)                        LOG("[Jaffar]  + Carry Magnet                  - Intensity: %.5f\n", magnets.carryMagnet);
 if (std::abs(magnets.maxWarpMagnet) > 0.0f)                      LOG("[Jaffar]  + Max Warp Magnet               - Intensity: %.5f\n", magnets.maxWarpMagnet);
 if (std::abs(magnets.eye1HealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Eye 1 Health Magnet           - Intensity: %.5f\n", magnets.eye1HealthMagnet);
 if (std::abs(magnets.eye2HealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Eye 2 Health Magnet           - Intensity: %.5f\n", magnets.eye2HealthMagnet);
 if (std::abs(magnets.eye3HealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Eye 3 Health Magnet           - Intensity: %.5f\n", magnets.eye3HealthMagnet);
 if (std::abs(magnets.eye4HealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Eye 4 Health Magnet           - Intensity: %.5f\n", magnets.eye4HealthMagnet);
 if (std::abs(magnets.eye1ReadinessMagnet) > 0.0f)                LOG("[Jaffar]  + Eye 1 Readiness Magnet           - Intensity: %.5f\n", magnets.eye1ReadinessMagnet);
 if (std::abs(magnets.eye2ReadinessMagnet) > 0.0f)                LOG("[Jaffar]  + Eye 2 Readiness Magnet           - Intensity: %.5f\n", magnets.eye2ReadinessMagnet);
 if (std::abs(magnets.eye3ReadinessMagnet) > 0.0f)                LOG("[Jaffar]  + Eye 3 Readiness Magnet           - Intensity: %.5f\n", magnets.eye3ReadinessMagnet);
 if (std::abs(magnets.eye4ReadinessMagnet) > 0.0f)                LOG("[Jaffar]  + Eye 4 Readiness Magnet           - Intensity: %.5f\n", magnets.eye4ReadinessMagnet);
 if (std::abs(magnets.shotsActiveMagnet) > 0.0f)                  LOG("[Jaffar]  + Shots Active Magnet           - Intensity: %.5f\n", magnets.shotsActiveMagnet);
}

