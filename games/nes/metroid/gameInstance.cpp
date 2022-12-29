#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting relevant metroid pointers

  // https://datacrystal.romhacking.net/wiki/Metroid:RAM_map

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
  equipmentFlags                   = (uint8_t*)   &_emu->_highMem[0x0878];

  door1State                       = (uint8_t*)   &_emu->_baseMem[0x0380];
  door2State                       = (uint8_t*)   &_emu->_baseMem[0x0390];
  door3State                       = (uint8_t*)   &_emu->_baseMem[0x03A0];
  door4State                       = (uint8_t*)   &_emu->_baseMem[0x03B0];

  bullet1State                     = (uint8_t*)   &_emu->_baseMem[0x03D0];
  bullet2State                     = (uint8_t*)   &_emu->_baseMem[0x03E0];
  bullet3State                     = (uint8_t*)   &_emu->_baseMem[0x03F0];

  bullet1PosX                      = (uint8_t*)   &_emu->_baseMem[0x03DE];
  bullet2PosX                      = (uint8_t*)   &_emu->_baseMem[0x03EE];
  bullet3PosX                      = (uint8_t*)   &_emu->_baseMem[0x03FE];

  bullet1PosY                      = (uint8_t*)   &_emu->_baseMem[0x03DD];
  bullet2PosY                      = (uint8_t*)   &_emu->_baseMem[0x03ED];
  bullet3PosY                      = (uint8_t*)   &_emu->_baseMem[0x03FD];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  hash.Update(*gameMode);
  hash.Update(*NMIFlag);
  hash.Update(*samusPosXRaw);
  hash.Update(*screenPosX1);
  hash.Update(*screenPosX2);
  hash.Update(*samusPosYRaw);
  hash.Update(*screenPosY1);
  hash.Update(*screenPosY2);

  hash.Update(*samusAnimation);
  hash.Update(*samusDirection);
  hash.Update(*samusDoorSide);
  hash.Update(*equipmentFlags);

  hash.Update(*door1State);
  hash.Update(*door2State);
  hash.Update(*door3State);
  hash.Update(*door4State);

  hash.Update(*bullet1State);
  hash.Update(*bullet2State);
  hash.Update(*bullet3State);
  hash.Update(*bullet1PosX);
  hash.Update(*bullet2PosX);
  hash.Update(*bullet3PosX);
  hash.Update(*bullet1PosY);
  hash.Update(*bullet2PosY);
  hash.Update(*bullet3PosY);

  // Samus-specific hashes
  hash.Update(&_emu->_baseMem[0x0300], 0x0020);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 uint8_t realScreenPosX1 = *screenPosX2 == 0 ? *screenPosX1+1 : *screenPosX1;
 samusPosX = (float)realScreenPosX1 * 256.0f + (float)*screenPosX2 + (float)*samusPosXRaw;

 uint8_t realScreenPosY1 = *screenPosY2 == 0 ? *screenPosY1+1 : *screenPosY1;
 samusPosY = (float)realScreenPosY1 * 256.0f + (float)*screenPosY2 + (float)*samusPosYRaw;

 bulletCount = (uint8_t)(*bullet1State > 0) + (uint8_t)(*bullet2State > 0) + (uint8_t)(*bullet3State > 0);
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*samusAnimation == 0x01) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "LA", "LB", "BA", "RA", "RB", "UA", "UL", "UR", "URB", "RBA", "URA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x02) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "LA", "LB", "BA", "RA", "RB", "UA", "UL", "UR", "URB", "RBA", "URA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x03) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "BA", "URB", "URA", "RBA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x05) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "UA", "UR", "UL", "UD", "UB", "RB", "RA", "LB", "LA", "BA", "LBA", "UBA", "UDB", "RBA", "URB", "ULA", "ULB", "URA", "URBA", "ULBA", "UDRB", "UDLB" });
 if (*samusAnimation == 0x07) moveList.insert(moveList.end(), { "A", "B", "D", "L", "U", "R", "RA", "UR", "UL", "UD", "UB", "UA", "RB", "LB", "LA", "DB", "BA", "LBA", "RBA", "DRB", "UBA", "DLB", "UDB", "ULA", "URA" });
 if (*samusAnimation == 0x08) moveList.insert(moveList.end(), { "A", "U", "R", "L", "D", "B", "UB", "UR", "UL", "UD", "DB", "UA", "RB", "RA", "BA", "LB", "LA", "RBA", "UBA", "LBA", "UDB", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x0A) moveList.insert(moveList.end(), { "A", "B", "D", "U", "L", "R", "UL", "UD", "UB", "UA", "RB", "RA", "UR", "LB", "LA", "DB", "BA", "URA", "URB", "ULB", "ULA", "RBA", "UDB", "UBA", "LBA", "DRB", "DLB", "ULBA", "URBA" });
 if (*samusAnimation == 0x0B) moveList.insert(moveList.end(), { "B", "L", "R", "U", "LB", "RB", "UB", "UL", "UR", "ULB", "URB" });
 if (*samusAnimation == 0x0C) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "LA", "UR", "UL", "UB", "UA", "RB", "RA", "BA", "LB", "RBA", "UBA", "LBA", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x0D) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "RB", "UR", "UL", "UB", "UA", "RA", "LB", "LA", "BA", "LRA", "LBA", "RBA", "DRA", "UBA", "DLA", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x0F) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "LA", "UR", "UL", "UB", "UA", "RB", "RA", "BA", "LB", "RBA", "UBA", "LBA", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x10) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "BA", "URB", "URA", "RBA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x11) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "BA", "URB", "URA", "RBA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x12) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "BA", "URB", "URA", "RBA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x13) moveList.insert(moveList.end(), { "A", "L", "R", "U", "UA" });
 if (*samusAnimation == 0x14) moveList.insert(moveList.end(), { "A", "L", "R", "U" });
 if (*samusAnimation == 0x15) moveList.insert(moveList.end(), { "A", "L", "R", "U" });
 if (*samusAnimation == 0x17) moveList.insert(moveList.end(), { "A", "L", "R", "U", "LA", "RA", "UA", "UL", "UR" });
 if (*samusAnimation == 0x18) moveList.insert(moveList.end(), { "A", "L", "R", "U", "LA", "RA", "UA", "UL", "UR" });
 if (*samusAnimation == 0x19) moveList.insert(moveList.end(), { "A", "L", "R", "U", "LA", "RA", "UA", "UL", "UR" });
 if (*samusAnimation == 0x1A) moveList.insert(moveList.end(), { "A", "L", "R", "U", "LA", "RA", "UA", "UL", "UR" });
 if (*samusAnimation == 0x20) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "LA", "UR", "UL", "UB", "UA", "RB", "RA", "BA", "LB", "RBA", "UBA", "LBA", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x21) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "DB", "UB", "UA", "RB", "RA", "BA", "UL", "LB", "LA", "DA", "UR", "ULA", "ULB", "UBA", "URA", "URB", "DBA", "RBA", "LBA", "DRB", "DLB", "LRBA", "DRBA", "ULBA", "DLBA", "URBA" });
 if (*samusAnimation == 0x23) moveList.insert(moveList.end(), { "A", "B", "U", "L", "R", "RA", "UR", "UL", "UA", "RB", "LB", "LA", "DR", "DL", "BA", "LBA", "DRB", "RBA", "DLB", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x24) moveList.insert(moveList.end(), { "A", "B", "L", "R", "U", "LA", "LB", "RA", "RB", "UA", "UL", "UR" });
 if (*samusAnimation == 0x25) moveList.insert(moveList.end(), { "A", "B", "L", "R", "U", "LA", "LB", "RA", "RB", "UA", "UL", "UR", "ULB", "URB" });
 if (*samusAnimation == 0x27) moveList.insert(moveList.end(), { "A", "U", "R", "L", "D", "B", "DB", "LA", "LB", "BA", "RA", "RB", "UA", "UB", "UR", "UD", "UL", "URA", "URB", "ULB", "ULA", "UDB", "UBA", "RBA", "LBA" });
 if (*samusAnimation == 0x28) moveList.insert(moveList.end(), { "A", "U", "R", "L", "D", "B", "UB", "UR", "UL", "UD", "DB", "UA", "RB", "RA", "BA", "LB", "LA", "RBA", "UBA", "LBA", "UDB", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x34) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "LR", "RB", "UL", "UR", "ULB", "URB" });
 if (*samusAnimation == 0x35) moveList.insert(moveList.end(), { "A", "B", "D", "L", "U", "R", "RB", "UR", "UL", "UD", "UB", "UA", "RA", "LB", "LA", "BA", "DB", "LBA", "URB", "URA", "ULB", "ULA", "UDB", "DBA", "UBA", "RBA", "DLBA", "ULBA", "DRBA", "LRBA", "URBA" });
 if (*samusAnimation == 0x36) moveList.insert(moveList.end(), { "A", "U", "R", "L", "D", "B", "UB", "UR", "UL", "UD", "DB", "UA", "RB", "RA", "BA", "LB", "LA", "RBA", "UBA", "LBA", "UDB", "ULA", "ULB", "URA", "URB", "ULBA", "URBA" });
 if (*samusAnimation == 0x38) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "RA", "UR", "UL", "UA", "RB", "LB", "LA", "BA", "RBA", "URB", "URA", "ULR", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x39) moveList.insert(moveList.end(), { "A", "B", "L", "R", "U", "LA", "LB", "RA", "RB", "UA", "UL", "UR", "RBA", "URB", "URA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x3A) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "LA", "LB", "BA", "RA", "RB", "UA", "UL", "UR", "URB", "RBA", "URA", "ULB", "ULA", "LBA", "ULBA", "URBA" });
 if (*samusAnimation == 0x3C) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "UA", "UR", "UL", "RB", "RA", "LB", "LA", "BA", "RBA", "LBA", "ULA", "ULB", "ULR", "URA", "URB", "UDLB", "UDRB", "ULBA", "ULRB", "URBA" });
 if (*samusAnimation == 0x3E) moveList.insert(moveList.end(), { "A", "B", "L", "U", "R", "UL", "UR", "UA", "RB", "RA", "LB", "LA", "BA", "URA", "URB", "ULR", "ULB", "UDL", "ULA", "UDR", "RBA", "LBA", "UDRB", "ULBA", "ULRB", "UDLB", "URBA" });
 if (*samusAnimation == 0x40) moveList.insert(moveList.end(), { "A", "U", "R", "L", "B", "UR", "UL", "UA", "RB", "RA", "BA", "LB", "LA", "RBA", "LBA", "URB", "ULA", "ULB", "URA", "URBA", "ULRB", "ULBA", "UDRB", "UDLB" });

 return moveList;
}

void GameInstance::printFullMoveList()
{
 for (uint16_t i = 0; i <= 0xFF; i++)
 {
  *samusAnimation = (uint8_t)i;
  auto moves = getPossibleMoves();
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
  size_t skippedFrames = 0;
  std::vector<INPUT_TYPE> moves;

  _emu->advanceState(move);
  moves.push_back(move);

  while (*gameMode != 3 || *NMIFlag == 1)
  {
   _emu->advanceState(0);
   moves.push_back(0);
   skippedFrames++;
   if (skippedFrames > 2048) break;
  }

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

  // Evaluating samus magnet's reward on position X
  diff = std::abs(magnets.samusHorizontalMagnet.center - samusPosX);
  reward += magnets.samusHorizontalMagnet.intensity * -diff;

  // Evaluating samus magnet's reward on position Y
  diff = std::abs(magnets.samusVerticalMagnet.center - samusPosY);
  reward += magnets.samusVerticalMagnet.intensity * -diff;

  // Evaluating bullet1 magnet's reward on position X
  diff = std::abs(magnets.bullet1HorizontalMagnet.center - (float)*bullet1PosX);
  reward += magnets.bullet1HorizontalMagnet.intensity * -diff;

  // Evaluating bullet1 magnet's reward on position Y
  diff = std::abs(magnets.bullet1VerticalMagnet.center - (float)*bullet1PosY);
  reward += magnets.bullet1VerticalMagnet.intensity * -diff;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second );
  LOG("[Jaffar]  + Game Mode:              %02u\n", *gameMode);
  LOG("[Jaffar]  + NMI Flag:               %02u\n", *NMIFlag);
  LOG("[Jaffar]  + Samus Pos X:            %f (%02u %02u %02u)\n", samusPosX, *screenPosX1, *screenPosX2, *samusPosXRaw);
  LOG("[Jaffar]  + Samus Pos Y:            %f (%02u %02u %02u)\n", samusPosY, *screenPosY1, *screenPosY2, *samusPosYRaw);
  LOG("[Jaffar]  + Samus Animation:        %03u\n", *samusAnimation);
  LOG("[Jaffar]  + Samus Direction:        %03u\n", *samusDirection);
  LOG("[Jaffar]  + Samus Door Side:        %03u\n", *samusDoorSide);
  LOG("[Jaffar]  + Samus Door State:       %03u\n", *samusDoorState);
  LOG("[Jaffar]  + Door States:            [ %02u, %02u, %02u, %02u ]\n", *door1State, *door2State, *door3State, *door4State);
  LOG("[Jaffar]  + Bullet Count:           %02u\n", bulletCount);
  LOG("[Jaffar]  + Bullet States:          [ %02u, %02u, %02u ]\n", *bullet1State, *bullet2State, *bullet3State);
  LOG("[Jaffar]  + Bullet Pos X:           [ %02u, %02u, %02u ]\n", *bullet1PosX, *bullet2PosX, *bullet3PosX);
  LOG("[Jaffar]  + Bullet Pos Y:           [ %02u, %02u, %02u ]\n", *bullet1PosY, *bullet2PosY, *bullet3PosY);
  LOG("[Jaffar]  + Equipment Flags:        %03u\n", *equipmentFlags);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.samusHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Samus Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.samusHorizontalMagnet.intensity, magnets.samusHorizontalMagnet.center);
  if (std::abs(magnets.samusVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Samus Vertical Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.samusVerticalMagnet.intensity, magnets.samusVerticalMagnet.center);
  if (std::abs(magnets.bullet1HorizontalMagnet.intensity) > 0.0f) LOG("[Jaffar]  + Bullet 1 Horizontal Magnet     - Intensity: %.5f, Center: %3.3f\n", magnets.bullet1HorizontalMagnet.intensity, magnets.bullet1HorizontalMagnet.center);
  if (std::abs(magnets.bullet1VerticalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Bullet 1 Vertical Magnet       - Intensity: %.5f, Center: %3.3f\n", magnets.bullet1VerticalMagnet.intensity, magnets.bullet1VerticalMagnet.center);
}
