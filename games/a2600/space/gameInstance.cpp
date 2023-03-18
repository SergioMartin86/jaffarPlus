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

  gameMode            = (uint8_t*) &_emu->_ram[0x01];
  gameTimer           = (uint8_t*) &_emu->_ram[0x4A];
  score1              = (uint8_t*) &_emu->_ram[0x66];
  score2              = (uint8_t*) &_emu->_ram[0x68];
  bullet1PosY         = (uint8_t*) &_emu->_ram[0x55];
  bullet2PosY         = (uint8_t*) &_emu->_ram[0x56];

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
   if (i != 0x01)
   if (i != 0x08)
   if (i != 0x4A)
   if (i != 0x70)
  {
    hash.Update(_emu->_ram[i]);
  }

  //hash.Update(_emu->_ram, 0x80);

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
 score = (double)(*score1 * 100 / 160) * 100.0 + (double)(*score2 * 100 / 160);
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
  if (*bullet1PosY == 0xF6 || *bullet2PosY == 0xF6) return { ".", "L", "R", "B" };
  return { ".", "L", "R" };
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

 reward += score;

 reward -= 0.0001 * *bullet1PosY;
 reward -= 0.0001 * *bullet2PosY;

 // Returning reward
 return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Mode:              %u\n", *gameMode);
 LOG("[Jaffar]  + Game Timer:             %u\n", *gameTimer);
 LOG("[Jaffar]  + Score:                  %f (%u %u)\n", score, *score1, *score2);
 LOG("[Jaffar]  + Bullet 1 Pos Y:         %u\n", *bullet1PosY);
 LOG("[Jaffar]  + Bullet 2 Pos Y:         %u\n", *bullet2PosY);
}

