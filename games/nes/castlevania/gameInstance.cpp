#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting relevant SMB pointers

  // Thanks to https://datacrystal.romhacking.net/wiki/Super_Simon_Bros.:RAM_map and https://tasvideos.org/GameResources/NES/SuperSimonBros for helping me find some of these items
  simonAnimation       = (uint8_t*)  &_emu->_baseMem[0x0001];
  simonState           = (uint8_t*)  &_emu->_baseMem[0x000E];
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  // Adding fixed hash elements
  hash.Update(*simonAnimation);
  hash.Update(*simonState);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 // Recalculating derived values
// simonPosX = (uint16_t)*simonBasePosX * 256 + (uint16_t)*simonRelPosX;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 // On the floor, try all possible combinations, prioritize jumping and right direction
 return { ".", "R", "L", "A", "B", "RA", "RB", "RU", "LA", "LB", "RL", "D", "DB", "U", "UB", "LDU", "RLB", "RLA", "LS", "S"  };
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;

 magnets.simonHorizontalMagnet.intensity = 0.0f;
 magnets.simonHorizontalMagnet.max = 0.0f;

 magnets.simonVerticalMagnet.intensity = 0.0f;
 magnets.simonVerticalMagnet.max = 0.0f;

 for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++) if (rulesStatus[ruleId] == true) magnets = _rules[ruleId]->_magnets;

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

  // Container for bounding calculations
  float boundedValue = 0.0;

//  // Evaluating simon magnet's reward on position X
//  boundedValue = (float)simonPosX;
//  boundedValue = std::min(boundedValue, magnets.simonHorizontalMagnet.max);
//  boundedValue = std::max(boundedValue, magnets.simonHorizontalMagnet.min);
//  reward += magnets.simonHorizontalMagnet.intensity * boundedValue;
//
//  // Evaluating simon magnet's reward on position Y
//  boundedValue = (float)*simonPosY;
//  boundedValue = std::min(boundedValue, magnets.simonVerticalMagnet.max);
//  boundedValue = std::max(boundedValue, magnets.simonVerticalMagnet.min);
//  reward += magnets.simonHorizontalMagnet.intensity * boundedValue;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _ruleCount; i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  LOG("[Jaffar]  + Simon Horizontal Magnet    - Intensity: %.1f, Max: %f\n", magnets.simonHorizontalMagnet.intensity, magnets.simonHorizontalMagnet.max);
  LOG("[Jaffar]  + Simon Vertical Magnet      - Intensity: %.1f, Max: %f\n", magnets.simonVerticalMagnet.intensity, magnets.simonVerticalMagnet.max);
}
