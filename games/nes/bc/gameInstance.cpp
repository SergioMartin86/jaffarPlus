#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  globalTimer            = (uint8_t* ) &_emu->_baseMem[0x000A];
  currentZone            = (uint8_t* ) &_emu->_baseMem[0x004D];
  playerInput1           = (uint8_t* ) &_emu->_baseMem[0x00F8];
  playerInput2           = (uint8_t* ) &_emu->_baseMem[0x00FA];
  playerPosX1            = (uint8_t* ) &_emu->_baseMem[0x00B6];
  playerPosX2            = (uint8_t* ) &_emu->_baseMem[0x00B7];
  playerPosX2Offset      = (uint8_t* ) &_emu->_baseMem[0x0061];
  hookLength             = (uint8_t* ) &_emu->_baseMem[0x06BF];
  playerPosY2            = (uint8_t* ) &_emu->_baseMem[0x0062];
  playerDirection1       = (uint8_t* ) &_emu->_baseMem[0x0620];
  playerDirection2       = (uint8_t* ) &_emu->_baseMem[0x0633];
  gameMode               = (uint8_t* ) &_emu->_baseMem[0x00CC];
  levelTransition        = (uint8_t* ) &_emu->_baseMem[0x001B];
  exitState              = (uint8_t* ) &_emu->_baseMem[0x004C];
  bufferedAction1        = (uint8_t* ) &_emu->_baseMem[0x065A];
  bufferedAction2        = (uint8_t* ) &_emu->_baseMem[0x066D];
  bufferedAction3        = (uint8_t* ) &_emu->_baseMem[0x065F];
  bufferedAction4        = (uint8_t* ) &_emu->_baseMem[0x0648];
  screenPosX1            = (uint8_t* ) &_emu->_baseMem[0x00A9];
  screenPosX2            = (uint8_t* ) &_emu->_baseMem[0x00AA];
  swingTimer             = (uint8_t* ) &_emu->_baseMem[0x005D];
  hookTimer              = (uint8_t* ) &_emu->_baseMem[0x060C];
  hookState              = (uint8_t* ) &_emu->_baseMem[0x065F];
  hookPosX               = (uint8_t* ) &_emu->_baseMem[0x0574];
  hookPosY               = (uint8_t* ) &_emu->_baseMem[0x05AE];
  actionTimer            = (uint8_t* ) &_emu->_baseMem[0x003C];
  crouchTimer            = (uint8_t* ) &_emu->_baseMem[0x0691];
  currentAction          = (uint8_t* ) &_emu->_baseMem[0x07E6];
  prevAction             = (uint8_t* ) &_emu->_baseMem[0x07E7];
  screenPosY1            = (int8_t* ) &_emu->_baseMem[0x00AC];
  screenPosY2            = (uint8_t* ) &_emu->_baseMem[0x00AD];
  hookActive             = (uint8_t* ) &_emu->_baseMem[0x064C];
  grabbedItem1           = (uint8_t* ) &_emu->_baseMem[0x04E3];
  tracePos               = (uint16_t* ) &_emu->_baseMem[0x07FE];

//  bullet1PosX             = (uint8_t* ) &_emu->_baseMem[0x0570];
  bullet1Active           = (uint8_t* ) &_emu->_baseMem[0x0537];
  bullet2Active           = (uint8_t* ) &_emu->_baseMem[0x0538];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Trace to Follow
  if (isDefined(config, "Trace File") == true)
   traceFile = config["Trace File"].get<std::string>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Trace File' was not defined\n");

  // Trace tolerance
  if (isDefined(config, "Trace Tolerance") == true)
   traceTolerance = config["Trace Tolerance"].get<float>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Trace Tolerance' was not defined\n");

  // Max Bullets tolerance
  if (isDefined(config, "Max Bullets") == true)
   maxBullets = config["Max Bullets"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Max Bullets' was not defined\n");

  // Loading trace
  if (traceFile != "")
  {
   useTrace = true;
   std::string traceRaw;
   if (loadStringFromFile(traceRaw, traceFile.c_str()) == false) EXIT_WITH_ERROR("Could not find/read trace file: %s\n", traceFile.c_str());

   std::istringstream f(traceRaw);
   std::string line;
   while (std::getline(f, line))
   {
    auto coordinates = split(line, ' ');
    trace.push_back(std::make_pair(std::atof(coordinates[0].c_str()), std::atof(coordinates[1].c_str())));
   }
  }

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

//  uint8_t emuState[_STATE_DATA_SIZE_PLAY];
//  _emu->serializeState(emuState);
//  _emu->advanceState(0);

  // If timer tolerance is set, use the game tick for hashing
  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));

//  hash.Update(*globalTimer      );
  hash.Update(*currentZone      );
//  hash.Update(*playerInput1     );
//  hash.Update(*playerInput2     );
  hash.Update(*playerPosX1      );
  hash.Update(*playerPosX2      );
  hash.Update(*playerPosX2Offset);
  hash.Update(*hookLength       );
  hash.Update(*playerPosY2      );
  hash.Update(*playerDirection1  );
  hash.Update(*playerDirection2  );
  hash.Update(*gameMode         );
  hash.Update(*levelTransition  );
  hash.Update(*exitState        );
  hash.Update(*bufferedAction1  );
  hash.Update(*bufferedAction2  );
  hash.Update(*bufferedAction3  );
  hash.Update(*bufferedAction4  );
  hash.Update(*screenPosX1      );
  hash.Update(*screenPosX2      );
  hash.Update(*screenPosY1      );
  hash.Update(*screenPosY2      );
//  hash.Update(*swingTimer);
  hash.Update(*actionTimer);
  hash.Update(*currentAction);
  hash.Update(*prevAction);
  hash.Update(*hookTimer);
  hash.Update(*hookPosX);
  hash.Update(*hookPosY);
//  hash.Update(*crouchTimer);
  hash.Update(*hookState);
  hash.Update(*hookActive);

//  hash.Update(*bullet1PosX);
  hash.Update(*bullet1Active);
  hash.Update(*bullet2Active);

  // Player info

//  for (size_t i = 0; i < 0x800; i++)
//   if (i != 0x000A)
//   if (i != 0x0036)
//   if (i != 0x0037)
//   if (i != 0x0038)
//   if (i != 0x0039)
//   if (i != 0x003D)
//   if (i != 0x00F8)
//   if (i != 0x00FA)
//    hash.Update(_emu->_baseMem[i]);

  constexpr size_t start = 0x40;
  constexpr size_t end = 0xF6;
  hash.Update(&_emu->_baseMem[start], end - start);

//  hash.Update(&_emu->_baseMem[0x0100], 0x6FF);
  hash.Update(&_emu->_baseMem[0x0400], 0x100);
  hash.Update(&_emu->_baseMem[0x0500], 0x100);
  hash.Update(&_emu->_baseMem[0x0600], 0x100);
  hash.Update(&_emu->_baseMem[0x0700], 0x100);

//  hash.Update(&_emu->_baseMem[0x0780], 0x0080);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));

//   Reload game state
//  _emu->deserializeState(emuState);

  return result;
}


void GameInstance::updateDerivedValues()
{
 float playerPosX2OffsetAdjusted = (float)*playerPosX2Offset;
 if (*hookActive > 0) playerPosX2OffsetAdjusted = (float)*hookPosX  * 0.1 + (float)*playerPosX2Offset * 0.9;

 float playerPosY2OffsetAdjusted = (float)*playerPosY2;
 if (*hookActive > 0) playerPosY2OffsetAdjusted = (float)*hookPosY * 0.1 + (float)*playerPosY2 * 0.9;

 playerPosX = (float)*playerPosX1 * 256.0f + (float)*playerPosX2 + playerPosX2OffsetAdjusted;
 playerPosY = (float)*screenPosY1 * 256.0f + (float)*screenPosY2 + playerPosY2OffsetAdjusted;

 bulletCount = 0;
 if (*bullet1Active == 1) bulletCount++;
 if (*bullet2Active == 1) bulletCount++;

 // Calculating trace position
 float minDistance = std::numeric_limits<float>::infinity();
 for (uint16_t i = 0; i < trace.size(); i++)
 {
  float tracePointDistance =  std::abs(playerPosX - trace[i].first) + std::abs(playerPosY - trace[i].second);
  if (tracePointDistance < traceTolerance) if (tracePointDistance <= minDistance) { minDistance = tracePointDistance; *tracePos = i; }
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
 std::vector<std::string> moveList = { "." };

 if (*currentAction == 0x0000) moveList.insert(moveList.end(), { "L", "R", "D", "DR", "DL" });
 if (*currentAction == 0x0001) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DR", "UL", "UR", "UB", "UA", "DL", "DB", "DA", "LR", "BA", "LB", "LA", "RA", "RB" });
 if (*currentAction == 0x0002) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DA", "UL", "UR", "UB", "UA", "DB", "LB", "LA", "RB", "RA", "BA" });
 if (*currentAction == 0x0003) moveList.insert(moveList.end(), { "B", "R", "U", "A", "L", "D", "UR", "UB", "UA", "UL", "DL", "UD", "DR", "DB", "DA", "LB", "LA", "RB", "RA", "BA", "ULA", "URA" });
 if (*currentAction == 0x0004) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "BA", "RB", "LB", "DB", "DR", "UB", "RA", "LA" });
 if (*currentAction == 0x0005) moveList.insert(moveList.end(), { "B", "A", "L", "R", "U", "UB", "RA", "LA" });
 if (*currentAction == 0x0006) moveList.insert(moveList.end(), { "B", "R", "U", "A", "L", "D", "UR", "UB", "UA", "UL", "DL", "DR", "UD", "DB", "DA", "LR", "LB", "LA", "RB", "RA", "BA" });
 if (*currentAction == 0x0007) moveList.insert(moveList.end(), { "B", "U", "D", "L", "R", "A", "DR", "UL", "UR", "UA", "DL", "RA", "DB", "BA", "LB", "LA", "RB" });
 if (*currentAction == 0x0008) moveList.insert(moveList.end(), { "U", "L", "R", "UA", "UL", "UR" });
 if (*currentAction == 0x0009) moveList.insert(moveList.end(), { "B", "U", "R", "A", "L", "D", "UA", "DB", "DA", "LB", "LA", "RB", "RA", "BA", "URA", "ULA" });
 if (*currentAction == 0x000B) moveList.insert(moveList.end(), { "B", "R", "U", "A", "L", "D", "UR", "UB", "UA", "UL", "DB", "DA", "UD", "LB", "LA", "RB", "RA", "BA", "URA", "ULA" });
 if (*currentAction == 0x000C) moveList.insert(moveList.end(), { "A", "R", "D", "U", "RA", "LA", "LR", "DA", "DR", "UA", "UR", "UL", "UD" });
 if (*currentAction == 0x000D) moveList.insert(moveList.end(), { "D", "R", "L" });
 if (*currentAction == 0x000E) moveList.insert(moveList.end(), { "D" });
 if (*currentAction == 0x000F) moveList.insert(moveList.end(), { "R", "L", "D", "U", "LR", "DR", "UR", "UL", "UD" });
 if (*currentAction == 0x0010) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DR", "UL", "UR", "UB", "UA", "DL", "DB", "DA", "LR", "BA", "LB", "LA", "RA", "RB" });
 if (*currentAction == 0x00FF) moveList.insert(moveList.end(), { "U", "B", "D", "L", "R", "A", "RA", "UA", "DB", "DA", "LB", "LA", "BA", "RB" });

 // Remove B if max bullet count was reached
 if (bulletCount >= maxBullets) for (size_t i = 0; i < moveList.size(); i++) if (moveList[i].find('B') != std::string::npos) moveList.erase(moveList.begin() + i--);

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

  auto magnets = getMagnetValues(rulesStatus);

  float diff = 0.0;

  diff = std::abs(magnets.playerHorizontalMagnet.center - playerPosX);
  float weightedDiffX = magnets.playerHorizontalMagnet.intensity * diff;
  reward -= weightedDiffX;

  diff = std::abs(magnets.playerVerticalMagnet.center - playerPosY);
  float weightedDiffY = magnets.playerVerticalMagnet.intensity * diff;
  reward -= weightedDiffY;

  // Evaluating trace distance magnet
  reward += magnets.traceMagnet * (float)*tracePos;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                           %.10f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Global Timer:                     %2u\n", *globalTimer);
 LOG("[Jaffar]  + Game Mode:                        %2u\n", *gameMode);
 LOG("[Jaffar]  + Exit State:                       %2u\n", *exitState);
 LOG("[Jaffar]  + Current Zone:                     %2u\n", *currentZone);

 LOG("[Jaffar]  + Hook Length:                      %2u (Active: %2u)\n", *hookLength, *hookActive);
 LOG("[Jaffar]  + Player Action:                    %2u (Prev: %2u)\n", *currentAction, *prevAction);
 LOG("[Jaffar]  + Player 1 Pos X:                   %f (%2u, %2u, %2u)\n", playerPosX, *playerPosX1, *playerPosX2, *playerPosX2Offset);
 LOG("[Jaffar]  + Player 1 Pos Y:                   %f (%2d, %2u, %2u)\n", playerPosY, *screenPosX1, *screenPosX2, *screenPosY2);

 LOG("[Jaffar]  + Bullet Count:                     %2u (%2u, %2u)\n", bulletCount, *bullet1Active, *bullet2Active);

 if (useTrace == true)
 {
  LOG("[Jaffar]  + Current Trace Pos:                %05u / %05lu (%s)\n", *tracePos, trace.size(), traceFile.c_str());
 }


 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 if (std::abs(magnets.playerHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Player Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.playerHorizontalMagnet.intensity, magnets.playerHorizontalMagnet.center);
 if (std::abs(magnets.playerVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Player Vertical Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.playerVerticalMagnet.intensity, magnets.playerVerticalMagnet.center);

}

std::string GameInstance::getFrameTrace() const
{
 return std::to_string(playerPosX) + std::string(" ") + std::to_string(playerPosY) + std::string("\n");
}


