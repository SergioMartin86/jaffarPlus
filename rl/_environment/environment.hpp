#include "korali.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include "train.hpp"

extern EmuInstance* emuInstance;
extern GameInstance* gameInstance;
extern uint8_t initialGameState[_STATE_DATA_SIZE_TRAIN];
extern Train* _train;

void runEnvironment(korali::Sample &s);
