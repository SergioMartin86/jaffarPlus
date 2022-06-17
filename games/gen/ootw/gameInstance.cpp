#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
 // Setting emulator
 _emu = emu;

 // Trick to create different hashes for intermediate frames
 uint64_t hashCounter;

 // Setting memory positions
 gameTimer      = (uint16_t*)   &_emu->_68KRam[0xE882];

 lesterPosX              = (uint16_t*)   &_emu->_68KRam[0xEA86];
 lesterPosY              = (uint16_t*)   &_emu->_68KRam[0xEA88];
 lesterFrame1            = (uint16_t*)   &_emu->_68KRam[0xECAC];
 lesterFrame2            = (uint16_t*)   &_emu->_68KRam[0xECAE];
 lesterFrame3            = (uint16_t*)   &_emu->_68KRam[0xECB0];
 lesterFrame4            = (uint8_t*)    &_emu->_68KRam[0xEC9D];
 lesterRoom              = (uint8_t*)    &_emu->_68KRam[0xEB53];

 lesterForceX            = (int16_t*)   &_emu->_68KRam[0xEC7F];
 lesterForceY            = (int16_t*)   &_emu->_68KRam[0xEC4E];
 lesterForceY2           = (int8_t*)    &_emu->_68KRam[0xEC8A];

 // Stage 01 Specific values
 stage01AppearTimer       = (uint8_t*)   &_emu->_68KRam[0xEAAC];
 stage01MonsterAnimation  = (uint8_t*)   &_emu->_68KRam[0xE800];
 stage01MetMonster        = (uint8_t*)   &_emu->_68KRam[0xEC82];
 stage01VineState         = (int16_t*)   &_emu->_68KRam[0xEC90];
 stage01Finish            = (uint8_t*)   &_emu->_68KRam[0xEAE8];

 // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

//  if (*gameTimer != 2) hash.Update(_emu->_68KRam, 0xFFFF);
  hash.Update(_emu->_68KRam, 0xFFFF);

  hash.Update(*lesterPosX);
  hash.Update(*lesterPosY);
  hash.Update(*lesterFrame1);
  hash.Update(*lesterFrame2);
  hash.Update(*lesterFrame3);
  hash.Update(*lesterFrame4);
  hash.Update(*lesterRoom);

  hash.Update(*lesterForceX);
  hash.Update(*lesterForceY);
  hash.Update(*lesterForceY2);

  hash.Update(*stage01AppearTimer);
  hash.Update(*stage01MonsterAnimation);
  hash.Update(*stage01MetMonster);
  hash.Update(*stage01VineState);
  hash.Update(*stage01Finish);

  uint64_t result;
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

// if (*gameTimer != 2) return moveList;

 moveList.insert(moveList.end(), { "U...........", "U.......C..."});


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

  // Container for bounded value and difference with center
  float boundedValue = 0.0;
  float diff = 0.0;

  // Evaluating lester magnet's reward on position X
  boundedValue = (float)*lesterPosX;
  boundedValue = std::min(boundedValue, magnets.lesterHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.lesterHorizontalMagnet.min);
  diff = std::abs(magnets.lesterHorizontalMagnet.center - boundedValue);
  reward += magnets.lesterHorizontalMagnet.intensity * -diff;

  // Evaluating lester magnet's reward on position Y
  boundedValue = (float)*lesterPosY;
  boundedValue = std::min(boundedValue, magnets.lesterVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.lesterVerticalMagnet.min);
  diff = std::abs(magnets.lesterVerticalMagnet.center - boundedValue);
  reward += magnets.lesterVerticalMagnet.intensity * -diff;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Timer:                             %u (%02u)\n", *gameTimer);
 LOG("[Jaffar]  + Reward:                            %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                              0x%lX\n", computeHash());
 LOG("[Jaffar]  + Room:                              %02u\n", *lesterRoom);
 LOG("[Jaffar]  + Lester Pos X:                      %04u\n", *lesterPosX);
 LOG("[Jaffar]  + Lester Pos Y:                      %04u\n", *lesterPosY);
 LOG("[Jaffar]  + Lester Force X:                    %04d\n", *lesterForceX);
 LOG("[Jaffar]  + Lester Force Y:                    %04d, %02d\n", *lesterForceY, *lesterForceY2);
 LOG("[Jaffar]  + Lester Frame:                      %04u, %04u, %04u, %02u\n", *lesterFrame1, *lesterFrame2, *lesterFrame3, *lesterFrame4);

 LOG("[Jaffar]  + Level 1 Values:\n");
 LOG("[Jaffar]  + Appear Timer:                      %02u\n", *stage01AppearTimer);
 LOG("[Jaffar]  + Monster Animation:                 %02u\n", *stage01MonsterAnimation);
 LOG("[Jaffar]  + Met Monster:                       %02u\n", *stage01MetMonster);
 LOG("[Jaffar]  + Vine State:                        %04d\n", *stage01VineState);
 LOG("[Jaffar]  + Finished State:                    %02u\n", *stage01Finish);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

  if (std::abs(magnets.lesterHorizontalMagnet.intensity) > 0.0f)  LOG("[Jaffar]  + Lester Horizontal Magnet      - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.lesterHorizontalMagnet.intensity, magnets.lesterHorizontalMagnet.center, magnets.lesterHorizontalMagnet.min, magnets.lesterHorizontalMagnet.max);
  if (std::abs(magnets.lesterVerticalMagnet.intensity) > 0.0f)    LOG("[Jaffar]  + Lester Vertical Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.lesterVerticalMagnet.intensity, magnets.lesterVerticalMagnet.center, magnets.lesterVerticalMagnet.min, magnets.lesterVerticalMagnet.max);
}

