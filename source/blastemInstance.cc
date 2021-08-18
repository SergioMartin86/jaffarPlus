#include "blastemInstance.h"
#include "utils.h"
#include <dlfcn.h>
#include <iostream>
#include "metrohash64.h"
#include <omp.h>

extern uint8_t _framesPerGameFrame;
extern uint16_t _curFrameId;
extern uint16_t prevFrameId;
extern size_t _stateWorkRamOffset;
extern move_t _nextMove;
extern uint8_t* _stateData;
extern size_t _stateSize;
extern int _detectedError;

extern "C" void blastem_start(int argc, char** argv, int isHeadlessMode, int isFastVdp);
extern "C" void blastem_resume();
extern "C" void updateFrameInfo();
extern "C" void reloadState();
extern "C" void redraw();
extern "C" void restartGenesis();
extern "C" void buildSystem();

void blastemInstance::initialize(char* romFile, char* saveFile, const bool headlessMode, const bool fastVdp)
{
 int argc = 4;
 char* argv[4];

 char exec[] = "jaffar";
 char flag[] = "-s";
 argv[0] = exec;
 argv[1] = romFile;
 argv[2] = flag;
 argv[3] = saveFile;
 blastem_start(argc, argv, headlessMode, fastVdp);
 _saveFile = std::string(saveFile);

 _startStateData = (uint8_t*) malloc(sizeof(uint8_t) * _STATE_DATA_SIZE);
 loadBinFromFile(_startStateData, sizeof(uint8_t) * _STATE_DATA_SIZE, _saveFile.c_str());
 reset();
}

void blastemInstance::reset()
{
 restartGenesis();
 loadState(_startStateData);
}

void blastemInstance::setRNGValue(const uint32_t& rngValue)
{
 _state.rngValue = rngValue;
 memcpyBigEndian32((uint32_t*)&_stateData[_stateWorkRamOffset + 0x19C4], (uint8_t*)&_state.rngValue);
 reloadState();
 _state = getGameState(_stateData);
}

void blastemInstance::setHPValue(const uint8_t& hp)
{
 _state.kidCurrentHP = hp;
 memcpyBigEndian8((uint8_t*)&_stateData[_stateWorkRamOffset + 0x4C4F], (uint8_t*)&_state.kidCurrentHP);

 if (hp > _state.kidMaxHP)
 {
  _state.kidMaxHP = hp;
  memcpyBigEndian8((uint8_t*)&_stateData[_stateWorkRamOffset + 0x4C50], (uint8_t*)&_state.kidMaxHP);
 }

 reloadState();
 _state = getGameState(_stateData);
}

void blastemInstance::setKidXValue(const uint16_t& x)
{
 _state.kidPositionX = x;
 memcpyBigEndian16((uint16_t*)&_stateData[_stateWorkRamOffset + 0x4C3E], (uint8_t*)&_state.kidPositionX);
 reloadState();
 _state = getGameState(_stateData);
}


gameStateStruct blastemInstance::getGameState(const uint8_t* state)
{
 gameStateStruct gameState;

 memcpyBigEndian32(&gameState.rngValue,          &state[_stateWorkRamOffset + 0x19C4]);
 memcpyBigEndian16(&gameState.gameFrame,         &state[_stateWorkRamOffset + 0x5002]);
 memcpyBigEndian16(&gameState.videoFrame,        &state[_stateWorkRamOffset + 0x19C8]);
 memcpyBigEndian8(&gameState.framesPerStep,      &state[_stateWorkRamOffset + 0x5005]);
 memcpyBigEndian8(&gameState.currentLevel,       &state[_stateWorkRamOffset + 0x4AA5]);
 memcpyBigEndian8(&gameState.drawnRoom,          &state[_stateWorkRamOffset + 0x4A36]);
 memcpyBigEndian16(&gameState.minutesLeft,       &state[_stateWorkRamOffset + 0x4C90]);
 memcpyBigEndian16(&gameState.twelthSecondsLeft, &state[_stateWorkRamOffset + 0x4FC8]);
 memcpyBigEndian32(&gameState.checkpointPointer, &state[_stateWorkRamOffset + 0x4FF0]);
 memcpyBigEndian8(&gameState.slowfallFramesLeft, &state[_stateWorkRamOffset + 0x4AA1]);

 // Level-Specific
 memcpyBigEndian8(&gameState.sandTile1,          &state[_stateWorkRamOffset + 0x6861]);
 memcpyBigEndian8(&gameState.sandTile2,          &state[_stateWorkRamOffset + 0x6871]);
 memcpyBigEndian8(&gameState.sandTile3,          &state[_stateWorkRamOffset + 0x6881]);
 memcpyBigEndian8(&gameState.sandTile4,          &state[_stateWorkRamOffset + 0x6891]);
 memcpyBigEndian8(&gameState.sandTile5,          &state[_stateWorkRamOffset + 0x68A1]);
 memcpyBigEndian8(&gameState.caveEntrancePos,    &state[_stateWorkRamOffset + 0x6851]);
 memcpyBigEndian8(&gameState.lvl3ExitDoor,       &state[_stateWorkRamOffset + 0x2B93]);
 memcpyBigEndian8(&gameState.lvl3FastRouteDoor,   &state[_stateWorkRamOffset + 0x213B]);
 memcpyBigEndian8(&gameState.lvl3FastExitDoor,   &state[_stateWorkRamOffset + 0x2377]);
 memcpyBigEndian8(&gameState.lvl4ExitDoor,       &state[_stateWorkRamOffset + 0x27CF]);
 memcpyBigEndian8(&gameState.lvl5Room2Door,      &state[_stateWorkRamOffset + 0x1E53]);
 memcpyBigEndian8(&gameState.lvl5CarpetHatch,    &state[_stateWorkRamOffset + 0x1E9F]);
 memcpyBigEndian8(&gameState.lvl6RightDoor,      &state[_stateWorkRamOffset + 0x2507]);
 memcpyBigEndian8(&gameState.lvl7PostPotionDoor, &state[_stateWorkRamOffset + 0x2313]);
 memcpyBigEndian8(&gameState.lvl7ExitDoor,       &state[_stateWorkRamOffset + 0x273F]);
 memcpyBigEndian8(&gameState.lvl8ExitDoor,       &state[_stateWorkRamOffset + 0x283F]);
 memcpyBigEndian8(&gameState.lvl11ExitDoor,      &state[_stateWorkRamOffset + 0x28C3]);
 memcpyBigEndian8(&gameState.lvl11PotionDoor,    &state[_stateWorkRamOffset + 0x1E8F]);
 memcpyBigEndian8(&gameState.lvl11PostPotionDoor,  &state[_stateWorkRamOffset + 0x203B]);
 memcpyBigEndian8(&gameState.lvl11ExitRoomLeftDoor,  &state[_stateWorkRamOffset + 0x299B]);
 memcpyBigEndian8(&gameState.lvl12FirstDoor,  &state[_stateWorkRamOffset + 0x2153]);
 memcpyBigEndian8(&gameState.lvl12ExitDoor,  &state[_stateWorkRamOffset + 0x20C3]);
 memcpyBigEndian8(&gameState.lvl12WeirdRoomDoor,  &state[_stateWorkRamOffset + 0x25DB]);
 memcpyBigEndian8(&gameState.lvl13FirstRoomDoor,  &state[_stateWorkRamOffset + 0x26CB]);
 memcpyBigEndian8(&gameState.lvl13ExitDoor,  &state[_stateWorkRamOffset + 0x2A7F]);
 memcpyBigEndian8(&gameState.lvl14FakeJaffarsLeft,  &state[_stateWorkRamOffset + 0x358D]);

 memcpyBigEndian8(&gameState.kidCurrentSequence, &state[_stateWorkRamOffset + 0x4C55]);
 memcpyBigEndian8(&gameState.kidCurrentSequenceStage, &state[_stateWorkRamOffset + 0x4C53]);
 memcpyBigEndian8(&gameState.kidLastSequence,    &state[_stateWorkRamOffset + 0x4C57]);
 memcpyBigEndian8(&gameState.kidFrame,           &state[_stateWorkRamOffset + 0x4C45]);
 memcpyBigEndian8(&gameState.kidCurrentHP,       &state[_stateWorkRamOffset + 0x4C4F]);
 memcpyBigEndian8(&gameState.kidMaxHP,           &state[_stateWorkRamOffset + 0x4C50]);
 memcpyBigEndian8(&gameState.kidRoom,            &state[_stateWorkRamOffset + 0x4C4B]);
 memcpyBigEndian8(&gameState.kidHasSword,        &state[_stateWorkRamOffset + 0x4C4D]);
 memcpyBigEndian8(&gameState.kidDirection,       &state[_stateWorkRamOffset + 0x4C3D]);
 memcpyBigEndian16(&gameState.kidPositionX,      &state[_stateWorkRamOffset + 0x4C3E]);
 memcpyBigEndian16(&gameState.kidPositionY,      &state[_stateWorkRamOffset + 0x4C40]);
 memcpyBigEndian8(&gameState.kidFallingSpeed,    &state[_stateWorkRamOffset + 0x4C4A]);
 memcpyBigEndian8(&gameState.kidFrameVariant,    &state[_stateWorkRamOffset + 0x4C8A]);
 memcpyBigEndian8(&gameState.kidPosition01,       &state[_stateWorkRamOffset + 0x4C6D]);
 memcpyBigEndian8(&gameState.kidPosition02,       &state[_stateWorkRamOffset + 0x4C71]);
 memcpyBigEndian8(&gameState.kidPosition03,       &state[_stateWorkRamOffset + 0x4A6F]);
 memcpyBigEndian8(&gameState.kidPosition04,       &state[_stateWorkRamOffset + 0x4A71]);
 memcpyBigEndian8(&gameState.kidPosition05,       &state[_stateWorkRamOffset + 0x4A75]);
 memcpyBigEndian8(&gameState.kidPosition06,       &state[_stateWorkRamOffset + 0x1965]);
 memcpyBigEndian8(&gameState.kidPosition07,       &state[_stateWorkRamOffset + 0x196F]);
 memcpyBigEndian8(&gameState.kidPosition08,       &state[_stateWorkRamOffset + 0x4C6B]);
 memcpyBigEndian8(&gameState.kidPosition09,       &state[_stateWorkRamOffset + 0x4C5B]);
 memcpyBigEndian8(&gameState.kidPosition10,      &state[_stateWorkRamOffset + 0x4C59]);
 memcpyBigEndian8(&gameState.kidPosition11,      &state[_stateWorkRamOffset + 0x4C48]);
 memcpyBigEndian8(&gameState.kidPosition12,      &state[_stateWorkRamOffset + 0x4CB9]);
 memcpyBigEndian8(&gameState.kidPosition13,      &state[_stateWorkRamOffset + 0x4CDB]);
 memcpyBigEndian8(&gameState.kidPosition15,      &state[_stateWorkRamOffset + 0x4CD9]);
 memcpyBigEndian8(&gameState.kidPosition16,      &state[_stateWorkRamOffset + 0x4CD5]);
 memcpyBigEndian8(&gameState.kidPosition17,      &state[_stateWorkRamOffset + 0x4CD1]);
 memcpyBigEndian8(&gameState.kidPosition19,      &state[_stateWorkRamOffset + 0x4C71]);
 memcpyBigEndian8(&gameState.kidPosition20,      &state[_stateWorkRamOffset + 0x4C6D]);
 memcpyBigEndian8(&gameState.kidPosition21,      &state[_stateWorkRamOffset + 0x4C6B]);
 memcpyBigEndian8(&gameState.kidPosition22,      &state[_stateWorkRamOffset + 0x4C5F]);
 memcpyBigEndian8(&gameState.kidPosition23,      &state[_stateWorkRamOffset + 0x4C54]);
 memcpyBigEndian8(&gameState.kidPosition24,      &state[_stateWorkRamOffset + 0x4C45]);
 memcpyBigEndian8(&gameState.kidPosition26,      &state[_stateWorkRamOffset + 0x196D]);
 memcpyBigEndian8(&gameState.kidPosition27,      &state[_stateWorkRamOffset + 0x1969]);
 memcpyBigEndian8(&gameState.kidPosition28,      &state[_stateWorkRamOffset + 0x1966]);
 memcpyBigEndian8(&gameState.kidPosition29,      &state[_stateWorkRamOffset + 0x1970]);

 memcpyBigEndian8(&gameState.kidPosition30,      &state[_stateWorkRamOffset + 0x195D]);
 memcpyBigEndian8(&gameState.kidPosition31,      &state[_stateWorkRamOffset + 0x196A]);
 memcpyBigEndian8(&gameState.kidPosition32,      &state[_stateWorkRamOffset + 0x1978]);
 memcpyBigEndian8(&gameState.kidPosition33,      &state[_stateWorkRamOffset + 0x1982]);
 memcpyBigEndian8(&gameState.kidPosition34,      &state[_stateWorkRamOffset + 0x198C]);
 memcpyBigEndian8(&gameState.kidPosition35,      &state[_stateWorkRamOffset + 0x199A]);
 memcpyBigEndian8(&gameState.kidPosition36,      &state[_stateWorkRamOffset + 0x4A31]);
 memcpyBigEndian8(&gameState.kidPosition37,      &state[_stateWorkRamOffset + 0x4A35]);
 memcpyBigEndian8(&gameState.kidPosition38,      &state[_stateWorkRamOffset + 0x4D05]);


 memcpyBigEndian8(&gameState.kidHangingTimeLeft, &state[_stateWorkRamOffset + 0x4C87]);

 memcpyBigEndian8(&gameState.guardFrame,         &state[_stateWorkRamOffset + 0x4AFB]);
 memcpyBigEndian8(&gameState.guardCurrentHP,     &state[_stateWorkRamOffset + 0x4B05]);
 memcpyBigEndian8(&gameState.guard2CurrentHP,    &state[_stateWorkRamOffset + 0x4B47]);
 memcpyBigEndian8(&gameState.guardMaxHP,         &state[_stateWorkRamOffset + 0x4B06]);
 memcpyBigEndian8(&gameState.guardRoom,          &state[_stateWorkRamOffset + 0x4B01]);
 memcpyBigEndian8(&gameState.guardDirection,     &state[_stateWorkRamOffset + 0x4AF3]);
 memcpyBigEndian16(&gameState.guardPositionX,    &state[_stateWorkRamOffset + 0x4AF4]);
 memcpyBigEndian16(&gameState.guardPositionY,    &state[_stateWorkRamOffset + 0x4AF6]);

 return gameState;
}

uint64_t blastemInstance::computeHash()
{
  MetroHash64 hash;

//  hash.Update(&_state.videoFrame, sizeof(uint16_t));
  hash.Update(&_state.framesPerStep, sizeof(uint8_t));
  hash.Update(&_state.currentLevel, sizeof(uint8_t));
  hash.Update(&_state.drawnRoom, sizeof(uint8_t));
//  hash.Update(&_state.minutesLeft, sizeof(uint16_t));
//  hash.Update(&_state.twelthSecondsLeft, sizeof(uint16_t));
//  hash.Update(&_state.checkpointPointer, sizeof(uint32_t));
//  hash.Update(&_state.slowfallFramesLeft, sizeof(uint8_t));

  // If beach level, we take into account sand tile status
  if (_state.currentLevel == 2)
  {
   hash.Update(&_state.sandTile1, sizeof(uint8_t));
   hash.Update(&_state.sandTile2, sizeof(uint8_t));
   hash.Update(&_state.sandTile3, sizeof(uint8_t));
   hash.Update(&_state.sandTile4, sizeof(uint8_t));
   hash.Update(&_state.sandTile5, sizeof(uint8_t));
   hash.Update(&_state.caveEntrancePos, sizeof(uint8_t));
  }

  if (_state.currentLevel == 3)
  {
//   hash.Update(&_state.lvl3ExitDoor, sizeof(uint8_t));
  }

  if (_state.currentLevel == 4)
  {
   //hash.Update(&_state.lvl4ExitDoor, sizeof(uint8_t));
  }

  if (_state.currentLevel == 5)
  {
   hash.Update(&_state.lvl5Room2Door, sizeof(uint8_t));
   hash.Update(&_state.lvl5CarpetHatch, sizeof(uint8_t));
  }

  if (_state.currentLevel == 7)
  {
   uint8_t doorState = _state.lvl7PostPotionDoor;
   uint8_t simplerDoorState = doorState / 10;
   hash.Update(&simplerDoorState, sizeof(uint8_t));
   hash.Update(&_state.lvl7ExitDoor, sizeof(uint8_t));
  }

  if (_state.currentLevel == 8)
  {
   hash.Update(&_state.lvl8ExitDoor, sizeof(uint8_t));
  }

  if (_state.currentLevel == 11)
  {
   hash.Update(&_state.lvl11PotionDoor, sizeof(uint8_t));
   hash.Update(&_state.lvl11PostPotionDoor, sizeof(uint8_t));
   hash.Update(&_state.lvl11ExitRoomLeftDoor, sizeof(uint8_t));
   hash.Update(&_state.lvl11ExitDoor, sizeof(uint8_t));
  }

  if (_state.currentLevel == 12)
  {
   hash.Update(&_state.lvl12FirstDoor, sizeof(uint8_t));
   hash.Update(&_state.lvl12ExitDoor, sizeof(uint8_t));
   hash.Update(&_state.lvl12WeirdRoomDoor, sizeof(uint8_t));
  }

  if (_state.currentLevel == 13)
  {
   uint8_t doorState = _state.lvl13FirstRoomDoor > 0 ? 1 : 0;
   hash.Update(&doorState, sizeof(uint8_t));
   hash.Update(&_state.lvl13ExitDoor, sizeof(uint8_t));
  }

  if (_state.currentLevel == 14)
  {
   hash.Update(&_state.lvl14FakeJaffarsLeft, sizeof(uint8_t));
  }

  hash.Update(&_state.kidCurrentSequence, sizeof(uint8_t));
  hash.Update(&_state.kidCurrentSequenceStage, sizeof(uint8_t));
  hash.Update(&_state.kidLastSequence, sizeof(uint8_t));
  hash.Update(&_state.kidFrame, sizeof(uint8_t));
  hash.Update(&_state.kidCurrentHP, sizeof(uint8_t));
  hash.Update(&_state.kidMaxHP, sizeof(uint8_t));
  hash.Update(&_state.kidRoom, sizeof(uint8_t));
  hash.Update(&_state.kidHasSword, sizeof(uint8_t));
  hash.Update(&_state.kidDirection, sizeof(uint8_t));
  hash.Update(&_state.kidPositionX, sizeof(uint16_t));
  hash.Update(&_state.kidPositionY, sizeof(uint16_t));
  hash.Update(&_state.kidFallingSpeed, sizeof(uint8_t));
  hash.Update(&_state.kidFrameVariant, sizeof(uint8_t));
  hash.Update(&_state.kidPosition01, sizeof(uint8_t));
  hash.Update(&_state.kidPosition02, sizeof(uint8_t));
  hash.Update(&_state.kidPosition03, sizeof(uint8_t));
  hash.Update(&_state.kidPosition04, sizeof(uint8_t));
  hash.Update(&_state.kidPosition05, sizeof(uint8_t));
  hash.Update(&_state.kidPosition06, sizeof(uint8_t));
  hash.Update(&_state.kidPosition07, sizeof(uint8_t));
  hash.Update(&_state.kidPosition08, sizeof(uint8_t));
  hash.Update(&_state.kidPosition09, sizeof(uint8_t));
  hash.Update(&_state.kidPosition10, sizeof(uint8_t));
  hash.Update(&_state.kidPosition11, sizeof(uint8_t));
  hash.Update(&_state.kidPosition12, sizeof(uint8_t));
  hash.Update(&_state.kidPosition13, sizeof(uint8_t));
  hash.Update(&_state.kidPosition15, sizeof(uint8_t));
  hash.Update(&_state.kidPosition16, sizeof(uint8_t));
  hash.Update(&_state.kidPosition17, sizeof(uint8_t));
  hash.Update(&_state.kidPosition19, sizeof(uint8_t));
  hash.Update(&_state.kidPosition20, sizeof(uint8_t));
  hash.Update(&_state.kidPosition21, sizeof(uint8_t));
  hash.Update(&_state.kidPosition22, sizeof(uint8_t));
  hash.Update(&_state.kidPosition23, sizeof(uint8_t));
  hash.Update(&_state.kidPosition24, sizeof(uint8_t));
  hash.Update(&_state.kidPosition26, sizeof(uint8_t));
  hash.Update(&_state.kidPosition27, sizeof(uint8_t));
  hash.Update(&_state.kidPosition28, sizeof(uint8_t));
  hash.Update(&_state.kidPosition29, sizeof(uint8_t));

  hash.Update(&_state.kidPosition30, sizeof(uint8_t));
  hash.Update(&_state.kidPosition31, sizeof(uint8_t));
  hash.Update(&_state.kidPosition32, sizeof(uint8_t));
  hash.Update(&_state.kidPosition33, sizeof(uint8_t));
  hash.Update(&_state.kidPosition34, sizeof(uint8_t));
  hash.Update(&_state.kidPosition35, sizeof(uint8_t));
  hash.Update(&_state.kidPosition36, sizeof(uint8_t));
  hash.Update(&_state.kidPosition37, sizeof(uint8_t));
  hash.Update(&_state.kidPosition38, sizeof(uint8_t));

  hash.Update(&_state.kidHangingTimeLeft, sizeof(uint8_t));

  hash.Update(&_state.guardFrame, sizeof(uint8_t));
  hash.Update(&_state.guardCurrentHP, sizeof(uint8_t));
  hash.Update(&_state.guard2CurrentHP, sizeof(uint8_t));
  hash.Update(&_state.guardMaxHP, sizeof(uint8_t));
  hash.Update(&_state.guardRoom, sizeof(uint8_t));
  hash.Update(&_state.guardDirection, sizeof(uint8_t));
  hash.Update(&_state.guardPositionX, sizeof(uint16_t));
  hash.Update(&_state.guardPositionY, sizeof(uint16_t));

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

void blastemInstance::printState()
{
 printf("[Jaffar2]  + Current Level: %2d\n", _state.currentLevel);
 printf("[Jaffar2]  + Current RNG Value: 0x%X\n", _state.rngValue);
 printf("[Jaffar2]  + Game / Video Frame: %d / %d\n", _state.gameFrame, _state.videoFrame);
 printf("[Jaffar2]  + Checkpoint Pointer: 0x%X\n", _state.checkpointPointer);

 if (_state.currentLevel == 12)
 {
  printf("[Jaffar2]  + Exit Door Status: %d\n", _state.lvl12ExitDoor);
  printf("[Jaffar2]  + Weird Door Status: %d\n", _state.lvl12WeirdRoomDoor);
 }

 if (_state.currentLevel == 14)
 {
  printf("[Jaffar2]  + Fake Jaffars Remaining: %d\n", _state.lvl14FakeJaffarsLeft);
 }

 printf("[Jaffar2]  + [Kid]   Room: %d, Pos.x: %3d, Pos.y: %3d (+%3d), Frame: %2d, Direction: %s, HP: %d/%d\n", _state.kidRoom, _state.kidPositionX, _state.kidPositionY, _state.kidFallingSpeed, _state.kidFrame, _state.kidDirection == 255 ? "L" : "R", _state.kidCurrentHP, _state.kidMaxHP);
 printf("[Jaffar2]  + [Guard] Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, Direction: %s, HP: %d(%d)/%d\n", _state.guardRoom, _state.guardPositionX, _state.guardPositionY, _state.guardFrame, _state.guardDirection == 255 ? "L" : "R", _state.guardCurrentHP, _state.guard2CurrentHP, _state.guardMaxHP);
}

//                                   Move Ids =    0    1    2    3    4    5     6     7     8    9     10    11    12    13   14    15   16
const std::vector<std::string> _possibleMoves = { ".", "B", "A", "L", "R", "D", "LA", "LD", "RA", "RD", "BR", "BL", "BU", "BD", "C",  "U", "S" };
// A = Jump
// B = Hold / Careful Step
// C = Attack
// S = Start

std::vector<uint8_t> blastemInstance::getPossibleMoveIds(const gameStateStruct& gameState)
{
  if (gameState.kidFrame == 0) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Standing??
  if (gameState.kidFrame == 1) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 2) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 3) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 4) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 5) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 6) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 7) return {0, 3, 4, 5, 6, 8}; // Running + Can Jump
  if (gameState.kidFrame == 8) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 9) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 10) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 11) return {0, 3, 4, 5, 6, 8}; // Running + Can Jump
  if (gameState.kidFrame == 12) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 13) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 14) return {0, 3, 4, 5}; // Running
  if (gameState.kidFrame == 15) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Standing
  if (gameState.kidFrame == 16) return {0}; // Standing Jump
  if (gameState.kidFrame == 17) return {0}; // Standing Jump
  if (gameState.kidFrame == 18) return {0}; // Standing Jump
  if (gameState.kidFrame == 19) return {0}; // Standing Jump
  if (gameState.kidFrame == 20) return {0}; // Standing Jump
  if (gameState.kidFrame == 21) return {0}; // Standing Jump
  if (gameState.kidFrame == 22) return {0}; // Standing Jump
  if (gameState.kidFrame == 23) return {0}; // Standing Jump
  if (gameState.kidFrame == 24) return {0}; // Standing Jump
  if (gameState.kidFrame == 25) return {0}; // Standing Jump
  if (gameState.kidFrame == 26) return {0}; // Standing Jump
  if (gameState.kidFrame == 27) return {0}; // Standing Jump
  if (gameState.kidFrame == 28) return {0}; // Standing Jump
  if (gameState.kidFrame == 29) return {0}; // Standing Jump
  if (gameState.kidFrame == 30) return {0}; // Standing Jump
  if (gameState.kidFrame == 31) return {0}; // Standing Jump
  if (gameState.kidFrame == 32) return {0}; // Standing Jump
  if (gameState.kidFrame == 33) return {0}; // Standing Jump
  if (gameState.kidFrame == 34) return {0}; // Running Jump
  if (gameState.kidFrame == 35) return {0}; // Running Jump
  if (gameState.kidFrame == 36) return {0}; // Running Jump
  if (gameState.kidFrame == 37) return {0}; // Running Jump
  if (gameState.kidFrame == 38) return {0}; // Running Jump
  if (gameState.kidFrame == 39) return {0}; // Running Jump
  if (gameState.kidFrame == 40) return {0}; // Running Jump
  if (gameState.kidFrame == 41) return {0}; // Running Jump
  if (gameState.kidFrame == 42) return {0}; // Running Jump
  if (gameState.kidFrame == 43) return {0}; // Post-Jump
  if (gameState.kidFrame == 44) return {0}; // Post-Jump
  if (gameState.kidFrame == 45) return {0}; // Turning
  if (gameState.kidFrame == 46) return {0}; // Turning
  if (gameState.kidFrame == 47) return {0}; // Turning
  if (gameState.kidFrame == 48) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Turning
  if (gameState.kidFrame == 49) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Stopping after run / Recovering
  if (gameState.kidFrame == 50) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Stopping after run / Recovering (Can act now)
  if (gameState.kidFrame == 51) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Stopping after run / Recovering (Can act now)
  if (gameState.kidFrame == 52) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Stopping after run / Recovering (Can act now)
  if (gameState.kidFrame == 53) return {0}; // Stopping after run
  if (gameState.kidFrame == 54) return {0}; // Stopping after run
  if (gameState.kidFrame == 55) return {0}; // Stopping after run
  if (gameState.kidFrame == 56) return {0}; // Stopping after run
  if (gameState.kidFrame == 57) return {0}; // Running Turn
  if (gameState.kidFrame == 58) return {0}; // Running Turn
  if (gameState.kidFrame == 59) return {0}; // Running Turn
  if (gameState.kidFrame == 60) return {0}; // Running Turn
  if (gameState.kidFrame == 61) return {0}; // Running Turn
  if (gameState.kidFrame == 62) return {0}; // Running Turn
  if (gameState.kidFrame == 63) return {0}; // Running Turn
  if (gameState.kidFrame == 64) return {0}; // Running Turn
  if (gameState.kidFrame == 65) return {0}; // Running Turn
  if (gameState.kidFrame == 66) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Standing?
  if (gameState.kidFrame == 67) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 68) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 69) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 70) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 71) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 72) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 73) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 74) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 75) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 76) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 77) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 78) return {0}; // Upwards Jump / Climbing
  if (gameState.kidFrame == 79) return {0, 1, 12}; // Upwards Jump / Climbing (Can Grab)
  if (gameState.kidFrame == 80) return {0, 1, 12}; // Upwards Jump / Climbing (Can Grab)
  if (gameState.kidFrame == 81) return {0}; // Let go after grab / After Upwards Jump
  if (gameState.kidFrame == 82) return {0}; // Let go after grab / After Upwards Jump
  if (gameState.kidFrame == 83) return {0}; // Let go after grab / After Upwards Jump
  if (gameState.kidFrame == 84) return {0}; // Let go after grab / After Upwards Jump
  if (gameState.kidFrame == 85) return {0}; // Let go after grab / After Upwards Jump
  if (gameState.kidFrame == 86) return {0}; // Hesitant Careful Step (before a ledge)
  if (gameState.kidFrame == 87) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 88) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 89) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 90) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 91) return {0, 1, 12}; // Grabbing ledge, can go up
  if (gameState.kidFrame == 92) return {0, 1, 12}; // Grabbing ledge, can go up
  if (gameState.kidFrame == 93) return {0, 1, 12}; // Grabbing ledge, can go up
  if (gameState.kidFrame == 94) return {0, 1, 12}; // Grabbing ledge, can go up
  if (gameState.kidFrame == 95) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 96) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 97) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 98) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 99) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 100) return {0, 1, 12}; // Hanging from ledge
  if (gameState.kidFrame == 101) return {0, 1, 12}; // Turning
  if (gameState.kidFrame == 102) return {0, 1, 12}; // Falling
  if (gameState.kidFrame == 103) return {0, 1, 12}; // Falling
  if (gameState.kidFrame == 104) return {0, 1, 12}; // Falling
  if (gameState.kidFrame == 105) return {0, 1, 12}; // Falling
  if (gameState.kidFrame == 106) return {0, 1, 12}; // Falling
  if (gameState.kidFrame == 107) return {0, 1}; // Pre-Crouching
  if (gameState.kidFrame == 108) return {0, 1}; // Pre-Crouching
  if (gameState.kidFrame == 109) return {0, 1, 7, 9}; // Crouching
  if (gameState.kidFrame == 110) return {0}; // Post-Crouching
  if (gameState.kidFrame == 111) return {0}; // Post-Crouching
  if (gameState.kidFrame == 111) return {0}; // Post-Crouching
  if (gameState.kidFrame == 112) return {0}; // Post-Crouching
  if (gameState.kidFrame == 113) return {0}; // Post-Crouching
  if (gameState.kidFrame == 114) return {0}; // Post-Crouching
  if (gameState.kidFrame == 115) return {0}; // Post-Crouching
  if (gameState.kidFrame == 116) return {0}; // Post-Crouching
  if (gameState.kidFrame == 117) return {0}; // Post-Crouching
  if (gameState.kidFrame == 118) return {0}; // Post-Crouching
  if (gameState.kidFrame == 119) return {0}; // Post-Crouching
  if (gameState.kidFrame == 121) return {0}; // Careful Step
  if (gameState.kidFrame == 122) return {0}; // Careful Step
  if (gameState.kidFrame == 123) return {0}; // Careful Step
  if (gameState.kidFrame == 124) return {0}; // Careful Step
  if (gameState.kidFrame == 125) return {0}; // Careful Step
  if (gameState.kidFrame == 126) return {0}; // Careful Step
  if (gameState.kidFrame == 127) return {0}; // Careful Step
  if (gameState.kidFrame == 128) return {0}; // Careful Step
  if (gameState.kidFrame == 129) return {0}; // Careful Step
  if (gameState.kidFrame == 130) return {0}; // Careful Step
  if (gameState.kidFrame == 131) return {0}; // Careful Step
  if (gameState.kidFrame == 132) return {0}; // Careful Step
  if (gameState.kidFrame == 133) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Final Sheathing Sword
  if (gameState.kidFrame == 134) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Final Sheathing Sword
  if (gameState.kidFrame == 135) return {0}; // Climbing Up
  if (gameState.kidFrame == 136) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 137) return {0}; // Climbing Up
  if (gameState.kidFrame == 138) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 139) return {0}; // Climbing Up
  if (gameState.kidFrame == 140) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 141) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 142) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 143) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 144) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 145) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 146) return {0}; // Climbing Up
  if (gameState.kidFrame == 147) return {0}; // Climbing Up
  if (gameState.kidFrame == 148) return {0}; // Climbing Down/Up
  if (gameState.kidFrame == 149) return {0}; // Climbing Up
  if (gameState.kidFrame == 150) return {0, 10, 11, 14}; // [Sword] Parrying 2 - Can Attack
  if (gameState.kidFrame == 151) return {0, 2, }; // [Sword] Attack
  if (gameState.kidFrame == 152) return {0, 2}; // [Sword] Attack
  if (gameState.kidFrame == 153) return {0, 2}; // [Sword] Attack
  if (gameState.kidFrame == 154) return {0, 2}; // [Sword] Attack
  if (gameState.kidFrame == 155) return {0, 2}; // [Sword] Attack
  if (gameState.kidFrame == 156) return {0, 2}; // [Sword] After Attack / Recovering from Hit
  if (gameState.kidFrame == 157) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] After Attack / Recovering from Hit
  if (gameState.kidFrame == 158) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] En Guarde
  if (gameState.kidFrame == 159) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Parry
  if (gameState.kidFrame == 160) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Walk Backward 2
  if (gameState.kidFrame == 161) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Attack While parrying
  if (gameState.kidFrame == 162) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Attack While parrying
  if (gameState.kidFrame == 163) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Walk Forward
  if (gameState.kidFrame == 164) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Walk Forward
  if (gameState.kidFrame == 165) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Walk Forward
  if (gameState.kidFrame == 166) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Standing??
  if (gameState.kidFrame == 167) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Attack
  if (gameState.kidFrame == 168) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Attack?
  if (gameState.kidFrame == 169) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Parrying 1
  if (gameState.kidFrame == 170) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] En Guarde
  if (gameState.kidFrame == 171) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] En Guarde
  if (gameState.kidFrame == 172) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Getting Hit
  if (gameState.kidFrame == 173) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Getting Hit
  if (gameState.kidFrame == 174) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Getting Hit
  if (gameState.kidFrame == 177) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Turning
  if (gameState.kidFrame == 178) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Turning
  if (gameState.kidFrame == 179) return {0}; // Dying
  if (gameState.kidFrame == 180) return {0}; // Dying
  if (gameState.kidFrame == 181) return {0}; // Dying
  if (gameState.kidFrame == 182) return {0}; // Dying
  if (gameState.kidFrame == 183) return {0}; // Dying
  if (gameState.kidFrame == 184) return {0}; // Falling
  if (gameState.kidFrame == 185) return {0}; // Dying
  if (gameState.kidFrame == 186) return {0}; // Gruesome Death
  if (gameState.kidFrame == 187) return {0}; // Gruesome Death
  if (gameState.kidFrame == 188) return {0}; // Gruesome Death
  if (gameState.kidFrame == 189) return {0}; // Gruesome Death
  if (gameState.kidFrame == 191) return {0}; // Drinking
  if (gameState.kidFrame == 192) return {0}; // Drinking
  if (gameState.kidFrame == 193) return {0}; // Drinking
  if (gameState.kidFrame == 194) return {0}; // Drinking
  if (gameState.kidFrame == 195) return {0}; // Drinking
  if (gameState.kidFrame == 196) return {0}; // Drinking
  if (gameState.kidFrame == 197) return {0}; // Drinking
  if (gameState.kidFrame == 198) return {0}; // Drinking
  if (gameState.kidFrame == 199) return {0}; // Drinking
  if (gameState.kidFrame == 200) return {0}; // Drinking
  if (gameState.kidFrame == 201) return {0}; // Drinking
  if (gameState.kidFrame == 202) return {0}; // Drinking
  if (gameState.kidFrame == 203) return {0}; // Drinking
  if (gameState.kidFrame == 204) return {0}; // Drinking
  if (gameState.kidFrame == 205) return {0}; // Drinking
  if (gameState.kidFrame == 206) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Standing??
  if (gameState.kidFrame == 207) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Drawing Sword
  if (gameState.kidFrame == 208) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Drawing Sword
  if (gameState.kidFrame == 209) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Drawing Sword
  if (gameState.kidFrame == 210) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Drawing Sword
  if (gameState.kidFrame == 211) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Turning
  if (gameState.kidFrame == 212) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Turning
  if (gameState.kidFrame == 213) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Turning
  if (gameState.kidFrame == 217) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 218) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 219) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 220) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 221) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 222) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 223) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 224) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 225) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 226) return {0}; // Leaving Level (Door)
  if (gameState.kidFrame == 229) return {0}; // Grabbing Sword
  if (gameState.kidFrame == 230) return {0}; // Grabbing Sword
  if (gameState.kidFrame == 231) return {0}; // Grabbing Sword
  if (gameState.kidFrame == 232) return {0}; // Grabbing Sword
  if (gameState.kidFrame == 233) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 234) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 235) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 236) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 237) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 238) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 239) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 240) return {0, 3, 4, 5, 10, 11, 14, 15}; // [Sword] Sheathing Sword
  if (gameState.kidFrame == 241) return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Standing??
  if (gameState.kidFrame == 242) return {0}; // Gruesome Death
  if (gameState.kidFrame == 243) return {0}; // Gruesome Death
  if (gameState.kidFrame == 244) return {0}; // [Sword] Attacking Downward
  if (gameState.kidFrame == 245) return {0}; // [Sword] Attacking Downward
  if (gameState.kidFrame == 246) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 247) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 248) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 249) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 250) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 251) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 252) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 253) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 254) return {0, 1, 2, 3, 4, 15}; // Crawling
  if (gameState.kidFrame == 255) return {0, 1, 2, 3, 4, 15}; // Crawling

  // Default, report unrecognized frame
  EXIT_WITH_ERROR("[Jaffar2] Warning: Frame %d not recognized.\n", gameState.kidFrame);
  return {0};
}


int blastemInstance::playFrame(const std::string& move)
{
 strcpy(_nextMove, move.c_str());
 blastem_resume();

 if (_detectedError == 1)
 {
  printf("Coming back from error\n");
  _detectedError = 0;
  return 1;
 }
 _state = getGameState(_stateData);
 return 0;
}

void blastemInstance::loadState(const uint8_t* state)
{
 memcpy(_stateData, state, _stateSize);
 reloadState();
 _state = getGameState(_stateData);
}

void blastemInstance::saveState(uint8_t* state)
{
 memcpy(state, _stateData, _stateSize);
}

void blastemInstance::redraw()
{
// redraw();
}

void blastemInstance::finalize()
{
 finalize();
}

