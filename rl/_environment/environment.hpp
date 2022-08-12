#include "korali.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"

extern EmuInstance* emuInstance;
extern GameInstance* gameInstance;
extern uint8_t initialGameState[_STATE_DATA_SIZE_TRAIN];
extern float maxReward;

void runEnvironment(korali::Sample &s);
