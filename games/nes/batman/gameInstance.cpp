#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  gameMode                    = (uint8_t*)   &_emu->_baseMem[0x0000];
  globalTimer                 = (uint8_t*)   &_emu->_baseMem[0x00B4];
  RNGValue                    = (uint8_t*)   &_emu->_baseMem[0x001F];
  batmanAnimationState        = (uint8_t*)   &_emu->_baseMem[0x00AA];
  batmanAction                = (uint8_t*)   &_emu->_baseMem[0x00DE];
  batmanDirection             = (uint8_t*)   &_emu->_baseMem[0x00DD]; //
  batmanHP                    = (uint8_t*)   &_emu->_baseMem[0x00B7];

  batmanScreenPosX            = (uint8_t*)   &_emu->_baseMem[0x00B6];
  batmanScreenPosY            = (uint8_t*)   &_emu->_baseMem[0x00B7];
  batmanWeapon                = (uint8_t*)   &_emu->_baseMem[0x00A6];
  batmanAmmo                  = (uint8_t*)   &_emu->_baseMem[0x00B8];

  batmanPosX1                 = (uint8_t*)   &_emu->_baseMem[0x00D3];
  batmanPosX2                 = (uint8_t*)   &_emu->_baseMem[0x00D4];
  batmanPosX3                 = (uint8_t*)   &_emu->_baseMem[0x00D5];
  batmanPosY1                 = (uint8_t*)   &_emu->_baseMem[0x00D0];
  batmanPosY2                 = (uint8_t*)   &_emu->_baseMem[0x00D1];
  batmanPosY3                 = (uint8_t*)   &_emu->_baseMem[0x00D2];
  bossX                       = (uint8_t*)   &_emu->_baseMem[0x0511]; //
  bossY                       = (uint8_t*)   &_emu->_baseMem[0x0000]; //
  bossHP                      = (uint8_t*)   &_emu->_baseMem[0x05E0];
  batmanJumpTimer             = (uint8_t*)   &_emu->_baseMem[0x00DF];
  levelStartTimer             = (uint8_t*)   &_emu->_baseMem[0x0012];

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

//////  hash.Update(*RNGValue    );
////  hash.Update(*gameMode);
////  hash.Update(*batmanHP    );
//  hash.Update(*batmanPosX1 );
//  hash.Update(*batmanPosX2 );
////  hash.Update(*batmanPosX3 );
//  hash.Update(*batmanPosY1 );
//  hash.Update(*batmanPosY2 );
////  hash.Update(*batmanPosY3 );
//  hash.Update(*bossX       );
//  hash.Update(*bossHP      );
//  hash.Update(*batmanAction);
//  hash.Update(*batmanDirection);
//  hash.Update(*batmanAnimationState);
////  hash.Update(*batmanScreenPosX);
////  hash.Update(*batmanScreenPosY);
//  hash.Update(*batmanWeapon);
////  hash.Update(*batmanAmmo);
//  hash.Update(*batmanJumpTimer);
////
////  hash.Update(_emu->_baseMem[0x00F7]); // Joy Hold!
//  hash.Update(_emu->_baseMem[0x00AA]);
//  hash.Update(_emu->_baseMem[0x00AB]);
//  hash.Update(_emu->_baseMem[0x00C3]);
//  hash.Update(_emu->_baseMem[0x00D7]);
//  hash.Update(_emu->_baseMem[0x00DC]);
//  hash.Update(_emu->_baseMem[0x00DD]);
//  hash.Update(_emu->_baseMem[0x00DE]);
//  hash.Update(_emu->_baseMem[0x00DF]);
//
//  hash.Update(&_emu->_baseMem[0x0000], 0xF3);
//  hash.Update(&_emu->_baseMem[0x0080], 0x34);
//  hash.Update(&_emu->_baseMem[0x00B5], 0x3F);
//  hash.Update(&_emu->_baseMem[0x0100], 0x6FF);

  hash.Update(&_emu->_baseMem[0x0030], 0x00F4 - 0x0030);
  hash.Update(&_emu->_baseMem[0x00F9], 0x0800 - 0x00F9);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 batmanPosX = (float)*batmanPosX1 * 256.0f + (float)*batmanPosX2 + (float)*batmanPosX3 / 256.0;
 batmanPosY = (float)*batmanPosY1 * 256.0f + (float)*batmanPosY2 + (float)*batmanPosY3 / 256.0;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

 if (*batmanAction == 0x0000) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA", "S", "RS", "LS",  });
 if (*batmanAction == 0x0001) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DL", "LRA" });
 if (*batmanAction == 0x0002) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DL", "LRA" });
 if (*batmanAction == 0x0003) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DL", "LRA" });
 if (*batmanAction == 0x0004) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA", "LB" });
 if (*batmanAction == 0x0005) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x0006) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x0007) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x0008) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA", "S", "SA", "LSA", "RSA" });
 if (*batmanAction == 0x0009) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x000A) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x000B) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x000C) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x000D) moveList.insert(moveList.end(), { "A", "B", "R", "L", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA", "S" });
 if (*batmanAction == 0x000E) moveList.insert(moveList.end(), { "A", "B", "R", "L", "RB", "LB", "RA", "LA" });
 if (*batmanAction == 0x0010) moveList.insert(moveList.end(), { "B", "R", "L", "D", "RB", "LB" });
 if (*batmanAction == 0x0011) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LB", "RBA" });
 if (*batmanAction == 0x0012) moveList.insert(moveList.end(), { "A", "B", "R", "L", "RA", "RB", "LB", "RBA" });
 if (*batmanAction == 0x0015) moveList.insert(moveList.end(), { "A" });
 if (*batmanAction == 0x0016) moveList.insert(moveList.end(), { "A", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x0020) moveList.insert(moveList.end(), { "A", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x0029) moveList.insert(moveList.end(), { "A", "R", "L", "D", "RA", "LA" });
 if (*batmanAction == 0x002A) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB" });
 if (*batmanAction == 0x002B) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LB" });
 if (*batmanAction == 0x002C) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RB", "LB", "DRA" });

 if (*batmanAction == 0x000D) moveList.insert(moveList.end(), { "DB", "DBA" });
 if (*batmanAction == 0x000E) moveList.insert(moveList.end(), { "D", "RBA" });
 if (*batmanAction == 0x0010) moveList.insert(moveList.end(), { "A", "BA", "RA", "LA", "DA", "DB", "RBA", "LBA", "DBA", "DRB", "DLB" });
 if (*batmanAction == 0x0011) moveList.insert(moveList.end(), { "BA", "LA", "DB", "LBA", "DBA", "RS", "LS" });
 if (*batmanAction == 0x0012) moveList.insert(moveList.end(), { "BA", "LA", "LBA", "DBA" });
 if (*batmanAction == 0x0017) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x0018) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RB", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x0019) moveList.insert(moveList.end(), { "D", "DR", "DL" });
 if (*batmanAction == 0x001A) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x001B) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RB", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x001C) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x001D) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x001E) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x0021) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x0022) moveList.insert(moveList.end(), { "B", "R", "L", "D", "RB", "LB", "DR", "DL" });
 if (*batmanAction == 0x0023) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DL", "LRA" });
 if (*batmanAction == 0x0024) moveList.insert(moveList.end(), { "A", "R", "L", "D", "RA", "LA", "DR", "DL", "LRA" });
 if (*batmanAction == 0x0025) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RB", "LB", "DR", "DL" });
 if (*batmanAction == 0x0026) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x0027) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DB", "DR", "DL", "LRA", "DRB", "DLB" });
 if (*batmanAction == 0x0028) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DR", "DL", "LRA" });
 if (*batmanAction == 0x0029) moveList.insert(moveList.end(), { "B", "BA", "RB", "LB", "DR", "RBA", "LBA", "DLA", "DLB" });
 if (*batmanAction == 0x002A) moveList.insert(moveList.end(), { "BA", "RBA", "LBA" });
 if (*batmanAction == 0x002B) moveList.insert(moveList.end(), { "LA", "DA", "DLA", "DLB" });
 if (*batmanAction == 0x002C) moveList.insert(moveList.end(), { "LA", "DA", "RA" });
 if (*batmanAction == 0x002D) moveList.insert(moveList.end(), { "B", "R", "A", "L", "D", "BA", "RA", "RB", "LA", "LB", "DA", "DB", "DR", "DL", "DLB", "DLA", "DRB", "DRA", "LBA", "RBA" });
 if (*batmanAction == 0x002E) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "BA", "RA", "RB", "LA", "LB", "DA", "RBA", "LBA", "DRA", "DRB", "DLA", "DR", "LR" });
 if (*batmanAction == 0x002F) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA", "DRA", "DLA" });
 if (*batmanAction == 0x0030) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "BA", "RA", "RB", "LA", "LB", "RBA", "LBA" });
 if (*batmanAction == 0x0031) moveList.insert(moveList.end(), { "B", "R", "D", "L", "A", "LA", "DB", "DA", "LB", "RB", "RA", "BA", "RBA", "LBA", "DRA", "DLA", "DLB", "DR" });


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
  diff = std::abs(magnets.batmanHorizontalMagnet.center - batmanPosX);
  float weightedDiffX = magnets.batmanHorizontalMagnet.intensity * diff;
  reward -= weightedDiffX;

  float jumpPreparedness = 0.0;
  if (*batmanAction == 8 && *batmanJumpTimer > 1) jumpPreparedness = 8.0 - (float)*batmanJumpTimer;
  if (*batmanAction == 13) jumpPreparedness = 8.0;
  float actualPosY = batmanPosY - jumpPreparedness;
  diff = std::abs(magnets.batmanVerticalMagnet.center - actualPosY);
  float weightedDiffY = magnets.batmanVerticalMagnet.intensity * diff;
  reward -= weightedDiffY;

  reward += magnets.batmanDistanceToBossMagnet * std::abs( (float)*batmanPosX2 - (float)*bossX );
  reward += magnets.batmanHPMagnet * (float)*batmanHP;
  reward += magnets.bossHPMagnet * (float)*bossHP;
  reward += magnets.ammoMagnet * (float)*batmanAmmo;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second );
  LOG("[Jaffar]  + Game Mode:              %02u\n", *gameMode);
  LOG("[Jaffar]  + Global Timer:           %02u\n", *globalTimer);
  LOG("[Jaffar]  + Level Start Timer:      %02u\n", *levelStartTimer);
  LOG("[Jaffar]  + Batman HP:              %02u\n", *batmanHP);
  LOG("[Jaffar]  + Batman Weapon:          %02u\n", *batmanWeapon);
  LOG("[Jaffar]  + Batman Ammo:            %02u\n", *batmanAmmo);
  LOG("[Jaffar]  + Batman Animation State: %02u\n", *batmanAnimationState);
  LOG("[Jaffar]  + Batman Action:          0x%02X\n", *batmanAction);
  LOG("[Jaffar]  + Batman Direction:       %02u\n", *batmanDirection);
  LOG("[Jaffar]  + Batman Jump Timer:      %02u\n", *batmanJumpTimer);
  LOG("[Jaffar]  + Batman Pos X:           %.2f (%02u %02u %02u)\n", batmanPosX, *batmanPosX1, *batmanPosX2, *batmanPosX3);
  LOG("[Jaffar]  + Batman Pos Y:           %.2f (%02u %02u %02u)\n", batmanPosY, *batmanPosY1, *batmanPosY2, *batmanPosY3);
  LOG("[Jaffar]  + Batman Screen Pos:      [ %03u, %03u ]\n", *batmanScreenPosX, *batmanScreenPosY);
  LOG("[Jaffar]  + Boss Pos:               [ %02u, %02u ]\n", *bossX, *bossY);
  LOG("[Jaffar]  + Boss HP:                %02u\n", *bossHP);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.batmanDistanceToBossMagnet) > 0.0f)           LOG("[Jaffar]  + Batman Distance To Boss Magnet  - Intensity: %.5f\n", magnets.batmanDistanceToBossMagnet);
  if (std::abs(magnets.batmanHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Batman Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.batmanHorizontalMagnet.intensity, magnets.batmanHorizontalMagnet.center);
  if (std::abs(magnets.batmanVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Batman Vertical Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.batmanVerticalMagnet.intensity, magnets.batmanVerticalMagnet.center);
  if (std::abs(magnets.bossHPMagnet) > 0.0f)                         LOG("[Jaffar]  + Boss HP Magnet                  - Intensity: %.5f\n", magnets.bossHPMagnet);
  if (std::abs(magnets.batmanHPMagnet) > 0.0f)                       LOG("[Jaffar]  + Batman HP Magnet                - Intensity: %.5f\n", magnets.batmanHPMagnet);
  if (std::abs(magnets.ammoMagnet) > 0.0f)                           LOG("[Jaffar]  + Ammo Magnet                     - Intensity: %.5f\n", magnets.ammoMagnet);
}
