#include "gameInstance.hpp"
#include "gameRule.hpp"
#define TILE_STATE_BASE 0x1E9E0

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
 // Setting emulator
 _emu = emu;

 gameTimer        = (uint16_t*)  &_emu->_baseMem[0x00002];
 gameFrame        = (uint8_t*)   &_emu->_baseMem[0x00000];
 isLagFrame       = (uint8_t*)   &_emu->_baseMem[0x0020C];
 inputCode        = (uint16_t*)  &_emu->_baseMem[0x00272];

 kidRoom          = (uint8_t*)   &_emu->_baseMem[0x00472];
 kidPosX          = (uint8_t*)   &_emu->_baseMem[0x00468];
 kidPosY          = (uint8_t*)   &_emu->_baseMem[0x00469];
 kidDirection     = (uint8_t*)   &_emu->_baseMem[0x0046A];
 kidHP            = (uint8_t*)   &_emu->_baseMem[0x00508];
 kidFrame         = (uint8_t*)   &_emu->_baseMem[0x00467];
 kidAction        = (uint8_t*)   &_emu->_baseMem[0x0046D];
 kidBuffered      = (uint8_t*)   &_emu->_baseMem[0x00512];
 kidCol           = (uint8_t*)   &_emu->_baseMem[0x0046B];
 kidRow           = (uint8_t*)   &_emu->_baseMem[0x0046C];
 kidHangingState  = (uint8_t*)   &_emu->_baseMem[0x00522];
 kidGrabState     = (uint8_t*)   &_emu->_baseMem[0x005B5];
 kidCrouchState   = (uint8_t*)   &_emu->_baseMem[0x00470];
 kidClimbingType1 = (uint8_t*)   &_emu->_baseMem[0x004CA];
 kidClimbingType2 = (uint8_t*)   &_emu->_baseMem[0x004CB];
 kidSequenceStep  = (uint8_t*)   &_emu->_baseMem[0x00595];
 exitDoorState    = (uint8_t*)   &_emu->_baseMem[0x0066A];

 tileStateBase    = (uint8_t*)   &_emu->_baseMem[TILE_STATE_BASE];
 kidPrevFrame     = (uint8_t*)   &_emu->_baseMem[0x1FF00];
 kidFrameDiff     = (int8_t*)    &_emu->_baseMem[0x1FF01];

 jingleState      = (uint8_t*)   &_emu->_baseMem[0x0215];
 exitJingleTimer  = (uint16_t*)  &_emu->_apuMem[0x0030];
 isPaused         = (uint8_t*)  &_emu->_baseMem[0x0455];
 menuMode         = (uint8_t*)  &_emu->_baseMem[0x001A];
 menuOption       = (uint8_t*)  &_emu->_baseMem[0x0855];
 exitLevelMode    = (uint8_t*)  &_emu->_baseMem[0x0D77];

 // Timer tolerance
 if (isDefined(config, "Timer Tolerance") == true)
  timerTolerance = config["Timer Tolerance"].get<uint8_t>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

 // Skip frames?
 if (isDefined(config, "Skip Frames") == true)
  skipFrames = config["Skip Frames"].get<bool>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Skip Frames' was not defined\n");

 // Skip frames?
 if (isDefined(config, "Exit Jingle Mode") == true)
  exitJingleMode = config["Exit Jingle Mode"].get<bool>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Exit Jingle Mode' was not defined\n");

 // Initialize derivative values
 updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  hash.Update(*gameFrame);
  hash.Update(*isLagFrame);

  if (timerTolerance > 0)
  {
   uint16_t actualTimer = ((uint16_t)*gameTimer * 255) + (uint16_t)*gameFrame;
   hash.Update(actualTimer % (timerTolerance+1));
  }

//  if (*isLagFrame != 15)
//  {
//   hash.Update(*gameTimer);
//   hash.Update( &_emu->_baseMem[0x0000], 0x100);
//  }

  if (*exitDoorState > 0) hash.Update(*gameTimer);

  // If kid is hurting, wait for him to recover
  if (*kidFrame == 109 && *kidCrouchState < 89) hash.Update(*kidCrouchState);

  // Kid Info:
  hash.Update(*kidRoom);
  hash.Update(*kidPosX);
  hash.Update(*kidPosY);
  hash.Update(*kidDirection);
  hash.Update(*kidHP);
  hash.Update(*kidFrame);
  hash.Update(*kidAction);
  hash.Update(*kidCol);
  hash.Update(*kidRow);
//    hash.Update(&_emu->_baseMem[0x0400], 0xC0);
  hash.Update(&_emu->_baseMem[0x04D0], 0x20);

  hash.Update(*kidGrabState);
  hash.Update(*kidHangingState);
  hash.Update(*kidClimbingType1);
  hash.Update(*kidClimbingType2);
  hash.Update(*kidSequenceStep);
  hash.Update(*kidPrevFrame);

  // Partially hashing all possible tile states
  for (size_t i = 0; i < 24*30; i++) hash.Update(tileStateBase[i] % 4);

  if (exitJingleMode == true)
  {
   hash.Update(*jingleState);
   hash.Update(*exitJingleTimer);
   hash.Update(*isPaused);
   hash.Update(*menuMode);
   hash.Update(*menuOption);
   hash.Update(*exitLevelMode);
   hash.Update(&_emu->_baseMem[0x0010], 0x04);
   hash.Update(&_emu->_baseMem[0x0530], 0x70);
   hash.Update(&_emu->_baseMem[0x0D77], 0x10);
   hash.Update(*inputCode);
  }

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
  *kidFrameDiff = *kidFrame - *kidPrevFrame;
  *kidPrevFrame = *kidFrame;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (exitJingleMode)
 {
  if (*exitLevelMode > 26) return { ".", "s", "A", "B", "U", "D" };
  else return {"."};
 }

// if (*gameFrame < 3) return moveList;

 if (*kidFrame == 0x0001) moveList.insert(moveList.end(), { "B" });
 if (*kidFrame == 0x0002) moveList.insert(moveList.end(), { "B" });
 if (*kidFrame == 0x0003) moveList.insert(moveList.end(), { "B", "R", "L", "DR", "DL" });
 if (*kidFrame == 0x0004) moveList.insert(moveList.end(), { "R", "L", "DR", "DL" });
 if (*kidFrame == 0x0005) moveList.insert(moveList.end(), { "R", "L", "DR", "DL" });
 if (*kidFrame == 0x0006) moveList.insert(moveList.end(), { "R", "L", "DR", "DL" });
 if (*kidFrame == 0x0007) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL" });
 if (*kidFrame == 0x0008) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL" });
 if (*kidFrame == 0x0009) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL", "RA", "UDRA", "LA", "UDLA" });
 if (*kidFrame == 0x000A) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL" });
 if (*kidFrame == 0x000B) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL" });
 if (*kidFrame == 0x000C) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL" });
 if (*kidFrame == 0x000D) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL" });
 if (*kidFrame == 0x000E) moveList.insert(moveList.end(), { "R", "L", "DR", "DL", "UR", "UL" });
 if (*kidFrame == 0x000F) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "BA", "RA", "LA", "UA", "UR", "UL" });
 if (*kidFrame == 0x0022) moveList.insert(moveList.end(), { "R", "L" });
 if (*kidFrame == 0x0030) moveList.insert(moveList.end(), { "R", "L" });
 if (*kidFrame == 0x0032) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "UR", "UL", "RBA", "LBA" });
 if (*kidFrame == 0x0033) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "LA", "UR", "UL", "RBA", "LBA" });
 if (*kidFrame == 0x0034) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "UR", "UL", "RBA", "LBA" });
 if (*kidFrame == 0x0036) moveList.insert(moveList.end(), { "R", "L" });
 if (*kidFrame == 0x0038) moveList.insert(moveList.end(), { "R", "L", "UR", "UL", "LBA", "RBA" });
 if (*kidFrame == 0x0043) moveList.insert(moveList.end(), { "R", "L" });
 if (*kidFrame == 0x0044) moveList.insert(moveList.end(), { "R", "L" });
 if (*kidFrame == 0x0045) moveList.insert(moveList.end(), { "R", "L" });
 if (*kidFrame == 0x0051) moveList.insert(moveList.end(), { "B" });
 if (*kidFrame == 0x0057) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x0058) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x0059) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x005A) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x005B) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x005C) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x005D) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x005E) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x005F) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x0060) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x0061) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x0062) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x0063) moveList.insert(moveList.end(), { "B", "U" });
 if (*kidFrame == 0x0066) moveList.insert(moveList.end(), { "ULB", "URB" });
 if (*kidFrame == 0x0069) moveList.insert(moveList.end(), { "B" });
 if (*kidFrame == 0x006A) moveList.insert(moveList.end(), { "B" });
 if (*kidFrame == 0x006B) moveList.insert(moveList.end(), { "R", "L", "UBA" });
 if (*kidFrame == 0x006C) moveList.insert(moveList.end(), { "DR", "DL" });
 if (*kidFrame == 0x006D) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "DA", "DR", "DL", "DRA", "DLA" });
 if (*kidFrame == 0x006F) moveList.insert(moveList.end(), { "R", "L", "D", "U" });
 if (*kidFrame == 0x0076) moveList.insert(moveList.end(), { "R", "L" });
 if (*kidFrame == 0x0077) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "LA" });


 return moveList;
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

   while (*gameFrame != 1 || *isLagFrame != 15)
   {
    INPUT_TYPE newMove = *gameFrame == 3 || *gameFrame == 4 ? move : 0;
    _emu->advanceState(newMove);
    moves.push_back(newMove);
    skippedFrames++;
    if (skippedFrames > 64) EXIT_WITH_ERROR("Exceeded skip frames\n");
   }
  }

  return moves;
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;
 memset(&magnets, 0, sizeof(magnets));

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
  {
    if (_rules[ruleId]->_magnets[*kidRoom].kidHorizontalMagnet.active == true) magnets.kidHorizontalMagnet = _rules[ruleId]->_magnets[*kidRoom].kidHorizontalMagnet;
    if (_rules[ruleId]->_magnets[*kidRoom].kidVerticalMagnet.active == true) magnets.kidVerticalMagnet = _rules[ruleId]->_magnets[*kidRoom].kidVerticalMagnet;
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
 diff = -255.0f + std::abs(magnets.kidHorizontalMagnet.center - boundedValue);
 reward += magnets.kidHorizontalMagnet.intensity * -diff;

 // Evaluating kid magnet's reward on position Y
 boundedValue = (float)*kidPosY;

 // If climbing down, add pos y
 if (*kidFrame >= 0x8D && *kidFrame <= 0x94 && *kidFrameDiff < 0) boundedValue += 15.0f - (0x94 - *kidFrame);

 // If jumpclimb up, subtract pos y
 if (*kidFrame >= 0x43 && *kidFrame <= 0x4F) boundedValue -= 16.0f - (0x4F - *kidFrame);

 // If hanging, subtract pos y
 if (*kidFrame == 0x50) boundedValue -= 20.0f;
 if (*kidFrame >= 0x57 && *kidFrame <= 0x5B) boundedValue -= 25.0f - (0x5B - *kidFrame);

 // Climbing up
 if (*kidFrame >= 0x87 && *kidFrame < 0x8D) boundedValue -= 32.0f + 7.0f - (0x8D - *kidFrame);

 boundedValue = std::min(boundedValue, magnets.kidVerticalMagnet.max);
 boundedValue = std::max(boundedValue, magnets.kidVerticalMagnet.min);
 diff = -255.0f + std::abs(magnets.kidVerticalMagnet.center - boundedValue);

 reward += magnets.kidVerticalMagnet.intensity * -diff;

 // Returning reward
 return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:             %05u (Tolerance: %02u)\n", *gameTimer, timerTolerance);
 LOG("[Jaffar]  + Game Frame:             %02u\n", *gameFrame);
 LOG("[Jaffar]  + Is Lag Frame:           %02u\n", *isLagFrame);
 LOG("[Jaffar]  + Input Code:             %05u\n", *inputCode);

 LOG("[Jaffar]  + Kid Room:               %02u\n", *kidRoom);
 LOG("[Jaffar]  + Kid Pos X:              %02u\n", *kidPosX);
 LOG("[Jaffar]  + Kid Pos Y:              %02u\n", *kidPosY);
 LOG("[Jaffar]  + Kid Direction:          %02u\n", *kidDirection);
 LOG("[Jaffar]  + Kid HP:                 %02u\n", *kidHP);
 LOG("[Jaffar]  + Kid Frame:              0x%02X (Prev: 0x%02X, Diff: %02d)\n", *kidFrame, *kidPrevFrame, *kidFrameDiff);
 LOG("[Jaffar]  + Kid Sequence Step:      0x%02X\n", *kidSequenceStep);
 LOG("[Jaffar]  + Kid Action (Buffered):  %02u (%02u)\n", *kidAction, *kidBuffered);
 LOG("[Jaffar]  + Kid Col:                %02u\n", *kidCol);
 LOG("[Jaffar]  + Kid Row:                %02u\n", *kidRow);
 LOG("[Jaffar]  + Kid Crouch State:       %02u\n", *kidCrouchState);
 LOG("[Jaffar]  + Kid Grab State:         %02u\n", *kidGrabState);
 LOG("[Jaffar]  + Kid Hanging State:      %02u\n", *kidHangingState);
 LOG("[Jaffar]  + Kid Climbing Type:      %02u / %02u\n", *kidClimbingType1, *kidClimbingType2);
 LOG("[Jaffar]  + Exit Door State:        %02u\n", *exitDoorState);

 if (exitJingleMode == true)
 {
  LOG("[Jaffar]  + Jingle State / Timer:   %02u (%05u)\n", *jingleState, *exitJingleTimer);
  LOG("[Jaffar]  + Is Paused:              %02u\n", *isPaused);
  LOG("[Jaffar]  + Menu Mode / Option:     %02u %02u\n", *menuMode, *menuOption);
  LOG("[Jaffar]  + Exit Level Mode:        %02u\n", *exitLevelMode);
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 LOG("[Jaffar]  + Kid Horizontal Magnet - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidHorizontalMagnet.intensity, magnets.kidHorizontalMagnet.center, magnets.kidHorizontalMagnet.min, magnets.kidHorizontalMagnet.max);
 LOG("[Jaffar]  + Kid Vertical Magnet   - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidVerticalMagnet.intensity, magnets.kidVerticalMagnet.center, magnets.kidVerticalMagnet.min, magnets.kidVerticalMagnet.max);

 for (const auto& tile : tileWatchList)
 LOG("[Jaffar]  + Tile State            - Room: %02u, Row: %02u, Col: %02u, Index: %03u, Mem: 0x%05X, Value: %03u\n", tile.room, tile.row, tile.col, tile.index, TILE_STATE_BASE + tile.index, tileStateBase[tile.index]);
}

