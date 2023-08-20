#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  bikePosX             = (uint8_t*)   &_emu->_baseMem[0x0050];
  bikePosXSubpixel     = (uint8_t*)   &_emu->_baseMem[0x0394];

  intraLoopAdvance     = (uint8_t*)   &_emu->_baseMem[0x00ED];
  loopsRemaining       = (uint8_t*)   &_emu->_baseMem[0x0057];
  currentLoop          = (uint8_t*)   &_emu->_baseMem[0x03A4];
  raceOverFlag         = (uint8_t*)   &_emu->_baseMem[0x0052];

  bikeMoving           = (uint8_t*)   &_emu->_baseMem[0x000E];
  bikeAngle            = (uint8_t*)   &_emu->_baseMem[0x00AC];
  bikeAirMode          = (uint8_t*)   &_emu->_baseMem[0x00B0];

  bikeSpeedX1          = (uint8_t*)   &_emu->_baseMem[0x0090];
  bikeSpeedX256        = (uint8_t*)   &_emu->_baseMem[0x0094];
  bikeEngineTemp       = (uint8_t*)   &_emu->_baseMem[0x03B6];

  bikeSpeedZ           = (uint8_t*)   &_emu->_baseMem[0x00DC];
  bikePosZ             = (uint8_t*)   &_emu->_baseMem[0x0070];
  bikePosZSubpixel     = (uint8_t*)   &_emu->_baseMem[0x00B8];

  bikePosY             = (uint8_t*)   &_emu->_baseMem[0x008C];
  bikeSpeedY1          = (uint8_t*)   &_emu->_baseMem[0x0270];
  bikeSpeedY2          = (uint8_t*)   &_emu->_baseMem[0x0274];

  bikeflightMode1      = (uint8_t*)   &_emu->_baseMem[0x00F6];
  bikeflightMode2      = (uint8_t*)   &_emu->_baseMem[0x0380];
  bikeflightMode3      = (uint8_t*)   &_emu->_baseMem[0x0384];

  gameCycle            = (uint8_t*)   &_emu->_baseMem[0x004C];
  currentBlockX        = (uint8_t*)   &_emu->_baseMem[0x004E];
  prevBlockX           = (uint8_t*)   &_emu->_baseMem[0x07C0];
  blockXTransitions    = (uint8_t*)   &_emu->_baseMem[0x07C1];

  curSpeed              = (uint16_t*)  &_emu->_baseMem[0x07C2];
  maxSpeed              = (uint16_t*)  &_emu->_baseMem[0x07C4];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 moves.push_back(move);
 updateDerivedValues();

 return moves;
}


// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(currentStep % timerTolerance);

  hash.Update(*bikePosX);
  hash.Update(*bikePosXSubpixel);
  hash.Update(*intraLoopAdvance);
  hash.Update(*loopsRemaining);
  hash.Update(*currentLoop);
  hash.Update(*raceOverFlag);

  hash.Update(*bikeMoving);
  hash.Update(*bikeAngle);
  hash.Update(*bikeAirMode);
  hash.Update(*bikeSpeedX1);
  hash.Update(*bikeSpeedX256);
  hash.Update(*bikeEngineTemp);

  hash.Update(*bikeSpeedZ);
  hash.Update(*bikePosZ);
  hash.Update(*bikePosZSubpixel);

  hash.Update(*bikePosY);
  hash.Update(*bikeSpeedY1);
  hash.Update(*bikeSpeedY2);

  hash.Update(*bikeflightMode1);
  hash.Update(*bikeflightMode2);
//  hash.Update(*bikeflightMode3);

//  for (size_t i = 0; i < 0x800; i++)
//   if (i != 0x0014)
//   if (i != 0x0015)
//   if (i != 0x005C)
//   if (i != 0x00FC)
//   if (i != 0x00FE)
//   if (i != 0x0020)
//   if (i != 0x003F)
//   if (i != 0x004C)
//   if (i != 0x006A)
//   if (i != 0x006B)
//   if (i != 0x031B)
//   if (i != 0x03A9)
//   if (i != 0x03B5)
//   if (i != 0x03D7)
//   if (i != 0x03DF)
//    hash.Update(_emu->_baseMem[i]);

//  hash.Update(*gameCycle);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 if (*prevBlockX != *currentBlockX) (*blockXTransitions)++;

 *prevBlockX = *currentBlockX;

 _emu->_baseMem[0x0081] = 255;
 _emu->_baseMem[0x0082] = 255;
 _emu->_baseMem[0x0083] = 255;

 *curSpeed =  256 * (uint16_t)*bikeSpeedX256 + (uint16_t)*bikeSpeedX1;
 if (*curSpeed > *maxSpeed) *maxSpeed = *curSpeed;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

 // Getting mini-hash
 auto stateMiniHash = getStateMoveHash();

 if (stateMiniHash == 0x0000) moveList.insert(moveList.end(), { "U", "D", "L", "R", "LB", "UL", "DL", "UR", "DR", "ULB", "DLB" });
 if (stateMiniHash == 0x0001) moveList.insert(moveList.end(), { "U", "D", "L", "R", "LB", "UL", "DL", "UR", "DR", "ULB", "DLB" });
 if (stateMiniHash == 0x0002) moveList.insert(moveList.end(), { "U", "D", "L", "R", "LB", "UL", "DL", "UR", "DR", "ULB", "DLB" });
 if (stateMiniHash == 0x0003) moveList.insert(moveList.end(), { "B", "U", "A", "D", "L", "R", "LR", "DR", "UR", "RB", "RA", "DL", "UL", "LB", "LA", "DB", "DA", "UB", "UA", "ULA", "ULB", "DLA", "DLB", "URA", "URB", "DRA", "DRB" });
 if (stateMiniHash == 0x0010) moveList.insert(moveList.end(), { "U", "D", "L", "R", "UL", "DL", "UR", "DR" });
 if (stateMiniHash == 0x0011) moveList.insert(moveList.end(), { "U", "D", "L", "R", "UL", "DL", "UR", "DR" });
 if (stateMiniHash == 0x0012) moveList.insert(moveList.end(), { "U", "D", "L", "R", "UL", "DL", "UR", "DR" });
 if (stateMiniHash == 0x0013) moveList.insert(moveList.end(), { "U", "D", "L", "R", "UL", "DL", "UR", "DR", "LR" });
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

  // Reward is clearly advancing
  float dCurrentLoop = (float) *currentLoop;
  float dLoopAdvance = (float) *intraLoopAdvance;
  float dSpeedX = (float) *bikeSpeedX256 + (float) *bikeSpeedX1 / 256.0;

  reward +=  256.0 * 256.0 * dCurrentLoop + *blockXTransitions*256.0 + (float)*bikePosX + dSpeedX / 0.1 ;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                            %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                              0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Cycle:                        %02u\n", *gameCycle);
 LOG("[Jaffar]  + Current / Remaining Loop:          %02u/%02u\n", *currentLoop, *loopsRemaining);
 LOG("[Jaffar]  + Loop Advance:                      %02u\n", *intraLoopAdvance);
 LOG("[Jaffar]  + Block X:                           Cur: %02u, Prev: %02u, Count: %02u\n", *currentBlockX, *prevBlockX, *blockXTransitions);
 LOG("[Jaffar]  + Bike Pos X:                        %02u (%02u)\n", *bikePosX, *bikePosXSubpixel);
 LOG("[Jaffar]  + Bike Pos Y:                        %02u\n", *bikePosY);
 LOG("[Jaffar]  + Bike Pos Z:                        %02u (%02u)\n", *bikePosZ, *bikePosZSubpixel);
 LOG("[Jaffar]  + Bike Pos Z:                        %02u (%02u)\n", *bikePosZ, *bikePosZSubpixel);
 LOG("[Jaffar]  + Bike Speed:                        %05u (Max: %05u)\n", *curSpeed, *maxSpeed);
 LOG("[Jaffar]  + Bike Speed X:                      %02u (%02u)\n", *bikeSpeedX256, *bikeSpeedX1);
 LOG("[Jaffar]  + Bike Speed Y:                      %02u (%02u)\n", *bikeSpeedY1, *bikeSpeedY2);
 LOG("[Jaffar]  + Bike Speed Z:                      %02u\n", *bikeSpeedZ);
 LOG("[Jaffar]  + Bike Air Mode:                     %02u\n", *bikeAirMode);
 LOG("[Jaffar]  + Bike Flight:                       %02u %02u %02u\n", *bikeflightMode1, *bikeflightMode2, *bikeflightMode3);
 LOG("[Jaffar]  + Bike Engine Temp:                  %02u\n", *bikeEngineTemp);
 LOG("[Jaffar]  + Bike Angle:                        %02u\n", *bikeAngle);
 LOG("[Jaffar]  + Bike Moving:                       %02u\n", *bikeMoving);
 LOG("[Jaffar]  + Race Over Flag:                    %02u\n", *raceOverFlag);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

}

uint64_t GameInstance::getStateMoveHash() const
{
 return *bikeAirMode * 8 + *gameCycle;
}

std::set<INPUT_TYPE> GameInstance::getCandidateMoves() const
{
 std::set<INPUT_TYPE> candidateMoves;

 for (INPUT_TYPE i = 0; i < 0b11111111; i++)
 {
   if ((i & 0b00001000) == 0) // NES Start
   if ((i & 0b00000100) == 0) // NES Select
   if (countButtonsPressedNumber(i) > 3 == false)
   candidateMoves.insert(i);
 }

 return candidateMoves;
}
