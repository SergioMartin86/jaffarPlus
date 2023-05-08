#include "gameInstance.hpp"
#include "gameRule.hpp"

inline int getPelletOffset(const uint8_t row, const uint8_t column) { return row * 0x20 + column; }
inline uint8_t getPelletPosX(const uint8_t column) { return 24 + 8 * column; }
inline uint8_t getPelletPosY(const uint8_t row) { return 16 + 8 * row; }

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting path
//  pelletPath = {
//    {19,8},{18,8},{17,8},
//    {17,7},{17,6},{17,5},{17,4},{16,4},{15,4},{14,4},{13,4},{12,4},{11,4},{10,4},{9,4},{8,4},{7,4},
//    {7,3},{7,2},{7,1},{7,0},
//    {6,0},{5,0},{6,4},{5,4},{4,4},{4,3},{4,2},{4,1},{4,0},
//    {3,0},{2,0},{1,0},{0,0},
//    {0,1},{0,2},{0,3},{0,4},
//    {1,4},{2,4},{3,4},{4,4},
//    {4,5},{4,6},
//    {5,6},{6,6},{7,6},
//    {7,7},{7,8},
//    {4,7},{4,8},
//    {3,8},{2,8},{1,8},{0,8},
//    {0,7},{0,6},{0,5},{4,9},
//    {4,10},{3,10},{2,10},{1,10},{0,10},
//    {0,11},{0,12},{0,13},
//    {4,11},{4,12},
//    {5,12},{6,12},{7,12},
//    {7,11},{7,10},
//    {4,13},{4,14},
//    {3,14},{2,14},{1,14},{0,14},
//    {0,15},{0,16},{0,17},{0,18},
//    {1,18},{2,18},{3,18},{4,18},
//    {4,17},{4,16},{4,15},
//    {5,14},{6,14},{7,14},
//    {7,15},{7,16},{7,17},{7,18},
//    {6,18},{5,18},
//    {8,14},{9,14},{10,14},{11,14},{12,14},{13,14},{14,14},{15,14},{16,14},{17,14},
//    {17,13},{17,12},{17,11},{17,10},
//    {18,10},{19,10},
//    {19,11},{19,12},{19,13},{19,14},
//    {18,14},{17,15},
//    {17,16},{17,17},{17,18},
//    {18,18},{19,18},
//    {19,17},{19,16},
//    {20,16},{21,16},{22,16},
//    {22,15},{22,14},
//    {21,14},{20,14},
//    {22,17},{22,18},
//    {23,18},{24,18},
//    {24,17},{24,16},{24,15},{24,14},{24,13},{24,12},{24,11},{24,10},{24,9},{24,8},
//    {23,8},{22,8},{22,7},{22,6},{21,6},{20,6},
//    {24,7},{24,6},{24,5},{24,4},{24,3},{24,2},{24,1},{24,0},
//    {23,0},{22,0},
//    {22,1},{22,2},{22,03},{22,4},
//    {21,4},{20,4},
//    {21,2},{20,2},{19,2},
//    {19,1},{19,0},
//    {18,0},{17,0},
//    {17,1},{17,2},{17,3},
//    {18,4},{19,4},
//    {19,5},{19,6},{19,7},
//    {20,12},{21,12},{22,12},
//    {22,11},{22,10},
//    {23,10}
//  };

  pelletPath = {
    {19, 8}, {18, 8}, {17, 8}, {17, 7}, {17, 6}, {17, 5}, {17, 4}, {17, 3}, {17, 2}, {17, 1}, {17, 0}, {18, 0}, {19, 0}, {19, 1}, {19, 2}, {20, 2}, {21, 2}, {22, 2}, {22, 3}, {22, 1}, {22, 0}, {23, 0}, {24, 0}, {24, 1}, {24, 2}, {24, 3}, {24, 4}, {24, 5}, {24, 6}, {24, 7}, {24, 8}, {23, 8}, {22, 8}, {22, 7}, {22, 6}, {21, 6}, {20, 6}, {19, 6}, {19, 7}, {19, 5}, {19, 4}, {20, 4}, {21, 4}, {22, 4}, {18, 4}, {16, 4}, {15, 4}, {14, 4}, {13, 4}, {12, 4}, {11, 4}, {10, 4}, {9, 4}, {8, 4}, {7, 4}, {6, 4}, {7, 3}, {7, 2}, {7, 1}, {7, 0}, {6, 0}, {5, 0}, {4, 0}, {4, 1}, {4, 2}, {3, 0}, {2, 0}, {1, 0}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {4, 9}, {4, 7}, {4, 6}, {4, 5}, {4, 4}, {4, 3}, {3, 4}, {2, 4}, {5, 4}, {5, 6}, {6, 6}, {7, 6}, {7, 7}, {7, 8}, {7, 10}, {7, 11}, {7, 12}, {6, 12}, {5, 12}, {4, 12}, {4, 11}, {4, 10}, {3, 10}, {2, 10}, {1, 10}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {1, 14}, {2, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {1, 18}, {2, 18}, {3, 18}, {4, 18}, {4, 17}, {4, 16}, {4, 15}, {4, 14}, {4, 13}, {3, 14}, {5, 14}, {5, 18}, {6, 18}, {7, 18}, {7, 17}, {7, 16}, {7, 15}, {7, 14}, {6, 14}, {8, 14}, {9, 14}, {10, 14}, {11, 14}, {12, 14}, {13, 14}, {14, 14}, {15, 14}, {16, 14}, {17, 14}, {17, 15}, {17, 16}, {17, 17}, {17, 18}, {18, 18}, {19, 18}, {19, 17}, {19, 16}, {20, 16}, {21, 16}, {22, 16}, {22, 15}, {22, 17}, {22, 18}, {23, 18}, {24, 18}, {24, 17}, {24, 16}, {24, 15}, {24, 14}, {24, 13}, {24, 12}, {24, 11}, {24, 10}, {24, 9}, {23, 10}, {22, 10}, {22, 11}, {22, 12}, {21, 12}, {20, 12}, {19, 12}, {19, 11}, {19, 10}, {18, 10}, {17, 10}, {17, 11}, {17, 12}, {17, 13}, {18, 14}, {19, 14}, {19, 13}, {20, 14}, {21, 14}, {22, 14}
  };

  // Thanks to https://datacrystal.romhacking.net/wiki/Pac-Man:RAM_map for helping me find some of these items
  globalTimer         = (uint8_t*)  &_emu->_baseMem[0x004B];
  playerPosX1         = (uint8_t*)  &_emu->_baseMem[0x001A];
  playerPosX2         = (uint8_t*)  &_emu->_baseMem[0x001B];
  playerPosY1         = (uint8_t*)  &_emu->_baseMem[0x001C];
  playerPosY2         = (uint8_t*)  &_emu->_baseMem[0x001D];
  playerFrame         = (uint8_t*)  &_emu->_baseMem[0x0032];
  playerState         = (uint8_t*)  &_emu->_baseMem[0x003F];
  playerDirection1    = (uint8_t*)  &_emu->_baseMem[0x0050];
  playerDirection2    = (uint8_t*)  &_emu->_baseMem[0x0051];
  currentLevel        = (uint8_t*)  &_emu->_baseMem[0x0093];
  currentDifficulty   = (uint8_t*)  &_emu->_baseMem[0x0068];
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

  ghost0Direction     = (uint8_t*)  &_emu->_baseMem[0x00BA];
  ghost1Direction     = (uint8_t*)  &_emu->_baseMem[0x00BB];
  ghost2Direction     = (uint8_t*)  &_emu->_baseMem[0x00BC];
  ghost3Direction     = (uint8_t*)  &_emu->_baseMem[0x00BD];

  captureCount        = (uint8_t*)  &_emu->_baseMem[0x00D9];
  captureCountPrev    = (uint8_t*)  &_emu->_baseMem[0x07F0];
  captureCountTotal   = (uint8_t*)  &_emu->_baseMem[0x07F1];
  captureCountReward  = (float*)    &_emu->_baseMem[0x07F2];

  score1              = (uint8_t*)  &_emu->_baseMem[0x0070];
  score2              = (uint8_t*)  &_emu->_baseMem[0x0071];
  score3              = (uint8_t*)  &_emu->_baseMem[0x0072];
  score4              = (uint8_t*)  &_emu->_baseMem[0x0073];
  score5              = (uint8_t*)  &_emu->_baseMem[0x0074];

  ghost0State         = (uint8_t*)  &_emu->_baseMem[0x00C2];
  ghost1State         = (uint8_t*)  &_emu->_baseMem[0x00C4];
  ghost2State         = (uint8_t*)  &_emu->_baseMem[0x00C6];
  ghost3State         = (uint8_t*)  &_emu->_baseMem[0x00C8];

  pelletMap           = (uint8_t*)  &_emu->_ppuNameTableMem[0x0062];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Disable Ghosts
  if (isDefined(config, "Disable Ghosts") == true)
   disableGhosts = config["Disable Ghosts"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Disable Ghosts' was not defined\n");

  // Skip Intermission
  if (isDefined(config, "Skip Intermission") == true)
   skipIntermission = config["Skip Intermission"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Skip Intermission' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  //if (skipIntermission) hash.Update(_emu->_baseMem, 0x800);
  if (skipIntermission) hash.Update(*globalTimer);

  // If timer tolerance is set, use the game tick for hashing
  if (timerTolerance > 0) hash.Update(*globalTimer % (timerTolerance+1));

  // While 'eating' a ghost, allow time to pass
  if (*playerFrame == 0) hash.Update(*globalTimer);

  hash.Update(*playerPosX1);
  hash.Update(*playerPosX2);
  hash.Update(*playerPosY1);
  hash.Update(*playerPosY2);
  hash.Update(*playerFrame);
  hash.Update(*playerDirection1);
  hash.Update(*playerDirection2);
  hash.Update(*currentLevel);
  hash.Update(*currentDifficulty);
  hash.Update(*remainingPellets);
  hash.Update(*playerState);

  hash.Update(*score1);
  hash.Update(*score2);
  hash.Update(*score3);
  hash.Update(*score4);
  hash.Update(*score5);

  hash.Update(*ghost0PosX1);
//  hash.Update(*ghost0PosX2);
  hash.Update(*ghost1PosX1);
//  hash.Update(*ghost1PosX2);
  hash.Update(*ghost2PosX1);
//  hash.Update(*ghost2PosX2);
  hash.Update(*ghost3PosX1);
//  hash.Update(*ghost3PosX2);

  hash.Update(*ghost0PosY1);
//  hash.Update(*ghost0PosY2);
  hash.Update(*ghost1PosY1);
//  hash.Update(*ghost1PosY2);
  hash.Update(*ghost2PosY1);
//  hash.Update(*ghost2PosY2);
  hash.Update(*ghost3PosY1);
//  hash.Update(*ghost3PosY2);

  hash.Update(*ghost0Direction);
  hash.Update(*ghost1Direction);
  hash.Update(*ghost2Direction);
  hash.Update(*ghost3Direction);

  hash.Update(*ghost0State);
  hash.Update(*ghost1State);
  hash.Update(*ghost2State);
  hash.Update(*ghost3State);

  hash.Update(*captureCount);
  hash.Update(*captureCountTotal);

  for (int i = 0; i < ROW_COUNT; i++)
   for (int j = 0; j < COL_COUNT; j++)
    hash.Update(pelletMap[getPelletOffset(i, j)]);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 // Disabling ghosts if required
 if (disableGhosts == true)
 {
  *ghost0PosX1 = 250;
  *ghost1PosX1 = 250;
  *ghost2PosX1 = 250;
  *ghost3PosX1 = 250;

  *ghost0PosX2 = 0;
  *ghost1PosX2 = 0;
  *ghost2PosX2 = 0;
  *ghost3PosX2 = 0;

  *ghost0PosY1 = 0;
  *ghost1PosY1 = 0;
  *ghost2PosY1 = 0;
  *ghost3PosY1 = 0;

  *ghost0PosY2 = 0;
  *ghost1PosY2 = 0;
  *ghost2PosY2 = 0;
  *ghost3PosY2 = 0;
 }

 // Recalculating derived values
 playerPosX = (float) *playerPosX1 + ((float) *playerPosX2) / 256.0;
 ghost0PosX = (float) *ghost0PosX1 + ((float) *ghost0PosX2) / 256.0;
 ghost1PosX = (float) *ghost1PosX1 + ((float) *ghost1PosX2) / 256.0;
 ghost2PosX = (float) *ghost2PosX1 + ((float) *ghost2PosX2) / 256.0;
 ghost3PosX = (float) *ghost3PosX1 + ((float) *ghost3PosY2) / 256.0;

 playerPosY = (float) *playerPosY1 + ((float) *playerPosY2) / 256.0;
 ghost0PosY = (float) *ghost0PosY1 + ((float) *ghost0PosY2) / 256.0;
 ghost1PosY = (float) *ghost1PosY1 + ((float) *ghost1PosY2) / 256.0;
 ghost2PosY = (float) *ghost2PosY1 + ((float) *ghost2PosY2) / 256.0;
 ghost3PosY = (float) *ghost3PosY1 + ((float) *ghost3PosY2) / 256.0;

 targetPelletsTaken = 0;
 for (size_t i = 0; i < pelletPath.size(); i++)
 {
  targetPelletRow = pelletPath[i][0];
  targetPelletCol = pelletPath[i][1];
  targetPelletPosX = (float)getPelletPosX(targetPelletCol);
  targetPelletPosY = (float)getPelletPosY(targetPelletRow);

  auto pelletOffset = getPelletOffset(targetPelletRow, targetPelletCol);
  auto pelletValue = pelletMap[pelletOffset];

  if (pelletValue == 7 || pelletValue == 8) targetPelletsTaken++;
  if (pelletValue != 7 && pelletValue != 8) break;
 }

 if (targetPelletRow == 7 && targetPelletCol == 10)
   if (*playerPosX1 < 103) { targetPelletPosX = 120; targetPelletPosY = 88; }

 targetPelletDistance = std::abs((float) *playerPosX1 - targetPelletPosX) + std::abs((float) *playerPosY1 - targetPelletPosY);

 score = *score1 * 10.0 + *score2 * 100.0 + *score3 * 1000.0 + *score4 * 10000.0 + *score5 * 100000.0;

 *captureCountReward -= 5.0;
 if (*captureCountReward < 0.0) *captureCountReward = 0.0;

 if (*captureCount == *captureCountPrev + 1)
 {
  *captureCountTotal += 1;
  *captureCountReward += 500.0;
 }
 *captureCountPrev   = *captureCount;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 moves.push_back(move);
 updateDerivedValues();

// for (int i = 0; i < ROW_COUNT; i++)
// {
//  for (int j = 0; j < COL_COUNT; j++)
//  {
//   int pelletValue = pelletMap[getPelletOffset(i, j)];
//   if (pelletValue == 7 || pelletValue == 8)
//   {
//    auto curPellet = std::make_pair(i,j);
//    if ( std::find(playPelletPath.begin(), playPelletPath.end(), curPellet) == playPelletPath.end() )  playPelletPath.push_back(curPellet);
//   }
//  }
// }

 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
  if (skipIntermission) return { "." };
  std::vector<std::string> moveList = { ".", "U", "D", "L", "R" };

//  uint8_t relPosX = *playerPosX1 - 24;
//  uint8_t relPosY = *playerPosY1 - 16;
//
//  if (*playerDirection == DIRECTION_LEFT)
//  {
//   moveList.insert(moveList.end(), { ".", "R" });
//    if (relPosX % 16 == 0 || relPosX % 16 == 1) moveList.insert(moveList.end(), { "U", "D" });
//  }
//
//  if (*playerDirection == DIRECTION_RIGHT)
//  {
//   moveList.insert(moveList.end(), { ".", "L" });
//    if (relPosX % 16 == 0 || relPosX % 16 == 15) moveList.insert(moveList.end(), { "U", "D" });
//  }
//
//  if (*playerDirection == DIRECTION_UP)
//  {
//   moveList.insert(moveList.end(), { ".", "D" });
//    if (relPosY % 8 == 0 || relPosY % 8 == 1) moveList.insert(moveList.end(), { "L", "R" });
//  }
//
//  if (*playerDirection == DIRECTION_DOWN)
//  {
//   moveList.insert(moveList.end(), { ".", "U" });
//    if (relPosY % 8 == 0 || relPosY % 8 == 7) moveList.insert(moveList.end(), { "L", "R" });
//  }

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
  // auto magnets = getMagnetValues(rulesStatus);

  reward += targetPelletsTaken * 1000.0;
//  reward -= *remainingPellets;
  reward -= targetPelletDistance;
  reward += *captureCountReward;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Global Timer:                     %u\n", *globalTimer);
 LOG("[Jaffar]  + Current Level:                    %2u\n", *currentLevel);
 LOG("[Jaffar]  + Current Difficulty:               %2u\n", *currentDifficulty);
 LOG("[Jaffar]  + Reward:                           %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Score:                            %f (%01u%01u%01u%01u%01u)\n", score, *score5, *score4, *score3, *score2, *score1);
 LOG("[Jaffar]  + Remaining Pellets:                %u\n", *remainingPellets);
 LOG("[Jaffar]  + Capture Count / Prev / Total / R: %02u / %02u / %02u / %f\n", *captureCount, *captureCountPrev, *captureCountTotal, *captureCountReward);
 LOG("[Jaffar]  + Player State:                     %02u\n", *playerState);
 LOG("[Jaffar]  + Player Frame:                     %02u\n", *playerFrame);
 LOG("[Jaffar]  + Player Direction:                 %02u %02u\n", *playerDirection1, *playerDirection2);
 LOG("[Jaffar]  + Player Pos X:                     %f (%02u %02u)\n", playerPosX, *playerPosX1, *playerPosX2);
 LOG("[Jaffar]  + Player Pos Y:                     %f (%02u %02u)\n", playerPosY, *playerPosY1, *playerPosY2);
 LOG("[Jaffar]  + Ghost 0 Pos X:                    %f (%02u %02u)\n", ghost0PosX, *ghost0PosX1, *ghost0PosX2);
 LOG("[Jaffar]  + Ghost 0 Pos Y:                    %f (%02u %02u)\n", ghost0PosY, *ghost0PosY1, *ghost0PosY2);
 LOG("[Jaffar]  + Ghost 1 Pos X:                    %f (%02u %02u)\n", ghost1PosX, *ghost1PosX1, *ghost1PosX2);
 LOG("[Jaffar]  + Ghost 1 Pos Y:                    %f (%02u %02u)\n", ghost1PosY, *ghost1PosY1, *ghost1PosY2);
 LOG("[Jaffar]  + Ghost 2 Pos X:                    %f (%02u %02u)\n", ghost2PosX, *ghost2PosX1, *ghost2PosX2);
 LOG("[Jaffar]  + Ghost 2 Pos Y:                    %f (%02u %02u)\n", ghost2PosY, *ghost2PosY1, *ghost2PosY2);
 LOG("[Jaffar]  + Ghost 3 Pos X:                    %f (%02u %02u)\n", ghost3PosX, *ghost3PosX1, *ghost3PosX2);
 LOG("[Jaffar]  + Ghost 3 Pos Y:                    %f (%02u %02u)\n", ghost3PosY, *ghost3PosY1, *ghost3PosY2);
 LOG("[Jaffar]  + Ghost States:                     %02u %02u %02u %02u\n", *ghost0State, *ghost1State, *ghost2State, *ghost3State);
 LOG("[Jaffar]  + Ghost Directions:                 %02u %02u %02u %02u\n", *ghost0Direction, *ghost1Direction, *ghost2Direction, *ghost3Direction);
 LOG("[Jaffar]  + Target Pellets Taken:             %02u\n", targetPelletsTaken);
 LOG("[Jaffar]  + Target Pellet Row / Col:          %02u / %02u\n", targetPelletRow, targetPelletCol);
 LOG("[Jaffar]  + Target Pellet X / Y:              %02u / %02u\n", targetPelletPosX, targetPelletPosY);
 LOG("[Jaffar]  + Target Pellet Distance:           %f\n", targetPelletDistance);
 LOG("[Jaffar]  + Pellet Map:\n");

 LOG("         ");
 for (int j = 0; j < COL_COUNT; j++) LOG("%02u  ", j);
 LOG("\n");

 for (int i = 0; i < ROW_COUNT; i++)
 {
  LOG("    %02u : ", i);
  for (int j = 0; j < COL_COUNT; j++)
  {
   int pelletValue = pelletMap[getPelletOffset(i, j)];
   char symbol = ' ';
   if (pelletValue == 1 || pelletValue == 2) symbol = 'O';
   if (pelletValue == 3 || pelletValue == 9) symbol = 'o';
   if (pelletValue == 7 || pelletValue == 8) symbol = '.';
   if (targetPelletRow == i && targetPelletCol == j) symbol = 'X';
   LOG("%c   ", symbol);
  }
  LOG("\n");
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

// for (size_t i = 0; i < playPelletPath.size(); i++) LOG("{%d, %d}, ", playPelletPath[i].first, playPelletPath[i].second);

 // auto magnets = getMagnetValues(rulesStatus);
}

