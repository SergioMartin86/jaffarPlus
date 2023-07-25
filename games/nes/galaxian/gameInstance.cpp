#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  gameMode    = (uint8_t*)  &_emu->_baseMem[0x0044];
  gameTimer   = (uint8_t*)  &_emu->_baseMem[0x0104];
  shotCharge  = (uint8_t*)  &_emu->_baseMem[0x0044];
  shotPosX    = (uint8_t*)  &_emu->_baseMem[0x0283];
  shotPosY    = (uint8_t*)  &_emu->_baseMem[0x0280];
  shipPosX    = (uint8_t*)  &_emu->_baseMem[0x00E4];
  score10e1   = (uint8_t*)  &_emu->_baseMem[0x011B];
  score10e2   = (uint8_t*)  &_emu->_baseMem[0x011A];
  score10e3   = (uint8_t*)  &_emu->_baseMem[0x0119];
  score10e4   = (uint8_t*)  &_emu->_baseMem[0x0118];
  score10e5   = (uint8_t*)  &_emu->_baseMem[0x0117];
  score10e6   = (uint8_t*)  &_emu->_baseMem[0x0116];
  shipState   = (uint8_t*)  &_emu->_baseMem[0x0041];
  flagCount   = (uint8_t*)  &_emu->_baseMem[0x0059];

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

  // If timer tolerance is set, use the game tick for hashing
  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));

  hash.Update(*gameMode  );
  hash.Update(*gameTimer );
  hash.Update(*shotCharge);
  hash.Update(*shotPosX  );
  hash.Update(*shotPosY  );
  hash.Update(*shipPosX  );
  hash.Update(*score10e1 );
  hash.Update(*score10e2 );
  hash.Update(*score10e3 );
  hash.Update(*score10e4 );
  hash.Update(*score10e5 );
  hash.Update(*score10e6 );
  hash.Update(*shipState );
  hash.Update(*flagCount );

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 killedEnemies = 0;

 double _score10e1 = *score10e1 == 16 ? 0 : (double)*score10e1 * 10.0;
 double _score10e2 = *score10e2 == 16 ? 0 : (double)*score10e2 * 100.0;
 double _score10e3 = *score10e3 == 16 ? 0 : (double)*score10e3 * 1000.0;
 double _score10e4 = *score10e4 == 16 ? 0 : (double)*score10e4 * 10000.0;
 double _score10e5 = *score10e5 == 16 ? 0 : (double)*score10e5 * 100000.0;
 double _score10e6 = *score10e6 == 16 ? 0 : (double)*score10e6 * 1000000.0;
 score = _score10e1 + _score10e2 + _score10e3 + _score10e4 + _score10e5 + _score10e6;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 moves.push_back(move);
 updateDerivedValues();

 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
  if (*shotPosY == 200) return { "L", "R", "LB", "RB" };
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

  reward += score;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                           %.10f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Mode:                        %2u\n", *gameMode  );
 LOG("[Jaffar]  + Game Timer:                       %2u\n", *gameTimer );
 LOG("[Jaffar]  + Score:                            %f\n",  score );
 LOG("[Jaffar]  + Killed Enemies:                   %u\n",  killedEnemies );
 LOG("[Jaffar]  + Shot Charge:                      %2u\n", *shotCharge);
 LOG("[Jaffar]  + Shot Pos X:                       %2u\n", *shotPosX  );
 LOG("[Jaffar]  + Shot Pos Y:                       %2u\n", *shotPosY  );
 LOG("[Jaffar]  + Ship Pos X:                       %2u\n", *shipPosX  );
 LOG("[Jaffar]  + Ship State:                       %2u\n", *shipState );
 LOG("[Jaffar]  + Flag Count:                       %2u\n", *flagCount );

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");
}

