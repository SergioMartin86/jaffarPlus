#include "gameInstance.hpp"
#include "gameRule.hpp"
#include "engine.h"
#include "sys.h"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  _emu = emu;

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  randomSeed         = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_RANDOM_SEED];
  pauseSlices        = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_PAUSE_SLICES];
  scrollY            = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_SCROLL_Y];
  heroAction         = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_HERO_ACTION];
  heroPosX           = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_HERO_POS_LEFT_RIGHT];
  heroPosMask        = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_HERO_POS_MASK];
  heroActionPosMask  = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_HERO_ACTION_POS_MASK];
  heroPosJumpCrouch  = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_HERO_POS_JUMP_DOWN];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;


  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);

 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 return { "." };
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
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Random Seed:            %04X\n", (uint16_t)*randomSeed);
 LOG("[Jaffar]  + Pause Slices:           %d\n", *pauseSlices);
 LOG("[Jaffar]  + Scroll Y:               %d\n", *scrollY);
 LOG("[Jaffar]  + Hero Action:            %d\n", *heroAction);
 LOG("[Jaffar]  + Hero Pos X:             %d\n", *heroPosX);
 LOG("[Jaffar]  + Hero Pos Mask:          %d\n", *heroPosMask);
 LOG("[Jaffar]  + Hero Action Pos Mask:   %d\n", *heroActionPosMask);
 LOG("[Jaffar]  + Hero Pos Jump Crouch:   %d\n", *heroPosJumpCrouch);

 LOG("[Jaffar]  + Ram Map:");
 for (int i = 0; i < VM_NUM_VARIABLES; i++)
 {
  if (i % 16 == 0) LOG("\n[Jaffar]    ");
  LOG("%04X ", (uint16_t)_emu->_engine->vm.vmVariables[i]);
 }
 LOG("\n");
}

