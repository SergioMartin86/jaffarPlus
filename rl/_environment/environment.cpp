#include "environment.hpp"

EmuInstance* emuInstance;
GameInstance* gameInstance;
uint8_t initialGameState[_STATE_DATA_SIZE_TRAIN];
float maxReward;

std::vector<float> getState()
{
 std::vector<float> state;
 state.push_back(gameInstance->marblePosX / 512.0f);
 state.push_back(gameInstance->marblePosY / 512.0f);
 state.push_back(gameInstance->marblePosZ / 256.0f);
 state.push_back((float)*gameInstance->marbleVelX / 256.0f);
 state.push_back((float)*gameInstance->marbleVelY / 256.0f);
 state.push_back(gameInstance->surfaceAngleX);
 state.push_back(gameInstance->surfaceAngleY);
 return state;
}

INPUT_TYPE parseAction(const std::vector<float>& action)
{
 std::string moveString;
 if (action[0] > 0.0f) moveString.append("U"); else moveString.append("D");
 if (action[1] > 0.0f) moveString.append("R"); else moveString.append("L");
 moveString.append("A");

 return EmuInstance::moveStringToCode(moveString);
}

void runEnvironment(korali::Sample &s)
{
 // Loading initial state
 gameInstance->pushState(initialGameState);

 // Setting initial state
 s["State"] = getState();

 // Getting rule count
 size_t ruleCount = gameInstance->_rules.size();

 // Initializing Step Counter and fail state
 size_t currentStep = 0;
 bool isFailState = false;

 // Storing move history
 std::string moveHistory;

 // Reward calculation
 float newStateReward;

 while(currentStep < 1500 && isFailState == false)
 {
  // Getting new action
  s.update();

  // Evaluating current state
  bool* rulesStatus = (bool*) calloc(ruleCount, sizeof(bool));
  gameInstance->evaluateRules(rulesStatus);
  float oldStateReward = gameInstance->getStateReward(rulesStatus);

  // Getting current state
  auto oldState = getState();

  // Reading new action
  std::vector<float> action = s["Action"];

  // Parsing action
  auto input = parseAction(action);

  // Advancing Frame
  gameInstance->advanceState(input);

  // Re-evaluating state
  gameInstance->evaluateRules(rulesStatus);
  newStateReward = gameInstance->getStateReward(rulesStatus);
  stateType StateType = gameInstance->getStateType(rulesStatus);
  isFailState = StateType == f_fail;

  // Storing reward
  float reward = newStateReward - oldStateReward;
  reward -= 0.0001f; // Punishing slowness
  if (StateType == f_win) reward += 10000.0f;
  if (isFailState) reward = -1000.0f;

  s["Reward"] = reward;

  // Storing new state
  auto newState = getState();
  s["State"] = newState;

  // Storing move history
  moveHistory += EmuInstance::moveCodeToString(input) + std::string("\n");

  // Printing Action:
//  if (reward > 100)
//  {
//   for (size_t i = 0; i < ruleCount; i++) printf("Rule %lu: %d\n", i, rulesStatus[i] ? 1 : 0);
//   printf("Action %lu: %s, Pos: (%.3f, %.3f, %.3f) -> (%.3f, %.3f, %.3f), Vel: (%.3f, %.3f) -> (%.3f, %.3f) / R: %f -> %f = %f \n", currentStep, EmuInstance::moveCodeToString(input).c_str(), oldState[0], oldState[1], oldState[2], newState[0], newState[1], newState[2], oldState[3], oldState[4], newState[3], newState[4], oldStateReward, newStateReward, reward);
//   exit(0);
//  }

  //  Printing State:
  //    if (curActionIndex % 100 == 0)
  //    {
  //      printf("State %lu: [ %.3f", curActionIndex, state[0]);
  //      for (size_t i = 1; i < state.size(); i++) printf(", %.3f", state[i]);
  //      printf("]\n");
  //    }

  // Increasing action count
  currentStep++;
 }

 // Storing current solution
 saveStringToFile(moveHistory, "last.sol");

 // If best state, save solution
 if (newStateReward > maxReward)
 {
  maxReward = newStateReward;
  printf("New Maximum Reward: %f\n", maxReward);
  saveStringToFile(moveHistory,"best.sol");
 }

 // Setting finalization status
 if (currentStep < _MAX_MOVELIST_SIZE)
  s["Termination"] = "Terminal";
 else
  s["Termination"] = "Truncated";
}

