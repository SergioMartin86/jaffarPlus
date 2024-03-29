#include "gameInstance.hpp"
#include "gameRule.hpp"

#define ENEMY_COUNT 13
#define ENEMY_OFFSET 0x0B

#define ORB_COUNT 4
#define ORB_OFFSET 0x04

#define CLONE_COUNT 2
#define CLONE_OFFSET 0x01

#define PROJECTILE_COUNT 2
#define PROJECTILE_OFFSET 0x09

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  frameCounter             = (uint8_t*)   &_emu->_baseMem[0x00C1]; //
  currentLevel             = (uint8_t*)   &_emu->_baseMem[0x007E];
  ninjaAirMode             = (uint8_t*)   &_emu->_baseMem[0x0046];
  ninjaFrame               = (uint8_t*)   &_emu->_baseMem[0x0050];
  ninjaAnimation           = (uint8_t*)   &_emu->_baseMem[0x0460];
  ninjaCurrentAction       = (uint8_t*)   &_emu->_baseMem[0x04A8];
  ninjaState               = (uint8_t*)   &_emu->_baseMem[0x0520];
  ninjaWeapon              = (uint8_t*)   &_emu->_baseMem[0x007D]; //
  ninjaLives               = (uint8_t*)   &_emu->_baseMem[0x00A5]; //
  ninjaPower               = (uint8_t*)   &_emu->_baseMem[0x00AE]; //
  ninjaPowerMax            = (uint8_t*)   &_emu->_baseMem[0x00AF]; //
  ninjaHP                  = (uint8_t*)   &_emu->_baseMem[0x0080]; //
  ninjaPosX                = (uint8_t*)   &_emu->_baseMem[0x0550]; //
  ninjaPosXFrac            = (uint8_t*)   &_emu->_baseMem[0x0538]; //
  ninjaSpeedX              = (int8_t*)    &_emu->_baseMem[0x05B0]; //
  ninjaSpeedXFrac          = (uint8_t*)   &_emu->_baseMem[0x0598]; //
  ninjaPosY                = (uint8_t*)   &_emu->_baseMem[0x0580]; //
  ninjaPosYFrac            = (uint8_t*)   &_emu->_baseMem[0x0568]; //
  ninjaSpeedY              = (int8_t*)    &_emu->_baseMem[0x05E0]; //
  ninjaSpeedYFrac          = (uint8_t*)   &_emu->_baseMem[0x05C8]; //
  screenScroll1            = (uint8_t*)   &_emu->_baseMem[0x0039];
  screenScroll2            = (uint8_t*)   &_emu->_baseMem[0x003A];
  screenScroll3            = (uint8_t*)   &_emu->_baseMem[0x003B];
  ninjaInvincibilityTimer  = (uint8_t*)   &_emu->_baseMem[0x0068];
  gameMode                 = (uint8_t*)   &_emu->_baseMem[0x01FE];
  bossHP                   = (uint8_t*)   &_emu->_baseMem[0x0081];
  windTimer                = (uint8_t*)   &_emu->_baseMem[0x0042];
  windCycle                = (uint8_t*)   &_emu->_baseMem[0x0066];
  heartTimer               = (uint8_t*)   &_emu->_baseMem[0x051F];
  heartState               = (uint8_t*)   &_emu->_baseMem[0x0537];
  headHP                   = (uint8_t*)   &_emu->_baseMem[0x060F];

  cloneOffset              = (uint8_t*)   &_emu->_baseMem[0x004B];
  clonePosXArray           = (uint8_t*)   &_emu->_baseMem[0x0100];
  clonePosYArray           = (uint8_t*)   &_emu->_baseMem[0x0140];
  cloneStateArray          = (uint8_t*)   &_emu->_baseMem[0x0180];

  // Object Activation Bits
  ObjectActivationBits1    = (uint8_t*)   &_emu->_baseMem[0x0048];
  ObjectActivationBits2    = (uint8_t*)   &_emu->_baseMem[0x0049];
  ObjectActivationBits3    = (uint8_t*)   &_emu->_baseMem[0x004A];

  collisionFlags           = (uint8_t*)   &_emu->_baseMem[0x04C0];

  lastInputKey             = (uint8_t*)   &_emu->_baseMem[0x002C];
  lastInputTime            = (uint8_t*)   &_emu->_baseMem[0x07FF];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Last Input Frame Tolerance
  if (isDefined(config, "Last Input Key Accepted") == true)
   lastInputKeyAccepted = config["Last Input Key Accepted"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Last Input Key Accepted' was not defined\n");

  // Trace to Follow
  if (isDefined(config, "Trace File") == true)
   traceFile = config["Trace File"].get<std::string>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Trace File' was not defined\n");

  // Trace tolerance
  if (isDefined(config, "Trace Tolerance") == true)
   traceTolerance = config["Trace Tolerance"].get<float>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Trace Tolerance' was not defined\n");

  // Enable B
  if (isDefined(config, "Enable B") == true)
   enableB = config["Enable B"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Enable B' was not defined\n");

  // Loading trace
  if (traceFile != "")
  {
   useTrace = true;
   std::string traceRaw;
   if (loadStringFromFile(traceRaw, traceFile.c_str()) == false) EXIT_WITH_ERROR("Could not find/read trace file: %s\n", traceFile.c_str());

   std::istringstream f(traceRaw);
   std::string line;
   while (std::getline(f, line))
   {
    auto coordinates = split(line, ' ');
    trace.push_back(std::make_pair(std::atof(coordinates[0].c_str()), std::atof(coordinates[1].c_str())));
   }
  }

  // Initialize derivative values
  updateDerivedValues();
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 _emu->advanceState(move); moves.push_back(move);
 updateDerivedValues();
 return moves;
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  // if in transition, take frame counter as hash value
//  if (*gameMode != 0) hash.Update(*frameCounter);

  hash.Update(*currentLevel);
  hash.Update(*ninjaLives);
  hash.Update(*ninjaFrame);
  hash.Update(*ninjaAnimation);
  hash.Update(*ninjaCurrentAction);
  hash.Update(*ninjaState);
  hash.Update(*ninjaAirMode);
  hash.Update(*ninjaWeapon);
  hash.Update(*ninjaPower);
  hash.Update(*ninjaPowerMax);
  hash.Update(*ninjaHP);
  hash.Update(*ninjaPosX);
  hash.Update(*ninjaPosXFrac);
  hash.Update(*ninjaSpeedX);
//  hash.Update(*ninjaSpeedXFrac);
  hash.Update(*ninjaPosY);
//  hash.Update(*ninjaPosYFrac);
  hash.Update(*ninjaSpeedY);
//  hash.Update(*ninjaSpeedYFrac);
  hash.Update(*bossHP);
  hash.Update(*screenScroll1);
  hash.Update(*screenScroll2);
  hash.Update(*screenScroll3);
  hash.Update(*gameMode);

//  hash.Update(*windTimer % 4);
  hash.Update(*windCycle);

  hash.Update(*heartState);
  if (*heartState == 128) hash.Update(*headHP);
  if (*heartState == 162) hash.Update(*heartTimer);

//  hash.Update(*lastInputTime);

  hash.Update(*cloneOffset);
//  hash.Update(clonePosXArray, 0x40);
//  hash.Update(clonePosYArray, 0x40);
//  hash.Update(cloneStateArray, 0x40);
  hash.Update(collisionFlags, 0x18);
  hash.Update(collisionFlags, 0x01);

  // Hashing Active Objects
  hash.Update(ObjectActivationFlags);

  // Special Weapon Symbol Information
  for (uint8_t i = 0; i < PROJECTILE_COUNT; i++) if (ObjectActivationFlags[PROJECTILE_OFFSET+i] == 1)
  {
    hash.Update(*(ninjaState+PROJECTILE_OFFSET+i));
//    hash.Update(*(ninjaPosX+PROJECTILE_OFFSET+i));
//    hash.Update(*(ninjaPosY+PROJECTILE_OFFSET+i));
  }

  // Clone Information
  for (uint8_t i = 0; i < CLONE_COUNT; i++) if (ObjectActivationFlags[CLONE_OFFSET+i] == 1)
  {
    hash.Update(*(ninjaState+CLONE_OFFSET+i));
//    hash.Update(*(ninjaPosX+CLONE_OFFSET+i));
//    hash.Update(*(ninjaPosY+CLONE_OFFSET+i));
  }

  // Orb Information
  for (uint8_t i = 0; i < ORB_COUNT; i++) if (ObjectActivationFlags[ORB_OFFSET+i] == 1)
  {
    hash.Update(*(ninjaState+ORB_OFFSET+i));
//    hash.Update(*(ninjaPosX+ORB_OFFSET+i));
//    hash.Update(*(ninjaPosY+ORB_OFFSET+i));
  }

  // Enemy Information
  for (uint8_t i = 0; i < ENEMY_COUNT; i++)
  {
    hash.Update(*(ninjaState+ENEMY_OFFSET+i));
//    hash.Update(*(ninjaPosX+ENEMY_OFFSET+i));
//    hash.Update(*(ninjaPosXFrac+ENEMY_OFFSET+i));
//    hash.Update(*(ninjaPosY+ENEMY_OFFSET+i));
//    hash.Update(*(ninjaPosYFrac+ENEMY_OFFSET+i));
  }

  // Hashing Invincivility
  if (*ninjaInvincibilityTimer == 0) hash.Update(0);
  if (*ninjaInvincibilityTimer == 128) hash.Update(1);
  if (*ninjaInvincibilityTimer > 0 && *ninjaInvincibilityTimer < 128) hash.Update(2);

  // Adding time tolerance
  if (timerTolerance > 0) hash.Update(*frameCounter % (timerTolerance+1));

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

void GameInstance::updateDerivedValues()
{
 double _screenScroll3 = (double)*screenScroll3;
 double _screenScroll2 = (double)*screenScroll2;
 double _screenScroll1 = (double)*screenScroll1;
 absolutePosX = _screenScroll3 * 256.0 + (_screenScroll2 + (double)*ninjaPosX) + (_screenScroll1 + (double)*ninjaPosXFrac)/ 256.0;

 // Object Activation Bits
 for (uint8_t i = 0; i < 24; i++) ObjectActivationFlags[i] = 0;

 for (uint8_t i = 0; i < 8; i++)
  if (((i == 7) && (*ObjectActivationBits1 & 0b10000000)) ||
      ((i == 6) && (*ObjectActivationBits1 & 0b01000000)) ||
      ((i == 5) && (*ObjectActivationBits1 & 0b00100000)) ||
      ((i == 4) && (*ObjectActivationBits1 & 0b00010000)) ||
      ((i == 3) && (*ObjectActivationBits1 & 0b00001000)) ||
      ((i == 2) && (*ObjectActivationBits1 & 0b00000100)) ||
      ((i == 1) && (*ObjectActivationBits1 & 0b00000010)) ||
      ((i == 0) && (*ObjectActivationBits1 & 0b00000001)))
      ObjectActivationFlags[i+0] = 1;

 for (uint8_t i = 0; i < 8; i++)
  if (((i == 7) && (*ObjectActivationBits2 & 0b10000000)) ||
      ((i == 6) && (*ObjectActivationBits2 & 0b01000000)) ||
      ((i == 5) && (*ObjectActivationBits2 & 0b00100000)) ||
      ((i == 4) && (*ObjectActivationBits2 & 0b00010000)) ||
      ((i == 3) && (*ObjectActivationBits2 & 0b00001000)) ||
      ((i == 2) && (*ObjectActivationBits2 & 0b00000100)) ||
      ((i == 1) && (*ObjectActivationBits2 & 0b00000010)) ||
      ((i == 0) && (*ObjectActivationBits2 & 0b00000001)))
      ObjectActivationFlags[i+8] = 1;

 for (uint8_t i = 0; i < 8; i++)
  if (((i == 7) && (*ObjectActivationBits3 & 0b10000000)) ||
      ((i == 6) && (*ObjectActivationBits3 & 0b01000000)) ||
      ((i == 5) && (*ObjectActivationBits3 & 0b00100000)) ||
      ((i == 4) && (*ObjectActivationBits3 & 0b00010000)) ||
      ((i == 3) && (*ObjectActivationBits3 & 0b00001000)) ||
      ((i == 2) && (*ObjectActivationBits3 & 0b00000100)) ||
      ((i == 1) && (*ObjectActivationBits3 & 0b00000010)) ||
      ((i == 0) && (*ObjectActivationBits3 & 0b00000001)))
      ObjectActivationFlags[i+16] = 1;

 int bossIdx = 23;
 double _ninjaPosX = 256.0*((double)*ninjaPosX) + (double)*ninjaPosXFrac;
 double _bossPosX = 256.0*((double)*(ninjaPosX+bossIdx)) + (double)*(ninjaPosXFrac+bossIdx);
 double _ninjaPosY = 256.0*((double)*ninjaPosY);
 double _bossPosY = 256.0*((double)*(ninjaPosY+bossIdx));
 ninjaBossDistance = (std::abs(_ninjaPosX - _bossPosX) + std::abs(_ninjaPosY - _bossPosY))/256.0f;

 if (*lastInputKey != 0) *lastInputTime = *frameCounter;

 // Calculating trace position
 float minDistance = std::numeric_limits<float>::infinity();
 for (size_t i = 0; i < trace.size(); i++)
 {
  float tracePointDistance =  std::abs(absolutePosX - trace[i].first) + std::abs((float)*ninjaPosY - trace[i].second);
  if (tracePointDistance < minDistance) { minDistance = tracePointDistance; tracePos = i; }
 }
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

// if (lastInputKeyAccepted != 0) if (*frameCounter > lastInputKeyAccepted && *frameCounter < lastInputKeyAccepted + 50) return moveList;

// bool foundAction = false;

// // First pass stage 00
// if (*ninjaCurrentAction == 0x0000)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", "RL.....A", ".LDU...A"}); }
// if (*ninjaCurrentAction == 0x0001)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU...."}); }
// if (*ninjaCurrentAction == 0x0002)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", "RL.....A"}); }
// if (*ninjaCurrentAction == 0x0003)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", ".RDU...."}); }
// if (*ninjaCurrentAction == 0x0004)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", ".RDU...."}); }
// if (*ninjaCurrentAction == 0x0005)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", ".RDU...."}); }
// if (*ninjaCurrentAction == 0x0006)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", ".L......", "R.......", ".L.....A", "R......A", "RL......"}); }
// if (*ninjaCurrentAction == 0x0009)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", ".L.....A", "R......A"}); }
// if (*ninjaCurrentAction == 0x000C)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "...U....", "..D.....", ".L......", ".L.....A", "R.......", "R......A", "RL......"}); }
// if (*ninjaCurrentAction == 0x000D)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", ".L.....A", "R.......", "R......A", "RL......", "RL.....A", ".LDU....", "R.DU...."}); }
// if (*ninjaCurrentAction == 0x000E)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", ".L.....A", "R.......", "R......A", "RL......", "RL.....A", ".LDU....", ".RDU...."}); }
// if (*ninjaCurrentAction == 0x000F)  { foundAction = true; moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", "RL.....A", ".LDU..B.", "R.DU..B."}); }
// if (*ninjaCurrentAction == 0x0010)  { foundAction = true; moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", "RL.....A", "RL....B."}); }
// if (*ninjaCurrentAction == 0x0011)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", "RL.....A", "RL....B."}); }
// if (*ninjaCurrentAction == 0x0012)  { foundAction = true; moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", "RL.....A"}); }
// if (*ninjaCurrentAction == 0x0013)  { foundAction = true; moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L.....A", "R......A", "RL.....A"}); }
// if (*ninjaCurrentAction == 0x0014)  { foundAction = true; moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "RL......", "RL.....A"}); }
// if (*ninjaCurrentAction == 0x0015)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", ".L......", ".L.....A", "R.......", "R......A", "RL......", ".LDU...A"}); }
// if (*ninjaCurrentAction == 0x0016)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "...U....", "..D.....", ".L......", ".L.....A", "R.......", "R......A", "RL......"}); }
// if (*ninjaCurrentAction == 0x001B)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R.......", "R......A", "R.....B.", "R.D.....", "RL......", ".LDU....", "RL.....A", "RL....B."}); }
// if (*ninjaCurrentAction == 0x001F)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "...U....", "..D.....", ".L......", ".L.....A", "R.......", "R......A", "RL......", "RL.....A", ".LDU....", "R.DU...."}); }
// if (*ninjaCurrentAction == 0x0020)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", ".L......", ".L.....A", "R.......", "R......A", "RL......"}); }
// if (*ninjaCurrentAction == 0x0021)  { foundAction = true; moveList.insert(moveList.end(), { ".......A", "...U....", "..D.....", ".L......", ".L.....A", "R.......", "R......A", "RL......"}); }
//
//  moveList.insert(moveList.end(), { ".......A", "......B.", "...U..B.", "..D.....", "..D...B.", ".L......", ".L.....A", ".L....B.", ".LDU....", "R.......", "R......A", "R.....B.", "RL....B.", "RL.....A", ".RDU....", "RL......", ".LDU..B.", "R.DU..B.", ".LDU....A", "R.DU...A"});

// if (*ninjaAnimation == 0x0000) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", "R......A", "R.....B.", "R.D.....", "R.......", "RL......", "RL.....A", ".LDU...."});
// if (*ninjaAnimation == 0x0001) moveList.insert(moveList.end(), { ".......A", "......B.", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", "R......A", "R.D.....", "R.......", "RL......", ".LDU....", ".L....B.", "R.....B."});
// if (*ninjaAnimation == 0x0002) moveList.insert(moveList.end(), { ".......A", "......B.", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", "R......A", "R.D.....", "R.......", "RL......", ".LDU...."});
// if (*ninjaAnimation == 0x0003) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.D.....", "R.......", "RL......", "RL.....A", ".LDU...."});
// if (*ninjaAnimation == 0x0004) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", "R......A", "R.....B."});
// if (*ninjaAnimation == 0x0005) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", "...U..B.", ".L.....A", "R......A", "R.....B.", "R.D.....", "R.......", "RL......"});
// if (*ninjaAnimation == 0x0006) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.D.....", "R.......", "RL......", ".LDU...."});
// if (*ninjaAnimation == 0x0007) moveList.insert(moveList.end(), { ".......A", "...U....", "......B.", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.D.....", "R.......", "RL......", ".LDU....", "RL.....A"});
// if (*ninjaAnimation == 0x0008) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", "R......A", "R.....B.", "R.D.....", "R.......", "RL......", "RL.....A", ".LDU...."});
// if (*ninjaAnimation == 0x0009) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..D...B.", ".L.....A", "R......A", "R.....B.", "R.D.....", "R.......", "RL......", "RL.....A", ".LDU...."});
// if (*ninjaAnimation == 0x000A) moveList.insert(moveList.end(), { ".......A", ".L.....A", "R......A"});
// if (*ninjaAnimation == 0x000B) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", ".L......", ".L.....A", ".L....B.", "R......A", "R.....B.", "R.......", "RL......"});
// if (*ninjaAnimation == 0x000E) moveList.insert(moveList.end(), { ".......A", ".L......", ".L.....A", "R......A", "R.......", "RL......"});
// if (*ninjaAnimation == 0x000F) moveList.insert(moveList.end(), { ".......A", "...U....", "..D.....", ".L......", ".L.....A", "R......A", "R.......", "RL......", ".LDU...."});
//
// if (*ninjaAnimation == 0x0000) moveList.insert(moveList.end(), { ".LDU..B.", "RLDU...A"});
// if (*ninjaAnimation == 0x0003) moveList.insert(moveList.end(), { "RLDU...A"});
// if (*ninjaAnimation == 0x0004) moveList.insert(moveList.end(), { ".L.....A", ".L....B.", "RL.....A"});
// if (*ninjaAnimation == 0x0005) moveList.insert(moveList.end(), { ".LD....A", ".LD...B.", "RL.U...A"});
// if (*ninjaAnimation == 0x0006) moveList.insert(moveList.end(), { "RLDU...A", "RL.....A", "R......A"});
// if (*ninjaAnimation == 0x0007) moveList.insert(moveList.end(), { "RLDU...A"});
// if (*ninjaAnimation == 0x000B) moveList.insert(moveList.end(), { "RLD....A", "RL.....A"});
//
// if (*ninjaAnimation == 0x0001) moveList.insert(moveList.end(), { ".L.U..BA", "R.DU..BA"});
// if (*ninjaAnimation == 0x0002) moveList.insert(moveList.end(), { ".L.U..BA", "R.DU..BA"});

 // Pacifist

 if (*ninjaCurrentAction == 0x0000) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x0001) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "UDL" });
 if (*ninjaCurrentAction == 0x0002) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x0003) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "UDL" });
 if (*ninjaCurrentAction == 0x0004) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "UDL" });
 if (*ninjaCurrentAction == 0x0005) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "UDL" });
 if (*ninjaCurrentAction == 0x000F) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x0010) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x0011) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x0012) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x0013) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x0014) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR",  "LRA", "UDL" });
 if (*ninjaCurrentAction == 0x001B) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LR", "LRA", "UDL" });

 if (enableB == true)
 {
  if (*ninjaCurrentAction == 0x0000) moveList.insert(moveList.end(), { "B", "RB", "LB", "DB", "UB" });
  if (*ninjaCurrentAction == 0x0001) moveList.insert(moveList.end(), { "B", "DB", "UB" });
  if (*ninjaCurrentAction == 0x0002) moveList.insert(moveList.end(), { "B", "RB", "LB", "DB", "UB" });
  if (*ninjaCurrentAction == 0x0003) moveList.insert(moveList.end(), { "B", "DB", "UB" });
  if (*ninjaCurrentAction == 0x0004) moveList.insert(moveList.end(), { "B", "DB", "UB" });
  if (*ninjaCurrentAction == 0x0005) moveList.insert(moveList.end(), { "B", "DB", "UB" });
  if (*ninjaCurrentAction == 0x0006) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA" });
  if (*ninjaCurrentAction == 0x0007) moveList.insert(moveList.end(), { "A", "RA", "LA" });
  if (*ninjaCurrentAction == 0x0008) moveList.insert(moveList.end(), { "A", "RA", "LA" });
  if (*ninjaCurrentAction == 0x0009) moveList.insert(moveList.end(), { "A", "RA", "LA" });
  if (*ninjaCurrentAction == 0x000A) moveList.insert(moveList.end(), { "A", "RA", "LA" });
  if (*ninjaCurrentAction == 0x000B) moveList.insert(moveList.end(), { "A", "RA", "LA" });
  if (*ninjaCurrentAction == 0x000C) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA" });
  if (*ninjaCurrentAction == 0x000D) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "RB", "LA", "LB", "LRA" });
  if (*ninjaCurrentAction == 0x000E) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "RB", "LA", "LB" });
  if (*ninjaCurrentAction == 0x000F) moveList.insert(moveList.end(), { "B", "RB", "LB", "UB" });
  if (*ninjaCurrentAction == 0x0010) moveList.insert(moveList.end(), { "B", "RB", "LB", "UB" });
  if (*ninjaCurrentAction == 0x0011) moveList.insert(moveList.end(), { "B", "RB", "LB", "UB" });
  if (*ninjaCurrentAction == 0x0012) moveList.insert(moveList.end(), { "B", "RB", "LB", "UB" });
  if (*ninjaCurrentAction == 0x0013) moveList.insert(moveList.end(), { "B", "RB", "LB" });
  if (*ninjaCurrentAction == 0x0014) moveList.insert(moveList.end(), { "B", "RB", "LB", "UB" });
  if (*ninjaCurrentAction == 0x0015) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA" });
  if (*ninjaCurrentAction == 0x0016) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA" });
  if (*ninjaCurrentAction == 0x001B) moveList.insert(moveList.end(), { "B", "RB", "LB", "DB", "UB" });
  if (*ninjaCurrentAction == 0x001D) moveList.insert(moveList.end(), { "A", "RA", "LA" });
  if (*ninjaCurrentAction == 0x001E) moveList.insert(moveList.end(), { "A", "RA", "LA" });
  if (*ninjaCurrentAction == 0x001F) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "LRA" });
  if (*ninjaCurrentAction == 0x0020) moveList.insert(moveList.end(), { "A", "R", "L", "RA", "LA" });
  if (*ninjaCurrentAction == 0x0021) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA" });
 }

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
  // We calculate a different reward if this is a winning frame
  auto stateType = getStateType(rulesStatus);

  if (stateType == f_win) return (float)*ninjaHP;

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

  // Evaluating ninja magnet's reward on position X
  boundedValue = absolutePosX;
  boundedValue = std::min(boundedValue, magnets.ninjaHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.ninjaHorizontalMagnet.min);
  diff = std::abs(magnets.ninjaHorizontalMagnet.center - boundedValue);
  reward += magnets.ninjaHorizontalMagnet.intensity * -diff;

  // Evaluating ninja magnet's reward on position Y
  boundedValue = (float)*ninjaPosY;
  boundedValue = std::min(boundedValue, magnets.ninjaVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.ninjaVerticalMagnet.min);
  diff = std::abs(magnets.ninjaVerticalMagnet.center - boundedValue);
  reward += magnets.ninjaVerticalMagnet.intensity * -diff;

  // Evaluating ninja power magnet
  boundedValue = (float)*ninjaPower;
  boundedValue = std::min(boundedValue, magnets.ninjaPowerMagnet.max);
  boundedValue = std::max(boundedValue, magnets.ninjaPowerMagnet.min);
  diff = std::abs(magnets.ninjaPowerMagnet.center - boundedValue);
  reward += magnets.ninjaPowerMagnet.intensity * -diff;

  // Evaluating ninja HP magnet
  boundedValue = (float)*ninjaHP;
  boundedValue = std::min(boundedValue, magnets.ninjaHPMagnet.max);
  boundedValue = std::max(boundedValue, magnets.ninjaHPMagnet.min);
  diff = std::abs(magnets.ninjaHPMagnet.center - boundedValue);
  reward += magnets.ninjaHPMagnet.intensity * -diff;

  // Evaluating boss HP magnet
  boundedValue = (float)*bossHP;
  boundedValue = std::min(boundedValue, magnets.bossHPMagnet.max);
  boundedValue = std::max(boundedValue, magnets.bossHPMagnet.min);
  diff = std::abs(magnets.bossHPMagnet.center - boundedValue);
  reward += magnets.bossHPMagnet.intensity * -diff;

  // Evaluating head HP magnet
  boundedValue = (float)*headHP;
  boundedValue = std::min(boundedValue, magnets.headHPMagnet.max);
  boundedValue = std::max(boundedValue, magnets.headHPMagnet.min);
  diff = std::abs(magnets.headHPMagnet.center - boundedValue);
  reward += magnets.headHPMagnet.intensity * -diff;

  // Evaluating ninja/boss distance magnet
  reward += magnets.ninjaBossDistanceMagnet * ninjaBossDistance;

  // Evaluating trace distance magnet
  reward += magnets.traceMagnet * (float)tracePos;

  // Returning reward
  return reward;
}



void GameInstance::printStateInfo(const bool* rulesStatus) const
{

 LOG("[Jaffar]  + Frame Counter:                     %02u (Tolerance: %02u)\n", *frameCounter, timerTolerance);
 LOG("[Jaffar]  + Last Input Time / Key:             %02u / %02u\n", *lastInputTime, *lastInputKey);
 LOG("[Jaffar]  + Reward:                            %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                              0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Current Level:                     %02u\n", *currentLevel);
 LOG("[Jaffar]  + Game Mode:                         %02u\n", *gameMode);
 LOG("[Jaffar]  + Ninja Animation / Action / State   0x%02X / 0x%02X / 0x%02X\n", *ninjaAnimation, *ninjaCurrentAction, *ninjaState);
 LOG("[Jaffar]  + Ninja Frame / Air Mode:            %02u / %02u\n", *ninjaFrame, *ninjaAirMode);
 LOG("[Jaffar]  + Ninja Lives:                       %02u\n", *ninjaLives);
 LOG("[Jaffar]  + Ninja Weapon:                      %02u\n", *ninjaWeapon);
 LOG("[Jaffar]  + Ninja Power:                       %02u (Max: %02u)\n", *ninjaPower, *ninjaPowerMax);
 LOG("[Jaffar]  + Ninja HP:                          %02u\n", *ninjaHP);
 LOG("[Jaffar]  + Ninja Position X:                  %f: %02u + %02u\n", absolutePosX, *ninjaPosX, *ninjaPosXFrac);
 LOG("[Jaffar]  + Ninja Position Y:                  %02u + %02u\n", *ninjaPosY, *ninjaPosYFrac);
 LOG("[Jaffar]  + Ninja Speed X:                     %02d + %02u\n", *ninjaSpeedX, *ninjaSpeedXFrac);
 LOG("[Jaffar]  + Ninja Speed Y:                     %02d + %02u\n", *ninjaSpeedY, *ninjaSpeedYFrac);
 LOG("[Jaffar]  + Screen Scroll:                     %02u + %02u + %02u\n", *screenScroll3, *screenScroll2, *screenScroll1);
 LOG("[Jaffar]  + Ninja Invincibility Timer:         %02u\n", *ninjaInvincibilityTimer);
 LOG("[Jaffar]  + Boss HP:                           %02u\n", *bossHP);
 LOG("[Jaffar]  + Head HP:                           %02u\n", *headHP);
 LOG("[Jaffar]  + Heart State / Timer:               %02u / %02u\n", *heartState, *heartTimer);
 LOG("[Jaffar]  + Ninja/Boss Distance:               %.3f\n", ninjaBossDistance);
 LOG("[Jaffar]  + Wind Cycle / Timer:                %02u / %02u\n", *windCycle, *windTimer);

 if (useTrace == true)
 {
  LOG("[Jaffar]  + Current Trace Pos:                %05lu / %05lu (%s)\n", tracePos, trace.size(), traceFile.c_str());
 }

 LOG("[Jaffar]  + Active Objects:\n");


 for (uint8_t i = 0; i < PROJECTILE_COUNT; i++) if (ObjectActivationFlags[PROJECTILE_OFFSET+i] == 1)
   LOG("[Jaffar]    + Projectile %02u - S:(0x%2X) - X(%03u, %03u) Y(%03u, %03u)\n", PROJECTILE_OFFSET+i, *(ninjaState+PROJECTILE_OFFSET+i), *(ninjaPosX+PROJECTILE_OFFSET+i), *(ninjaPosXFrac+PROJECTILE_OFFSET+i), *(ninjaPosY+PROJECTILE_OFFSET+i), *(ninjaPosYFrac+PROJECTILE_OFFSET+i));

 for (uint8_t i = 0; i < CLONE_COUNT; i++) if (ObjectActivationFlags[CLONE_OFFSET+i] == 1)
   LOG("[Jaffar]    + Clone      %02u - S:(0x%2X) - X(%03u, %03u) Y(%03u, %03u)\n", CLONE_OFFSET+i, *(ninjaState+CLONE_OFFSET+i), *(ninjaPosX+CLONE_OFFSET+i), *(ninjaPosXFrac+CLONE_OFFSET+i), *(ninjaPosY+CLONE_OFFSET+i), *(ninjaPosYFrac+CLONE_OFFSET+i));

 for (uint8_t i = 0; i < ORB_COUNT; i++) if (ObjectActivationFlags[ORB_OFFSET+i] == 1)
   LOG("[Jaffar]    + Orb        %02u - S:(0x%2X) - X(%03u, %03u) Y(%03u, %03u)\n", ORB_OFFSET+i, *(ninjaState+ORB_OFFSET+i), *(ninjaPosX+ORB_OFFSET+i), *(ninjaPosXFrac+ORB_OFFSET+i), *(ninjaPosY+ORB_OFFSET+i), *(ninjaPosYFrac+ORB_OFFSET+i));

 for (uint8_t i = 0; i < ENEMY_COUNT; i++) if (ObjectActivationFlags[ENEMY_OFFSET+i] == 1)
   LOG("[Jaffar]    + Enemy      %02u - S:(0x%2X) - X(%03u, %03u) Y(%03u, %03u)\n", ENEMY_OFFSET+i, *(ninjaState+ENEMY_OFFSET+i), *(ninjaPosX+ENEMY_OFFSET+i), *(ninjaPosXFrac+ENEMY_OFFSET+i), *(ninjaPosY+ENEMY_OFFSET+i), *(ninjaPosYFrac+ENEMY_OFFSET+i));


 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.ninjaHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Ninja Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaHorizontalMagnet.intensity, magnets.ninjaHorizontalMagnet.center, magnets.ninjaHorizontalMagnet.min, magnets.ninjaHorizontalMagnet.max);
 if (std::abs(magnets.ninjaVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Ninja Vertical Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaVerticalMagnet.intensity, magnets.ninjaVerticalMagnet.center, magnets.ninjaVerticalMagnet.min, magnets.ninjaVerticalMagnet.max);
 if (std::abs(magnets.ninjaPowerMagnet.intensity) > 0.0f)          LOG("[Jaffar]  + Ninja Power Magnet             - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaPowerMagnet.intensity, magnets.ninjaPowerMagnet.center, magnets.ninjaPowerMagnet.min, magnets.ninjaPowerMagnet.max);
 if (std::abs(magnets.ninjaHPMagnet.intensity) > 0.0f)             LOG("[Jaffar]  + Ninja HP Magnet                - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaHPMagnet.intensity, magnets.ninjaHPMagnet.center, magnets.ninjaHPMagnet.min, magnets.ninjaHPMagnet.max);
 if (std::abs(magnets.bossHPMagnet.intensity) > 0.0f)              LOG("[Jaffar]  + Boss HP Magnet                 - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossHPMagnet.intensity, magnets.bossHPMagnet.center, magnets.bossHPMagnet.min, magnets.bossHPMagnet.max);
 if (std::abs(magnets.headHPMagnet.intensity) > 0.0f)              LOG("[Jaffar]  + Head HP Magnet                 - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.headHPMagnet.intensity, magnets.headHPMagnet.center, magnets.headHPMagnet.min, magnets.headHPMagnet.max);
 if (std::abs(magnets.ninjaBossDistanceMagnet) > 0.0f)             LOG("[Jaffar]  + Ninja/Boss Distance Magnet     - Intensity: %.5f\n", magnets.ninjaBossDistanceMagnet);
 if (std::abs(magnets.traceMagnet) > 0.0f)                         LOG("[Jaffar]  + Trace Magnet                   - Intensity: %.5f\n", magnets.traceMagnet);
}

std::string GameInstance::getFrameTrace() const
{
 return std::to_string(absolutePosX) + std::string(" ") + std::to_string(float(*ninjaPosY)) + std::string("\n");
}

