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
  bottomTextTimer        = (uint8_t*)   &_emu->_baseMem[0x06E5];
  gameState              = (uint8_t*)   &_emu->_baseMem[0x001C];
  passwordTimer          = (uint8_t*)   &_emu->_baseMem[0x007D];

  kidPosX                = (int16_t*)   &_emu->_baseMem[0x060F];
  kidPosY                = (uint8_t*)   &_emu->_baseMem[0x0611];
  kidDirection           = (uint8_t*)   &_emu->_baseMem[0x0615];
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
  screenTransition2      = (uint8_t*)   &_emu->_baseMem[0x01E0];
  exitDoorState          = (uint8_t*)   &_emu->_baseMem[0x0400];

  screenDrawn            = (uint8_t*)   &_emu->_baseMem[0x0732];
  isPaused               = (uint8_t*)   &_emu->_baseMem[0x06D1];
  bufferedCommand        = (uint16_t*)  &_emu->_baseMem[0x04C9];
  doorOpeningTimer       = (uint8_t*)   &_emu->_baseMem[0x04CD];
  currentDoorState       = (uint8_t*)   &_emu->_baseMem[0x073A];

  guardPresent           = (uint8_t*)   &_emu->_baseMem[0x06E0];
  guardDisappearMode     = (uint8_t*)   &_emu->_baseMem[0x04F8];

  rawFrameCount          = (uint16_t*)   &_emu->_highMem[0x1000];
  lagFrameCounter        = (uint16_t*)   &_emu->_highMem[0x1002];

  // Level-specific tiles
  lvl1FirstTileBG          = (uint8_t*)   &_emu->_baseMem[0x06A4];
  lvl1FirstTileFG          = (uint8_t*)   &_emu->_baseMem[0x06B0];
  lvl1Room19DoorTimer      = (uint8_t*)   &_emu->_baseMem[0x05E9];
  lvl2LastTileFG           = (uint8_t*)   &_emu->_baseMem[0x0665];
  lvl2ExitDoorState        = (uint8_t*)   &_emu->_baseMem[0x0708];
  lvl3PreCheckpointGateTimer = (uint8_t*) &_emu->_baseMem[0x05E9];
  lvl3ExitDoorState        = (uint8_t*)   &_emu->_baseMem[0x0400];
  lvl3SkeletonLooseTile    = (uint8_t*)   &_emu->_baseMem[0x0502];
  lvl4ExitDoorState        = (uint8_t*)   &_emu->_baseMem[0x06F7];
  lvl5GateTimer            = (uint8_t*)   &_emu->_baseMem[0x0538];
  lvl7SlowFallPotionState  = (uint8_t*)   &_emu->_baseMem[0x0708];
  lvl9Room15DoorState      = (uint8_t*)   &_emu->_baseMem[0x05E9];
  lvl10Room0DoorState      = (uint8_t*)   &_emu->_baseMem[0x04B8];
  lvl10Room4DoorState      = (uint8_t*)   &_emu->_baseMem[0x0541];

  if (isDefined(config, "Hash Includes") == true)
   for (const auto& entry : config["Hash Includes"])
    hashIncludes.insert(entry.get<std::string>());
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Hash Includes' was not defined\n");

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Skip frames?
  if (isDefined(config, "Skip Frames") == true)
   skipFrames = config["Skip Frames"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Skip Frames' was not defined\n");

  // Enable Pause
  if (isDefined(config, "Enable Pause") == true)
   enablePause = config["Enable Pause"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Enable Pause' was not defined\n");

  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0) hash.Update(*globalTimer % (timerTolerance+1));

  hash.Update(*currentLevel);
  hash.Update(*framePhase);
  hash.Update(*doorOpeningTimer);
  hash.Update(*currentDoorState);
  hash.Update(*exitDoorState);
  hash.Update(*bottomTextTimer);
  hash.Update(*bufferedCommand);
  hash.Update(*gameState);

  hash.Update(*kidPosX);
  hash.Update(*kidPosY);
  hash.Update(*kidFrame);
  hash.Update(*kidDirection);
  hash.Update(*kidMovement);
  hash.Update(*kidFallWait);
  hash.Update(*kidHP);
  hash.Update(*kidRoom);
  hash.Update(*kidFightMode);
  hash.Update(*kidJumpingState);

  hash.Update(*guardPosX);
  hash.Update(*guardHP);
  hash.Update(*guardNotice);
  hash.Update(*guardFrame);
  hash.Update(*guardMovement);
  hash.Update(*drawnRoom);
  hash.Update(*screenTransition);
  hash.Update(*screenTransition2);
  hash.Update(*screenDrawn);
  hash.Update(*isPaused);

  hash.Update(*guardPresent);
  hash.Update(*guardDisappearMode);
  hash.Update(*lvl4ExitDoorState);

  hash.Update(screenTransition2, 0x20);

  if (*gameState == 8) hash.Update(*passwordTimer);
  if (*gameState == 8 && *passwordTimer == 0) hash.Update(*globalTimer);
  if (*gameState == 1 && *kidFrame == 0 && *kidMovement == 91) hash.Update(*globalTimer);

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

  if (*currentLevel == 2)
  {
   hash.Update(*lvl3PreCheckpointGateTimer);
   hash.Update(*lvl3ExitDoorState);
   hash.Update(*lvl3SkeletonLooseTile);
  }

  if (*currentLevel == 4)
  {
   hash.Update(*lvl5GateTimer);
  }

  if (*currentLevel == 6)
  {
   hash.Update(*lvl7SlowFallPotionState);
  }

  if (*currentLevel == 7)
  {
   hash.Update(*lvl9Room15DoorState);
  }

  if (*currentLevel == 9)
  {
   hash.Update(*lvl10Room0DoorState);
   hash.Update(*lvl10Room4DoorState);
  }

  // Keypad input: (0x0040:0x40A), [0x0000:0x0037), 0x003A,
  // Timer: 0x0791
//  hash.Update( &_emu->_baseMem[0x0000], 0x0036);
//  hash.Update( &_emu->_baseMem[0x0042], 0x03C7);
//  hash.Update( &_emu->_baseMem[0x040C], 0x0384);
//  hash.Update( &_emu->_baseMem[0x0793], 0x006D);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 isCorrectRender = 1;
 if (_emu->_nes->emu.ppu.isCorrectRender == false) isCorrectRender = 0;
 if (_emu->_nes->emu.isCorrectExecution == false) isCorrectRender = 0;

 framesPerState = 4;
 if ( (*guardPresent > 0) && (*guardDisappearMode == 0) ) framesPerState = 5;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
  size_t skippedFrames = 0;
  std::vector<INPUT_TYPE> moves;

  if (skipFrames == false)
  {
   _emu->advanceState(move);
   moves.push_back(move);
  }

  if (skipFrames == true)
  {
   _emu->advanceState(0);
   moves.push_back(0);

   while (*framePhase != 1 || *isPaused != 2 || *screenTransition == 255)
   {
    INPUT_TYPE newMove = *framePhase == 2 || *framePhase == 3 ? move : 0;
    if (move == 0b00010000 && *framePhase == 4) newMove = move;
    _emu->advanceState(newMove);
    moves.push_back(newMove);
    skippedFrames++;
    if (skippedFrames > 128) break;
    if (_emu->_nes->emu.isCorrectExecution == false) { *kidHP = 0; break; }
   }
  }

  *rawFrameCount += moves.size();
  *lagFrameCounter += moves.size() - 4;
  updateDerivedValues();

  return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList({"."});

 if (*kidFrame == 0x0001) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0002) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0003) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0004) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0005) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0006) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x0007) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x0008) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x0009) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x000A) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x000B) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x000C) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x000D) moveList.insert(moveList.end(), { "L", "R", "LA", "RA" });
 if (*kidFrame == 0x000E) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x000F) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "DB", "LB", "RB", "UB", "UD", "UBA", "UDB", "UDBA" });
 if (*kidFrame == 0x0014) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x001A) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0023) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0025) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0026) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0027) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0028) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0029) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x002A) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x002B) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x002C) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0035) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0036) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0037) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0039) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x004E) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x004F) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0050) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0053) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0054) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0055) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0057) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0058) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0059) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x005A) moveList.insert(moveList.end(), { "A", "L", "R", "U" });
 if (*kidFrame == 0x005B) moveList.insert(moveList.end(), { "A", "L", "R", "U" });
 if (*kidFrame == 0x005C) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x005D) moveList.insert(moveList.end(), { "A", "L", "R", "U" });
 if (*kidFrame == 0x005E) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x005F) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0060) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0061) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0062) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0063) moveList.insert(moveList.end(), { "A", "U" });
 if (*kidFrame == 0x0066) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x0067) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x0068) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x0069) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x006A) moveList.insert(moveList.end(), { "A", "B", "L", "R" });
 if (*kidFrame == 0x006B) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x006C) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x006D) moveList.insert(moveList.end(), { "D", "L", "R", "DL", "DR" });
 if (*kidFrame == 0x006E) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x006F) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0070) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0071) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0072) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0073) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0074) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0075) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0076) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x0077) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x007D) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0096) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0097) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0098) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x0099) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x009A) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x009B) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x009C) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x009D) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x009E) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x00A0) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x00A4) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x00A5) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x00AA) moveList.insert(moveList.end(), { "A", "L", "R", "BA" });
 if (*kidFrame == 0x00AB) moveList.insert(moveList.end(), { "A", "B", "L", "R" });
 if (*kidFrame == 0x00AC) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x00AD) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x00AE) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x00D8) moveList.insert(moveList.end(), { "A", "L", "R" });

 // Nothing to do
 return moveList;
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

void GameInstance::printFullMoveList()
{
 for (uint16_t i = 0; i <= 0xFF; i++)
 {
  *kidFrame = (uint8_t)i;
  auto moves = getPossibleMoves();
  if (moves.size() == 1) continue;

  size_t vecSize = moves.size();
  for (size_t j = 0; j < vecSize; j++)
  {
   INPUT_TYPE code = EmuInstance::moveStringToCode(moves[j]);
   if ( ((code & 0b01000000) > 0) && ((code & 0b10000000) == 0)) moves.push_back(simplifyMove(EmuInstance::moveCodeToString((code & ~0b01000000) | 0b10000000)));
   if ( ((code & 0b10000000) > 0) && ((code & 0b01000000) == 0)) moves.push_back(simplifyMove(EmuInstance::moveCodeToString((code & ~0b10000000) | 0b01000000)));
  }

  std::set<std::string> moveSet( moves.begin(), moves.end() );
  moves.assign( moveSet.begin(), moveSet.end() );
  std::sort(moves.begin(), moves.end(), moveCountComparerString);

  printf("if (*%s == 0x%04X) moveList.insert(moveList.end(), { \"%s\"", "kidFrame", *kidFrame, moves[1].c_str());
  for (size_t j = 2; j < moves.size(); j++)
   printf(", \"%s\"", moves[j].c_str());
  printf(" });\n");
 }

 exit(0);
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
  if (*currentLevel == 12 && *isPaused == 2) reward += *currentLevel * 5000000.0f;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
 *this->globalTimer = (uint8_t) RNGState;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Current Level:          %u\n", *currentLevel);
  LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
  LOG("[Jaffar]  + RNG State:              0x%X\n", *RNGState);
  LOG("[Jaffar]  + Global Timer:           %02u\n", *globalTimer);
  LOG("[Jaffar]  + Kid Frame / Direction:  %02u %02u\n", *kidFrame, *kidDirection);
  LOG("[Jaffar]  + Frame Phase:            %02u / %02u\n", *framePhase, framesPerState);
  LOG("[Jaffar]  + Is Paused:              %02u\n", *isPaused);
  LOG("[Jaffar]  + Screen Trans / Drawn:   %02u / %02u\n", *screenTransition, *screenDrawn);
  LOG("[Jaffar]  + Bottom / Pass Timer:    %02u, %02u\n", *bottomTextTimer, *passwordTimer);
  LOG("[Jaffar]  + Game State:             %02u\n", *gameState);

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

  if (*currentLevel == 2)
  {
   LOG("[Jaffar]    + Pre Checkpoint Gate Timer:  %02u\n", *lvl3PreCheckpointGateTimer);
   LOG("[Jaffar]    + Exit Door State: %02u\n", *lvl3ExitDoorState);
   LOG("[Jaffar]    + Skeleton Loose Tile: %02u\n", *lvl3SkeletonLooseTile);
  }

  if (*currentLevel == 4)
  {
   LOG("[Jaffar]    + Gate Timer:  %02u\n", *lvl5GateTimer);
  }

  if (*currentLevel == 6)
  {
    LOG("[Jaffar]    + Slowfall Potion State:  %02u\n", *lvl7SlowFallPotionState);
  }

  if (*currentLevel == 8)
  {
    LOG("[Jaffar]    + Room 15 Door State:  %02u\n", *lvl9Room15DoorState);
  }

  if (*currentLevel == 9)
  {
    LOG("[Jaffar]    + Room 0 Door State:  %02u\n", *lvl10Room0DoorState);
    LOG("[Jaffar]    + Room 4 Door State:  %02u\n", *lvl10Room4DoorState);
  }


  LOG("[Jaffar]    + Exit Door State: %02u\n", *lvl4ExitDoorState);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.kidHorizontalMagnet.intensity) > 0.0f) LOG("[Jaffar]  + Kid Horizontal Magnet - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidHorizontalMagnet.intensity, magnets.kidHorizontalMagnet.center, magnets.kidHorizontalMagnet.min, magnets.kidHorizontalMagnet.max);
  if (std::abs(magnets.kidVerticalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Kid Vertical Magnet   - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidVerticalMagnet.intensity, magnets.kidVerticalMagnet.center, magnets.kidVerticalMagnet.min, magnets.kidVerticalMagnet.max);
}
