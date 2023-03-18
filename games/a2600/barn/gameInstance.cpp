#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  gameMode            = (uint8_t*) &_emu->_ram[0x24];
  gameTimer           = (uint8_t*) &_emu->_ram[0x37];
  chronoTimer1        = (uint8_t*) &_emu->_ram[0x38];
  chronoTimer2        = (uint8_t*) &_emu->_ram[0x41];
  playerPosX1         = (uint8_t*) &_emu->_ram[0x73];
  playerPosX2         = (uint8_t*) &_emu->_ram[0x5D];
  playerPosY          = (uint8_t*) &_emu->_ram[0x0F];
  score               = (uint8_t*) &_emu->_ram[0x53];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(*gameTimer % (timerTolerance+1));

  for (size_t i = 0; i < 0x80; i++)
   if (i != 0x37)
   if (i != 0x38)
   if (i != 0x41)
  {
    hash.Update(_emu->_ram[i]);
  }

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
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

void GameInstance::updateDerivedValues()
{
 playerPosX = 1600 - 160.0f * (float) *playerPosX1 + (float) *playerPosX2;
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
  return { ".", "l", "L", "R", "D", "U", "B", "DB", "UB" };
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

 reward -= *score;
 if (*playerPosX1 == 0 && *playerPosX2 == 0) reward -= 100000.0;
 reward -= 0.00001 * std::abs(playerPosX - 1478.0);

 // Returning reward
 return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:             %u\n", *gameTimer);
 LOG("[Jaffar]  + Game Mode:              %u\n", *gameMode);
 LOG("[Jaffar]  + Chrono:                 %u %u\n", *chronoTimer1, *chronoTimer2);
 LOG("[Jaffar]  + Score:                  %u\n", *score);
 LOG("[Jaffar]  + Player Pos X:           %f (%u %u)\n", playerPosX, *playerPosX1, *playerPosX2);
 LOG("[Jaffar]  + Player Pos Y:           %u\n", *playerPosY);
}

