#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting relevant metroid pointers

  // https://datacrystal.romhacking.net/wiki/Metroid:RAM_map

  pauseMode                        = (uint8_t*)   &_emu->_baseMem[0x0000];
  frameCounter                     = (uint8_t*)   &_emu->_baseMem[0x002D];
  NMIFlag                          = (uint8_t*)   &_emu->_baseMem[0x001A];
  gameMode                         = (uint8_t*)   &_emu->_baseMem[0x001E];
  samusPosXRaw                     = (uint8_t*)   &_emu->_baseMem[0x0051];
  screenPosX1                      = (uint8_t*)   &_emu->_baseMem[0x0050];
  screenPosX2                      = (uint8_t*)   &_emu->_baseMem[0x00FD];
  samusPosYRaw                     = (uint8_t*)   &_emu->_baseMem[0x0052];
  screenPosY1                      = (uint8_t*)   &_emu->_baseMem[0x004F];
  screenPosY2                      = (uint8_t*)   &_emu->_baseMem[0x00FC];
  samusAnimation                   = (uint8_t*)   &_emu->_baseMem[0x0306];
  samusDirection                   = (uint8_t*)   &_emu->_baseMem[0x004D];
  samusDoorSide                    = (uint8_t*)   &_emu->_baseMem[0x004E];
  samusDoorState                   = (uint8_t*)   &_emu->_baseMem[0x0056];
  samusJumpState                   = (uint8_t*)   &_emu->_baseMem[0x0314];
  equipmentFlags                   = (uint8_t*)   &_emu->_highMem[0x0878];
  samusSelectedWeapon              = (uint8_t*)   &_emu->_baseMem[0x0056];
  missileCount                     = (uint8_t*)   &_emu->_highMem[0x0879];
  samusHP1                         = (uint8_t*)   &_emu->_baseMem[0x0107];
  samusHP2                         = (uint8_t*)   &_emu->_baseMem[0x0106];

  door1State                       = (uint8_t*)   &_emu->_baseMem[0x0380];
  door2State                       = (uint8_t*)   &_emu->_baseMem[0x0390];
  door3State                       = (uint8_t*)   &_emu->_baseMem[0x03A0];
  door4State                       = (uint8_t*)   &_emu->_baseMem[0x03B0];

  door1Timer                       = (uint8_t*)   &_emu->_baseMem[0x038F];
  door2Timer                       = (uint8_t*)   &_emu->_baseMem[0x039F];
  door3Timer                       = (uint8_t*)   &_emu->_baseMem[0x03AF];
  door4Timer                       = (uint8_t*)   &_emu->_baseMem[0x03BF];

  bullet1State                     = (uint8_t*)   &_emu->_baseMem[0x03D0];
  bullet2State                     = (uint8_t*)   &_emu->_baseMem[0x03E0];
  bullet3State                     = (uint8_t*)   &_emu->_baseMem[0x03F0];

  bullet1PosX                      = (uint8_t*)   &_emu->_baseMem[0x03DE];
  bullet2PosX                      = (uint8_t*)   &_emu->_baseMem[0x03EE];
  bullet3PosX                      = (uint8_t*)   &_emu->_baseMem[0x03FE];

  bullet1PosY                      = (uint8_t*)   &_emu->_baseMem[0x03DD];
  bullet2PosY                      = (uint8_t*)   &_emu->_baseMem[0x03ED];
  bullet3PosY                      = (uint8_t*)   &_emu->_baseMem[0x03FD];

  lagFrameCounter                  = (uint16_t*)  &_emu->_highMem[0x1FFD];
  pauseFrameCounter                = (uint16_t*)  &_emu->_highMem[0x1FFB];
  customValue                      = (uint8_t*)   &_emu->_highMem[0x1FFF];

  // Initialize derivative values
  updateDerivedValues();

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0)  hash.Update(currentStep % (timerTolerance+1));

  hash.Update(*pauseMode);
  hash.Update(*gameMode);
  hash.Update(*NMIFlag);
  hash.Update(*samusPosXRaw);
  hash.Update(*screenPosX1);
  hash.Update(*screenPosX2);
  hash.Update(*samusPosYRaw);
  hash.Update(*screenPosY1);
  hash.Update(*screenPosY2);
//  hash.Update(*lagFrameCounter);
  hash.Update(*pauseFrameCounter);
  hash.Update(*missileCount);
  hash.Update(*samusSelectedWeapon);

//  hash.Update(*samusAnimation);
  hash.Update(*samusDirection);
  hash.Update(*samusDoorSide);
  hash.Update(*samusJumpState);
  hash.Update(*equipmentFlags);

  hash.Update(*door1State);
  hash.Update(*door2State);
  hash.Update(*door3State);
  hash.Update(*door4State);

//  hash.Update(*door1Timer);
//  hash.Update(*door2Timer);
//  hash.Update(*door3Timer);
//  hash.Update(*door4Timer);

  hash.Update(*bullet1State);
  hash.Update(*bullet2State);
  hash.Update(*bullet3State);
//  hash.Update(*bullet1PosX);
//  hash.Update(*bullet2PosX);
//  hash.Update(*bullet3PosX);
//  hash.Update(*bullet1PosY);
//  hash.Update(*bullet2PosY);
//  hash.Update(*bullet3PosY);

//  hash.Update(*customValue);

  // Samus-specific hashes
  hash.Update(_emu->_baseMem[0x0053]); // Run Frame
  hash.Update(_emu->_baseMem[0x0300]); // Buffered ACtion
  hash.Update(_emu->_baseMem[0x0304]); // Buffered ACtion
  hash.Update(_emu->_baseMem[0x0305]); // Buffered ACtion
  hash.Update(_emu->_baseMem[0x0306]); // Buffered ACtion
  hash.Update(_emu->_baseMem[0x0308]); // Vertical Speed
  hash.Update(_emu->_baseMem[0x030F]); // Buffered ACtion
  hash.Update(_emu->_baseMem[0x0314]); // Buffered ACtion
  hash.Update(_emu->_baseMem[0x0316]); // Jump State2
  hash.Update(_emu->_baseMem[0x0310]); // Vertical Accel
  hash.Update(_emu->_baseMem[0x0315]); // Buffered ACtion
  hash.Update(_emu->_baseMem[0x030A]); // Hit By Enemy
//    hash.Update(&_emu->_baseMem[0x0300], 0x0020);
//    hash.Update(&_emu->_baseMem[0x0314], 0x0010);

//    hash.Update(&_emu->_baseMem[0x0000], 0x0800);
//    hash.Update(&_emu->_highMem[0x0000], 0x2000);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 uint8_t realScreenPosX1 = *screenPosX2 == 0 ? *screenPosX1+1 : *screenPosX1;
 samusPosX = (uint16_t)realScreenPosX1 * 256 + (uint16_t)*screenPosX2 + (uint16_t)*samusPosXRaw;
 samusPosY = (uint16_t)*screenPosY1 * 256 + (uint16_t)*screenPosY2 + (uint16_t)*samusPosYRaw;

 bulletCount = (uint8_t)(*bullet1State > 0) + (uint8_t)(*bullet2State > 0) + (uint8_t)(*bullet3State > 0);
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

 if (*samusAnimation == 0x0001) moveList.insert(moveList.end(), { "A", "B", "R", "L", "U", "RB", "LB" });
 if (*samusAnimation == 0x0002) moveList.insert(moveList.end(), { "A", "B", "R", "L", "U", "RB", "LB" });
 if (*samusAnimation == 0x0003) moveList.insert(moveList.end(), { "A", "B", "R", "L", "U", "RB", "LB" });
 if (*samusAnimation == 0x0005) moveList.insert(moveList.end(), { "A", "B", "R", "L", "U", "RB", "LB", "UA", "UB", "UR", "UL", "URB", "ULB" });
 if (*samusAnimation == 0x0007) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DB", "UD", "UL", "UR", "UB", "UA", "DA", "LB", "LA", "RB", "RA", "BA", "LBA", "DBA", "RBA", "UBA", "URA", "URB", "ULA", "ULB", "UDB" });
 if (*samusAnimation == 0x0008) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DB", "UD", "UL", "UR", "UB", "UA", "DA", "LB", "LA", "RB", "RA", "BA", "LBA", "DBA", "RBA", "UBA", "URA", "URB", "ULA", "ULB", "UDB" });
 if (*samusAnimation == 0x000A) moveList.insert(moveList.end(), { "B", "R", "U", "A", "L", "D", "UR", "UB", "UA", "UL", "DL", "DB", "LR", "UD", "LB", "LA", "RB", "RA", "BA", "ULB", "ULA", "URB", "UDB", "URA", "UBA", "DLB", "DBA", "LRB", "LBA", "RBA" });
 if (*samusAnimation == 0x000B) moveList.insert(moveList.end(), { "B", "R", "U", "L", "D", "A", "LR", "UL", "UR", "UB", "DL", "DB", "LB", "LA", "RB", "RA", "BA", "LBA", "DBA", "RBA", "URB", "ULB" });
 if (*samusAnimation == 0x000C) moveList.insert(moveList.end(), { "B", "R", "U", "A", "L", "UR", "UB", "DL", "DB", "UL", "LR", "LB", "LA", "RB", "RA", "BA", "ULA", "URB", "URA", "ULB", "UBA", "DLA", "LRA", "LBA", "RBA" });
 if (*samusAnimation == 0x000D) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "DB", "UL", "UR", "UB", "UA", "DL", "LR", "LB", "BA", "LA", "RB", "RA", "UDA", "ULB", "ULA", "URB", "URA", "UBA", "RBA", "DLB", "DLA", "LBA", "LRB", "LRA" });
 if (*samusAnimation == 0x000F) moveList.insert(moveList.end(), { "B", "R", "L", "A", "D", "DA", "UL", "UR", "UB", "UA", "DB", "LR", "LB", "LA", "RB", "RA", "BA", "LBA", "DLA", "RBA", "UBA", "URA", "URB", "ULA", "ULB", "UDA" });
 if (*samusAnimation == 0x0010) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DB", "UL", "UR", "UB", "UA", "LB", "LA", "RB", "RA", "BA", "LBA", "DLA", "RBA", "UBA", "URA", "URB", "ULA", "ULB" });
 if (*samusAnimation == 0x0011) moveList.insert(moveList.end(), { "B", "R", "A", "L", "U", "LA", "UR", "UL", "UB", "DB", "LB", "RB", "RA", "BA", "ULA", "ULB", "URB", "URA", "UBA", "LBA", "RBA" });
 if (*samusAnimation == 0x0012) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "DB", "UL", "UR", "UB", "UA", "LB", "LA", "RB", "RA", "BA", "LBA", "RBA", "UBA", "URA", "URB", "ULA", "ULB" });
 if (*samusAnimation == 0x0013) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "UA", "UD", "UDR" });
 if (*samusAnimation == 0x0014) moveList.insert(moveList.end(), { "A", "R", "L", "U" });
 if (*samusAnimation == 0x0017) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RB", "DR", "DL", "UD", "DRA", "DLA", "UDR", "UDL" });
 if (*samusAnimation == 0x0018) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "UDR" });
 if (*samusAnimation == 0x0019) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U" });
 if (*samusAnimation == 0x001A) moveList.insert(moveList.end(), { "A", "R", "L", "U" });
 if (*samusAnimation == 0x0020) moveList.insert(moveList.end(), { "B", "U", "D", "L", "R", "A", "RB", "UL", "UR", "UB", "UA", "DL", "DB", "BA", "LR", "LB", "LA", "RA", "LRA", "DLA", "LBA", "UBA", "RBA", "URA", "URB", "ULA", "ULB" });
 if (*samusAnimation == 0x0021) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DA", "UL", "UR", "UB", "UA", "DL", "DB", "LR", "BA", "LB", "LA", "RA", "RB", "RBA", "ULB", "ULA", "URB", "URA", "UBA", "LRB", "DLB", "DLA", "DBA", "LBA", "LRA" });
 if (*samusAnimation == 0x0023) moveList.insert(moveList.end(), { "A", "B", "R", "L", "U", "RB", "LB", "LR", "LRB", "DLB" });
 if (*samusAnimation == 0x0024) moveList.insert(moveList.end(), { "A", "B", "R", "L", "U", "RB", "LB" });
 if (*samusAnimation == 0x0025) moveList.insert(moveList.end(), { "A", "B", "R", "L", "U" });
 if (*samusAnimation == 0x0027) moveList.insert(moveList.end(), { "B", "U", "D", "L", "A", "R", "DB", "UL", "UR", "UB", "UA", "UD", "RB", "DA", "BA", "LB", "LA", "RA", "UDB", "UDA", "ULB", "ULA", "URB", "URA", "UBA", "DBA", "LBA", "RBA" });
 if (*samusAnimation == 0x0028) moveList.insert(moveList.end(), { "B", "U", "D", "L", "A", "R", "DB", "UL", "UR", "UB", "UA", "UD", "RB", "DA", "BA", "LB", "LA", "RA", "UDB", "UDA", "ULB", "ULA", "URB", "URA", "UBA", "DBA", "LBA", "RBA" });
 if (*samusAnimation == 0x0034) moveList.insert(moveList.end(), { "B", "U", "D", "L", "R", "A", "RB", "RA", "LA", "LB", "LR", "BA", "DB", "UL", "DL", "UA", "UB", "UR", "ULB", "URB", "DBA", "RBA" });
 if (*samusAnimation == 0x0035) moveList.insert(moveList.end(), { "B", "R", "U", "A", "L", "D", "UR", "UB", "UA", "UL", "DL", "DB", "DA", "UD", "LR", "LB", "LA", "RB", "RA", "BA", "ULB", "UDB", "ULA", "URB", "URA", "UDR", "UBA", "DLB", "DLA", "DBA", "LRB", "LRA", "LBA", "RBA" });
 if (*samusAnimation == 0x0036) moveList.insert(moveList.end(), { "B", "R", "L", "U", "A", "D", "DB", "UL", "UR", "UB", "UA", "DL", "LR", "LB", "LA", "RB", "RA", "BA", "LRA", "LBA", "DLA", "RBA", "UBA", "URA", "URB", "ULA", "ULB", "UDA" });
 if (*samusAnimation == 0x0038) moveList.insert(moveList.end(), { "B", "R", "L", "U", "RB", "LB", "UA", "UB", "UR", "UL", "UD", "URB", "ULB" });
 if (*samusAnimation == 0x0039) moveList.insert(moveList.end(), { "B", "R", "L", "U", "RB", "LB", "UA", "UB", "UR", "UL", "URB", "ULB" });
 if (*samusAnimation == 0x003A) moveList.insert(moveList.end(), { "B", "R", "L", "U", "RB", "LB", "UA", "UB", "UR", "UL", "URB", "ULB" });
 if (*samusAnimation == 0x003C) moveList.insert(moveList.end(), { "B", "R", "L", "U", "LB", "UA", "UB", "UR", "UL", "URB", "ULB", "UDL" });
 if (*samusAnimation == 0x003E) moveList.insert(moveList.end(), { "B", "R", "L", "U", "RB", "LB", "UA", "UB", "UR", "UL", "URB", "ULB" });
 if (*samusAnimation == 0x0040) moveList.insert(moveList.end(), { "B", "U", "LB", "UA", "UB", "UR", "UL", "URB", "ULB" });

 return moveList;
}

void GameInstance::printFullMoveList(const bool* rulesStatus)
{
 for (uint16_t i = 0; i <= 0xFF; i++)
 {
  *samusAnimation = (uint8_t)i;
  auto moves = getPossibleMoves(rulesStatus);
  if (moves.size() == 1) continue;

  size_t vecSize = moves.size();
  for (size_t j = 0; j < vecSize; j++)
  {
   INPUT_TYPE code = EmuInstance::moveStringToCode(moves[j]);
   if ( ((code & 0b01000000) > 0) && ((code & 0b10000000) == 0)) moves.push_back(simplifyMove(EmuInstance::moveCodeToString((code & ~0b01000000) | 0b10000000)));
   if ( ((code & 0b10000000) > 0) && ((code & 0b01000000) == 0)) moves.push_back(simplifyMove(EmuInstance::moveCodeToString((code & ~0b10000000) | 0b01000000)));
  }

  std::set<std::string> moveSet( moves.begin(), moves.end() );
  moves.assign( moveSet.begin(), moveSet.end() );
  std::sort(moves.begin(), moves.end(), moveCountComparerString);

  printf("if (*%s == 0x%02X) moveList.insert(moveList.end(), { \"%s\"", "samusAnimation", *samusAnimation, moves[1].c_str());
  for (size_t j = 2; j < moves.size(); j++)
   printf(", \"%s\"", moves[j].c_str());
  printf(" });\n");
 }

 exit(0);
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 _emu->advanceState(move); moves.push_back(move);

 // count lag frames
  if (*NMIFlag == 1) (*lagFrameCounter)++;
  if (*pauseMode == 0) (*pauseFrameCounter)++;

 updateDerivedValues();

 return moves;
}


// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
    magnets = _rules[ruleId]->_magnets;

 // Evaluating custom value
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true) if (_rules[ruleId]->_customValueActive == true) *customValue = _rules[ruleId]->_customValue;

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

  // Container for bounded value and difference with center
  float diff = 0.0;

  // Evaluating samus magnet's reward on position X and Y
  diff = std::abs(magnets.samusHorizontalMagnet.center - (float)samusPosX);
  float weightedDiffX = magnets.samusHorizontalMagnet.intensity * diff;

  diff = std::abs(magnets.samusVerticalMagnet.center - (float)samusPosY);
  float weightedDiffY = magnets.samusVerticalMagnet.intensity * diff;

  reward += -sqrt(weightedDiffX*weightedDiffX + weightedDiffY*weightedDiffY);

  // Evaluating bullet1 magnet's reward on position X
  diff = std::abs(magnets.bullet1HorizontalMagnet.center - (float)*bullet1PosX);
  reward += magnets.bullet1HorizontalMagnet.intensity * -diff;

  // Evaluating bullet1 magnet's reward on position Y
  diff = std::abs(magnets.bullet1VerticalMagnet.center - (float)*bullet1PosY);
  reward += magnets.bullet1VerticalMagnet.intensity * -diff;

  // Evaluating lag frame counter reward
  reward += magnets.lagFrameCounterMagnet * ((float)*lagFrameCounter + (float)*pauseFrameCounter);

  // Evaluating missile count
  reward += magnets.missileCountMagnet * (float)*missileCount;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Pause Mode:             %03u\n", *pauseMode);
  LOG("[Jaffar]  + Frame Counter:          %03u\n", *frameCounter);
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second );
  LOG("[Jaffar]  + Game Mode:              %02u\n", *gameMode);
  LOG("[Jaffar]  + NMI Flag:               %02u\n", *NMIFlag);
  LOG("[Jaffar]  + Lag Frame Counter:      %05u\n", *lagFrameCounter);
  LOG("[Jaffar]  + Pause Frame Counter:    %05u\n", *pauseFrameCounter);
  LOG("[Jaffar]  + Samus Pos X:            %05u (%02u %02u %02u)\n", samusPosX, *screenPosX1, *screenPosX2, *samusPosXRaw);
  LOG("[Jaffar]  + Samus Pos Y:            %05u (%02u %02u %02u)\n", samusPosY, *screenPosY1, *screenPosY2, *samusPosYRaw);
  LOG("[Jaffar]  + Samus Animation:        %03u\n", *samusAnimation);
  LOG("[Jaffar]  + Samus Direction:        %03u\n", *samusDirection);
  LOG("[Jaffar]  + Samus Door Side:        %03u\n", *samusDoorSide);
  LOG("[Jaffar]  + Samus Door State:       %03u\n", *samusDoorState);
  LOG("[Jaffar]  + Samus Jump State:       %03u\n", *samusJumpState);
  LOG("[Jaffar]  + Door States:            [ %02u, %02u, %02u, %02u ]\n", *door1State, *door2State, *door3State, *door4State);
  LOG("[Jaffar]  + Door Timers:            [ %02u, %02u, %02u, %02u ]\n", *door1Timer, *door2Timer, *door3Timer, *door4Timer);
  LOG("[Jaffar]  + Bullet Count:           %02u\n", bulletCount);
  LOG("[Jaffar]  + Bullet States:          [ %02u, %02u, %02u ]\n", *bullet1State, *bullet2State, *bullet3State);
  LOG("[Jaffar]  + Bullet Pos X:           [ %02u, %02u, %02u ]\n", *bullet1PosX, *bullet2PosX, *bullet3PosX);
  LOG("[Jaffar]  + Bullet Pos Y:           [ %02u, %02u, %02u ]\n", *bullet1PosY, *bullet2PosY, *bullet3PosY);
  LOG("[Jaffar]  + Equipment Flags:        %03u\n", *equipmentFlags);
  LOG("[Jaffar]  + Samus Selected Weapon:  %03u\n", *samusSelectedWeapon);
  LOG("[Jaffar]  + Custom Value:           %02u\n", *customValue);
  LOG("[Jaffar]  + Samus Missile Count:    %02u\n", *missileCount);
  LOG("[Jaffar]  + Samus HP:               %02u x 10 + %02u\n", *samusHP1, *samusHP2);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.samusHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Samus Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.samusHorizontalMagnet.intensity, magnets.samusHorizontalMagnet.center);
  if (std::abs(magnets.samusVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Samus Vertical Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.samusVerticalMagnet.intensity, magnets.samusVerticalMagnet.center);
  if (std::abs(magnets.bullet1HorizontalMagnet.intensity) > 0.0f) LOG("[Jaffar]  + Bullet 1 Horizontal Magnet     - Intensity: %.5f, Center: %3.3f\n", magnets.bullet1HorizontalMagnet.intensity, magnets.bullet1HorizontalMagnet.center);
  if (std::abs(magnets.bullet1VerticalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Bullet 1 Vertical Magnet       - Intensity: %.5f, Center: %3.3f\n", magnets.bullet1VerticalMagnet.intensity, magnets.bullet1VerticalMagnet.center);
  if (std::abs(magnets.lagFrameCounterMagnet) > 0.0f)             LOG("[Jaffar]  + Lag Frame Counter Magnet       - Intensity: %.5f\n", magnets.lagFrameCounterMagnet);
  if (std::abs(magnets.missileCountMagnet) > 0.0f)                LOG("[Jaffar]  + Missile Count Magnet           - Intensity: %.5f\n", magnets.missileCountMagnet);
}
