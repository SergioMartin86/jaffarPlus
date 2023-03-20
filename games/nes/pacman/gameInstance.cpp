#include "gameInstance.hpp"
#include "gameRule.hpp"

inline int getPelletOffset(const uint8_t row, const uint8_t column) { return column * 0x20 + row; }
inline uint8_t getPelletPosX(const uint8_t column) { return 24 + 6 * column; }
inline uint8_t getPelletPosY(const uint8_t row) { return 16 + 8 * row; }

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting path
  pelletPath = {
    {19,8},
    {18,8},
    {17,8},
    {17,7},
    {17,6},
    {17,5},
    {17,4},
    {16,4},
    {15,4},
    {14,4},
    {13,4},
    {12,4},
    {11,4},
    {10,4},
    {9,4},
    {8,4},
    {7,4},
    {7,3},
    {7,2},
    {7,1},
    {7,0},
    {6,0},
    {5,0},
    {6,4},
    {5,4},
    {4,4},
    {4,3},
    {4,2},
    {4,1},
    {4,0},
    {3,0},
    {2,0},
    {1,0},
    {0,0},
    {0,1},
    {0,2},
    {0,3},
    {0,4},
    {1,4},
    {2,4},
    {3,4},
    {5,5},
    {5,6},
    {6,6},
    {7,6},
    {7,7},
    {7,8}};

  // Thanks to https://datacrystal.romhacking.net/wiki/Pac-Man:RAM_map for helping me find some of these items
  globalTimer         = (uint8_t*)  &_emu->_baseMem[0x004B];
  playerPosX1         = (uint8_t*)  &_emu->_baseMem[0x001A];
  playerPosX2         = (uint8_t*)  &_emu->_baseMem[0x001B];
  playerPosY1         = (uint8_t*)  &_emu->_baseMem[0x001C];
  playerPosY2         = (uint8_t*)  &_emu->_baseMem[0x001D];
  playerFrame         = (uint8_t*)  &_emu->_baseMem[0x0032];
  playerDirection     = (uint8_t*)  &_emu->_baseMem[0x0050];
  currentLevel        = (uint8_t*)  &_emu->_baseMem[0x0068];
  remainingPellets    = (uint8_t*)  &_emu->_baseMem[0x006A];
  ghost0PosX1         = (uint8_t*)  &_emu->_baseMem[0x001E];
  ghost0PosX2         = (uint8_t*)  &_emu->_baseMem[0x001F];
  ghost1PosX1         = (uint8_t*)  &_emu->_baseMem[0x0022];
  ghost1PosX2         = (uint8_t*)  &_emu->_baseMem[0x0023];
  ghost2PosX1         = (uint8_t*)  &_emu->_baseMem[0x0026];
  ghost2PosX2         = (uint8_t*)  &_emu->_baseMem[0x0027];
  ghost3PosX1         = (uint8_t*)  &_emu->_baseMem[0x002A];
  ghost3PosX2         = (uint8_t*)  &_emu->_baseMem[0x002B];
  ghost0PosY1         = (uint8_t*)  &_emu->_baseMem[0x0020];
  ghost0PosY2         = (uint8_t*)  &_emu->_baseMem[0x0021];
  ghost1PosY1         = (uint8_t*)  &_emu->_baseMem[0x0024];
  ghost1PosY2         = (uint8_t*)  &_emu->_baseMem[0x0025];
  ghost2PosY1         = (uint8_t*)  &_emu->_baseMem[0x0028];
  ghost2PosY2         = (uint8_t*)  &_emu->_baseMem[0x0029];
  ghost3PosY1         = (uint8_t*)  &_emu->_baseMem[0x002C];
  ghost3PosY2         = (uint8_t*)  &_emu->_baseMem[0x002D];

  pelletMap           = (uint8_t*)  &_emu->_ppuNameTableMem[0x0062];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  // If timer tolerance is set, use the game tick for hashing
  if (timerTolerance > 0) hash.Update(*globalTimer % (timerTolerance+1));

  hash.Update(*playerPosX1);
  hash.Update(*playerPosX2);
  hash.Update(*playerPosY1);
  hash.Update(*playerPosY2);
  hash.Update(*playerFrame);
  hash.Update(*playerDirection);
  hash.Update(*currentLevel);
  hash.Update(*remainingPellets);

  hash.Update(*ghost0PosX1);
  hash.Update(*ghost0PosX2);
  hash.Update(*ghost1PosX1);
  hash.Update(*ghost1PosX2);
  hash.Update(*ghost2PosX1);
  hash.Update(*ghost2PosX2);
  hash.Update(*ghost3PosX1);
  hash.Update(*ghost3PosX2);

  hash.Update(*ghost0PosY1);
  hash.Update(*ghost0PosY2);
  hash.Update(*ghost1PosY1);
  hash.Update(*ghost1PosY2);
  hash.Update(*ghost2PosY1);
  hash.Update(*ghost2PosY2);
  hash.Update(*ghost3PosY1);
  hash.Update(*ghost3PosY2);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 // Recalculating derived values
 playerPosX = (float) *playerPosX1 + ((float) *playerPosX2) / 256.0;
 playerPosY = (float) *playerPosY1 + ((float) *playerPosY2) / 256.0;
 ghost0PosY = (float) *ghost0PosY1 + ((float) *ghost0PosY2) / 256.0;
 ghost1PosY = (float) *ghost1PosY1 + ((float) *ghost1PosY2) / 256.0;
 ghost2PosY = (float) *ghost2PosY1 + ((float) *ghost2PosY2) / 256.0;
 ghost3PosY = (float) *ghost3PosY1 + ((float) *ghost3PosY2) / 256.0;

 targetPelletsTaken = 0;
 for (size_t i = 0; i < pelletPath.size(); i++)
 {
  targetPelletRow = pelletPath[i][1];
  targetPelletCol = pelletPath[i][0];
  targetPelletPosX = (float)getPelletPosX(targetPelletCol);
  targetPelletPosY = (float)getPelletPosY(targetPelletRow);

  auto pelletOffset = getPelletOffset(targetPelletRow, targetPelletCol);
  auto pelletValue = pelletMap[pelletOffset];

  if (pelletValue == 7 || pelletValue == 8) targetPelletsTaken++;
  if (pelletValue != 7 && pelletValue != 8) { targetPelletDistance = std::abs((float) *playerPosX1 - targetPelletPosX) + std::abs((float) *playerPosY1 - targetPelletPosY); break; }
 }
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
  std::vector<std::string> moveList = { };

  uint8_t relPosX = *playerPosX1 - 24;
  uint8_t relPosY = *playerPosY1 - 16;

  if (*playerDirection == DIRECTION_LEFT)
  {
   moveList.insert(moveList.end(), { ".", "R" });
    if (relPosX % 16 == 0 || relPosX % 16 == 1) moveList.insert(moveList.end(), { "U", "D" });
  }

  if (*playerDirection == DIRECTION_RIGHT)
  {
   moveList.insert(moveList.end(), { ".", "L" });
    if (relPosX % 16 == 0 || relPosX % 16 == 15) moveList.insert(moveList.end(), { "U", "D" });
  }

  if (*playerDirection == DIRECTION_UP)
  {
   moveList.insert(moveList.end(), { ".", "D" });
    if (relPosY % 8 == 0 || relPosY % 8 == 1) moveList.insert(moveList.end(), { "L", "R" });
  }

  if (*playerDirection == DIRECTION_DOWN)
  {
   moveList.insert(moveList.end(), { ".", "U" });
    if (relPosY % 8 == 0 || relPosY % 8 == 7) moveList.insert(moveList.end(), { "L", "R" });
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

  reward += targetPelletsTaken * 1000.0;
  reward -= *remainingPellets;
  reward -= targetPelletDistance;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Global Timer:            %u\n", *globalTimer);
 LOG("[Jaffar]  + Current Stage:           %2u\n", *currentLevel);
 LOG("[Jaffar]  + Reward:                  %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                    0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Remaining Pellets:       %u\n", *remainingPellets);
 LOG("[Jaffar]  + Player Direction:        %02u\n", *playerDirection);
 LOG("[Jaffar]  + Player Pos X:            %f (%02u %02u)\n", playerPosX, *playerPosX1, *playerPosX2);
 LOG("[Jaffar]  + Player Pos Y:            %f (%02u %02u)\n", playerPosY, *playerPosY1, *playerPosY2);
 LOG("[Jaffar]  + Ghost 0 Pos X:           %f (%02u %02u)\n", ghost0PosX, *ghost0PosX1, *ghost0PosX2);
 LOG("[Jaffar]  + Ghost 0 Pos Y:           %f (%02u %02u)\n", ghost0PosY, *ghost0PosY1, *ghost0PosY2);
 LOG("[Jaffar]  + Ghost 1 Pos X:           %f (%02u %02u)\n", ghost1PosX, *ghost1PosX1, *ghost1PosX2);
 LOG("[Jaffar]  + Ghost 1 Pos Y:           %f (%02u %02u)\n", ghost1PosY, *ghost1PosY1, *ghost1PosY2);
 LOG("[Jaffar]  + Ghost 2 Pos X:           %f (%02u %02u)\n", ghost2PosX, *ghost2PosX1, *ghost2PosX2);
 LOG("[Jaffar]  + Ghost 2 Pos Y:           %f (%02u %02u)\n", ghost2PosY, *ghost2PosY1, *ghost2PosY2);
 LOG("[Jaffar]  + Ghost 3 Pos X:           %f (%02u %02u)\n", ghost3PosX, *ghost3PosX1, *ghost3PosX2);
 LOG("[Jaffar]  + Ghost 3 Pos Y:           %f (%02u %02u)\n", ghost3PosY, *ghost3PosY1, *ghost3PosY2);
 LOG("[Jaffar]  + Target Pellets Taken:    %02u\n", targetPelletsTaken);
 LOG("[Jaffar]  + Target Pellet Pos:       %02u, %02u\n", targetPelletRow, targetPelletCol);
 LOG("[Jaffar]  + Target Pellet Distance:  %f\n", targetPelletDistance);
 LOG("[Jaffar]  + Pellet Map:\n");

 LOG("         ");
 for (int j = 0; j < COL_COUNT; j++) LOG("%02u  ", j);
 LOG("\n");

 for (int i = 0; i < ROW_COUNT; i++)
 {
  LOG("    %02u : ", i);
  for (int j = 0; j < COL_COUNT; j++)
  {
   int pelletValue = pelletMap[getPelletOffset(j, i)];
   char symbol = ' ';
   if (pelletValue == 1 || pelletValue == 2) symbol = 'O';
   if (pelletValue == 3 || pelletValue == 9) symbol = 'o';
   if (pelletValue == 7 || pelletValue == 8) symbol = '.';
   if (targetPelletRow == j && targetPelletCol == i) symbol = 'X';
   LOG("%c   ", symbol);
  }
  LOG("\n");
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
}

