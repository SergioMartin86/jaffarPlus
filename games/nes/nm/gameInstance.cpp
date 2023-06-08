#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameTimer                       = (uint8_t*)   &_emu->_baseMem[0x007C];
  trafficLightTimer               = (uint8_t*)   &_emu->_baseMem[0x06A3];
  carRPM                          = (uint8_t*)   &_emu->_baseMem[0x0650];
  carSpeed                        = (uint8_t*)   &_emu->_baseMem[0x0618];
  carGear                         = (uint8_t*)   &_emu->_baseMem[0x0694];
  playerPos1                      = (uint8_t*)   &_emu->_baseMem[0x0334];
  playerPos2                      = (uint8_t*)   &_emu->_baseMem[0x0335];
  playerPos3                      = (uint8_t*)   &_emu->_baseMem[0x0336];
  lapProgress1                    = (uint8_t*)   &_emu->_baseMem[0x0048];
  lapProgress2                    = (uint8_t*)   &_emu->_baseMem[0x0049];
  carTireDamage                   = (uint8_t*)   &_emu->_baseMem[0x036F];
  carPosX                         = (uint8_t*)   &_emu->_baseMem[0x0546];
  carTireAngle                    = (uint8_t*)   &_emu->_baseMem[0x007C];
  carTurnState1                   = (uint8_t*)   &_emu->_baseMem[0x064B];
  carTurnState2                   = (uint8_t*)   &_emu->_baseMem[0x064C];
  gamePhase                       = (uint8_t*)   &_emu->_baseMem[0x07FF];

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

  if (timerTolerance > 0)
  {
   int timerPhase = *gameTimer % timerTolerance;
   hash.Update(&timerPhase);
  }

  hash.Update(*trafficLightTimer);
  hash.Update(*carRPM);
  hash.Update(*carSpeed);
  hash.Update(*carGear);
  hash.Update(*playerPos1);
  hash.Update(*playerPos2);
  hash.Update(*playerPos3);
  hash.Update(*lapProgress1);
  hash.Update(*lapProgress2);
  hash.Update(*carTireDamage);
  hash.Update(*carPosX);
  hash.Update(*carTireAngle);
  hash.Update(*carTurnState1);
  hash.Update(*carTurnState2);

  hash.Update(&_emu->_baseMem[0x500], 0x200);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 lapProgress = (float)*lapProgress2 * 256.0f + (float)*lapProgress1;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 _emu->advanceState(move); moves.push_back(move);
 *gamePhase = *gameTimer % 8;
 updateDerivedValues();
 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

 if (*gamePhase == 0x0000) moveList.insert(moveList.end(), { "A", "R", "L", "D", "RA", "LA", "UL", "UR", "UA" });
 if (*gamePhase >  0x0000) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA" });

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

  // Evaluating magnets

  // Container for bounded value and difference with center
  float boundedValue = 0.0;
  float diff = 0.0;

  // Evaluating car magnet's reward on position X
  boundedValue = *carPosX;
  boundedValue = std::min(boundedValue, magnets.carHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.carHorizontalMagnet.min);
  diff = std::abs(magnets.carHorizontalMagnet.center - boundedValue);
  reward += magnets.carHorizontalMagnet.intensity * -diff;

  reward += magnets.lapProgressMagnet * lapProgress;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer / Phase:                 %03u / %03u\n", *gameTimer, *gamePhase);
 LOG("[Jaffar]  + Lap Progress:                       %f (%03d, %03d)\n", lapProgress, *lapProgress1, *lapProgress2);
 LOG("[Jaffar]  + Player Position:                    (%03u, %03u, %03u)\n", *playerPos1, *playerPos2, *playerPos3);
 LOG("[Jaffar]  + Traffic Light:                      (%03u)\n", *trafficLightTimer);
 LOG("[Jaffar]  + Car RPM:                            (%03u)\n", *carRPM);
 LOG("[Jaffar]  + Car Speed:                          (%03u)\n", *carSpeed);
 LOG("[Jaffar]  + Car Gear:                           (%03u)\n", *carGear);
 LOG("[Jaffar]  + Car Tire Damage:                    (%03u)\n", *carTireDamage);
 LOG("[Jaffar]  + Car Pos X:                          (%03u)\n", *carPosX);
 LOG("[Jaffar]  + Car Tire Angle:                     (%03u)\n", *carTireAngle);
 LOG("[Jaffar]  + Car Turn State:                     (%03u, %03u)\n", *carTurnState1, *carTurnState2);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.carHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Car Horizontal Magnet              - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.carHorizontalMagnet.intensity, magnets.carHorizontalMagnet.center, magnets.carHorizontalMagnet.min, magnets.carHorizontalMagnet.max);
 if (std::abs(magnets.lapProgressMagnet) > 0.0f)                 LOG("[Jaffar]  + Lap Progress Magnet                - Intensity: %.5f\n", magnets.lapProgressMagnet);
}

