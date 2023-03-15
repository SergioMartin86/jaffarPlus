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

  gameTimer           = (uint8_t*) &_emu->_ram[0x00];
  playerPosX          = (uint8_t*) &_emu->_ram[0x20];
  playerPosY          = (uint8_t*) &_emu->_ram[0x22];
  oppPosX             = (uint8_t*) &_emu->_ram[0x21];
  oppPosY             = (uint8_t*) &_emu->_ram[0x23];
  playerPunching      = (uint8_t*) &_emu->_ram[0x63];
  score               = (uint8_t*) &_emu->_ram[0x12];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(*gameTimer % (timerTolerance+1));

//  hash.Update(_emu->_ram, 0x80);

  for (uint8_t i = 0; i < 0x80; i++)
  if (i != 0x00)
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
 distance = std::abs(*playerPosX - *oppPosX) + std::abs(*playerPosY - *oppPosY);
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
  std::vector<std::string> moveList = { "." };

  moveList.insert(moveList.end(), { "L", "R", "D", "B", "U", "DR", "UR", "UB", "RB", "URB", "DRB" });

  return moveList;
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

 reward += *score * 10000;
 reward -= distance;
 reward += (float)*playerPosX * 0.1;
 reward += (float)*oppPosX * 0.1;

 // Returning reward
 return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:             %u\n", *gameTimer);
 LOG("[Jaffar]  + Score:                  %u\n", *score);
 LOG("[Jaffar]  + Player Pos X:           %u\n", *playerPosX);
 LOG("[Jaffar]  + Player Pos Y:           %u\n", *playerPosY);
 LOG("[Jaffar]  + Opp Pos X:              %u\n", *oppPosX);
 LOG("[Jaffar]  + Opp Pos Y:              %u\n", *oppPosY);
 LOG("[Jaffar]  + Distance:               %f\n", distance);
 LOG("[Jaffar]  + Player Punching:        %u\n", *playerPunching);
}

