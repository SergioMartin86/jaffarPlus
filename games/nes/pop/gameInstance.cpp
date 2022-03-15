#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  globalTimer            = (uint8_t*)   &_emu->_baseMem[0x0791];
  currentLevel           = (uint8_t*)   &_emu->_baseMem[0x0070];
  RNGState               = (uint8_t*)   &_emu->_baseMem[0x0060];
  framePhase             = (uint8_t*)   &_emu->_baseMem[0x002C];
  kidPosX                = (int16_t*)   &_emu->_baseMem[0x060F];
  kidPosY                = (uint8_t*)   &_emu->_baseMem[0x0611];
  kidFrame               = (uint8_t*)   &_emu->_baseMem[0x0617];
  kidMovement            = (uint8_t*)   &_emu->_baseMem[0x0613];
  kidFallWait            = (uint8_t*)   &_emu->_baseMem[0x06E4];
  kidHP                  = (uint8_t*)   &_emu->_baseMem[0x06CF];
  kidRoom                = (uint8_t*)   &_emu->_baseMem[0x04DE];
  kidJumpingState        = (uint8_t*)   &_emu->_baseMem[0x0616];
  kidFightMode           = (uint8_t*)   &_emu->_baseMem[0x06EE];
  guardPosX              = (uint8_t*)   &_emu->_baseMem[0x061D];
  guardHP                = (uint8_t*)   &_emu->_baseMem[0x06D0];
  guardNotice            = (uint8_t*)   &_emu->_baseMem[0x0601];
  guardFrame             = (uint8_t*)   &_emu->_baseMem[0x0621];
  guardMovement          = (uint8_t*)   &_emu->_baseMem[0x0625];
  drawnRoom              = (uint8_t*)   &_emu->_baseMem[0x0051];
  screenTransition       = (uint8_t*)   &_emu->_baseMem[0x04AC];
  exitDoorState          = (uint8_t*)   &_emu->_baseMem[0x0400];

  screenDrawn            = (uint8_t*)   &_emu->_baseMem[0x0732];
  isPaused               = (uint8_t*)   &_emu->_baseMem[0x06D1];
  bufferedCommand        = (uint16_t*)  &_emu->_baseMem[0x04C9];
  doorOpeningTimer       = (uint8_t*)   &_emu->_baseMem[0x04CD];
  currentDoorState       = (uint8_t*)   &_emu->_baseMem[0x073A];

  guardPresent           = (uint8_t*)   &_emu->_baseMem[0x06E0];
  guardDisappearMode     = (uint8_t*)   &_emu->_baseMem[0x04F8];

  // Level-specific tiles
  lvl1FirstTileBG          = (uint8_t*)   &_emu->_baseMem[0x06A4];
  lvl1FirstTileFG          = (uint8_t*)   &_emu->_baseMem[0x06B0];
  lvl1Room19DoorTimer      = (uint8_t*)   &_emu->_baseMem[0x05E9];
  lvl2LastTileFG           = (uint8_t*)   &_emu->_baseMem[0x0665];
  lvl2ExitDoorState        = (uint8_t*)   &_emu->_baseMem[0x0708];

  if (isDefined(config, "Hash Includes") == true)
   for (const auto& entry : config["Hash Includes"])
    hashIncludes.insert(entry.get<std::string>());
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Hash Includes' was not defined\n");

  updateDerivedValues();
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  hash.Update(*currentLevel);
  hash.Update(*framePhase);
  hash.Update(*doorOpeningTimer);
  hash.Update(*currentDoorState);
  hash.Update(*exitDoorState);

  hash.Update(*kidPosX);
  hash.Update(*kidPosY);
  hash.Update(*kidFrame);
  hash.Update(*kidMovement);
  hash.Update(*kidFallWait);
  hash.Update(*kidHP);
  hash.Update(*kidRoom);
  hash.Update(*kidFightMode);
  hash.Update(*kidJumpingState);

  hash.Update(*bufferedCommand);

  hash.Update(*guardPosX);
  hash.Update(*guardHP);
  hash.Update(*guardNotice);
  hash.Update(*guardFrame);
  hash.Update(*guardMovement);
  hash.Update(*drawnRoom);
  hash.Update(*screenTransition);
  hash.Update(*screenDrawn);
  hash.Update(*isPaused);

  hash.Update(*guardPresent);
  hash.Update(*guardDisappearMode);

  // Level-specific tiles
  if (*currentLevel == 0)
  {
   hash.Update(*lvl1FirstTileBG);
   hash.Update(*lvl1FirstTileFG);
  }

  if (*currentLevel == 1)
  {
   hash.Update(*lvl2LastTileFG);
   hash.Update(*lvl2ExitDoorState);
  }

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 isCorrectRender = 1;
 if (_emu->_nes->emu.ppu.isCorrectRender == false) isCorrectRender = 0;

 // Advancing useless frames
 uint16_t advanceCounter = 0;

 // Number of frames per state
 uint16_t framesPerState = 4;
 if ( (*guardPresent > 0) && (*guardDisappearMode == 0) ) framesPerState = 5;

  while (*screenTransition == 255 && advanceCounter < 32) { _emu->advanceState(0); advanceCounter++; }
  while ( (advanceCounter < 32) && (*isPaused != 2) ) { _emu->advanceState(0); advanceCounter++; }
  if (*kidJumpingState == 28 && *framePhase == 4) return; // Allows for ending level

  while ( (advanceCounter < 64) && (framesPerState == 4) && (*framePhase != 2) ) { _emu->advanceState(0); advanceCounter++; }
  while ( (advanceCounter < 64) && (framesPerState == 5) && (*framePhase != 3) ) { _emu->advanceState(0); advanceCounter++; }
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 // Allows for ending level
 if (*kidJumpingState == 28 && *framePhase == 4) return { ".", "U" };

 if (*kidFrame == 1)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 2)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 3)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 4)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 5)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 6)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 7)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 8)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA", "LDB", "RDB", "LB", "RB" }; // Running
  if (*kidFrame == 9)  return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 10) return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 11) return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA", "LDB", "RDB", "LB", "RB" }; // Running
  if (*kidFrame == 12) return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 13) return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 14) return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" }; // Running
  if (*kidFrame == 15) return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "RB", "LB", "UD", "DRA", "DLA", "UB", "DA", "DB", "LD", "DUBA", "DUB", "UA", "RL", "DUR", "DUL", "DURL", "DURLA", "DURLB", "DURLAB", "DUBL", "DUBR", "DUBRA", "DUBLA", "UBA"}; // Normal Standing

  if (*kidFrame == 16) return { "." }; // Standing Jump
  if (*kidFrame == 17) return { "." }; // Standing Jump
  if (*kidFrame == 18) return { "." }; // Standing Jump
  if (*kidFrame == 19) return { "." }; // Standing Jump
  if (*kidFrame == 20) return { ".", "DB", "B", "DA", "LA", "RA", "RDA", "LDA" }; // Standing Jump
  if (*kidFrame == 21) return { ".", "LA", "RA", "A", "DA", "DB", "B" }; // Standing Jump
  if (*kidFrame == 22) return { "." }; // Standing Jump
  if (*kidFrame == 23) return { ".", "LA", "RA", "DB", "DA", "RDA", "LDA", "A", "B" }; // Standing Jump
  if (*kidFrame == 24) return { ".", "RA", "LA", "A", "RDA", "LDA", "DB", "DA", "B" }; // Standing Jump
  if (*kidFrame == 25) return { ".", "RA", "LA", "B"}; // Standing Jump
  if (*kidFrame == 26) return { ".", "A", "R", "L" }; // Standing Jump
  if (*kidFrame == 27) return { ".", "DB", "RDA", "LDA" }; // Standing Jump
  if (*kidFrame == 28) return { ".", "A", "UA", "L", "RBA"}; // Standing Jump
  if (*kidFrame == 29) return { "." }; // Standing Jump
  if (*kidFrame == 30) return { "." }; // Standing Jump
  if (*kidFrame == 31) return { "." }; // Standing Jump
  if (*kidFrame == 32) return { "." }; // Standing Jump
  if (*kidFrame == 33) return { "." }; // Standing Jump

  if (*kidFrame == 34) return { ".", "A", "B", "DA", "LA", "RA", "DB", "RDA", "RDB", "LDA", "LDB" }; // Jumping Animation
  if (*kidFrame == 35) return { "." }; // Jumping Animation
  if (*kidFrame == 37) return { ".", "DB", "A", "B", "LA", "RA", "DA", "RDA", "LDA" }; // Jumping Animation
  if (*kidFrame == 38) return { "." }; // Jumping Animation
  if (*kidFrame == 39) return { ".", "DA", "B", "LA", "RA", "DB", "A", "RDA", "LDA" }; // Jumping Animation
  if (*kidFrame == 40) return { ".", "A", "B", "RDA", "LDA", "DB", "DA", "LA", "RA" }; // Jumping Animation
  if (*kidFrame == 41) return { ".", "DA", "RDA", "B", "LDA", "RA", "LA", "A", "DB" }; // Jumping Animation
  if (*kidFrame == 42) return { ".", "A", "B", "LA", "RA", "DA", "RDA", "LDA" }; // Jumping Animation
  if (*kidFrame == 43) return { ".", "A", "R", "L", "DA", "RA", "LA" }; // Jumping Animation
  if (*kidFrame == 44) return { ".", "A", "RDA", "LDA" }; // Jumping Animation

  if (*kidFrame == 45) return { ".", "A", "RB", "RA", "LB", "LA", "DB", "R", "L" }; // Turning Animation
  if (*kidFrame == 46) return { ".", "B", "A", "RB", "R", "L", "LB", "DB", "LA", "RA" }; // Turning Animation
  if (*kidFrame == 47) return { "." }; // Turning Animation
  if (*kidFrame == 48) return { "." }; // Turning Animation

  if (*kidFrame == 49) return { ".", "RDA", "LDA", "RA", "LA" }; // Slowing Down from Running
  if (*kidFrame == 50) return { "." }; // Slowing Down from Running
  if (*kidFrame == 51) return { "." }; // Slowing Down from Running
  if (*kidFrame == 52) return { "." }; // Slowing Down from Running
  if (*kidFrame == 53) return { ".", "A", "B", "DB", "DA", "RA", "LA" }; // Slowing Down from Running
  if (*kidFrame == 54) return { "." }; // Slowing Down from Running
  if (*kidFrame == 55) return { ".", "B", "DA", "A", "RDA", "LDA", "LA", "RA", "DB" }; // Slowing Down from Running
  if (*kidFrame == 56) return { ".", "A", "LA", "RA", "DA", "DB", "RDA", "B" }; // Slowing Down from Running
  if (*kidFrame == 57) return { ".", "DB", "B", "DA", "RDA", "LDA", "A", "LA", "RA" }; // Slowing Down from Running
  if (*kidFrame == 58) return { ".", "DB", "B", "DA", "RDA", "LDA", "A", "LA", "RA" }; // Slowing Down from Running
  if (*kidFrame == 59) return { ".", "DB", "DA", "LA", "RA", "B", "A" }; // Slowing Down from Running
  if (*kidFrame == 60) return { ".", "DB", "DA", "B", "RA", "LA", "A", "RDA", "LDA" }; // Slowing Down from Running
  if (*kidFrame == 61) return { "." }; // Slowing Down from Running
  if (*kidFrame == 62) return { "." }; // Slowing Down from Running
  if (*kidFrame == 63) return { "." }; // Slowing Down from Running
  if (*kidFrame == 64) return { "." }; // Slowing Down from Running
  if (*kidFrame == 65) return { ".", "A", "B", "LA", "RA", "DA", "LDA", "DB" }; // Running / Turning

  if (*kidFrame == 67) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 68) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 69) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 70) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 71) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 72) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 73) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 74) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 75) return { ".", "A", "RA", "LA", "RDA", "LDA", "DB" }; // Climbing/Getting Down
  if (*kidFrame == 76) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 77) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 78) return { ".", "U", "A", "UA" }; // Climbing/Getting Down
  if (*kidFrame == 79) return { ".", "A", "U" }; // Climbing/Getting down
  if (*kidFrame == 80) return { ".", "A", "U" }; // Climbing/Getting down

  if (*kidFrame == 83) return { ".", "L", "R" }; // Climbing/Getting Down Cancel
  if (*kidFrame == 84) return { ".", "L", "R" }; // Climbing/Getting Down Cancel
  if (*kidFrame == 85) return { ".", "L", "R" }; // Climbing/Getting Down Cancel

  if (*kidFrame == 87) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 88) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 89) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 90) return { ".", "L", "A", "U", "R" }; // Climbing/Hanging
  if (*kidFrame == 91) return { ".", "L", "A", "U", "R" }; // Climbing/Hanging
  if (*kidFrame == 92) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 93) return { ".", "L", "A", "U", "R" }; // Climbing/Hanging
  if (*kidFrame == 94) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 95) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 96) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 97) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 98) return { ".", "A", "U" }; // Climbing/Hanging
  if (*kidFrame == 99) return { ".", "A", "U" }; // Climbing/Hanging

  if (*kidFrame == 102) return { ".", "A" }; // Falling Animation
  if (*kidFrame == 103) return { ".", "A" }; // Falling Animation
  if (*kidFrame == 104) return { ".", "A" }; // Falling Animation
  if (*kidFrame == 105) return { ".", "A" }; // Falling Animation

  if (*kidFrame == 106) return { ".", "U", "A", "UA" }; // Clinging From Wall

  if (*kidFrame == 107) return { ".", "L", "R", "LA", "RA", "A", "LDA", "RDA", "DA" }; // Fall landing -- Can accelerate run
  if (*kidFrame == 108) return { ".", "L", "R" }; // Crouching/Falling
  if (*kidFrame == 109) return { ".", "D", "LD", "RD", "A", "DB", "RA", "LA", "RDA", "LDA" }; // Crouching/Falling
  if (*kidFrame == 110) return { ".", "L", "R" }; // Crouching/Falling
  if (*kidFrame == 111) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 112) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 113) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 114) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 115) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 116) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 117) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 118) return { ".", "L", "R" }; // Getting up
  if (*kidFrame == 119) return { ".", "L", "R" }; // Getting up

  if (*kidFrame == 121) return { "." }; // Careful Step
  if (*kidFrame == 122) return { ".", "A", "DB", "DA", "RDA", "LDA", "B", "RA", "LA" }; // Careful Step
  if (*kidFrame == 123) return { ".", "RDA", "A", "B", "RA", "LDA", "RDA", "LA", "DB" }; // Careful Step
  if (*kidFrame == 124) return { ".", "DA", "LA", "RA", "A", "DB", "RDA", "LDA" }; // Careful Step
  if (*kidFrame == 125) return { "." }; // Careful Step
  if (*kidFrame == 126) return { "." }; // Careful Step
  if (*kidFrame == 127) return { "." }; // Careful Step
  if (*kidFrame == 128) return { "." }; // Careful Step
  if (*kidFrame == 129) return { ".", "DA" }; // Careful Step
  if (*kidFrame == 130) return { "." }; // Careful Step
  if (*kidFrame == 131) return { "." }; // Careful Step

  if (*kidFrame == 135) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 136) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 137) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 138) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 139) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 140) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 141) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 142) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 143) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 144) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 145) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 146) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 147) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 148) return { "." }; // Climbing/Getting Down
  if (*kidFrame == 149) return { "." }; // Climbing/Getting Down

  if (*kidFrame == 150) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 151) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 152) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 153) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 154) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 155) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 156) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 157) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 158) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 159) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 160) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 161) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 162) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 163) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 164) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 165) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 170) return { ".", "A", "B", "BA", "L", "R" }; // Combat
  if (*kidFrame == 171) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 172) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 173) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 174) return { ".", "A", "B", "L", "R" }; // Combat
  if (*kidFrame == 216) return { ".", "A", "B", "BA", "L", "R" }; // Combat

 // if (*kidFrame == 255 || *kidFrame == 0)
 // {
 //  std::vector<std::string> allMoves;
 //  for (uint16_t i = 0; i < 256; i++) if (((uint8_t)i & 0b00001000) == 0) if (((uint8_t)i & 0b00000100) == 0)
 //    allMoves.push_back(EmuInstance::moveCodeToString((uint8_t)i));
 //  return allMoves;
 // }

 // std::vector<std::string> allMoves;
 // for (uint16_t i = 0; i < 256; i++) if (((uint8_t)i & 0b00001000) == 0) if (((uint8_t)i & 0b00000100) == 0)
 //   allMoves.push_back(EmuInstance::moveCodeToString((uint8_t)i));
 // return allMoves;

 // Nothing to do
 return { "." };
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;
 memset(&magnets, 0, sizeof(magnets));

 // If kid room and drawn room don't agree, no magnet effect
 if (*kidRoom != *drawnRoom) return magnets;

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
  {
    if (_rules[ruleId]->_magnets[*drawnRoom].kidHorizontalMagnet.active == true) magnets.kidHorizontalMagnet = _rules[ruleId]->_magnets[*drawnRoom].kidHorizontalMagnet;
    if (_rules[ruleId]->_magnets[*drawnRoom].kidVerticalMagnet.active == true) magnets.kidVerticalMagnet = _rules[ruleId]->_magnets[*drawnRoom].kidVerticalMagnet;
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

  // Getting magnet values for the kid
  auto magnets = getMagnetValues(rulesStatus);

  // Container for bounded value and difference with center
  float boundedValue = 0.0;
  float diff = 0.0;

  // Evaluating kid magnet's reward on position X
  boundedValue = (float)*kidPosX;
  boundedValue = std::min(boundedValue, magnets.kidHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.kidHorizontalMagnet.min);
  diff = -358.0f + std::abs(magnets.kidHorizontalMagnet.center - boundedValue);
  reward += magnets.kidHorizontalMagnet.intensity * -diff;

  // Evaluating kid magnet's reward on position Y
  boundedValue = (float)*kidPosY;
  boundedValue = std::min(boundedValue, magnets.kidVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.kidVerticalMagnet.min);
  diff = -255.0f + std::abs(magnets.kidVerticalMagnet.center - boundedValue);
  reward += magnets.kidVerticalMagnet.intensity * -diff;

  // Rewarding level skipping
  if (*currentLevel <= 12 && *isPaused == 2) reward += *currentLevel * 500000.0f;

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
  LOG("[Jaffar]  + Current Level:          %u\n", *currentLevel);
  LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
  LOG("[Jaffar]  + RNG State:              0x%X\n", *RNGState);
  LOG("[Jaffar]  + Global Timer:           %02u\n", *globalTimer);
  LOG("[Jaffar]  + Kid Frame:              %02u\n", *kidFrame);
  LOG("[Jaffar]  + Frame Phase:            %02u\n", *framePhase);
  LOG("[Jaffar]  + Is Paused:              %02u\n", *isPaused);
  LOG("[Jaffar]  + Screen Trans / Drawn:   %02u / %02u\n", *screenTransition, *screenDrawn);

  LOG("[Jaffar]  + Current Door State:     %02u\n", *currentDoorState);
  LOG("[Jaffar]  + Door Opening Timer:     %02u\n", *doorOpeningTimer);
  LOG("[Jaffar]  + Drawn Room:             %02u\n", *drawnRoom);
  LOG("[Jaffar]  + Exit Door:              %02u\n", *exitDoorState);

  LOG("[Jaffar]  + Kid Room:               %02u\n", *kidRoom);
  LOG("[Jaffar]  + Kid Pos X:              %04d\n", *kidPosX);
  LOG("[Jaffar]  + Kid Pos Y:              %02u\n", *kidPosY);
  LOG("[Jaffar]  + Kid Movement:           %02u\n", *kidMovement);
  LOG("[Jaffar]  + Kid Fall Wait:          %02u\n", *kidFallWait);
  LOG("[Jaffar]  + Kid HP:                 %02u\n", *kidHP);
  LOG("[Jaffar]  + Kid Fight Mode:         %02u\n", *kidFightMode);
  LOG("[Jaffar]  + Kid Jumping State:      %02u\n", *kidJumpingState);

  LOG("[Jaffar]  + Guard Pos X:            %02u\n", *guardPosX);
  LOG("[Jaffar]  + Guard HP:               %02u\n", *guardHP);
  LOG("[Jaffar]  + Guard Notice:           %02u\n", *guardNotice);
  LOG("[Jaffar]  + Guard Frame:            %02u\n", *guardFrame);
  LOG("[Jaffar]  + Guard Movement:         %02u\n", *guardMovement);
  LOG("[Jaffar]  + Guard Present:          %02u / %02u\n", *guardPresent, *guardDisappearMode);

  LOG("[Jaffar]  + Level %02u Specific Values: \n", *currentLevel);

  if (*currentLevel == 0)
  {
   LOG("[Jaffar]    + Room 19 Door Timer:  %02u\n", *lvl1Room19DoorTimer);
   LOG("[Jaffar]    + First Tile:          %02u / %02u\n", *lvl1FirstTileBG, *lvl1FirstTileFG);
  }

  if (*currentLevel == 1)
  {
   LOG("[Jaffar]    + Last Tile:       %02u\n", *lvl2LastTileFG);
   LOG("[Jaffar]    + Exit Door State: %02u\n", *lvl2ExitDoorState);
  }

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  LOG("[Jaffar]  + Kid Horizontal Magnet - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidHorizontalMagnet.intensity, magnets.kidHorizontalMagnet.center, magnets.kidHorizontalMagnet.min, magnets.kidHorizontalMagnet.max);
  LOG("[Jaffar]  + Kid Vertical Magnet   - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidVerticalMagnet.intensity, magnets.kidVerticalMagnet.center, magnets.kidVerticalMagnet.min, magnets.kidVerticalMagnet.max);
}
