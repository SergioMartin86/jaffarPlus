#include "gameInstance.hpp"
#include "gameRule.hpp"
#define TILE_STATE_BASE 0x1E9E0
#define TILE_TYPE_BASE 0x1E170

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
 // Setting emulator
 _emu = emu;

 gameTimer        = (uint16_t*)  &_emu->_baseMem[0x00002];
 IGTTicks         = (uint16_t*)  &_emu->_baseMem[0x0052D];
 gameFrame        = (uint8_t*)   &_emu->_baseMem[0x00000];
 isLagFrame       = (uint8_t*)   &_emu->_baseMem[0x0020C];
 inputCode        = (uint16_t*)  &_emu->_baseMem[0x00272];
 soundEffectActive = (uint8_t*)   &_emu->_baseMem[0x01D3EB];

 kidRoom          = (uint8_t*)   &_emu->_baseMem[0x00472];
 kidFightMode     = (uint8_t*)   &_emu->_baseMem[0x00475];
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
 kidTeleporting   = (uint8_t*)   &_emu->_baseMem[0x00474];
 kidSequenceStep  = (uint8_t*)   &_emu->_baseMem[0x00595];
 exitDoorState    = (uint8_t*)   &_emu->_baseMem[0x0066A];
 bossSequence     = (uint8_t*)   &_emu->_baseMem[0x009EE];

 guardRoom        = (uint8_t*)   &_emu->_baseMem[0x00472];
 guardPosX        = (uint8_t*)   &_emu->_baseMem[0x00478];
 guardPosY        = (uint8_t*)   &_emu->_baseMem[0x00479];
 guardDirection   = (uint8_t*)   &_emu->_baseMem[0x0047A];
 guardHP          = (uint8_t*)   &_emu->_baseMem[0x0050B];
 guardMaxHP       = (uint8_t*)   &_emu->_baseMem[0x0050D];
 guardFrame       = (uint8_t*)   &_emu->_baseMem[0x00477];

 tileStateBase    = (uint8_t*)   &_emu->_baseMem[TILE_STATE_BASE];
 tileTypeBase     = (uint8_t*)   &_emu->_baseMem[TILE_TYPE_BASE];
 kidPrevFrame     = (uint8_t*)   &_emu->_baseMem[0x1FF00];
 kidFrameDiff     = (int8_t*)    &_emu->_baseMem[0x1FF01];
 kidPrevRoom      = (uint8_t*)    &_emu->_baseMem[0x1FF02];
 customValue      = (uint8_t*)    &_emu->_baseMem[0x1FF03];
 rawFrameCount    = (uint16_t*)  &_emu->_baseMem[0x1FF50];
 lagFrameCounter  = (uint16_t*)  &_emu->_baseMem[0x1FF52];

 jingleState      = (uint8_t*)   &_emu->_baseMem[0x0215];
 exitJingleTimer  = (uint16_t*)  &_emu->_apuMem[0x0030];
 isPaused         = (uint8_t*)  &_emu->_baseMem[0x0455];
 menuMode         = (uint8_t*)  &_emu->_baseMem[0x001A];
 menuOption       = (uint8_t*)  &_emu->_baseMem[0x0855];
 exitLevelMode    = (uint8_t*)  &_emu->_baseMem[0x0541];

 // Timer tolerance
 if (isDefined(config, "Timer Tolerance") == true)
  timerTolerance = config["Timer Tolerance"].get<uint8_t>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

 // Skip frames?
 if (isDefined(config, "Skip Frames") == true)
  skipFrames = config["Skip Frames"].get<bool>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Skip Frames' was not defined\n");

 // Exit jingle mode
 if (isDefined(config, "Exit Jingle Mode") == true)
  exitJingleMode = config["Exit Jingle Mode"].get<bool>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Exit Jingle Mode' was not defined\n");

 // Discount for lag frames
 if (isDefined(config, "Lag Discount") == true)
  lagDiscount = config["Lag Discount"].get<float>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Lag Discount' was not defined\n");

 // Discount for Fight Mode
 if (isDefined(config, "Fight Mode Discount") == true)
  fightModeDiscount = config["Fight Mode Discount"].get<float>();
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Fight Mode Discount' was not defined\n");

 // Tile Hash Map
 if (isDefined(config, "Tile Hashes") == true)
 {
  for (const auto& hash : config["Tile Hashes"])
  {
   int room = -1;
   if (isDefined(hash, "Room") == true)
   {
    if (hash["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Tile Hash room must be an integer.\n");
    room = hash["Room"].get<int>();
   }

   int row = -1;
   if (isDefined(hash, "Row") == true)
   {
    if (hash["Row"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Tile Hash row must be an integer.\n");
    row = hash["Row"].get<int>();
   }

   int col = -1;
   if (isDefined(hash, "Col") == true)
   {
    if (hash["Col"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Tile Hash Col must be an integer.\n");
    col = hash["Col"].get<int>();
   }

   std::string type;
   if (isDefined(hash, "Type") == true)
   {
    if (hash["Type"].is_string() == false) EXIT_WITH_ERROR("[ERROR] Tile Hash Type must be a string.\n");
    type = hash["Type"].get<std::string>();
   }

   int index = room * 30 + row * 10 + col;

   bool recognizedType = false;
   if (type == "Modulo") { tileModuloHashList[index] = tileWatch_t {.room = room, .row = row, .col = col, .index = index}; recognizedType = true; }
   if (type == "Boolean") {  tileBooleanHashList[index] = tileWatch_t {.room = room, .row = row, .col = col, .index = index}; recognizedType = true; }
   if (type == "Full") { tileFullHashList[index] = tileWatch_t {.room = room, .row = row, .col = col, .index = index}; recognizedType = true; }
   if (type == "Type") { tileTypeHashList[index] = tileWatch_t {.room = room, .row = row, .col = col, .index = index}; recognizedType = true; }
   if (recognizedType == false) EXIT_WITH_ERROR("[ERROR] Unrecognized Hash Type '%s'.\n", type.c_str());
  }

  tileWatchList.insert(tileModuloHashList.begin(), tileModuloHashList.end());
  tileWatchList.insert(tileBooleanHashList.begin(), tileBooleanHashList.end());
  tileWatchList.insert(tileFullHashList.begin(), tileFullHashList.end());
  tileWatchList.insert(tileTypeHashList.begin(), tileTypeHashList.end());
 }
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Tile Hashes' was not defined\n");


 // Tile Hash Map
 if (isDefined(config, "Game Property Hashes") == true)
 {
  for (const auto& hash : config["Game Property Hashes"])
  {
   if (hash.is_string() == false) EXIT_WITH_ERROR("[ERROR] Property Hash Type must be a string.\n");
   std::string property = hash.get<std::string>();

   bool recognized = false;
   if (property == "Kid HP") recognized = true;
   if (property == "Guard HP") recognized = true;
   if (property == "Boss Sequence") recognized = true;
   if (recognized == false) EXIT_WITH_ERROR("[ERROR] Unrecognized Game Property '%s'.\n", property.c_str());
   gamePropertyHashList.insert(property);
  }
 }
 else EXIT_WITH_ERROR("[Error] Game Configuration 'Game Property Hashes' was not defined\n");


 // Resetting frame count
 *rawFrameCount = 0;

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
  hash.Update(*soundEffectActive);

  if (timerTolerance > 0)
  {
   uint16_t actualTimer = ((uint16_t)*gameTimer * 255) + (uint16_t)*gameFrame;
   hash.Update(actualTimer % (timerTolerance+1));
  }

  if (*isLagFrame != 15)
  {
   hash.Update(*gameTimer);
   hash.Update( &_emu->_baseMem[0x0000], 0x100);
  }

  if (*exitDoorState > 0) hash.Update(*gameTimer);

  // If kid is hurting, wait for him to recover
  if (*kidFrame == 109 && *kidCrouchState < 89) hash.Update(*kidCrouchState);

  // If not skipping non-playable frames, hash the buffered input
  if (skipFrames == false) hash.Update(*inputCode);

  // Kid Info:
  hash.Update(*kidRoom);
  hash.Update(*kidFightMode);
  hash.Update(*kidPosX);
  hash.Update(*kidPosY);
  hash.Update(*kidDirection);
  if (gamePropertyHashList.contains("Kid HP")) hash.Update(*kidHP);
  if (gamePropertyHashList.contains("Boss Sequence")) hash.Update(*bossSequence);
  hash.Update(*kidFrame);
  hash.Update(*kidAction);
  hash.Update(*kidCol);
  hash.Update(*kidRow);
  hash.Update(&_emu->_baseMem[0x0467], 0x20);
  hash.Update(&_emu->_baseMem[0x04D0], 0x20);

  // Guard Info
  hash.Update(*guardRoom);
  hash.Update(*guardPosX);
  hash.Update(*guardPosY);
  hash.Update(*guardDirection);
  if (gamePropertyHashList.contains("Guard HP")) hash.Update(*guardHP);
  hash.Update(*guardFrame);

  hash.Update(*kidGrabState);
  hash.Update(*kidHangingState);
  hash.Update(*kidClimbingType1);
  hash.Update(*kidClimbingType2);
  hash.Update(*kidTeleporting);
  hash.Update(*kidSequenceStep);
  if (skipFrames == false) hash.Update(*kidPrevFrame);

  // User-defined hashes
  for (const auto& tile : tileModuloHashList) hash.Update(tileStateBase[tile.second.index] % timerTolerance);
  for (const auto& tile : tileBooleanHashList) hash.Update(tileStateBase[tile.second.index] > 0);
  for (const auto& tile : tileFullHashList) hash.Update(tileStateBase[tile.second.index]);
  for (const auto& tile : tileTypeHashList) hash.Update(tileTypeBase[tile.second.index]);

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

  hash.Update(*customValue);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
  *kidFrameDiff = *kidFrame - *kidPrevFrame;
  *kidPrevFrame = *kidFrame;

  kidPosYActual = (float)*kidPosY;

  // If climbing down, add pos y. Otherwise subtract
  if (*kidFrame >= 0x8D && *kidFrame <= 0x94 && *kidFrameDiff < 0) kidPosYActual += (0x94 - *kidFrame);
  if (*kidFrame >= 0x8D && *kidFrame <= 0x94 && *kidFrameDiff > 0) kidPosYActual += 7.0f - (*kidFrame - 0x8D);

  // If jumpclimb up, subtract pos y
  if (*kidFrame >= 0x43 && *kidFrame <= 0x4F) kidPosYActual -= 16.0f - (0x4F - *kidFrame);

  // If hanging, subtract pos y
  if (*kidFrame == 0x50) kidPosYActual -= 20.0f;
  if (*kidFrame >= 0x57 && *kidFrame <= 0x5B) kidPosYActual -= 25.0f - (0x5B - *kidFrame);

  // Climbing up
  if (*kidFrame >= 0x87 && *kidFrame < 0x8D) kidPosYActual -= 32.0f + 7.0f - (0x8D - *kidFrame);

  *kidPrevRoom = *kidRoom;
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

 if (skipFrames == false) if (*gameFrame < 3 || *isLagFrame != 15) return moveList;

 if (*kidFrame == 0x0001) moveList.insert(moveList.end(), { "B", "L", "R", "U", "BA", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0002) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "DA", "DL", "DR", "LA", "LB", "RA", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x0003) moveList.insert(moveList.end(), { "B", "D", "L", "R", "DL", "DR", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x0004) moveList.insert(moveList.end(), { "B", "D", "L", "R", "U", "DL", "DR", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x0005) moveList.insert(moveList.end(), { "D", "L", "R", "U", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0006) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0007) moveList.insert(moveList.end(), { "A", "D", "L", "R", "U", "DB", "DL", "DR", "LA", "LB", "RA", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x0008) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0009) moveList.insert(moveList.end(), { "D", "L", "R", "U", "DL", "DR", "LA", "LB", "RA", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x000A) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x000B) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x000C) moveList.insert(moveList.end(), { "L", "R", "U", "DL", "DR", "UL", "UR" });
 if (*kidFrame == 0x000D) moveList.insert(moveList.end(), { "D", "L", "R", "DB", "DL", "DR", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x000E) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "UL", "UR" });
 if (*kidFrame == 0x000F) moveList.insert(moveList.end(), { "A", "B", "D", "U", "L", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LR", "LB", "LA", "DR", "DL", "DB", "DA", "BA" });
 if (*kidFrame == 0x0010) moveList.insert(moveList.end(), { "D", "L", "R", "DL", "DR", "LB", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x0011) moveList.insert(moveList.end(), { "D", "L", "R", "U", "DL", "DR", "LB", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x0012) moveList.insert(moveList.end(), { "D", "L", "R", "DL", "DR", "LB", "RB", "UA" });
 if (*kidFrame == 0x0013) moveList.insert(moveList.end(), { "B", "D", "L", "R", "LA", "LB", "RA", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x0014) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB", "UA", "UB" });
 if (*kidFrame == 0x0015) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0016) moveList.insert(moveList.end(), { "LB", "RB" });
 if (*kidFrame == 0x0017) moveList.insert(moveList.end(), { "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0018) moveList.insert(moveList.end(), { "B", "D", "L", "R", "U", "LB", "RB", "UA", "UB" });
 if (*kidFrame == 0x0019) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x001A) moveList.insert(moveList.end(), { "B", "D", "L", "R", "U", "LB", "RB", "UB" });
 if (*kidFrame == 0x001B) moveList.insert(moveList.end(), { "D", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x001E) moveList.insert(moveList.end(), { "LB", "RB" });
 if (*kidFrame == 0x0022) moveList.insert(moveList.end(), { "D", "L", "R", "DL", "DR", "LB", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x0023) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB", "UB" });
 if (*kidFrame == 0x0024) moveList.insert(moveList.end(), { "L", "R", "U", "LB", "RB", "UA" });
 if (*kidFrame == 0x0025) moveList.insert(moveList.end(), { "B", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0026) moveList.insert(moveList.end(), { "D", "L", "R", "LB", "RB", "UB" });
 if (*kidFrame == 0x0027) moveList.insert(moveList.end(), { "L", "R", "U", "LB", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x0028) moveList.insert(moveList.end(), { "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0029) moveList.insert(moveList.end(), { "LB", "RB" });
 if (*kidFrame == 0x002A) moveList.insert(moveList.end(), { "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x002B) moveList.insert(moveList.end(), { "B", "L", "R" });
 if (*kidFrame == 0x002C) moveList.insert(moveList.end(), { "D", "L", "R", "BA", "DL", "DR", "LB", "RB" });
 if (*kidFrame == 0x002D) moveList.insert(moveList.end(), { "B", "L", "R", "BA", "LB", "LR", "RB" });
 if (*kidFrame == 0x002E) moveList.insert(moveList.end(), { "B", "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x002F) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0030) moveList.insert(moveList.end(), { "B", "L", "R", "DL", "DR", "LA", "LB", "RA", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x0031) moveList.insert(moveList.end(), { "A", "D", "L", "R", "U", "DA", "DL", "DR", "LA", "LB", "RA", "RB", "UA" });
 if (*kidFrame == 0x0032) moveList.insert(moveList.end(), { "A", "B", "D", "U", "L", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "DR", "DL", "DB", "DA", "BA" });
 if (*kidFrame == 0x0033) moveList.insert(moveList.end(), { "A", "B", "D", "U", "L", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "DR", "DL", "DB", "DA", "BA" });
 if (*kidFrame == 0x0034) moveList.insert(moveList.end(), { "A", "B", "D", "U", "L", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "DR", "DL", "DB", "DA", "BA" });
 if (*kidFrame == 0x0035) moveList.insert(moveList.end(), { "B", "L", "R", "U", "DB", "DL", "DR", "LB", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x0036) moveList.insert(moveList.end(), { "D", "L", "R", "BA", "DL", "DR", "LA", "LB", "RA", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x0037) moveList.insert(moveList.end(), { "B", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0038) moveList.insert(moveList.end(), { "A", "D", "L", "R", "U", "BA", "LA", "LB", "RA", "RB", "UA", "UL", "UR" });
 if (*kidFrame == 0x0039) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x003A) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x003B) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x003C) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x003D) moveList.insert(moveList.end(), { "B", "D", "L", "R", "LB", "RB", "UA" });
 if (*kidFrame == 0x003E) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x003F) moveList.insert(moveList.end(), { "A", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0040) moveList.insert(moveList.end(), { "LB", "RB" });
 if (*kidFrame == 0x0041) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "LB", "RB" });
 if (*kidFrame == 0x0043) moveList.insert(moveList.end(), { "D", "L", "R", "U", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0044) moveList.insert(moveList.end(), { "L", "R", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x0045) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0046) moveList.insert(moveList.end(), { "BA", "LB", "RB" });
 if (*kidFrame == 0x0047) moveList.insert(moveList.end(), { "B", "D", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0048) moveList.insert(moveList.end(), { "L", "R", "LB", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x0049) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x004A) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x004B) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x004C) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x004D) moveList.insert(moveList.end(), { "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x004E) moveList.insert(moveList.end(), { "D", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x004F) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "LB", "RB", "UB" });
 if (*kidFrame == 0x0051) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0052) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0053) moveList.insert(moveList.end(), { "B", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0054) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0055) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "BA", "DA", "LA", "LB", "RA", "RB", "UA", "UL", "UR" });
 if (*kidFrame == 0x0057) moveList.insert(moveList.end(), { "B", "L", "R", "U", "UL", "UR" });
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
 if (*kidFrame == 0x0066) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0067) moveList.insert(moveList.end(), { "B", "D", "L", "R", "U" });
 if (*kidFrame == 0x0068) moveList.insert(moveList.end(), { "B", "LB", "RB" });
 if (*kidFrame == 0x0069) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "BA", "LA", "LB", "RA", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x006A) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x006B) moveList.insert(moveList.end(), { "B", "L", "R", "BA", "LA", "LB", "RA", "RB", "UA", "UL", "UR" });
 if (*kidFrame == 0x006C) moveList.insert(moveList.end(), { "A", "D", "DA", "DL", "DR", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x006D) moveList.insert(moveList.end(), { "A", "D", "L", "U", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "DR", "DL", "DA", "BA" });
 if (*kidFrame == 0x006E) moveList.insert(moveList.end(), { "B", "D", "L", "R", "U", "DL", "DR", "LB", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x006F) moveList.insert(moveList.end(), { "A", "D", "L", "R", "U", "LB", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x0070) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x0071) moveList.insert(moveList.end(), { "A", "L", "R", "UL", "UR" });
 if (*kidFrame == 0x0072) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0073) moveList.insert(moveList.end(), { "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0074) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0075) moveList.insert(moveList.end(), { "L", "R", "DL", "DR", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0076) moveList.insert(moveList.end(), { "L", "R", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x0077) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "LA", "RA" });
 if (*kidFrame == 0x0079) moveList.insert(moveList.end(), { "B", "L", "R", "U", "DL", "DR", "LB", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x007A) moveList.insert(moveList.end(), { "B", "D", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x007B) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x007C) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x007D) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x007E) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x007F) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0080) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0081) moveList.insert(moveList.end(), { "B", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0082) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0083) moveList.insert(moveList.end(), { "L", "R", "LB", "RB" });
 if (*kidFrame == 0x0084) moveList.insert(moveList.end(), { "A", "B", "D", "U", "L", "R", "UR", "UL", "UB", "UA", "RB", "RA", "LB", "LA", "DR", "DL", "DB", "DA", "BA" });
 if (*kidFrame == 0x0085) moveList.insert(moveList.end(), { "B", "LB", "RB" });
 if (*kidFrame == 0x0086) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "LA", "LB", "RA", "RB", "UB" });
 if (*kidFrame == 0x0096) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "BA", "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x0097) moveList.insert(moveList.end(), { "D", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x0098) moveList.insert(moveList.end(), { "B", "D", "L", "R", "DL", "DR", "LA", "RA", "UB" });
 if (*kidFrame == 0x0099) moveList.insert(moveList.end(), { "B", "D", "L", "R", "BA", "DB", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x009A) moveList.insert(moveList.end(), { "D", "L", "R", "U", "BA", "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x009B) moveList.insert(moveList.end(), { "B", "LB", "RB" });
 if (*kidFrame == 0x009C) moveList.insert(moveList.end(), { "A", "L", "R" });
 if (*kidFrame == 0x009D) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x009E) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "DB", "DL", "DR", "LA", "LB", "RA", "RB", "UA" });
 if (*kidFrame == 0x009F) moveList.insert(moveList.end(), { "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x00A0) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "DA", "DL", "DR", "LA", "LB", "RA", "RB", "UB", "UD" });
 if (*kidFrame == 0x00A1) moveList.insert(moveList.end(), { "A", "D", "L", "R", "U", "BA", "LA", "LB", "RA", "RB", "UB", "UL", "UR" });
 if (*kidFrame == 0x00A2) moveList.insert(moveList.end(), { "L", "R" });
 if (*kidFrame == 0x00A4) moveList.insert(moveList.end(), { "A", "D", "L", "R", "U", "DA", "DL", "DR", "LA", "LB", "RA", "RB", "UD" });
 if (*kidFrame == 0x00A5) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "BA", "DL", "DR", "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x00A7) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x00A9) moveList.insert(moveList.end(), { "A", "D", "L", "R", "LA", "LB", "RA", "RB" });
 if (*kidFrame == 0x00AA) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "DB", "DL", "DR", "LA", "LB", "RA", "RB", "UA", "UB", "UD" });
 if (*kidFrame == 0x00AB) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "DB", "DL", "DR", "LA", "LB", "LR", "RA", "RB", "UB" });
 if (*kidFrame == 0x00AC) moveList.insert(moveList.end(), { "B", "D", "L", "R", "U", "BA", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x00B2) moveList.insert(moveList.end(), { "B", "L", "R", "LB", "RB" });
 if (*kidFrame == 0x00B3) moveList.insert(moveList.end(), { "B", "L", "R", "U", "BA", "LB", "RB", "UB" });
 if (*kidFrame == 0x00B4) moveList.insert(moveList.end(), { "B", "BA" });
 if (*kidFrame == 0x00B9) moveList.insert(moveList.end(), { "B", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x00CF) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "BA", "DL", "DR", "LA", "LB", "RA", "RB", "UL", "UR" });
 if (*kidFrame == 0x00D0) moveList.insert(moveList.end(), { "A", "B", "D", "BA", "DL", "DR", "LA", "RA" });
 if (*kidFrame == 0x00D1) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x00D2) moveList.insert(moveList.end(), { "A" });
 if (*kidFrame == 0x00E9) moveList.insert(moveList.end(), { "B", "L", "R", "BA", "LB", "RB", "UL", "UR" });
 if (*kidFrame == 0x00EA) moveList.insert(moveList.end(), { "B", "D", "L", "R", "BA", "LA", "LB", "RA", "RB", "UA", "UB", "UL", "UR" });
 if (*kidFrame == 0x00EC) moveList.insert(moveList.end(), { "D", "L", "R", "LB", "RB", "UB" });
 if (*kidFrame == 0x00ED) moveList.insert(moveList.end(), { "L", "R", "U" });
 if (*kidFrame == 0x00EE) moveList.insert(moveList.end(), { "D", "L", "R", "U", "LB", "RB" });
 if (*kidFrame == 0x00EF) moveList.insert(moveList.end(), { "B" });
 if (*kidFrame == 0x00F0) moveList.insert(moveList.end(), { "A", "B", "D", "L", "R", "U", "BA", "LB", "RB", "UA", "UB" });

 return moveList;
}

void GameInstance::printFullMoveList()
{
 *gameFrame = 3;
 *isLagFrame = 15;

 for (uint16_t i = 0; i <= 0xFF; i++)
 {
  *kidFrame = (uint8_t)i;
  auto moves = getPossibleMoves();
  if (moves.size() == 1) continue;

  size_t vecSize = moves.size();
  for (size_t j = 0; j < vecSize; j++)
  {
   INPUT_TYPE code = EmuInstance::moveStringToCode(moves[j]);
   if ( ((code & SNES_LEFT_MASK) > 0) && ((code & SNES_RIGHT_MASK) == 0)) moves.push_back(simplifyMove(EmuInstance::moveCodeToString((code & ~SNES_LEFT_MASK) | SNES_RIGHT_MASK)));
   if ( ((code & SNES_RIGHT_MASK) > 0) && ((code & SNES_LEFT_MASK) == 0)) moves.push_back(simplifyMove(EmuInstance::moveCodeToString((code & ~SNES_RIGHT_MASK) | SNES_LEFT_MASK)));
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
    INPUT_TYPE newMove = *gameFrame >= 3 ? move : 0;
    _emu->advanceState(newMove);
    moves.push_back(newMove);
    skippedFrames++;
    if (skippedFrames > 128) EXIT_WITH_ERROR("Exceeded skip frames\n");
   }
  }

  *rawFrameCount += moves.size();
  *lagFrameCounter += moves.size() - 4;
  updateDerivedValues();

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
    if (_rules[ruleId]->_magnets[*kidRoom].guardHorizontalMagnet.active == true) magnets.guardHorizontalMagnet = _rules[ruleId]->_magnets[*kidRoom].guardHorizontalMagnet;
    if (_rules[ruleId]->_magnets[*kidRoom].guardVerticalMagnet.active == true) magnets.guardVerticalMagnet = _rules[ruleId]->_magnets[*kidRoom].guardVerticalMagnet;
    magnets.kidGuardDistanceMagnet = _rules[ruleId]->_magnets[*kidRoom].kidGuardDistanceMagnet;
    magnets.kidDirectionMagnet = _rules[ruleId]->_magnets[*kidRoom].kidDirectionMagnet;
    magnets.guardHPMagnet = _rules[ruleId]->_magnets[*kidRoom].guardHPMagnet;
  }

 // Evaluating custom value
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true) if (_rules[ruleId]->_customValueActive == true) *customValue = _rules[ruleId]->_customValue;

 return magnets;
}

// Obtains the score of a given frame
float GameInstance::getStateReward(const bool* rulesStatus) const
{
 // We calculate a different reward if this is a winning frame
 auto stateType = getStateType(rulesStatus);
 if (stateType == f_win) return -1.0f * (float)*rawFrameCount;

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

 // Evaluating guard magnet's reward on position X
 boundedValue = (float)*guardPosX;
 boundedValue = std::min(boundedValue, magnets.guardHorizontalMagnet.max);
 boundedValue = std::max(boundedValue, magnets.guardHorizontalMagnet.min);
 diff = -255.0f + std::abs(magnets.guardHorizontalMagnet.center - boundedValue);
 reward += magnets.guardHorizontalMagnet.intensity * -diff;

 // Evaluating kid magnet's reward on position Y
 boundedValue = kidPosYActual;
 boundedValue = std::min(boundedValue, magnets.kidVerticalMagnet.max);
 boundedValue = std::max(boundedValue, magnets.kidVerticalMagnet.min);
 diff = -255.0f + std::abs(magnets.kidVerticalMagnet.center - boundedValue);
 reward += magnets.kidVerticalMagnet.intensity * -diff;

 // Evaluating guard magnet's reward on position Y
 boundedValue = (float)*guardPosY;
 boundedValue = std::min(boundedValue, magnets.guardVerticalMagnet.max);
 boundedValue = std::max(boundedValue, magnets.guardVerticalMagnet.min);
 diff = -255.0f + std::abs(magnets.guardVerticalMagnet.center - boundedValue);
 reward += magnets.guardVerticalMagnet.intensity * -diff;

 // Subtract frame count punishment to account for lag
 reward -= (float)*lagFrameCounter * lagDiscount;

 // Subtract fight mode reward
 reward -= (float)*kidFightMode * fightModeDiscount;

 // Kid Direction Magnet
 reward += *kidDirection == 0 ? 1.0 : -1.0  * magnets.kidDirectionMagnet;

 // Guard HP Magnet
 if (*kidRoom == *guardRoom) reward += (float)(*guardMaxHP - *guardHP)  * magnets.guardHPMagnet;

 // Guard Distance
  if (*kidRoom == *guardRoom) reward += (256.0f - std::abs((float)(*guardPosX) - (float)(*kidPosX))) * magnets.kidGuardDistanceMagnet;

 // Kid HP Magnet
 reward += *kidHP * magnets.kidHPMagnet;

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
 LOG("[Jaffar]  + Game Timer:             %05u (Tolerance: %02u) - IGT: %05u\n", *gameTimer, timerTolerance, *IGTTicks);
 LOG("[Jaffar]  + Raw Frame Count:        %05u (Lag: %05u)\n", *rawFrameCount, *lagFrameCounter);
 LOG("[Jaffar]  + Game Frame:             %02u\n", *gameFrame);
 LOG("[Jaffar]  + Is Lag Frame:           %02u\n", *isLagFrame);
 LOG("[Jaffar]  + Input Code:             %05u\n", *inputCode);

 LOG("[Jaffar]  + Kid                     Room: %02u, Pos X: %02u, Pos Y: %02u (%3.0f), Dir: %03u, HP: %02u, Frame: %02u\n", *kidRoom, *kidPosX, *kidPosY, kidPosYActual, *kidDirection, *kidHP, *kidFrame);
 LOG("[Jaffar]  + Guard                   Room: %02u, Pos X: %02u, Pos Y: %02u, Dir: %03u, HP: %02u, Frame: %02u\n", *guardRoom, *guardPosX, *guardPosY, *guardDirection, *guardHP, *guardFrame);

// LOG("[Jaffar]  + Kid Frame:              0x%02X (Prev: 0x%02X, Diff: %02d)\n", *kidFrame, *kidPrevFrame, *kidFrameDiff);
 LOG("[Jaffar]  + Kid Sequence Step:      0x%02X\n", *kidSequenceStep);
 LOG("[Jaffar]  + Kid Action (Buffered):  %02u (%02u)\n", *kidAction, *kidBuffered);
 LOG("[Jaffar]  + Kid Col:                %02u\n", *kidCol);
 LOG("[Jaffar]  + Kid Row:                %02u\n", *kidRow);
 LOG("[Jaffar]  + Kid Crouch State:       %02u\n", *kidCrouchState);
 LOG("[Jaffar]  + Kid Fight Mode:         %02u\n", *kidFightMode);
 LOG("[Jaffar]  + Kid Grab State:         %02u\n", *kidGrabState);
 LOG("[Jaffar]  + Kid Hanging State:      %02u\n", *kidHangingState);
 LOG("[Jaffar]  + Kid Climbing Type:      %02u / %02u\n", *kidClimbingType1, *kidClimbingType2);
 LOG("[Jaffar]  + Exit Door State:        %02u\n", *exitDoorState);
 LOG("[Jaffar]  + Custom Value:           %02u\n", *customValue);
 LOG("[Jaffar]  + Kid Teleporting:        %02u\n", *kidTeleporting);
 LOG("[Jaffar]  + Exit Level Mode:        %02u\n", *exitLevelMode);
 LOG("[Jaffar]  + Boss Sequence:          %02u\n", *bossSequence);

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
 if (std::abs(magnets.kidHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Kid Horizontal Magnet     - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidHorizontalMagnet.intensity, magnets.kidHorizontalMagnet.center, magnets.kidHorizontalMagnet.min, magnets.kidHorizontalMagnet.max);
 if (std::abs(magnets.kidVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Kid Vertical Magnet       - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.kidVerticalMagnet.intensity, magnets.kidVerticalMagnet.center, magnets.kidVerticalMagnet.min, magnets.kidVerticalMagnet.max);
 if (std::abs(magnets.kidDirectionMagnet) > 0.0f)              LOG("[Jaffar]  + Kid Direction Magnet      - Intensity: %.1f\n", magnets.kidDirectionMagnet);
 if (std::abs(magnets.guardHorizontalMagnet.intensity) > 0.0f) LOG("[Jaffar]  + Guard Horizontal Magnet   - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.guardHorizontalMagnet.intensity, magnets.guardHorizontalMagnet.center, magnets.guardHorizontalMagnet.min, magnets.guardHorizontalMagnet.max);
 if (std::abs(magnets.guardVerticalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Guard Vertical Magnet     - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.guardVerticalMagnet.intensity, magnets.guardVerticalMagnet.center, magnets.guardVerticalMagnet.min, magnets.guardVerticalMagnet.max);
 if (std::abs(magnets.guardHPMagnet) > 0.0f)                   LOG("[Jaffar]  + Guard HP Magnet           - Intensity: %.1f\n", magnets.guardHPMagnet);
 if (std::abs(magnets.kidHPMagnet) > 0.0f)                     LOG("[Jaffar]  + Kid HP Magnet             - Intensity: %.1f\n", magnets.kidHPMagnet);
 if (std::abs(magnets.kidGuardDistanceMagnet) > 0.0f)          LOG("[Jaffar]  + Kid Guard Distance Magnet - Intensity: %.1f\n", magnets.kidGuardDistanceMagnet);

 for (const auto& tile : tileWatchList)
   LOG("[Jaffar]  + Tile Info            - Room: %02u, Row: %02u, Col: %02u, Index: %03u, Mem: 0x%05X / 0x%05X, Type: %03u, State: %03u\n", tile.second.room, tile.second.row, tile.second.col, tile.second.index, TILE_TYPE_BASE + tile.second.index, TILE_STATE_BASE + tile.second.index, tileTypeBase[tile.second.index], tileStateBase[tile.second.index]);
}

