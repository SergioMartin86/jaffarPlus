#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  globalTimer                 = (uint8_t*)   &_emu->_baseMem[0x00A5];
  RNGValue                    = (uint8_t*)   &_emu->_baseMem[0x00B0];
  lagFrame                    = (uint8_t*)   &_emu->_baseMem[0x00A4];
  lagType                     = (uint8_t*)   &_emu->_baseMem[0x00A1];
  playerAction                = (uint8_t*)   &_emu->_baseMem[0x0400];
  playerDirection             = (uint8_t*)   &_emu->_baseMem[0x0440];
  playerHP                    = (uint8_t*)   &_emu->_baseMem[0x00B1];
  playerPosX1                 = (uint8_t*)   &_emu->_baseMem[0x04E0];
  playerPosX2                 = (uint8_t*)   &_emu->_baseMem[0x04C0];
  playerPosX3                 = (uint8_t*)   &_emu->_baseMem[0x04C1];
  playerPosY1                 = (uint8_t*)   &_emu->_baseMem[0x0540];
  playerPosY2                 = (uint8_t*)   &_emu->_baseMem[0x0520];
  playerPosY3                 = (uint8_t*)   &_emu->_baseMem[0x0521];
  bossX                       = (uint8_t*)   &_emu->_baseMem[0x04DD];
  bossY                       = (uint8_t*)   &_emu->_baseMem[0x053D];
  bossHP                      = (uint8_t*)   &_emu->_baseMem[0x05FF];
  bossTimer                   = (uint8_t*)   &_emu->_baseMem[0x065F];
  openLevel                   = (uint8_t*)   &_emu->_baseMem[0x007D];

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

//  hash.Update(*RNGValue    );
  hash.Update(*lagFrame    );
//  hash.Update(*lagType     );
  hash.Update(*playerHP    );
  hash.Update(*playerPosX1 );
  hash.Update(*playerPosX2 );
  hash.Update(*playerPosX3 );
  hash.Update(*playerPosY1 );
  hash.Update(*playerPosY2 );
  hash.Update(*playerPosY3 );
  hash.Update(*bossX       );
  hash.Update(*bossY       );
  hash.Update(*bossHP      );
  hash.Update(*bossTimer);
  hash.Update(*playerAction);

  if (*lagFrame == 1) hash.Update(&_emu->_baseMem[0x00A0], 0x040);

  hash.Update(&_emu->_baseMem[0x0100], 0x0080);
//  hash.Update(&_emu->_baseMem[0x0260], 0x0090);
//  hash.Update(&_emu->_baseMem[0x0300], 0x0100);
  hash.Update(&_emu->_baseMem[0x0400], 0x0100);
  hash.Update(&_emu->_baseMem[0x0600], 0x0080);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 playerPosX = (float)*playerPosX1 * 256.0f + (float)*playerPosX2 + (float)*playerPosX3 / 256.0;
 playerPosY = (float)*playerPosY1 * 256.0f + (float)*playerPosY2 + (float)*playerPosY3 / 256.0;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};
 if (*playerAction == 0x0000) moveList.insert(moveList.end(), { "A", "B", "R", "L", "BA", "RA", "LA", "DA", "DB", "UB", "DRA", "DLA", "UBA", "UDB" });
 if (*playerAction == 0x0001) moveList.insert(moveList.end(), { "A", "B", "R", "L", "RA", "RB", "LA", "LB", "DB", "UB" });
 if (*playerAction == 0x0002) moveList.insert(moveList.end(), { "A", "B", "R", "L", "RA", "RB", "LA", "LB", "DA", "DB", "UB", "DRA", "DLA", "UDB" });
 if (*playerAction == 0x0003) moveList.insert(moveList.end(), { "B", "R", "A", "L", "D", "BA", "RA", "RB", "LA", "LB", "UB", "DA", "UBA", "DLA", "DRA", "DBA", "LBA", "RBA", "DRBA", "DLBA" });
 if (*playerAction == 0x0004) moveList.insert(moveList.end(), { "B", "U", "D", "L", "R", "A", "RB", "RA", "LA", "LB", "DA", "DB", "UB", "UD", "DRA", "DLA", "UDB" });
 if (*playerAction == 0x0005) moveList.insert(moveList.end(), { "B", "U", "D", "L", "R", "A", "RB", "RA", "LA", "LB", "DA", "DB", "UB", "UD", "DRA", "DLA", "UDB" });
 if (*playerAction == 0x0006) moveList.insert(moveList.end(), { "B", "U", "D", "L", "A", "R", "RB", "RA", "LA", "LB", "UD", "BA", "DA", "DB", "UB", "UBA", "UDB", "DLA", "DRA", "LBA", "RBA" });
 if (*playerAction == 0x0007) moveList.insert(moveList.end(), { "B", "U", "D", "L", "A", "R", "RB", "RA", "LA", "LB", "UD", "BA", "DA", "DB", "UB", "UBA", "UDB", "DLA", "DRA", "LBA", "RBA" });
 if (*playerAction == 0x0009) moveList.insert(moveList.end(), { "B", "R", "A", "L", "U", "BA", "RA", "RB", "LA", "LB", "UB", "DA", "UBA", "DLA", "DRA", "DBA", "LBA", "RBA", "DRBA", "DLBA" });
 if (*playerAction == 0x0010) moveList.insert(moveList.end(), { "B", "R", "L", "RB", "LB", "DB", "DRB", "DLB" });
 if (*playerAction == 0x0011) moveList.insert(moveList.end(), { "R", "L" });
 if (*playerAction == 0x0012) moveList.insert(moveList.end(), { "R", "L" });
 if (*playerAction == 0x0013) moveList.insert(moveList.end(), { "B", "R", "L", "RB", "LB" });
 if (*playerAction == 0x0014) moveList.insert(moveList.end(), { "R", "L" });
 return moveList;
}


std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 _emu->advanceState(move); moves.push_back(move);
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

  // Evaluating samus magnet's reward on position X and Y
  diff = std::abs(magnets.playerHorizontalMagnet.center - playerPosX);
  float weightedDiffX = magnets.playerHorizontalMagnet.intensity * diff;

  diff = std::abs(magnets.playerVerticalMagnet.center - playerPosY);
  float weightedDiffY = magnets.playerVerticalMagnet.intensity * diff;

  reward += -sqrt(weightedDiffX*weightedDiffX + weightedDiffY*weightedDiffY);

  reward += magnets.bossHPMagnet * (float)*bossHP;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second );
  LOG("[Jaffar]  + Global Timer:           %02u\n", *globalTimer);
  LOG("[Jaffar]  + Lag Frame:              %02u (Type: %02u)\n", *lagFrame, *lagType);
  LOG("[Jaffar]  + Player HP:              %02u\n", *playerHP);
  LOG("[Jaffar]  + Player Action:          %02u\n", *playerAction);
  LOG("[Jaffar]  + Player Direction:       %02u\n", *playerDirection);
  LOG("[Jaffar]  + Player Pos X:           %.2f (%02u %02u %02u)\n", playerPosX, *playerPosX1, *playerPosX2, *playerPosX3);
  LOG("[Jaffar]  + Player Pos Y:           %.2f (%02u %02u %02u)\n", playerPosY, *playerPosY1, *playerPosY2, *playerPosY3);
  LOG("[Jaffar]  + Boss Pos:               [ %02u, %02u ]\n", *bossX, *bossY);
  LOG("[Jaffar]  + Boss HP:                %02u\n", *bossHP);
  LOG("[Jaffar]  + Boss Timer:             %03u\n", *bossTimer);
  LOG("[Jaffar]  + Open Level:             %03u\n", *openLevel);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.playerHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Samus Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.playerHorizontalMagnet.intensity, magnets.playerHorizontalMagnet.center);
  if (std::abs(magnets.playerVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Samus Vertical Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.playerVerticalMagnet.intensity, magnets.playerVerticalMagnet.center);
  if (std::abs(magnets.bossHPMagnet) > 0.0f)                       LOG("[Jaffar]  + Boss HP Magnet                 - Intensity: %.5f\n", magnets.bossHPMagnet);
}
