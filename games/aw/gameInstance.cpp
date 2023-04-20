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
  lesterRoom              = (int16_t*) &_emu->_engine->vm.vmVariables[0x66];
  lesterAction            = (int16_t*) &_emu->_engine->vm.vmVariables[ScriptVars::VM_VARIABLE_HERO_ACTION];
  lesterState           = (int16_t*) &_emu->_engine->vm.vmVariables[0x63];
  gameScriptState         = (int16_t*) &_emu->_engine->vm.vmVariables[0x2A];
  gameAnimState           = (int16_t*) &_emu->_engine->vm.vmVariables[0x0F];
  lesterDeadState         = (int16_t*) &_emu->_engine->vm.vmVariables[0x03];

  lesterMomentum1          = (int16_t*) &_emu->_engine->vm.vmVariables[0x15];
  lesterMomentum2          = (int16_t*) &_emu->_engine->vm.vmVariables[0x16];
  lesterMomentum3          = (int16_t*) &_emu->_engine->vm.vmVariables[0x17];

  lesterHasGun             = (int16_t*) &_emu->_engine->vm.vmVariables[0x0A];
  lesterGunAmmo            = (int16_t*) &_emu->_engine->vm.vmVariables[0x06];
  lesterGunLoad            = (int16_t*) &_emu->_engine->vm.vmVariables[0x0F];
  lesterDirection          = (int16_t*) &_emu->_engine->vm.vmVariables[0x63];

  alienState             = (int16_t*) &_emu->_engine->vm.vmVariables[0x6B];
  alienRoom              = (int16_t*) &_emu->_engine->vm.vmVariables[0x6A];
  alienPosX              = (int16_t*) &_emu->_engine->vm.vmVariables[0x68];

  gameTimer              = (int16_t*) &_emu->_engine->vm.vmVariables[0x31];
  elevatorPosY           = (int16_t*) &_emu->_engine->vm.vmVariables[0x14];
  fumesState             = (int16_t*) &_emu->_engine->vm.vmVariables[0xE8];

  // Initialize derivative values
  updateDerivedValues();

  // Clearing cheat engine data
  vmVariablesPrevValues.resize(0x100);
  vmVariablesDisplay.resize(0x100);
  vmVariablesClear();
}

// Cheat engine like data
void GameInstance::vmVariablesClear()
{
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++) vmVariablesPrevValues[i] = _emu->_engine->vm.vmVariables[i];
 for (size_t i = 0; i < vmVariablesDisplay.size(); i++) vmVariablesDisplay[i] = true;
}

void GameInstance::vmVariablesKeepEqual()
{
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++)
  if (vmVariablesPrevValues[i] != _emu->_engine->vm.vmVariables[i]) vmVariablesDisplay[i] = false;
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++) vmVariablesPrevValues[i] = _emu->_engine->vm.vmVariables[i];
}

void GameInstance::vmVariablesKeepDifferent()
{
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++)
  if (vmVariablesPrevValues[i] == _emu->_engine->vm.vmVariables[i]) vmVariablesDisplay[i] = false;
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++) vmVariablesPrevValues[i] = _emu->_engine->vm.vmVariables[i];
}

void GameInstance::vmVariablesKeepGreater()
{
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++)
  if (vmVariablesPrevValues[i] <= _emu->_engine->vm.vmVariables[i]) vmVariablesDisplay[i] = false;
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++) vmVariablesPrevValues[i] = _emu->_engine->vm.vmVariables[i];
}

void GameInstance::vmVariablesKeepSmaller()
{
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++)
  if (vmVariablesPrevValues[i] >= _emu->_engine->vm.vmVariables[i]) vmVariablesDisplay[i] = false;
 for (size_t i = 0; i < vmVariablesPrevValues.size(); i++) vmVariablesPrevValues[i] = _emu->_engine->vm.vmVariables[i];
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));

  uint8_t emuState[_STATE_DATA_SIZE_PLAY];
  _emu->serializeState(emuState);
  _emu->advanceState(0);

  hash.Update((uint8_t *)_emu->_engine->vm.threadsData, sizeof(_emu->_engine->vm.threadsData));
  hash.Update((uint8_t *)_emu->_engine->vm._scriptStackCalls, sizeof(_emu->_engine->vm._scriptStackCalls));
//  hash.Update((uint8_t *)_emu->_engine->vm.vmIsChannelActive, sizeof(_emu->_engine->vm.vmIsChannelActive));

//  hash.Update(_emu->_engine->buttonPressCount);

  for (int i = 0x00; i < 0x30; i++) hash.Update(_emu->_engine->vm.vmVariables[i]);
  for (int i = 0x40; i < 0xF0; i++) hash.Update(_emu->_engine->vm.vmVariables[i]);
//  for (int i = 0xF0; i < 0xFF; i++) hash.Update(_emu->_engine->vm.vmVariables[i]);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));

  // Reload game state
  _emu->deserializeState(emuState);

  return result;
}


void GameInstance::updateDerivedValues()
{
 lesterFullMomentum = std::max(*lesterMomentum1, *lesterMomentum2) + std::abs(*lesterMomentum3);
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);

 updateDerivedValues();

 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 if (levelCode == "LDKD") return { ".", "R", "L", "U", "B", "D", "UL", "UR", "UB", "DL", "DR", "DB", "LB", "RB", "ULB", "URB", "DLB", "DRB" };
 if (levelCode == "HTDC") return { ".", "R", "L", "U", "B", "D", "UL", "UR", "UB", "DL", "DR", "DB", "LB", "RB", "ULB", "URB", "DLB", "DRB" };
 if (levelCode == "CLLD") return { ".", "B", "R", "L", "U" };
 if (levelCode == "LBKG") return { ".", "R", "L", "U", "B", "D", "UL", "UR", "UB", "DL", "DR", "DB", "LB", "RB", "ULB", "URB", "DLB", "DRB" };

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
    if (*lesterRoom < _ROOM_COUNT_)
    {
     if (_rules[lastRuleFound]->_magnets[*lesterRoom].lesterHorizontalMagnet.active == true) magnets.lesterHorizontalMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].lesterHorizontalMagnet;
     if (_rules[lastRuleFound]->_magnets[*lesterRoom].lesterVerticalMagnet.active == true) magnets.lesterVerticalMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].lesterVerticalMagnet;
     if (_rules[lastRuleFound]->_magnets[*lesterRoom].elevatorVerticalMagnet.active == true) magnets.elevatorVerticalMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].elevatorVerticalMagnet;
     magnets.lesterAngularMomentumMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].lesterAngularMomentumMagnet;
     magnets.lesterDirectionMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].lesterDirectionMagnet;
     magnets.lesterGunLoadMagnet = _rules[lastRuleFound]->_magnets[*lesterRoom].lesterGunLoadMagnet;
    }

    if (*alienRoom < _ROOM_COUNT_)
    {
     if (_rules[lastRuleFound]->_magnets[*alienRoom].alienHorizontalMagnet.active == true) magnets.alienHorizontalMagnet = _rules[lastRuleFound]->_magnets[*alienRoom].alienHorizontalMagnet;
    }
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
  diff = -336.0f + std::abs(magnets.alienHorizontalMagnet.center - (float)*alienPosX);
  reward += magnets.alienHorizontalMagnet.intensity * -diff;

  // Evaluating lester magnet's reward on position X
  diff = -336.0f + std::abs(magnets.lesterHorizontalMagnet.center - (float)*lesterPosX);
  reward += magnets.lesterHorizontalMagnet.intensity * -diff;

  // Evaluating lester magnet's reward on position Y
  diff = -336.0f + std::abs(magnets.lesterVerticalMagnet.center - (float)*lesterPosY);
  reward += magnets.lesterVerticalMagnet.intensity * -diff;

  diff = -255.0f + std::abs(magnets.elevatorVerticalMagnet.center - (float)*elevatorPosY);
  reward += magnets.elevatorVerticalMagnet.intensity * -diff;

  // Evaluating lester magnet's reward on position Y
  reward += magnets.lesterAngularMomentumMagnet * lesterFullMomentum;

  // Evaluating lester magnet's reward on position Y
  if (magnets.lesterDirectionMagnet < 0.0 && *lesterDirection == LESTER_DIR_LEFT) reward += std::abs(magnets.lesterDirectionMagnet);
  if (magnets.lesterDirectionMagnet > 0.0 && *lesterDirection == LESTER_DIR_RIGHT) reward += std::abs(magnets.lesterDirectionMagnet);

  // Evaluating lester magnet's reward on position Y
  reward += magnets.lesterGunLoadMagnet * *lesterGunLoad;

  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                     %f\n", getStateReward(rulesStatus));
// LOG("[Jaffar]  + Hash:                       0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game State:                 %04d / %04d\n", *gameScriptState, *gameAnimState);
 LOG("[Jaffar]  + Timer:                      %04X\n", *gameTimer);
 LOG("[Jaffar]  + Fumes State:                %04X\n", *fumesState);

 LOG("[Jaffar]  + Level Code:                 %s\n", levelCode.c_str());

 LOG("[Jaffar]  + Lester Pos X:               %04d\n", *lesterPosX);
 LOG("[Jaffar]  + Lester Pos Y:               %04d\n", *lesterPosY);
 LOG("[Jaffar]  + Lester Direction:           %04d (%s)\n", *lesterDirection, *lesterDirection == LESTER_DIR_LEFT ? "Left" : "Right");
 LOG("[Jaffar]  + Lester Room:                %04d\n", *lesterRoom);
 LOG("[Jaffar]  + Lester Action:              %04d\n", *lesterAction);
 LOG("[Jaffar]  + Lester State:               %04d\n", *lesterState);
 LOG("[Jaffar]  + Lester Dead State:          %04d\n", *lesterDeadState);
 LOG("[Jaffar]  + Elevator Pos Y:             %04d\n", *elevatorPosY);

 LOG("[Jaffar]  + Lester Has Gun:             %04d\n", *lesterHasGun);
 LOG("[Jaffar]  + Lester Gun Ammo:            %04d\n", *lesterGunAmmo);
 LOG("[Jaffar]  + Lester Gun Load:            %04d\n", *lesterGunLoad);

 LOG("[Jaffar]  + Alien State:                %04d\n", *alienState);
 LOG("[Jaffar]  + Alien Room:                 %04d\n", *alienRoom);
 LOG("[Jaffar]  + Alien Pos X:                %04d\n", *alienPosX);

 LOG("[Jaffar]  + Ram Map: \n");
 LOG("[Jaffar]      ");
 for (int i = 0; i < 16; i++) LOG("%1X    ", (uint8_t)i);
 for (int i = 0; i < VM_NUM_VARIABLES; i++)
 {
  if (i % 16 == 0) LOG("\n[Jaffar]  %1X  ", i / 16);
  if (vmVariablesDisplay[i] == true) LOG("%04X ", (uint16_t)_emu->_engine->vm.vmVariables[i]);
  if (vmVariablesDisplay[i] == false) LOG("---- ");
 }
 LOG("\n");

 for (const auto& i : watchVMVariables)
  LOG("[Jaffar]  + Watch VM Index:           %02X (%d) = %02X\n", i, i, (uint16_t)_emu->_engine->vm.vmVariables[i]);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 if (std::abs(magnets.alienHorizontalMagnet.intensity) > 0.0f)    LOG("[Jaffar]  + Alien Horizontal Magnet         - Intensity: %.1f, Center: %3.3f\n", magnets.alienHorizontalMagnet.intensity, magnets.alienHorizontalMagnet.center);
 if (std::abs(magnets.lesterHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Lester Horizontal Magnet        - Intensity: %.1f, Center: %3.3f\n", magnets.lesterHorizontalMagnet.intensity, magnets.lesterHorizontalMagnet.center);
 if (std::abs(magnets.lesterVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Lester Vertical Magnet          - Intensity: %.1f, Center: %3.3f\n", magnets.lesterVerticalMagnet.intensity, magnets.lesterVerticalMagnet.center);
 if (std::abs(magnets.elevatorVerticalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Elevator Vertical Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.elevatorVerticalMagnet.intensity, magnets.elevatorVerticalMagnet.center);
 if (std::abs(magnets.lesterAngularMomentumMagnet) > 0.0f)        LOG("[Jaffar]  + Lester Angular Momentum Magnet  - Intensity: %.1f\n", magnets.lesterAngularMomentumMagnet);
 if (std::abs(magnets.lesterDirectionMagnet) > 0.0f)              LOG("[Jaffar]  + Lester Direction Magnet         - Intensity: %.1f\n", magnets.lesterDirectionMagnet);
 if (std::abs(magnets.lesterGunLoadMagnet) > 0.0f)                LOG("[Jaffar]  + Lester Gun Load Magnet  - Intensity: %.1f\n", magnets.lesterGunLoadMagnet);

}

