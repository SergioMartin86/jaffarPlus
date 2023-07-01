#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  gameMode                            = (uint8_t*)   &_emu->_baseMem[0x0031];
  screenPosX1                         = (uint8_t*)   &_emu->_baseMem[0x0045];
  screenPosX2                         = (uint8_t*)   &_emu->_baseMem[0x0044];
  playerPosX                          = (uint8_t*)   &_emu->_baseMem[0x003F];
  playerPosY                          = (uint8_t*)   &_emu->_baseMem[0x0040];
  playerJumpStrength                  = (uint8_t*)   &_emu->_baseMem[0x0049];
  playerAttackState                   = (uint8_t*)   &_emu->_baseMem[0x004B];
  playerAction                        = (uint8_t*)   &_emu->_baseMem[0x004D];
  playerDirection                     = (uint8_t*)   &_emu->_baseMem[0x0042];

  shunHP2                           = (uint8_t*)   &_emu->_baseMem[0x005B];
  shunMP2                           = (uint8_t*)   &_emu->_baseMem[0x0065];

  seiyaHP2                           = (uint8_t*)   &_emu->_baseMem[0x0059];
  seiyaMP2                           = (uint8_t*)   &_emu->_baseMem[0x0063];

  shiryuMP2                           = (uint8_t*)   &_emu->_baseMem[0x0069];
  hyogaMP2                           = (uint8_t*)   &_emu->_baseMem[0x0067];

  MenuShunMP1                             = (uint8_t*)   &_emu->_baseMem[0x05BD];
  MenuShunMP2                             = (uint8_t*)   &_emu->_baseMem[0x05BC];
  MenuShunHP1                             = (uint8_t*)   &_emu->_baseMem[0x05CF];
  MenuShunHP2                             = (uint8_t*)   &_emu->_baseMem[0x05CE];

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

  hash.Update(*gameMode          );
  hash.Update(*screenPosX1       );
  hash.Update(*screenPosX2       );
  hash.Update(*playerPosX        );
  hash.Update(*playerPosY        );
  hash.Update(*playerJumpStrength);
  hash.Update(*playerAttackState );
  hash.Update(*playerDirection   );
//  hash.Update(*shunHP2         );
  hash.Update(*shunMP2         );
//  hash.Update(*seiyaHP2         );
  hash.Update(*seiyaMP2         );
  hash.Update(*shiryuMP2         );
  hash.Update(*hyogaMP2         );
  hash.Update(*MenuShunMP1           );
  hash.Update(*MenuShunMP2           );
  hash.Update(*MenuShunHP1           );
  hash.Update(*MenuShunHP2           );

  // Player Stats
  hash.Update(_emu->_baseMem[0x027]);
  hash.Update(_emu->_baseMem[0x028]);
  hash.Update(_emu->_baseMem[0x029]);
  hash.Update(_emu->_baseMem[0x02A]);
  hash.Update(_emu->_baseMem[0x030]);
  hash.Update(_emu->_baseMem[0x031]);
  hash.Update(_emu->_baseMem[0x032]);
  hash.Update(_emu->_baseMem[0x033]);
  hash.Update(_emu->_baseMem[0x03E]);
  hash.Update(_emu->_baseMem[0x03F]);
  hash.Update(_emu->_baseMem[0x040]);
  hash.Update(_emu->_baseMem[0x041]);
  hash.Update(_emu->_baseMem[0x042]);
  hash.Update(_emu->_baseMem[0x043]);
  hash.Update(_emu->_baseMem[0x044]);
  hash.Update(_emu->_baseMem[0x045]);
  hash.Update(_emu->_baseMem[0x046]);
  hash.Update(_emu->_baseMem[0x047]);
//  hash.Update(_emu->_baseMem[0x048]);
  hash.Update(_emu->_baseMem[0x049]);
  hash.Update(_emu->_baseMem[0x04A]);
  hash.Update(_emu->_baseMem[0x04B]);
  hash.Update(_emu->_baseMem[0x04C]);
  hash.Update(_emu->_baseMem[0x04D]);
  hash.Update(&_emu->_baseMem[0x04E], 0x008);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 screenPosX = (float)*screenPosX1 * 256.0f + (float)*screenPosX2;
 realPlayerPosX = *playerPosX <= 160 ? (float) *playerPosX : -256.0f + (float) *playerPosX;

}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

 if (*playerAction == 0x0000) moveList.insert(moveList.end(), { "D", "L", "R", "A", "RA", "UA", "DA", "LA", "DRA", "DLA", "B", "RB", "LB" });
 if (*playerAction == 0x0001) moveList.insert(moveList.end(), { "D", "L", "R", "A", "RA", "UA", "DA", "LA", "DRA", "DLA", "B", "LB", "RB" });
 if (*playerAction == 0x0010) moveList.insert(moveList.end(), { "D", "L", "R", "A", "RA", "UA", "DA", "LA", "DRA", "DLA", "B", "LB", "RB" });
 if (*playerAction == 0x0011) moveList.insert(moveList.end(), { "D", "L", "R", "A", "RA", "UA", "DA", "LA", "DRA", "DLA", "B", "LB", "RB" });
 if (*playerAction == 0x0012) moveList.insert(moveList.end(), { "A", "R", "L", "LA", "UA", "LRA", "RB", "LB", "BA" });
 if (*playerAction == 0x0012) moveList.insert(moveList.end(), { "D", "DA", "DR", "DL", "DRA", "DLA",  "B" });
 if (*playerAction == 0x0020) moveList.insert(moveList.end(), { "R", "L", "D", "DA", "DR", "DL", "DRA" "DLA", "B", "RB", "LB" });
 if (*playerAction == 0x0030) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA", "B", "BA", "RB" });
 if (*playerAction == 0x0031) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA", "B", "BA", "RB" });
 if (*playerAction == 0x0032) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA", "B", "BA", "RB" });
 if (*playerAction == 0x0033) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA", "B", "BA", "RB" });
 if (*playerAction == 0x0050) moveList.insert(moveList.end(), { "B" });

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

  // Evaluating samus magnet's reward on position X and Y
  reward -= magnets.playerHorizontalMagnet.intensity * std::abs(magnets.playerHorizontalMagnet.center - realPlayerPosX);
  reward -= magnets.playerVerticalMagnet.intensity * std::abs(magnets.playerVerticalMagnet.center - (float)*playerPosY);
  reward -= magnets.screenHorizontalMagnet.intensity * std::abs(magnets.screenHorizontalMagnet.center - screenPosX);

  reward += magnets.seiyaHPMagnet * (float)*seiyaHP2;
  reward += magnets.shunHPMagnet * (float)*shunHP2;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second );
  LOG("[Jaffar]  + Game Mode:              %02u\n", *gameMode);
  LOG("[Jaffar]  + Player Action:          %02u\n", *playerAction);
  LOG("[Jaffar]  + Shun HP:                %02u\n", *shunHP2);
  LOG("[Jaffar]  + Shun MP:                %02u\n", *shunMP2);
  LOG("[Jaffar]  + Seiya HP:               %02u\n", *seiyaHP2);
  LOG("[Jaffar]  + Seiya MP:               %02u\n", *seiyaMP2);
  LOG("[Jaffar]  + Shiryu MP:              %02u\n", *shiryuMP2);
  LOG("[Jaffar]  + Hyoga MP:               %02u\n", *hyogaMP2);
  LOG("[Jaffar]  + Player Direction:       %02u\n", *playerDirection);
  LOG("[Jaffar]  + Player Pos X:           %02u (Real: %f)\n", *playerPosX, realPlayerPosX);
  LOG("[Jaffar]  + Player Pos Y:           %02u\n", *playerPosY);
  LOG("[Jaffar]  + Screen Pos X:           %.2f (%02u %02u)\n", screenPosX, *screenPosX1, *screenPosX2);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.playerHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Player Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.playerHorizontalMagnet.intensity, magnets.playerHorizontalMagnet.center);
  if (std::abs(magnets.playerVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Player Vertical Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.playerVerticalMagnet.intensity, magnets.playerVerticalMagnet.center);
  if (std::abs(magnets.screenHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Screen Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.screenHorizontalMagnet.intensity, magnets.screenHorizontalMagnet.center);
  if (std::abs(magnets.seiyaHPMagnet) > 0.0f)                      LOG("[Jaffar]  + Seiya HP Magnet                 - Intensity: %.5f\n", magnets.seiyaHPMagnet);
  if (std::abs(magnets.shunHPMagnet) > 0.0f)                       LOG("[Jaffar]  + Shun HP Magnet                  - Intensity: %.5f\n", magnets.shunHPMagnet);
}
