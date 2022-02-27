#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  RNGState               = (uint8_t*)   &_emu->_baseMem[0x0060];
  framePhase             = (uint8_t*)   &_emu->_baseMem[0x002C];
  kidPosX                = (uint16_t*)  &_emu->_baseMem[0x060F];
  kidPosY                = (uint8_t*)   &_emu->_baseMem[0x0611];
  kidFrame               = (uint8_t*)   &_emu->_baseMem[0x0617];
  kidMovement            = (uint8_t*)   &_emu->_baseMem[0x0613];
  kidFallWait            = (uint8_t*)   &_emu->_baseMem[0x06E4];
  kidHP                  = (uint8_t*)   &_emu->_baseMem[0x06CF];
  kidRoom                = (uint8_t*)   &_emu->_baseMem[0x04DE];
  guardPosX              = (uint8_t*)   &_emu->_baseMem[0x061D];
  guardHP                = (uint8_t*)   &_emu->_baseMem[0x06D0];
  guardNotice            = (uint8_t*)   &_emu->_baseMem[0x0601];
  guardFrame             = (uint8_t*)   &_emu->_baseMem[0x0621];
  guardMovement          = (uint8_t*)   &_emu->_baseMem[0x0625];
  drawnRoom              = (uint8_t*)   &_emu->_baseMem[0x0051];
  screenTransition       = (uint8_t*)   &_emu->_baseMem[0x04AC];
  globalTimer            = (uint8_t*)   &_emu->_baseMem[0x0791];
  screenDrawn            = (uint8_t*)   &_emu->_baseMem[0x0732];
  isPaused               = (uint8_t*)   &_emu->_baseMem[0x06D1];

  if (isDefined(config, "Hash Includes") == true)
   for (const auto& entry : config["Hash Includes"])
    hashIncludes.insert(entry.get<std::string>());
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Hash Includes' was not defined\n");
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  hash.Update(*framePhase);
  hash.Update(*kidPosX);
  hash.Update(*kidPosY);
  hash.Update(*kidFrame);
  hash.Update(*kidMovement);
  hash.Update(*kidFallWait);
  hash.Update(*kidHP);
  hash.Update(*kidRoom);
  hash.Update(*guardPosX);
  hash.Update(*guardHP);
  hash.Update(*guardNotice);
  hash.Update(*guardFrame);
  hash.Update(*guardMovement);
  hash.Update(*drawnRoom);
  hash.Update(*screenTransition);
  hash.Update(*screenDrawn);
  hash.Update(*isPaused);
  hash.Update(*globalTimer);

  // If in screen transition, take the global timer for hash
  if (*screenTransition == 255) hash.Update(*globalTimer);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 // Recalculating derived values
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 // On the floor, try all possible combinations, prioritize jumping and right direction
 return { ".", "L", "R", "U", "LA", "A", "RA", "D", "B", "UD", "UB", "DRA", "DLA" };

 // Try all possible combinations
// std::vector<std::string> moves;
// for (uint8_t i = 0; i < 255; i++)
// {
//  if (i & 0b00000100) continue;
//  moves.push_back(EmuInstance::moveCodeToString(i));
// }
// return moves;
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;
 memset(&magnets, 0, sizeof(magnets));

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
    magnets = _rules[ruleId]->_magnets[*kidRoom];

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

  // Evaluating kid magnet's reward on position X
  boundedValue = (float)*kidPosX;
  boundedValue = std::min(boundedValue, magnets.kidHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.kidHorizontalMagnet.min);
  diff = std::abs(magnets.kidHorizontalMagnet.center - boundedValue);
  reward += magnets.kidHorizontalMagnet.intensity * -diff;

  // Evaluating kid magnet's reward on position Y
  boundedValue = (float)*kidPosY;
  boundedValue = std::min(boundedValue, magnets.kidVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.kidVerticalMagnet.min);
  diff = std::abs(magnets.kidVerticalMagnet.center - boundedValue);
  reward += magnets.kidVerticalMagnet.intensity * -diff;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
 *this->RNGState = (uint8_t) RNGState;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
  LOG("[Jaffar]  + RNG State:              0x%X\n", *RNGState);
  LOG("[Jaffar]  + Frame Phase:            %02u\n", *framePhase);
  LOG("[Jaffar]  + Global Timer:           %02u\n", *globalTimer);
  LOG("[Jaffar]  + Is Paused:              %02u\n", *isPaused);
  LOG("[Jaffar]  + Screen Trans / Drawn:   %02u / %02u\n", *screenTransition, *screenDrawn);

  LOG("[Jaffar]  + Kid Room:               %02u\n", *kidRoom);
  LOG("[Jaffar]  + Kid Pos X:              %04u\n", *kidPosX);
  LOG("[Jaffar]  + Kid Pos Y:              %02u\n", *kidPosY);
  LOG("[Jaffar]  + Kid Frame:              %02u\n", *kidFrame);
  LOG("[Jaffar]  + Kid Movement:           %02u\n", *kidMovement);
  LOG("[Jaffar]  + Kid Fall Wait:          %02u\n", *kidFallWait);
  LOG("[Jaffar]  + Kid HP:                 %02u\n", *kidHP);

  LOG("[Jaffar]  + Guard Pos X:            %02u\n", *guardPosX);
  LOG("[Jaffar]  + Guard HP:               %02u\n", *guardHP);
  LOG("[Jaffar]  + Guard Notice:           %02u\n", *guardNotice);
  LOG("[Jaffar]  + Guard Frame:            %02u\n", *guardFrame);
  LOG("[Jaffar]  + Guard Movement          %02u\n", *guardMovement);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  LOG("[Jaffar]  + Kid Horizontal Magnet        - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidHorizontalMagnet.intensity, magnets.kidHorizontalMagnet.center, magnets.kidHorizontalMagnet.min, magnets.kidHorizontalMagnet.max);
  LOG("[Jaffar]  + Kid Vertical Magnet          - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidVerticalMagnet.intensity, magnets.kidVerticalMagnet.center, magnets.kidVerticalMagnet.min, magnets.kidVerticalMagnet.max);
}
