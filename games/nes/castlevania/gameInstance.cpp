#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting relevant SMB pointers

  // Thanks to https://datacrystal.romhacking.net/wiki/Castlevania:RAM_map for helping me find some of these items

  gameMode               = (uint8_t*)   &_emu->_baseMem[0x0018];
  gameSubMode            = (uint8_t*)   &_emu->_baseMem[0x0019];
  stageTimer             = (uint16_t*)  &_emu->_baseMem[0x001A];
  currentStage           = (uint8_t*)   &_emu->_baseMem[0x0028];
  currentSubStage        = (uint8_t*)   &_emu->_baseMem[0x0046];
  simonLives             = (uint8_t*)   &_emu->_baseMem[0x002A];
  screenOffset           = (uint16_t*)  &_emu->_baseMem[0x002E];
  simonStairMode         = (uint8_t*)   &_emu->_baseMem[0x003E];
  simonPosY              = (uint8_t*)   &_emu->_baseMem[0x003F];
  simonPosX              = (uint16_t*)  &_emu->_baseMem[0x0040];
  simonHealth            = (uint8_t*)   &_emu->_baseMem[0x0045];
  simonInvulnerability   = (uint8_t*)   &_emu->_baseMem[0x005B];
  simonKneelingMode      = (uint8_t*)   &_emu->_baseMem[0x005F];
  subweaponShotCount     = (uint8_t*)   &_emu->_baseMem[0x0064];
  whipLength             = (uint8_t*)   &_emu->_baseMem[0x0070];
  simonHeartCount        = (uint8_t*)   &_emu->_baseMem[0x0071];
  simonImage             = (uint8_t*)   &_emu->_baseMem[0x0159];
  subweaponNumber        = (uint8_t*)   &_emu->_baseMem[0x015B];
  simonFacingDirection   = (uint8_t*)   &_emu->_baseMem[0x0450];
  simonState             = (uint8_t*)   &_emu->_baseMem[0x046C];
  simonSubState          = (uint8_t*)   &_emu->_baseMem[0x0584];
  simonVerticalSpeed     = (uint8_t*)   &_emu->_baseMem[0x04DC];
  simonVerticalDirection = (uint8_t*)   &_emu->_baseMem[0x0514];
  bossHealth             = (uint8_t*)   &_emu->_baseMem[0x01A9];
  bossPosX               = (uint8_t*)   &_emu->_baseMem[0x0393];
  bossPosY               = (uint8_t*)   &_emu->_baseMem[0x035B];
  bossIsActive           = (uint8_t*)   &_emu->_baseMem[0x0059];
  bossImage              = (uint8_t*)   &_emu->_baseMem[0x032D];
  bossState              = (uint8_t*)   &_emu->_baseMem[0x038F];
  grabItemTimer          = (uint8_t*)   &_emu->_baseMem[0x007E];
  enemy1PosX             = (uint8_t*)   &_emu->_baseMem[0x0390];
  enemy2PosX             = (uint8_t*)   &_emu->_baseMem[0x0391];
  enemy3PosX             = (uint8_t*)   &_emu->_baseMem[0x0392];
  enemy1State            = (uint8_t*)   &_emu->_baseMem[0x0438];
  enemy2State            = (uint8_t*)   &_emu->_baseMem[0x0439];
  enemy3State            = (uint8_t*)   &_emu->_baseMem[0x043A];
  freezeTimeTimer        = (uint8_t*)   &_emu->_baseMem[0x057C];
  batMedusa1PosX         = (uint8_t*)   &_emu->_baseMem[0x0395];
  batMedusa1PosY         = (uint8_t*)   &_emu->_baseMem[0x035D];
  itemDropCounter        = (uint8_t*)   &_emu->_baseMem[0x007B];
  RNGState               = (uint8_t*)   &_emu->_baseMem[0x006F];

  if (isDefined(config, "Hash Includes") == true)
   for (const auto& entry : config["Hash Includes"])
    hashIncludes.insert(entry.get<std::string>());
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Hash Includes' was not defined\n");
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  // Adding fixed hash elements
  hash.Update(*gameMode);
  hash.Update(*gameSubMode);
  hash.Update(*currentStage);
  hash.Update(*currentSubStage);
  hash.Update(*screenOffset);
  hash.Update(*simonStairMode);
  hash.Update(*simonPosY);
  hash.Update(*simonPosX);
  hash.Update(*simonKneelingMode);
  hash.Update(*subweaponShotCount);
  hash.Update(*whipLength);
  hash.Update(*simonImage);
  hash.Update(*simonFacingDirection);
  hash.Update(*simonState);
  hash.Update(*simonSubState);
  hash.Update(*simonVerticalSpeed);
  hash.Update(*simonVerticalDirection);
  hash.Update(*bossHealth);
  hash.Update(*bossIsActive);
  hash.Update(*grabItemTimer);
  hash.Update(*simonHealth);
  hash.Update(*simonHeartCount);
  hash.Update(*itemDropCounter);
  hash.Update(*bossImage);
  hash.Update(*bossState);

  // Conditional hashes
  if (hashIncludes.contains("Subweapon Number")) hash.Update(*subweaponNumber);
  if (hashIncludes.contains("Enemy 1 Pos X")) hash.Update(*enemy1PosX);
  if (hashIncludes.contains("Enemy 2 Pos X")) hash.Update(*enemy2PosX);
  if (hashIncludes.contains("Enemy 3 Pos X")) hash.Update(*enemy3PosX);
  if (hashIncludes.contains("Enemy 1 State")) hash.Update(*enemy1State);
  if (hashIncludes.contains("Enemy 2 State")) hash.Update(*enemy2State);
  if (hashIncludes.contains("Enemy 3 State")) hash.Update(*enemy3State);
  if (hashIncludes.contains("Bat / Medusa 1 Pos X")) hash.Update(*batMedusa1PosX);
  if (hashIncludes.contains("Bat / Medusa 1 Pos Y")) hash.Update(*batMedusa1PosY);

  // Turning invulnerability into a boolean
  hash.Update(*simonInvulnerability % 2);

  // Freeze timer is around 180 frames. To reduce exploration space, we only take either 0 or a few possibilities for positive.
  if (hashIncludes.contains("Freeze Time Timer"))
  {
   uint8_t value = *freezeTimeTimer;
   if (value > 0) value = (value % 30) + 1;
   hash.Update(value);
  }

  // Candelabra states go from 0x191 to 0x1A8
  if (hashIncludes.contains("Candelabra States")) for (size_t i = 0x0191; i < 0x1A8; i++) hash.Update(_emu->_baseMem[i]);

  // Only hash boss position if active
  if (*bossIsActive > 0)
  {
   hash.Update(*bossPosX);
   hash.Update(*bossPosY);
  }

  // Unused hashes
  //hash.Update(*simonLives);
  //hash.Update(*freezeTimeTimer);

  // If simon is getting knocked back or paralyzed, use timer
  if (*simonState == 0x05 || *simonState == 0x09) hash.Update(*stageTimer);

  // If simon is standing-attacking, use timer
  if (*simonState == 0x02) hash.Update(*stageTimer);

  // If we are in an animation, add timer to hash
  if (*gameMode == 0x08 || *gameMode == 0x0A) hash.Update(*stageTimer);
  if (*gameMode == 0x05 && *gameSubMode == 0x02) hash.Update(*stageTimer);

  // If grabbing item, add timer to hash
  if (*grabItemTimer > 0) hash.Update(*stageTimer);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 // Recalculating derived values
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*simonState == 0x00 && *simonSubState == 0x00) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x01) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x02) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U..B.", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU..B."});
 if (*simonState == 0x00 && *simonSubState == 0x04) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x08) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU..B."});
 if (*simonState == 0x00 && *simonSubState == 0x09) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x0A) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x0C) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x40) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD....A", "RLD...BA"});
 if (*simonState == 0x00 && *simonSubState == 0x41) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.....A", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD...B.", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x42) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x44) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x48) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.U...A", "RL.U..BA", "RLD....A", "RLD...BA"});
 if (*simonState == 0x00 && *simonSubState == 0x49) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.....A", "RL....BA", "RL.U...A", "RL.U..BA", "RLD.....", "RLD...B.", "RLDU....", "RLDU..B."});
 if (*simonState == 0x00 && *simonSubState == 0x4A) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x00 && *simonSubState == 0x4C) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x80) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x81) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL....B.", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x82) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL....B.", "RL....BA", "RL.U...A", "RLD....A", "RLD...BA", "RLDU...A"});
 if (*simonState == 0x00 && *simonSubState == 0x84) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x85) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU..B."});
 if (*simonState == 0x00 && *simonSubState == 0x86) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x88) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL....B.", "RL....BA", "RL.U..B.", "RL.U..BA", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x89) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x8A) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.U..B.", "RL.U..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x8C) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x8D) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0x8E) moveList.insert(moveList.end(), { "......B.", "...U..B.", "..D.....", "..D...B.", "..DU....", "..DU..B.", ".L....B.", ".L.U..B.", ".LD.....", ".LD...B.", ".LDU....", ".LDU..B.", "R.....B.", "R..U..B.", "R.D.....", "R.D...B.", "R.DU....", "R.DU..B."});
 if (*simonState == 0x00 && *simonSubState == 0xC0) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U...A", "RL.U..BA", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xC1) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xC2) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL.U..BA", "RLD....A", "RLD...BA", "RLDU...A", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xC4) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xC5) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xC6) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xC8) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xC9) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL....B.", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xCA) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RLD....A", "RLD...BA", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xCC) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xCD) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA", "RL......", "RL....B.", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x00 && *simonSubState == 0xCE) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", ".L......", ".L.....A", ".L....B.", ".L....BA", ".L.U....", ".L.U...A", ".L.U..B.", ".L.U..BA", ".LD.....", ".LD....A", ".LD...B.", ".LD...BA", ".LDU....", ".LDU...A", ".LDU..B.", ".LDU..BA", "R.......", "R......A", "R.....B.", "R.....BA", "R..U....", "R..U...A", "R..U..B.", "R..U..BA", "R.D.....", "R.D....A", "R.D...B.", "R.D...BA", "R.DU....", "R.DU...A", "R.DU..B.", "R.DU..BA"});
 if (*simonState == 0x01 && *simonSubState == 0x80) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x81) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x82) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x84) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x85) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x86) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x88) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x89) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x8A) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x8C) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x8D) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0x8E) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xC0) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xC1) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xC2) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......"});
 if (*simonState == 0x01 && *simonSubState == 0xC4) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xC5) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xC6) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......"});
 if (*simonState == 0x01 && *simonSubState == 0xC8) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xC9) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xCA) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......"});
 if (*simonState == 0x01 && *simonSubState == 0xCC) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xCD) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL....B.", "RL.U..B."});
 if (*simonState == 0x01 && *simonSubState == 0xCE) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......"});
 if (*simonState == 0x02 && *simonSubState == 0x00) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x04) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x08) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x0C) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x40) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x41) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x42) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x44) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x45) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x46) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x48) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x49) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x4A) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x4C) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x4D) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x4E) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x80) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x81) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x82) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x84) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x85) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x86) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x88) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x89) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x8A) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x8C) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x8D) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0x8E) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC0) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC1) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC2) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC4) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC5) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC6) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC8) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xC9) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xCA) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xCC) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xCD) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA", "RL......", "RL.....A", "RL....B.", "RL....BA", "RL.U....", "RL.U...A", "RL.U..B.", "RL.U..BA", "RLD.....", "RLD....A", "RLD...B.", "RLD...BA", "RLDU....", "RLDU...A", "RLDU..B.", "RLDU..BA"});
 if (*simonState == 0x02 && *simonSubState == 0xCE) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});
 if (*simonState == 0x03 && *simonSubState == 0x00) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x04) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RL......", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x05) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RL......", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x06) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RL......", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x08) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x0C) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RL......", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x0D) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RL......", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x0E) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RL......", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x40) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0x44) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0x45) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0x46) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0x48) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0x4C) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0x4D) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "RL......", "RLD....."});
 if (*simonState == 0x03 && *simonSubState == 0x4E) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", "R.......", "R.D.....", "R.D...B."});
 if (*simonState == 0x03 && *simonSubState == 0x80) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x84) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x85) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RL......", "RLD.....", "RLD...B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x86) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RLD.....", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x88) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x8C) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x8D) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RLDU..B."});
 if (*simonState == 0x03 && *simonSubState == 0x8E) moveList.insert(moveList.end(), { "..D.....", "..D...B.", "..DU..B.", ".L......", ".LD.....", ".LD...B.", ".LDU..B.", "R.......", "R.D.....", "R.D...B.", "R.DU..B.", "RLD...B."});
 if (*simonState == 0x03 && *simonSubState == 0xC0) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0xC4) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "R.D...B."});
 if (*simonState == 0x03 && *simonSubState == 0xC5) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "R.D...B."});
 if (*simonState == 0x03 && *simonSubState == 0xC6) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0xC8) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
 if (*simonState == 0x03 && *simonSubState == 0xCC) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "R.D...B."});
 if (*simonState == 0x03 && *simonSubState == 0xCD) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "RLD....."});
 if (*simonState == 0x03 && *simonSubState == 0xCE) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "R.D...B."});
 if (*simonState == 0x04 && *simonSubState == 0x00) moveList.insert(moveList.end(), { ".......A", "......B.", "...U..B.", ".L......", "R......."});
 if (*simonState == 0x04 && *simonSubState == 0x01) moveList.insert(moveList.end(), { ".......A", "......B.", "...U..B.", ".L......", "R.......", "RL......"});
 if (*simonState == 0x04 && *simonSubState == 0x02) moveList.insert(moveList.end(), { ".......A", "......B.", "...U..B.", ".L......", "R......."});
 if (*simonState == 0x04 && *simonSubState == 0x80) moveList.insert(moveList.end(), { ".......A", "......B.", "...U..B.", ".L......", "R......."});
 if (*simonState == 0x09 && *simonSubState == 0x01) moveList.insert(moveList.end(), { "......B.", "...U..B.", ".L......", ".L....B.", ".L.U..B."});
 if (*simonState == 0x09 && *simonSubState == 0x02) moveList.insert(moveList.end(), { "......B.", "...U..B.", "R.......", "R.....B.", "R..U..B."});
 if (*simonState == 0x09 && *simonSubState == 0x04) moveList.insert(moveList.end(), { "......B.", "...U..B.", ".L......", ".L....B.", ".L.U..B.", "R.......", "R.....B.", "R..U..B."});

 return moveList;
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;
 memset(&magnets, 0, sizeof(magnets));

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
    magnets = _rules[ruleId]->_magnets;

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

  // Evaluating simon magnet's reward on position X
  boundedValue = (float)*simonPosX;
  boundedValue = std::min(boundedValue, magnets.simonHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.simonHorizontalMagnet.min);
  diff = std::abs(magnets.simonHorizontalMagnet.center - boundedValue);
  reward += magnets.simonHorizontalMagnet.intensity * -diff;

  // Evaluating simon magnet's reward on position Y
  boundedValue = (float)*simonPosY;
  boundedValue = std::min(boundedValue, magnets.simonVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.simonVerticalMagnet.min);
  diff = std::abs(magnets.simonVerticalMagnet.center - boundedValue);
  reward += magnets.simonVerticalMagnet.intensity * -diff;

  // Evaluating bat/medusa horizontal magnet
  boundedValue = (float)*batMedusa1PosX;
  boundedValue = std::min(boundedValue, magnets.batMedusaHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.batMedusaHorizontalMagnet.min);
  diff = std::abs(magnets.batMedusaHorizontalMagnet.center - boundedValue);
  reward += magnets.batMedusaHorizontalMagnet.intensity * -diff;

  // Evaluating bat/medusa vertical magnet
  boundedValue = (float)*batMedusa1PosY;
  boundedValue = std::min(boundedValue, magnets.batMedusaVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.batMedusaVerticalMagnet.min);
  diff = std::abs(magnets.batMedusaVerticalMagnet.center - boundedValue);
  reward += magnets.batMedusaVerticalMagnet.intensity * -diff;

  // Evaluating simon magnet's reward on hearts
  boundedValue = (float)*simonHeartCount;
  boundedValue = std::min(boundedValue, magnets.simonHeartMagnet.max);
  boundedValue = std::max(boundedValue, magnets.simonHeartMagnet.min);
  diff = std::abs(magnets.simonHeartMagnet.center - boundedValue);
  reward += magnets.simonHeartMagnet.intensity * -diff;

  // Evaluating boss magnet's reward on position X
  boundedValue = (float)*bossPosX;
  boundedValue = std::min(boundedValue, magnets.bossHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.bossHorizontalMagnet.min);
  diff = std::abs(magnets.bossHorizontalMagnet.center - boundedValue);
  reward += magnets.bossHorizontalMagnet.intensity * -diff;

  // Evaluating boss magnet's reward on position Y
  boundedValue = (float)*bossPosY;
  boundedValue = std::min(boundedValue, magnets.bossVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.bossVerticalMagnet.min);
  diff = std::abs(magnets.bossVerticalMagnet.center - boundedValue);
  reward += magnets.bossVerticalMagnet.intensity * -diff;

  // Evaluating freeze time magnet
  reward += magnets.freezeTimeMagnet * *freezeTimeTimer;

  // Evaluating simon's stairs magnet
  if (magnets.simonStairMagnet.mode == *simonStairMode) reward += magnets.simonStairMagnet.reward;

  // Evaluating simon's weapon magnet
  if (magnets.simonWeaponMagnet.weapon == *subweaponNumber) reward += magnets.simonWeaponMagnet.reward;

  // Punish boss health
  reward -= 50.0f * *bossHealth;

  // Reward boss active
  reward += *bossIsActive;

  // Reward shot count
  reward += 10.0f * *subweaponShotCount;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
 *this->RNGState = (uint8_t) RNGState;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Current Stage:          %02u-%02u\n", *currentStage, *currentSubStage);
  LOG("[Jaffar]  + Simon State:            0x%02X-0x%02X\n", *simonState, *simonSubState);
  LOG("[Jaffar]  + Timer:                  %u\n", *stageTimer);
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
  LOG("[Jaffar]  + RNG State:              0x%X\n", *RNGState);
  LOG("[Jaffar]  + Game Mode:              %02u-%02u\n", *gameMode, *gameSubMode);
  LOG("[Jaffar]  + Screen Offset:          %04u\n", *screenOffset);
  LOG("[Jaffar]  + Simon Heart Count:      %02u\n", *simonHeartCount);
  LOG("[Jaffar]  + Simon Image:            %02u\n", *simonImage);
  LOG("[Jaffar]  + Simon Facing Direction: %02u\n", *simonFacingDirection);
  LOG("[Jaffar]  + Simon Vertical Speed:   %02u-%02u\n", *simonVerticalSpeed, *simonVerticalDirection);
  LOG("[Jaffar]  + Simon Lives:            %02u\n", *simonLives);
  LOG("[Jaffar]  + Simon Stair Mode:       %02u\n", *simonStairMode);
  LOG("[Jaffar]  + Simon Health:           %02u\n", *simonHealth);
  LOG("[Jaffar]  + Simon Pos X:            %04u (%02u %02u)\n", *simonPosX, *((uint8_t*)simonPosX), *(((uint8_t*)simonPosX)+1));
  LOG("[Jaffar]  + Simon Pos Y:            %04u\n", *simonPosY);
  LOG("[Jaffar]  + Simon Invulnerability:  %02u\n", *simonInvulnerability);
  LOG("[Jaffar]  + Simon Kneeling Mode:    %02u\n", *simonKneelingMode);
  LOG("[Jaffar]  + Subweapon Number:       %02u\n", *subweaponNumber);
  LOG("[Jaffar]  + Subweapon Shot Count:   %02u\n", *subweaponShotCount);
  LOG("[Jaffar]  + Whip Length:            %02u\n", *whipLength);
  LOG("[Jaffar]  + Boss Health:            %02u\n", *bossHealth);
  LOG("[Jaffar]  + Boss Pos X:             %04u\n", *bossPosX);
  LOG("[Jaffar]  + Boss Pos Y:             %02u\n", *bossPosY);
  LOG("[Jaffar]  + Boss Image / State:     %02u / %02u\n", *bossImage, *bossState);
  LOG("[Jaffar]  + Boss Is Active:         %02u\n", *bossIsActive);
  LOG("[Jaffar]  + Enemy State:            %02u, %02u, %02u\n", *enemy1State, *enemy2State, *enemy3State);
  LOG("[Jaffar]  + Enemy Pos X:            %02u, %02u, %02u\n", *enemy1PosX, *enemy2PosX, *enemy3PosX);
  LOG("[Jaffar]  + Bat / Medusa Pos X:     %02u\n", *batMedusa1PosX);
  LOG("[Jaffar]  + Bat / Medusa Pos Y:     %02u\n", *batMedusa1PosY);
  LOG("[Jaffar]  + Grab Item Timer:        %02u\n", *grabItemTimer);
  LOG("[Jaffar]  + Freeze Time Timer:      %02u\n", *freezeTimeTimer);
  LOG("[Jaffar]  + Item Drop Counter:      %02u\n", *itemDropCounter);

  LOG("[Jaffar]  + Candelabra States: ");
    for (size_t i = 0x0191; i < 0x1A8; i++) LOG("%d", _emu->_baseMem[i] > 0 ? 1 : 0);
  LOG("\n");

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  LOG("[Jaffar]  + Simon Horizontal Magnet        - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.simonHorizontalMagnet.intensity, magnets.simonHorizontalMagnet.center, magnets.simonHorizontalMagnet.min, magnets.simonHorizontalMagnet.max);
  LOG("[Jaffar]  + Simon Vertical Magnet          - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.simonVerticalMagnet.intensity, magnets.simonVerticalMagnet.center, magnets.simonVerticalMagnet.min, magnets.simonVerticalMagnet.max);
  LOG("[Jaffar]  + Simon Heart Magnet             - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.simonHeartMagnet.intensity, magnets.simonHeartMagnet.center, magnets.simonHeartMagnet.min, magnets.simonHeartMagnet.max);
  LOG("[Jaffar]  + Boss Horizontal Magnet         - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossHorizontalMagnet.intensity, magnets.bossHorizontalMagnet.center, magnets.bossHorizontalMagnet.min, magnets.bossHorizontalMagnet.max);
  LOG("[Jaffar]  + Boss Vertical Magnet           - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossVerticalMagnet.intensity, magnets.bossVerticalMagnet.center, magnets.bossVerticalMagnet.min, magnets.bossVerticalMagnet.max);
  LOG("[Jaffar]  + Bat / Medusa Horizontal Magnet - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.batMedusaHorizontalMagnet.intensity, magnets.batMedusaHorizontalMagnet.center, magnets.batMedusaHorizontalMagnet.min, magnets.batMedusaHorizontalMagnet.max);
  LOG("[Jaffar]  + Bat / Medusa Vertical Magnet   - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.batMedusaVerticalMagnet.intensity, magnets.batMedusaVerticalMagnet.center, magnets.batMedusaVerticalMagnet.min, magnets.batMedusaVerticalMagnet.max);
  LOG("[Jaffar]  + Freeze Time Magnet             - Intensity: %.1f\n", magnets.freezeTimeMagnet);
  LOG("[Jaffar]  + Simon Stairs Magnet            - Reward:    %.1f, Mode: %u\n", magnets.simonStairMagnet.reward, magnets.simonStairMagnet.mode);
  LOG("[Jaffar]  + Simon Weapon Magnet            - Reward:    %.1f, Weapon: %u\n", magnets.simonWeaponMagnet.reward, magnets.simonWeaponMagnet.weapon);
}
