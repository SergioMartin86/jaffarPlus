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

  if (isDefined(config, "Level Code") == true)
   levelCode = config["Level Code"].get<std::string>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Level Code' was not defined\n");

  // LDKD
  lesterSwimState         = (int16_t*) &_emu->_engine->vm.vmVariables[0xE5];
  lesterPosX              = (int16_t*) &_emu->_engine->vm.vmVariables[0x01];
  lesterPosY              = (int16_t*) &_emu->_engine->vm.vmVariables[0x02];
  lesterRoom              = (int16_t*) &_emu->_engine->vm.vmVariables[0x2A];
  lesterAction            = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_HERO_ACTION];
  lesterAirMode           = (int16_t*) &_emu->_engine->vm.vmVariables[0x63];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  hash.Update((uint8_t *)_emu->_engine->vm.threadsData, sizeof(_emu->_engine->vm.threadsData));

  if (levelCode == "LDKD")
  {
   hash.Update(*lesterSwimState);
   hash.Update(*lesterAirMode);
   for (int i = 0; i < 0x30; i++) hash.Update(_emu->_engine->vm.vmVariables[i]);
  }

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
 bool ruleFound = false;
 size_t lastRuleFound = 0;

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++) if (rulesStatus[ruleId] == true) { ruleFound = true; lastRuleFound = ruleId; }

 if (ruleFound == true)
  {
    if (_rules[lastRuleFound]->_magnets[*lesterRoom].lesterHorizontalMagnet.active == true) magnets.lesterHorizontalMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].lesterHorizontalMagnet;
    if (_rules[lastRuleFound]->_magnets[*lesterRoom].lesterVerticalMagnet.active == true) magnets.lesterVerticalMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].lesterVerticalMagnet;
  }

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

  // Getting magnet value
  auto magnets = getMagnetValues(rulesStatus);

  // Container for bounded value and difference with center
  float diff = 0.0;

  // Evaluating lester magnet's reward on position X
  diff = -336.0f + std::abs(magnets.lesterHorizontalMagnet.center - (float)*lesterPosX);
  reward += magnets.lesterHorizontalMagnet.intensity * -diff;

  // Evaluating lester magnet's reward on position Y
  diff = -336.0f + std::abs(magnets.lesterVerticalMagnet.center - (float)*lesterPosY);
  reward += magnets.lesterVerticalMagnet.intensity * -diff;

  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Level Code:             %s\n", levelCode.c_str());

 if (levelCode == "LDKD")
 {
  LOG("[Jaffar]  + Lester Swim State:      %04d\n", *lesterSwimState);
  LOG("[Jaffar]  + Lester Pos X:           %04d\n", *lesterPosX);
  LOG("[Jaffar]  + Lester Pos Y:           %04d\n", *lesterPosY);
  LOG("[Jaffar]  + Lester Room:            %04d\n", *lesterRoom);
  LOG("[Jaffar]  + Lester Action:          %04d\n", *lesterAction);
  LOG("[Jaffar]  + Lester Air Mode:        %04d\n", *lesterAirMode);
 }

 LOG("[Jaffar]  + Ram Map: \n");
 LOG("[Jaffar]      ");
 for (int i = 0; i < 16; i++) LOG("%1X    ", (uint8_t)i);
 for (int i = 0; i < VM_NUM_VARIABLES; i++)
 {
  if (i % 16 == 0) LOG("\n[Jaffar]  %1X  ", i / 16);
  LOG("%04X ", (uint16_t)_emu->_engine->vm.vmVariables[i]);
 }
 LOG("\n");

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 if (std::abs(magnets.lesterHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Lester Horizontal Magnet     - Intensity: %.1f, Center: %3.3f\n", magnets.lesterHorizontalMagnet.intensity, magnets.lesterHorizontalMagnet.center);
 if (std::abs(magnets.lesterVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Lester Vertical Magnet       - Intensity: %.1f, Center: %3.3f\n", magnets.lesterVerticalMagnet.intensity, magnets.lesterVerticalMagnet.center);

}

