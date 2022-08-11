#include "environment.hpp"

EmuInstance* emuInstance;
GameInstance* gameInstance;
uint8_t initialGameState[_STATE_DATA_SIZE_TRAIN];
Train* _train;

std::vector<float> getState()
{
 std::vector<float> state;
 state.push_back(gameInstance->marblePosX / 512.0f);
 state.push_back(gameInstance->marblePosY / 512.0f);
 state.push_back(gameInstance->marblePosZ / 256.0f);
 state.push_back((float)*gameInstance->marbleVelX / 256.0f);
 state.push_back((float)*gameInstance->marbleVelY / 256.0f);
 return state;
}

INPUT_TYPE parseAction(const std::vector<float>& action)
{
 std::string moveString;
 if (action[0] > 0.5f) moveString.append("U");
 if (action[1] > 0.5f) moveString.append("D");
 if (action[2] > 0.5f) moveString.append("L");
 if (action[3] > 0.5f) moveString.append("R");
 if (action[4] > 0.5f) moveString.append("A");

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

 // Defining Jaffar steps
 size_t jaffarSteps = 10;

 while(currentStep < _MAX_MOVELIST_SIZE && isFailState == false)
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

  //printf("Running Step %lu\n", currentStep);
  gameInstance->popState(_train->_initialStateData);
  _train->reset();
  for (size_t i = 0; i < jaffarSteps && _train->_stateDB.size() > 0; i++)
  {
   //printf("  + Running Substep: %lu, Reward: %f\n", i, _train->_bestStateReward);
   _train->computeStates();
   _train->_currentStep++;
  };

  float trainReward = _train->_bestStateReward;
  if (_train->_stateDB.size() == 0)
  {
   trainReward = oldStateReward;
   isFailState = true;
  }

  //printf("Step: %u, DB Size: %lu, Reward: %f\n", _train->_currentStep, _train->_stateDB.size(), trainReward);

  // Storing reward
  float reward = trainReward - oldStateReward;
  reward -= 0.0001f; // Punishing slowness
  s["Reward"] = reward;

  // Storing new state
  auto newState = getState();
  s["State"] = newState;

  // Printing Action:
  //for (size_t i = 0; i < ruleCount; i++) printf("Rule %lu: %d\n", i, rulesStatus[i] ? 1 : 0);
  //printf("Action %lu: %s, Pos: (%.3f, %.3f, %.3f) -> (%.3f, %.3f, %.3f), Vel: (%.3f, %.3f) -> (%.3f, %.3f) / R: %f -> %f = %f \n", currentStep, EmuInstance::moveCodeToString(input).c_str(), oldState[0], oldState[1], oldState[2], newState[0], newState[1], newState[2], oldState[3], oldState[4], newState[3], newState[4], oldStateReward, trainReward, reward);

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

 // Setting finalization status
 if (currentStep < _MAX_MOVELIST_SIZE)
  s["Termination"] = "Terminal";
 else
  s["Termination"] = "Truncated";
}

