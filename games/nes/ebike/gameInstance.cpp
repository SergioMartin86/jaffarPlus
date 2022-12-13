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

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

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
  //hash.Update(*bikeEngineTemp);

  hash.Update(*bikeSpeedZ);
  hash.Update(*bikePosZ);
  //hash.Update(*bikePosZSubpixel);

  hash.Update(*bikePosY);
  hash.Update(*bikeSpeedY1);
  //hash.Update(*bikeSpeedY2);

  hash.Update(*bikeflightMode1);
  hash.Update(*bikeflightMode2);
  hash.Update(*bikeflightMode3);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{

}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*bikeAirMode == 0x00) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "...U...A", "...U..B.", "..D.....", "..D....A", "..D...B.", ".L......", ".L.....A", ".L....B.", ".L.U....", ".L.U...A", ".L.U..B.", ".LD.....", ".LD....A", ".LD...B.", "R.......", "R......A", "R.....B.", "R..U....", "R..U...A", "R..U..B.", "R.D.....", "R.D....A", "R.D...B."});
 if (*bikeAirMode == 0x00) moveList.insert(moveList.end(), { "...U.sBA", "..DU..BA", ".L...sBA", ".L.U..BA", ".LD...BA", "R....sBA", "R..U..BA", "R.D...BA", "RL....BA", "RL...sB.", "RL.U.s.."});
 if (*bikeAirMode == 0x02) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "...U...A", "...U..B.", "..D.....", "..D...B.", ".L......", ".L....B.", ".L.U....", ".L.U...A", ".L.U..B.", ".LD.....", ".LD....A", ".LD...B.", "R.......", "R......A", "R.....B.", "R..U....", "R..U...A", "R..U..B.", "R.D.....", "R.D....A", "R.D...B.", "RL......", "RL....B.", "RL.U....", "RL.U..B.", "RLD.....", "RLD...B."});

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
  double dCurrentLoop = (double) *currentLoop;
  double dLoopAdvance = (double) *intraLoopAdvance;
  double dSpeedX256 = (double) *bikeSpeedX256;
  double dSpeedX1 = (double) *bikeSpeedX1;

  reward +=  (dCurrentLoop * 256.0 * 256.0 * 256.0 + dLoopAdvance*256.0*256.0 + dSpeedX256*256.0 + dSpeedX1) / (256.0);

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                            %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                              0x%lX\n", computeHash());
 LOG("[Jaffar]  + Current / Remaining Loop:          %02u/%02u\n", *currentLoop, *loopsRemaining);
 LOG("[Jaffar]  + Loop Advance:                      %02u\n", *intraLoopAdvance);
 LOG("[Jaffar]  + Bike Pos X:                        %02u (%02u)\n", *bikePosX, *bikePosXSubpixel);
 LOG("[Jaffar]  + Bike Pos Y:                        %02u\n", *bikePosY);
 LOG("[Jaffar]  + Bike Pos Z:                        %02u (%02u)\n", *bikePosZ, *bikePosZSubpixel);
 LOG("[Jaffar]  + Bike Speed X:                      %02u (%02u)\n", *bikeSpeedX256, *bikeSpeedX1);
 LOG("[Jaffar]  + Bike Speed Y:                      %02u (%02u)\n", *bikeSpeedY1, *bikeSpeedY2);
 LOG("[Jaffar]  + Bike Speed Z:                      %02u (%02u)\n", *bikeSpeedZ);
 LOG("[Jaffar]  + Bike Air Mode:                     %02u\n", *bikeAirMode);
 LOG("[Jaffar]  + Bike Flight:                       %02u %02u %02u\n", *bikeflightMode1, *bikeflightMode2, *bikeflightMode3);
 LOG("[Jaffar]  + Bike Engine Temp:                  %02u\n", *bikeEngineTemp);
 LOG("[Jaffar]  + Bike Angle:                        %02u\n", *bikeAngle);
 LOG("[Jaffar]  + Bike Moving:                       %02u\n", *bikeMoving);
 LOG("[Jaffar]  + Race Over Flag:                    %02u\n", *raceOverFlag);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

}
