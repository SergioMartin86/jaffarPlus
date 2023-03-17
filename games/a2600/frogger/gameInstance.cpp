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

  gameStarted         = (uint8_t*) &_emu->_ram[0x6F];
  currentStage        = (uint8_t*) &_emu->_ram[0x52];
  gameTimer           = (uint8_t*) &_emu->_ram[0x5B];
  waitTimer           = (uint8_t*) &_emu->_ram[0x61];
  playerState         = (uint8_t*) &_emu->_ram[0x50];
  playerPosX          = (uint8_t*) &_emu->_ram[0x30];
  playerPosY          = (uint8_t*) &_emu->_ram[0x5A];
  score1              = (uint8_t*) &_emu->_ram[0x4C];
  score2              = (uint8_t*) &_emu->_ram[0x4E];
  filledPlaces        = (uint8_t*) &_emu->_ram[0x63];
  musicTimer1         = (uint8_t*) &_emu->_ram[0x56];
  musicTimer2         = (uint8_t*) &_emu->_ram[0x59];
  startDelay          = (uint8_t*) &_emu->_ram[0x5F];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(*gameTimer % (timerTolerance+1));

  if (*gameStarted == 0) hash.Update(*gameTimer);
  if (*gameStarted == 0) hash.Update(*gameTimer);
  if (*gameStarted == 0) hash.Update(*musicTimer1);
  if (*gameStarted == 0) hash.Update(*musicTimer2);

  hash.Update(_emu->_ram, 52);

  hash.Update(*gameStarted);
  hash.Update(*currentStage);
  hash.Update(*filledPlaces);
  hash.Update(*playerState);
  hash.Update(*waitTimer);
  hash.Update(*musicTimer1);
  hash.Update(*musicTimer2);
  hash.Update(*playerPosX);
  hash.Update(*playerPosY);
  hash.Update(*score1);
  hash.Update(*score2);
  hash.Update(*startDelay);

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
 score = (double)*score1 * 153.0 + (double)*score2;

 slot1 = *filledPlaces & 1;
 slot2 = *filledPlaces & 2;
 slot3 = *filledPlaces & 4;
 slot4 = *filledPlaces & 8;
 slot5 = *filledPlaces & 16;
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
  if (*gameStarted == 0 && *gameTimer < 0) return { "." };
  if (*gameStarted == 0) return { ".", "r" };
  if (*musicTimer2 != 255) return { "." };
  if (*gameTimer % 2 == 0) return { ".", "L", "R", "D", "U" };
//  if (*gameTimer % 2 == 0) moveList.insert(moveList.end(), { "LR", "DR", "DL", "UR", "UL", "UD", "DLR", "ULR", "UDR", "UDL", "UDLR" });

  return { "." };
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

 reward += *currentStage * 10000.0;

 if (*musicTimer2 == 255) reward += *playerPosY;
 if (*musicTimer2 == 255) reward += slot1 ? 1000.0 : 0;
 if (*musicTimer2 == 255) reward += slot2 ? 1000.0 : 0;
 if (*musicTimer2 == 255) reward += slot3 ? 1000.0 : 0;
 if (*musicTimer2 == 255) reward += slot4 ? 1000.0 : 0;
 if (*musicTimer2 == 255) reward += slot5 ? 1000.0 : 0;
 if (*musicTimer2 != 255) reward += *musicTimer1 * 0.001;
 if (*musicTimer2 != 255) reward -= *musicTimer2 * 0.000001;

 // Returning reward
 return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Current Stage:          %u\n", *currentStage);
 LOG("[Jaffar]  + Game Timer:             %u\n", *gameTimer);
 LOG("[Jaffar]  + Wait Timer:             %u\n", *waitTimer);
 LOG("[Jaffar]  + Music Timers:           %u %u\n", *musicTimer1, *musicTimer2);
 LOG("[Jaffar]  + Score:                  %f (%u %u)\n", score, *score1, *score2);
 LOG("[Jaffar]  + Filled Places:          %1u %1u %1u %1u %1u (%u)\n", slot1, slot2, slot3, slot4, slot5, *filledPlaces);
 LOG("[Jaffar]  + Player State:           %u\n", *playerState);
 LOG("[Jaffar]  + Player Pos X:           %u\n", *playerPosX);
 LOG("[Jaffar]  + Player Pos Y:           %u\n", *playerPosY);
}

