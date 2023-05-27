#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting relevant SMB pointers

  // Thanks to https://datacrystal.romhacking.net/wiki/Super_Mario_Bros.:RAM_map and https://tasvideos.org/GameResources/NES/SuperMarioBros for helping me find some of these items
  globalTimer          = (uint8_t*)  &_emu->_baseMem[0x0009];
  screenScroll         = (uint16_t*) &_emu->_baseMem[0x071B];
  marioAnimation       = (uint8_t*)  &_emu->_baseMem[0x0001];
  marioState           = (uint8_t*)  &_emu->_baseMem[0x000E];
  marioDisappearState  = (uint8_t*)  &_emu->_baseMem[0x009C];
  marioSprite          = (uint8_t*)  &_emu->_baseMem[0x06D5];

  marioBasePosX        = (uint8_t*)  &_emu->_baseMem[0x006D];
  marioRelPosX         = (uint8_t*)  &_emu->_baseMem[0x0086];
  marioSubpixelPosX    = (uint8_t*)  &_emu->_baseMem[0x0400];

  marioRelPosY            = (uint8_t*)  &_emu->_baseMem[0x00CE];
  marioSubpixelPosY    = (uint8_t*)  &_emu->_baseMem[0x0416];

  marioMovingDirection = (uint8_t*)  &_emu->_baseMem[0x0045];
  marioFacingDirection = (uint8_t*)  &_emu->_baseMem[0x0033];
  marioFloatingMode    = (uint8_t*)  &_emu->_baseMem[0x001D];
  marioWalkingMode     = (uint8_t*)  &_emu->_baseMem[0x0702];
  marioWalkingDelay    = (uint8_t*)  &_emu->_baseMem[0x070C];
  marioWalkingFrame    = (uint8_t*)  &_emu->_baseMem[0x070D];
  marioMaxVelLeft      = (int8_t*)   &_emu->_baseMem[0x0450];
  marioMaxVelRight     = (int8_t*)   &_emu->_baseMem[0x0456];
  marioVelX            = (int8_t*)   &_emu->_baseMem[0x0057];
  marioXMoveForce      = (int8_t*)   &_emu->_baseMem[0x0705];
  marioVelY            = (int8_t*)   &_emu->_baseMem[0x009F];
  marioFracVelY        = (int8_t*)   &_emu->_baseMem[0x0433];
  marioGravity         = (uint8_t*)  &_emu->_baseMem[0x0709];
  marioFriction        = (uint8_t*)  &_emu->_baseMem[0x0701];
  timeLeft100          = (uint8_t*)  &_emu->_baseMem[0x07F8];
  timeLeft10           = (uint8_t*)  &_emu->_baseMem[0x07F9];
  timeLeft1            = (uint8_t*)  &_emu->_baseMem[0x07FA];

  screenBasePosX       = (uint8_t*)  &_emu->_baseMem[0x071A];
  screenRelPosX        = (uint8_t*)  &_emu->_baseMem[0x071C];

  currentWorldRaw      = (uint8_t*)  &_emu->_baseMem[0x075F];
  currentStageRaw      = (uint8_t*)  &_emu->_baseMem[0x075C];
  levelEntryFlag       = (uint8_t*)  &_emu->_baseMem[0x0752];
  gameMode             = (uint8_t*)  &_emu->_baseMem[0x0770];

  enemy1Active         = (uint8_t*)  &_emu->_baseMem[0x000F];
  enemy2Active         = (uint8_t*)  &_emu->_baseMem[0x0010];
  enemy3Active         = (uint8_t*)  &_emu->_baseMem[0x0011];
  enemy4Active         = (uint8_t*)  &_emu->_baseMem[0x0012];
  enemy5Active         = (uint8_t*)  &_emu->_baseMem[0x0013];

  enemy1State          = (uint8_t*)  &_emu->_baseMem[0x001E];
  enemy2State          = (uint8_t*)  &_emu->_baseMem[0x001F];
  enemy3State          = (uint8_t*)  &_emu->_baseMem[0x0020];
  enemy4State          = (uint8_t*)  &_emu->_baseMem[0x0021];
  enemy5State          = (uint8_t*)  &_emu->_baseMem[0x0022];

  enemy1Type           = (uint8_t*)  &_emu->_baseMem[0x0016];
  enemy2Type           = (uint8_t*)  &_emu->_baseMem[0x0017];
  enemy3Type           = (uint8_t*)  &_emu->_baseMem[0x0018];
  enemy4Type           = (uint8_t*)  &_emu->_baseMem[0x0019];
  enemy5Type           = (uint8_t*)  &_emu->_baseMem[0x001A];

  marioCollision       = (uint8_t*)  &_emu->_baseMem[0x0490];
  enemyCollision       = (uint8_t*)  &_emu->_baseMem[0x0491];
  hitDetectionFlag     = (uint8_t*)  &_emu->_baseMem[0x0722];

  powerUpActive        = (uint8_t*)  &_emu->_baseMem[0x0014];

  animationTimer       = (uint8_t*)  &_emu->_baseMem[0x0781];
  jumpSwimTimer        = (uint8_t*)  &_emu->_baseMem[0x0782];
  runningTimer         = (uint8_t*)  &_emu->_baseMem[0x0783];
  blockBounceTimer     = (uint8_t*)  &_emu->_baseMem[0x0784];
  sideCollisionTimer   = (uint8_t*)  &_emu->_baseMem[0x0785];
  jumpspringTimer      = (uint8_t*)  &_emu->_baseMem[0x0786];
  gameControlTimer     = (uint8_t*)  &_emu->_baseMem[0x0787];
  climbSideTimer       = (uint8_t*)  &_emu->_baseMem[0x0789];
  enemyFrameTimer      = (uint8_t*)  &_emu->_baseMem[0x078A];
  frenzyEnemyTimer     = (uint8_t*)  &_emu->_baseMem[0x078F];
  bowserFireTimer      = (uint8_t*)  &_emu->_baseMem[0x0790];
  stompTimer           = (uint8_t*)  &_emu->_baseMem[0x0791];
  airBubbleTimer       = (uint8_t*)  &_emu->_baseMem[0x0792];
  fallPitTimer         = (uint8_t*)  &_emu->_baseMem[0x0795];
  multiCoinBlockTimer  = (uint8_t*)  &_emu->_baseMem[0x079D];
  invincibleTimer      = (uint8_t*)  &_emu->_baseMem[0x079E];
  starTimer            = (uint8_t*)  &_emu->_baseMem[0x079F];

  player1Input         = (uint8_t*)  &_emu->_baseMem[0x06FC];
  player1Buttons       = (uint8_t*)  &_emu->_baseMem[0x074A];
  player1GamePad1      = (uint8_t*)  &_emu->_baseMem[0x000A];
  player1GamePad2      = (uint8_t*)  &_emu->_baseMem[0x000D];

  warpSelector         = (uint8_t*)  &_emu->_baseMem[0x06D6];
  warpAreaOffset       = (uint16_t*) &_emu->_baseMem[0x0750];
  lagIndicator         = (uint8_t*)  &_emu->_baseMem[0x01FB];
  lastInputTime        = (uint8_t*) &_emu->_baseMem[0x07F0];
  nextCastleFlag       = (uint8_t*) &_emu->_baseMem[0x0719];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  // If timer tolerance is set, use the game tick for hashing
  if (timerTolerance > 0) hash.Update(*globalTimer % (timerTolerance+1));

  // If beaten koopa, wait indefinitely
  if (*gameMode == 2) hash.Update(currentStep);

  // Adding fixed hash elements
  hash.Update(*screenScroll);
  hash.Update(*marioAnimation);
  hash.Update(*marioState);
  hash.Update(*marioSprite);
  hash.Update(*marioDisappearState);

  hash.Update(*marioBasePosX);
  hash.Update(*marioRelPosX);
//  hash.Update(*marioSubpixelPosX);

  hash.Update(*marioRelPosY);
//  hash.Update(*marioSubpixelPosY);


  // Swimming:
#define _SWIMMING
#ifdef _SWIMMING
  //  hash.Update(*marioXMoveForce);
    hash.Update(*marioFacingDirection);
    hash.Update(*marioMovingDirection);
    hash.Update(*marioFloatingMode);
  //  hash.Update(*marioWalkingMode);
  //  hash.Update(*marioWalkingDelay);
  //  hash.Update(*marioWalkingFrame);
  //    hash.Update(*marioMaxVelLeft);
  //    hash.Update(*marioMaxVelRight);
    hash.Update(*marioVelX);
  //  hash.Update(*marioVelY);
  //  hash.Update(*marioFracVelY);
    hash.Update(*marioGravity);
    hash.Update(*marioFriction);
    hash.Update(*lagIndicator);
#else
  // Not Swimming
  hash.Update(*marioXMoveForce);
  hash.Update(*marioFacingDirection);
  hash.Update(*marioMovingDirection);
  hash.Update(*marioFloatingMode);
  hash.Update(*marioWalkingMode);
  hash.Update(*marioWalkingDelay);
  hash.Update(*marioWalkingFrame);
    hash.Update(*marioMaxVelLeft);
    hash.Update(*marioMaxVelRight);
  hash.Update(*marioVelX);
  hash.Update(*marioVelY);
  hash.Update(*marioFracVelY);
  hash.Update(*marioGravity);
  hash.Update(*marioFriction);
  hash.Update(*lagIndicator);
#endif

  hash.Update(*screenBasePosX);
  hash.Update(*screenRelPosX);

  hash.Update(*currentWorldRaw);
  hash.Update(*currentStageRaw);
  hash.Update(*levelEntryFlag);
  hash.Update(*gameMode);

  // hash.Update(*enemy1Active);
  // hash.Update(*enemy2Active);
  // hash.Update(*enemy3Active);
  // hash.Update(*enemy4Active);
  // hash.Update(*enemy5Active);

  // hash.Update(*enemy1State);
  // hash.Update(*enemy2State);
  // hash.Update(*enemy3State);
  // hash.Update(*enemy4State);
  // hash.Update(*enemy5State);

//  hash.Update(*enemy1Type);
//  hash.Update(*enemy2Type);
//  hash.Update(*enemy3Type);
//  hash.Update(*enemy4Type);
//  hash.Update(*enemy5Type);
  hash.Update(*warpSelector);

//  hash.Update(*marioCollision);
//  hash.Update(*enemyCollision);
//  hash.Update(*hitDetectionFlag);

  // To Reduce timer pressure on hash, have 0, 1, and >1 as possibilities only
  hash.Update(*animationTimer < 2 ? *animationTimer : (uint8_t)2);
  hash.Update(*jumpSwimTimer < 2 ? *jumpSwimTimer : (uint8_t)2);
  hash.Update(*runningTimer < 2 ? *runningTimer : (uint8_t)2);
//  hash.Update(*blockBounceTimer < 2 ? *blockBounceTimer : (uint8_t)2);
//   hash.Update(*sideCollisionTimer);
//   hash.Update(*jumpspringTimer);
  // hash.Update(*gameControlTimer);
  // hash.Update(*climbSideTimer);
  // hash.Update(*enemyFrameTimer);
  // hash.Update(*frenzyEnemyTimer);
  // hash.Update(*bowserFireTimer);
  // hash.Update(*stompTimer);
  // hash.Update(*airBubbleTimer);
  // hash.Update(*fallPitTimer);
  // hash.Update(*multiCoinBlockTimer);
  // hash.Update(*invincibleTimer);
  // hash.Update(*starTimer);
  // hash.Update(*powerUpActive);


  hash.Update(*warpAreaOffset);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 // Recalculating derived values
  marioPosY = *marioRelPosY + 46;
  marioPosX = (uint16_t)*marioBasePosX * 256 + (uint16_t)*marioRelPosX;
  if (marioPosX > 60000) marioPosX = 0;
  screenPosX = (uint16_t)*screenBasePosX * 256 + (uint16_t)*screenRelPosX;
  marioScreenOffset = marioPosX - screenPosX;
  currentWorld = *currentWorldRaw + 1;
  currentStage = *currentStageRaw + 1;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 moves.push_back(move);
 updateDerivedValues();

 if (move != 0) *lastInputTime = *globalTimer;

 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
  if (*marioState != 8) return { "." };

  std::vector<std::string> moveList = { "." };

  if (*marioAnimation == 0x0004) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x000C) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x0035) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "RL......", "RL....B."});
  if (*marioAnimation == 0x0039) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x003C) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x0040) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x0043) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", ".LD.....", "R......A", "R.D....."});
  if (*marioAnimation == 0x004F) moveList.insert(moveList.end(), { ".......A", "......B.", "..D.....", ".L......", "R.......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "R.D.....", "RL......", "R.D...B.", "RL....B."});
  if (*marioAnimation == 0x00F8) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A"});
  if (*marioAnimation == 0x00F9) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A"});
  if (*marioAnimation == 0x00FA) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A"});
  if (*marioAnimation == 0x00FB) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x00FC) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A"});
  if (*marioAnimation == 0x00FD) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A"});
  if (*marioAnimation == 0x00FE) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "RL......"});
  if (*marioAnimation == 0x00FF) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});

  if (*marioAnimation == 0x0001) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x0002) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A"});
  if (*marioAnimation == 0x0035) moveList.insert(moveList.end(), { "...U...."});
  if (*marioAnimation == 0x0039) moveList.insert(moveList.end(), { "...U....", "RL....B."});
  if (*marioAnimation == 0x003C) moveList.insert(moveList.end(), { "...U....", "RL....B."});
  if (*marioAnimation == 0x0040) moveList.insert(moveList.end(), { "...U...."});
  if (*marioAnimation == 0x004F) moveList.insert(moveList.end(), { "...U....", ".LD.....", ".LD...B."});
  if (*marioAnimation == 0x0086) moveList.insert(moveList.end(), { ".L......", "R......."});
  if (*marioAnimation == 0x00F8) moveList.insert(moveList.end(), { "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x00FA) moveList.insert(moveList.end(), { "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x00FC) moveList.insert(moveList.end(), { "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x00FD) moveList.insert(moveList.end(), { "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x00FE) moveList.insert(moveList.end(), { "R.....B."});

  if (*marioAnimation == 0x0035) moveList.insert(moveList.end(), { "......B."});
  if (*marioAnimation == 0x0039) moveList.insert(moveList.end(), { "......B."});
  if (*marioAnimation == 0x003C) moveList.insert(moveList.end(), { "......B."});
  if (*marioAnimation == 0x0040) moveList.insert(moveList.end(), { "......B.", ".L....B."});
  if (*marioAnimation == 0x00F9) moveList.insert(moveList.end(), { "R.....B.", "R.....BA", "RL......"});

  if (*marioAnimation == 0x0001) moveList.insert(moveList.end(), { "..D.....", ".L....B.", "RL....B."});
  if (*marioAnimation == 0x0002) moveList.insert(moveList.end(), { "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x0003) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "RL......", "RL....B."});
  if (*marioAnimation == 0x0004) moveList.insert(moveList.end(), { "...U....", ".L....B.", "RL....B."});
  if (*marioAnimation == 0x0006) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "RL......", "RL....B."});
  if (*marioAnimation == 0x000A) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x000C) moveList.insert(moveList.end(), { "...U....", ".L....B.", "RL....B."});
  if (*marioAnimation == 0x0018) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "RL......", "RL....B."});
  if (*marioAnimation == 0x0086) moveList.insert(moveList.end(), { ".......A", "......B.", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.....BA", "RL......", "RL....B."});
  if (*marioAnimation == 0x00FD) moveList.insert(moveList.end(), { "..D.....", ".L....B."});
  if (*marioAnimation == 0x00FE) moveList.insert(moveList.end(), { "..D.....", ".L....B.", "RL....B."});
  if (*marioAnimation == 0x00FF) moveList.insert(moveList.end(), { "..D.....", ".L....B.", "RL....B."});

  if (*marioAnimation == 0x0001) moveList.insert(moveList.end(), { "...U...."});
  if (*marioAnimation == 0x000C) moveList.insert(moveList.end(), { "......B."});
  if (*marioAnimation == 0x0018) moveList.insert(moveList.end(), { "...U...."});
  if (*marioAnimation == 0x0086) moveList.insert(moveList.end(), { "..D....."});
  if (*marioAnimation == 0x00FE) moveList.insert(moveList.end(), { "...U...."});

  if (*marioAnimation == 0x0006) moveList.insert(moveList.end(), { ".LD.....", "R.D....."});
  if (*marioAnimation == 0x000E) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x0016) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "R.....B.", "R.....BA", "RL......"});
  if (*marioAnimation == 0x0006) moveList.insert(moveList.end(), { "...U...."});

  if (*marioAnimation == 0x000E) moveList.insert(moveList.end(), { "..D.....", ".L....B.", "RL....B."});
  if (*marioAnimation == 0x00FC) moveList.insert(moveList.end(), { "..D.....", ".L....B.", "RL....B."});

  if (*marioAnimation == 0x0006) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x000C) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x0035) moveList.insert(moveList.end(), { "..D....A", "RL.....A"});
  if (*marioAnimation == 0x0039) moveList.insert(moveList.end(), { "..D....A", "RL.....A"});
  if (*marioAnimation == 0x003C) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x0040) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x0045) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", "......BA", ".L.....A", "R......A", "RL......", "RL.....A"});
  if (*marioAnimation == 0x0047) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", "......BA", ".L.....A", "R......A", "RL......", "RL.....A"});
  if (*marioAnimation == 0x0049) moveList.insert(moveList.end(), { ".......A", "..D.....", ".L......", "R.......", "......BA", ".L.....A", "R......A", "RL......", "RL.....A"});
  if (*marioAnimation == 0x004F) moveList.insert(moveList.end(), { "..D....A", "R.D....A", "RL.....A"});
  if (*marioAnimation == 0x00F8) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x00F9) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x00FC) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x00FD) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x00FE) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x00FF) moveList.insert(moveList.end(), { "RL.....A"});

  if (*marioAnimation == 0x0012) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "RL......", "RL.....A"});
  if (*marioAnimation == 0x0018) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x0020) moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "RL......", "RL.....A"});
  if (*marioAnimation == 0x0035) moveList.insert(moveList.end(), { "...U...A"});
  if (*marioAnimation == 0x0039) moveList.insert(moveList.end(), { "...U...A"});
  if (*marioAnimation == 0x003C) moveList.insert(moveList.end(), { "..D....A"});
  if (*marioAnimation == 0x0040) moveList.insert(moveList.end(), { "..D....A"});
  if (*marioAnimation == 0x0045) moveList.insert(moveList.end(), { "...U...."});
  if (*marioAnimation == 0x0047) moveList.insert(moveList.end(), { "...U...."});
  if (*marioAnimation == 0x0049) moveList.insert(moveList.end(), { "...U...."});
  if (*marioAnimation == 0x004F) moveList.insert(moveList.end(), { "...U...A", ".LD....A"});
  if (*marioAnimation == 0x00FA) moveList.insert(moveList.end(), { "RL.....A"});
  if (*marioAnimation == 0x00FB) moveList.insert(moveList.end(), { "RL.....A"});

  if (*marioAnimation == 0x00F8) moveList.insert(moveList.end(), { "LB" });
  if (*marioAnimation == 0x00F9) moveList.insert(moveList.end(), { "LB" });
  if (*marioAnimation == 0x00FA) moveList.insert(moveList.end(), { "LB" });
  if (*marioAnimation == 0x00FB) moveList.insert(moveList.end(), { "LB" });

  if (*marioAnimation == 0x0091) moveList.insert(moveList.end(), { "R", "L", "D", "U", "LR", "DR", "DL", "UR", "UL", "DLR", "ULR" });
  if (*marioAnimation == 0x0093) moveList.insert(moveList.end(), { "R", "L", "D", "U", "LR", "DR", "DL", "UR", "UL", "DLR", "ULR" });
  if (*marioAnimation == 0x009F) moveList.insert(moveList.end(), { "R", "L", "D", "U", "LR", "DR", "DL", "UR", "UL", "DLR", "ULR" });

  if (*marioAnimation == 0x0031) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x0032) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x0033) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x0034) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x00E9) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "RB", "LA", "LR" });
  if (*marioAnimation == 0x00EE) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x00EF) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x00F0) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x00F1) moveList.insert(moveList.end(), { "R", "L" });
  if (*marioAnimation == 0x00FB) moveList.insert(moveList.end(), { "D" });

  return moveList;
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
  magnetSet_t magnets;

  for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
   if (rulesStatus[ruleId] == true)
     magnets = _rules[ruleId]->_magnets;

  return magnets;
}

// Obtains the score of a given frame
float GameInstance::getStateReward(const bool* rulesStatus) const
{
  // If beaten the game, the best score is that which had the last input
  if (currentWorld == 8 && currentStage == 4 && *gameMode == 2) return -*lastInputTime;

  // Getting rewards from rules
  float reward = 0.0;
  for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
   if (rulesStatus[ruleId] == true)
    reward += _rules[ruleId]->_reward;

  if (currentWorld == 8 && currentStage == 4) reward -= 0.001 * (*lastInputTime);

  // Getting magnet values for the kid
  auto magnets = getMagnetValues(rulesStatus);

  // Container for bounding calculations
  float boundedValue = 0.0;
  float diff = 0.0;

  // Evaluating mario magnet's reward on position X
  boundedValue = screenPosX;
  boundedValue = std::min(boundedValue, magnets.screenHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.screenHorizontalMagnet.min);
  diff = std::abs(magnets.screenHorizontalMagnet.center - boundedValue);
  reward += magnets.screenHorizontalMagnet.intensity * -diff;

  // Evaluating mario magnet's reward on position X
  boundedValue = marioScreenOffset;
  boundedValue = std::min(boundedValue, magnets.marioScreenOffsetMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marioScreenOffsetMagnet.min);
  diff = std::abs(magnets.marioScreenOffsetMagnet.center - boundedValue);
  reward += magnets.marioScreenOffsetMagnet.intensity * -diff;

  // Evaluating mario magnet's reward on position X
  boundedValue = marioPosX;
  boundedValue = std::min(boundedValue, magnets.marioHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marioHorizontalMagnet.min);
  diff = std::abs(magnets.marioHorizontalMagnet.center - boundedValue);
  reward += magnets.marioHorizontalMagnet.intensity * -diff;

  // Evaluating mario magnet's reward on position Y
  boundedValue = (float)marioPosY;
  boundedValue = std::min(boundedValue, magnets.marioVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marioVerticalMagnet.min);
  diff = std::abs(magnets.marioVerticalMagnet.center - boundedValue);
  reward += magnets.marioVerticalMagnet.intensity * -diff;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Global Timer / Lag:     %u (%02u)\n", *globalTimer, *lagIndicator);
 LOG("[Jaffar]  + Last Input Time:        %u\n", *lastInputTime);
 LOG("[Jaffar]  + Current World-Stage:    %1u-%1u\n", currentWorld, currentStage);
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Time Left:              %1u%1u%1u\n", *timeLeft100, *timeLeft10, *timeLeft1);
 LOG("[Jaffar]  + Mario Animation:        %02u\n", *marioAnimation);
 LOG("[Jaffar]  + Mario State / Sprite:   %02u (D: %02u) / %02u\n", *marioState, *marioDisappearState, *marioSprite);
 LOG("[Jaffar]  + Screen Pos X:           %04u (%02u * 256 = %04u + %02u)\n", screenPosX, *screenBasePosX, (uint16_t)*screenBasePosX * 255, *screenRelPosX);
 LOG("[Jaffar]  + Mario Pos X:            %04u (%02u * 256 = %04u + %02u)\n", marioPosX, *marioBasePosX, (uint16_t)*marioBasePosX * 255, *marioRelPosX);
 LOG("[Jaffar]  + Mario / Screen Offset:  %04d\n", marioScreenOffset);
 LOG("[Jaffar]  + Mario Pos Y:            %02u (%02u-XX)\n", marioPosY, *marioRelPosY);
 LOG("[Jaffar]  + Mario SubPixel X/Y:     %02u / %02u\n", *marioSubpixelPosX, *marioSubpixelPosY);
 LOG("[Jaffar]  + Mario Vel X:            %02d (Force: %02d, MaxL: %02d, MaxR: %02d)\n", *marioVelX, *marioXMoveForce, *marioMaxVelLeft, *marioMaxVelRight);
 LOG("[Jaffar]  + Mario Vel Y:            %02d (%02d)\n", *marioVelY, *marioFracVelY);
 LOG("[Jaffar]  + Mario Gravity:          %02u\n", *marioGravity);
 LOG("[Jaffar]  + Mario Friction:         %02u\n", *marioFriction);
 LOG("[Jaffar]  + Mario Moving Direction: %s\n", *marioMovingDirection == 1 ? "Right" : "Left");
 LOG("[Jaffar]  + Mario Facing Direction: %s\n", *marioFacingDirection == 1 ? "Right" : "Left");
 LOG("[Jaffar]  + Mario Floating Mode:    %02u\n", *marioFloatingMode);
 LOG("[Jaffar]  + Mario Walking:          %02u %02u %02u\n", *marioWalkingMode, *marioWalkingDelay, *marioWalkingFrame);
 LOG("[Jaffar]  + Player 1 Inputs:        %02u %02u %02u %02u\n", *player1Input, *player1Buttons, *player1GamePad1, *player1GamePad2);
 LOG("[Jaffar]  + Powerup Active:         %1u\n", *powerUpActive);
 LOG("[Jaffar]  + Enemy Active:           %1u%1u%1u%1u%1u\n", *enemy1Active, *enemy2Active, *enemy3Active, *enemy4Active, *enemy5Active);
 LOG("[Jaffar]  + Enemy State:            %02u %02u %02u %02u %02u\n", *enemy1State, *enemy2State, *enemy3State, *enemy4State, *enemy5State);
 LOG("[Jaffar]  + Enemy Type:             %02u %02u %02u %02u %02u\n", *enemy1Type, *enemy2Type, *enemy3Type, *enemy4Type, *enemy5Type);
 LOG("[Jaffar]  + Hit Detection Flags:    %02u %02u %02u\n", *marioCollision, *enemyCollision, *hitDetectionFlag);
 LOG("[Jaffar]  + LevelEntry / GameMode:  %02u / %02u\n", *levelEntryFlag, *gameMode);
 LOG("[Jaffar]  + Warp Selector / Offset: %02u / %04u\n", *warpSelector, *warpAreaOffset);
 LOG("[Jaffar]  + Timers:                 %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n", *animationTimer, *jumpSwimTimer, *runningTimer, *blockBounceTimer, *sideCollisionTimer, *jumpspringTimer, *gameControlTimer, *climbSideTimer, *enemyFrameTimer, *frenzyEnemyTimer, *bowserFireTimer, *stompTimer, *airBubbleTimer, *multiCoinBlockTimer, *invincibleTimer, *starTimer);
 LOG("[Jaffar]  + Next Castle Flag        %02u\n", *nextCastleFlag);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 if (std::abs(magnets.screenHorizontalMagnet.intensity) > 0.0f)    LOG("[Jaffar]  + Screen Horizontal Magnet     - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.screenHorizontalMagnet.intensity, magnets.screenHorizontalMagnet.center, magnets.screenHorizontalMagnet.min, magnets.screenHorizontalMagnet.max);
 if (std::abs(magnets.marioScreenOffsetMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Screen Offset Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.marioScreenOffsetMagnet.intensity, magnets.marioScreenOffsetMagnet.center, magnets.marioScreenOffsetMagnet.min, magnets.marioScreenOffsetMagnet.max);
 if (std::abs(magnets.marioHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Mario Horizontal Magnet     - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.marioHorizontalMagnet.intensity, magnets.marioHorizontalMagnet.center, magnets.marioHorizontalMagnet.min, magnets.marioHorizontalMagnet.max);
 if (std::abs(magnets.marioVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Mario Vertical Magnet       - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.marioVerticalMagnet.intensity, magnets.marioVerticalMagnet.center, magnets.marioVerticalMagnet.min, magnets.marioVerticalMagnet.max);
}

