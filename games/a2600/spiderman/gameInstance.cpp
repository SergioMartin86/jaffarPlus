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

  gameTimer           = (uint8_t*) &_emu->_ram[0x14];
  playerState         = (uint8_t*) &_emu->_ram[0x0E];
  verticalScroll1     = (uint8_t*) &_emu->_ram[0x19];
  verticalScroll2     = (uint8_t*) &_emu->_ram[0x1D];
  gameState           = (uint8_t*) &_emu->_ram[0x69];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(*gameTimer % (timerTolerance+1));

  hash.Update(_emu->_ram + 0x00, 0x13);
  hash.Update(_emu->_ram + 0x15, 0x6B);

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
 verticalScroll = (float)*verticalScroll1 * 25 + (float)*verticalScroll2;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 updateDerivedValues();

 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
  std::vector<std::string> moveList = { "." };

  if (*gameTimer % 2 == 0) return moveList; // The game does not read input on even time

  if (*playerState == 0x0000) moveList.insert(moveList.end(), { "RB", "LB", "DB", "UB" });
  if (*playerState == 0x0002) moveList.insert(moveList.end(), { "B" });
  if (*playerState == 0x0005) moveList.insert(moveList.end(), { "B" });
  if (*playerState == 0x0008) moveList.insert(moveList.end(), { "B" });
  if (*playerState == 0x0010) moveList.insert(moveList.end(), { "B" });
  if (*playerState == 0x0082) moveList.insert(moveList.end(), { "B", "D", "U" });
  if (*playerState == 0x0083) moveList.insert(moveList.end(), { "B", "D", "U" });
  if (*playerState == 0x0084) moveList.insert(moveList.end(), { "B", "D", "U" });
  if (*playerState == 0x0085) moveList.insert(moveList.end(), { "B", "D", "U" });
  if (*playerState == 0x0088) moveList.insert(moveList.end(), { "B", "D", "U" });
  if (*playerState == 0x0090) moveList.insert(moveList.end(), { "B", "D", "U" });

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

 // Container for bounded value and difference with center
 float diff = 0.0;

 // Evaluating simon magnet's reward on position X
 diff = std::abs(magnets.verticalScrollMagnet.center - verticalScroll);
 reward += magnets.verticalScrollMagnet.intensity * -diff;

 // Returning reward
 return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:             %u\n", *gameTimer);
 LOG("[Jaffar]  + Game State:             %u\n", *gameState);
 LOG("[Jaffar]  + Player State:           %u\n", *playerState);
 LOG("[Jaffar]  + Vertical Scroll:        %f (%02u/%02u)\n", verticalScroll, *verticalScroll1, *verticalScroll2, *gameTimer);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 if (std::abs(magnets.verticalScrollMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Vertical Scroll Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.verticalScrollMagnet.intensity, magnets.verticalScrollMagnet.center);
}

