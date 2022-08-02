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
  stageTimer             = (uint8_t*)   &_emu->_baseMem[0x001A];
  isLagFrame             = (uint8_t*)   &_emu->_baseMem[0x001B];
  isPaused               = (uint8_t*)   &_emu->_baseMem[0x0022];
  levelTransitionTimer   = (uint8_t*)   &_emu->_baseMem[0x001D];
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
  subweaponHitCount      = (uint8_t*)   &_emu->_baseMem[0x0079];
  whipLength             = (uint8_t*)   &_emu->_baseMem[0x0070];
  simonHeartCount        = (uint8_t*)   &_emu->_baseMem[0x0071];
  simonImage             = (uint8_t*)   &_emu->_baseMem[0x0159];
  subweaponNumber        = (uint8_t*)   &_emu->_baseMem[0x015B];
  simonFacingDirection   = (uint8_t*)   &_emu->_baseMem[0x0450];
  simonState             = (uint8_t*)   &_emu->_baseMem[0x046C];
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
  enemy4PosX             = (uint8_t*)   &_emu->_baseMem[0x0393];
  enemy5PosX             = (uint8_t*)   &_emu->_baseMem[0x0394];
  enemy6PosX             = (uint8_t*)   &_emu->_baseMem[0x0395];
  enemy7PosX             = (uint8_t*)   &_emu->_baseMem[0x0396];
  enemy0State            = (uint8_t*)   &_emu->_baseMem[0x0435];
  enemy1State            = (uint8_t*)   &_emu->_baseMem[0x0436];
  enemy2State            = (uint8_t*)   &_emu->_baseMem[0x0437];
  enemy3State            = (uint8_t*)   &_emu->_baseMem[0x0438];
  enemy4State            = (uint8_t*)   &_emu->_baseMem[0x0439];
  enemy5State            = (uint8_t*)   &_emu->_baseMem[0x043A];
  enemy6State            = (uint8_t*)   &_emu->_baseMem[0x043B];
  enemy7State            = (uint8_t*)   &_emu->_baseMem[0x043C];
  freezeTimeTimer        = (uint8_t*)   &_emu->_baseMem[0x057C];
  batMedusa1State        = (uint8_t*)   &_emu->_baseMem[0x04C9];
  batMedusa1PosX         = (uint8_t*)   &_emu->_baseMem[0x0395];
  batMedusa1PosY         = (uint8_t*)   &_emu->_baseMem[0x035D];
  batMedusa2State        = (uint8_t*)   &_emu->_baseMem[0x04CA];
  batMedusa2PosX         = (uint8_t*)   &_emu->_baseMem[0x0396];
  batMedusa2PosY         = (uint8_t*)   &_emu->_baseMem[0x035E];
  batMedusa3State        = (uint8_t*)   &_emu->_baseMem[0x04CB];
  batMedusa3PosX         = (uint8_t*)   &_emu->_baseMem[0x0397];
  batMedusa3PosY         = (uint8_t*)   &_emu->_baseMem[0x035F];
  batMedusa4State        = (uint8_t*)   &_emu->_baseMem[0x04CC];
  batMedusa5State        = (uint8_t*)   &_emu->_baseMem[0x04CD];
  itemDropCounter        = (uint8_t*)   &_emu->_baseMem[0x007B];
  RNGState               = (uint8_t*)   &_emu->_baseMem[0x006F];
  stairAnimationFrame    = (uint8_t*)   &_emu->_baseMem[0x0370];
  bossStateTimer         = (uint8_t*)   &_emu->_baseMem[0x0553];
  simonScreenOffsetX     = (uint8_t*)   &_emu->_baseMem[0x038C];
  screenMotionX          = (uint8_t*)   &_emu->_baseMem[0x0030];
  mummy2PosX             = (uint8_t*)   &_emu->_baseMem[0x0394];

  skeletonPosX           = (uint8_t*)   &_emu->_baseMem[0x0393];
  skeletonPosX2          = (uint8_t*)   &_emu->_baseMem[0x0394];
  skeletonBone1PosY      = (uint8_t*)   &_emu->_baseMem[0x0360];
  skeletonBone2PosY      = (uint8_t*)   &_emu->_baseMem[0x0361];
  skeletonBone3PosY      = (uint8_t*)   &_emu->_baseMem[0x0362];

  subweapon1PosX      = (uint8_t*)   &_emu->_baseMem[0x03A0];
  subweapon2PosX      = (uint8_t*)   &_emu->_baseMem[0x03A1];
  subweapon3PosX      = (uint8_t*)   &_emu->_baseMem[0x03A2];
  subweapon1PosY      = (uint8_t*)   &_emu->_baseMem[0x0368];
  subweapon2PosY      = (uint8_t*)   &_emu->_baseMem[0x0369];
  subweapon3PosY      = (uint8_t*)   &_emu->_baseMem[0x036A];

  subweapon1State     = (uint8_t*)   &_emu->_baseMem[0x0448];
  subweapon2State     = (uint8_t*)   &_emu->_baseMem[0x0449];
  subweapon3State     = (uint8_t*)   &_emu->_baseMem[0x044A];
  subweapon1Bounce    = (uint8_t*)   &_emu->_baseMem[0x0464];
  subweapon2Bounce    = (uint8_t*)   &_emu->_baseMem[0x0465];
  subweapon3Bounce    = (uint8_t*)   &_emu->_baseMem[0x0466];
  subweapon1Direction = (uint8_t*)   &_emu->_baseMem[0x0480];
  subweapon2Direction = (uint8_t*)   &_emu->_baseMem[0x0481];
  subweapon3Direction = (uint8_t*)   &_emu->_baseMem[0x0482];

  enemyStates = (uint8_t*)   &_emu->_baseMem[0x0434];

  enemy1HolyWaterLockState = (uint8_t*)   &_emu->_baseMem[0x056C];
  holyWaterFire1Timer      = (uint8_t*)   &_emu->_baseMem[0x057C];

  jumpingInertia      = (uint8_t*)   &_emu->_baseMem[0x0584];

  if (isDefined(config, "Hash Includes") == true)
   for (const auto& entry : config["Hash Includes"])
    hashIncludes.insert(entry.get<std::string>());
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Hash Includes' was not defined\n");

  // Hash for tiles
  if (isDefined(config, "Tile Hash Includes") == true)
   for (const auto& entry : config["Tile Hash Includes"])
    {
     uint16_t tilePosNum = (uint16_t)std::stoul(entry.get<std::string>(), 0, 16);
     tileHashIncludes.push_back(tilePosNum);
    }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Tile Hash Includes' was not defined\n");

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Enabling Pause
  if (isDefined(config, "Enable Pause") == true)
   enablePause = config["Enable Pause"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Enable Pause' was not defined\n");

  if (isDefined(config, "Allow B") == true)
   allowB = config["Allow B"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Allow B' was not defined\n");


  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  // Adding fixed hash elements
  hash.Update(*isLagFrame);
  hash.Update(*isPaused);
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
  hash.Update(*subweaponHitCount);
  hash.Update(*whipLength);
  hash.Update(*simonImage);
  hash.Update(*simonFacingDirection);
  hash.Update(*simonState);
  hash.Update(*simonVerticalSpeed);
  hash.Update(*simonVerticalDirection);
  hash.Update(*bossHealth);
  hash.Update(*bossIsActive);
  hash.Update(*grabItemTimer);
  hash.Update(*simonHealth);
  hash.Update(*itemDropCounter);
  hash.Update(*bossImage);
  hash.Update(*bossState);
  hash.Update(*stairAnimationFrame);
  hash.Update(*bossStateTimer);
  hash.Update(*simonScreenOffsetX);
  hash.Update(*levelTransitionTimer);
  hash.Update(*screenMotionX);
//  hash.Update(*jumpingInertia);

  // Using stage timer to allow pauses
  hash.Update(*stageTimer % timerTolerance);

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
  if (hashIncludes.contains("Bat / Medusa 2 Pos X")) hash.Update(*batMedusa2PosX);
  if (hashIncludes.contains("Bat / Medusa 2 Pos Y")) hash.Update(*batMedusa2PosY);
  if (hashIncludes.contains("Bat / Medusa 3 Pos X")) hash.Update(*batMedusa3PosX);
  if (hashIncludes.contains("Bat / Medusa 3 Pos Y")) hash.Update(*batMedusa3PosY);
  if (hashIncludes.contains("Enemy 1 Holy Water Lock State")) hash.Update(*enemy1HolyWaterLockState);
  if (hashIncludes.contains("Holy Water Fire 1 Timer")) hash.Update(*holyWaterFire1Timer);
  if (hashIncludes.contains("Skeleton Position X")) hash.Update(*skeletonPosX);
  if (hashIncludes.contains("Skeleton Position X2")) hash.Update(*skeletonPosX2);
  if (hashIncludes.contains("Skeleton Bone 1 Pos Y")) hash.Update(*skeletonBone1PosY);
  if (hashIncludes.contains("Skeleton Bone 2 Pos Y")) hash.Update(*skeletonBone2PosY);
  if (hashIncludes.contains("Skeleton Bone 3 Pos Y")) hash.Update(*skeletonBone3PosY);
  if (hashIncludes.contains("Subweapon 1 Position X")) hash.Update(*subweapon1PosX);
  if (hashIncludes.contains("Subweapon 2 Position X")) hash.Update(*subweapon2PosX);
  if (hashIncludes.contains("Subweapon 3 Position X")) hash.Update(*subweapon3PosX);
  if (hashIncludes.contains("Subweapon 1 Position Y")) hash.Update(*subweapon1PosY);
  if (hashIncludes.contains("Subweapon 2 Position Y")) hash.Update(*subweapon2PosY);
  if (hashIncludes.contains("Subweapon 3 Position Y")) hash.Update(*subweapon3PosY);

//  hash.Update(*enemy0State);
//  hash.Update(*enemy1State);
//  hash.Update(*enemy2State);
//  hash.Update(*enemy3State);
//  hash.Update(*enemy4State);
//  hash.Update(*enemy5State);
//  hash.Update(*batMedusa1State);
//  hash.Update(*batMedusa2State);
//  hash.Update(*batMedusa3State);
//  hash.Update(*batMedusa4State);
//  hash.Update(*batMedusa5State);

//  hash.Update(*subweapon1State);
//  hash.Update(*subweapon2State);
//  hash.Update(*subweapon3State);
//  hash.Update(*subweapon1Bounce);
//  hash.Update(*subweapon2Bounce);
//  hash.Update(*subweapon3Bounce);
//  hash.Update(*subweapon1Direction);
//  hash.Update(*subweapon2Direction);
//  hash.Update(*subweapon3Direction);

  // All object info
  if (hashIncludes.contains("Detailed Enemy State")) hash.Update(&_emu->_baseMem[0x354], 0x200);

  // Updating nametable
  if (hashIncludes.contains("Full NES Nametable")) hash.Update(_emu->_ppuNameTableMem, 0x1000);
  for (const auto& tilePos : tileHashIncludes ) hash.Update(_emu->_ppuNameTableMem[tilePos]);

  // Turning invulnerability into a boolean
  hash.Update(*simonInvulnerability > 0);

  if (hashIncludes.contains("Simon Heart Count"))  hash.Update(*simonHeartCount);

  // Freeze timer is around 180 frames. To reduce exploration space, we only take whether it's active or not
  if (hashIncludes.contains("Freeze Time Timer"))  hash.Update(*freezeTimeTimer == 0 ? 0 : (*freezeTimeTimer % 4) + 1);

  // Candelabra states go from 0x191 to 0x1A8
  if (hashIncludes.contains("Candelabra States")) for (size_t i = 0x0191; i < 0x1A8; i++) hash.Update(_emu->_baseMem[i]);

  // Only hash boss position if active
  if (*bossIsActive > 0)
  {
   hash.Update(*bossPosX);
   hash.Update(*bossPosY);
   hash.Update(*mummy2PosX);
  }

  // Only compute timer if not paused
  if (isPaused == 0)
  {
   // If simon is getting knocked back or paralyzed, use timer
   if (*simonState == 0x05 || *simonState == 0x09) hash.Update(*stageTimer);

   // If simon is standing-attacking, use timer
   if (*simonState == 0x02) hash.Update(*stageTimer);

   // If we are in an animation, add timer to hash
   if (*gameMode == 0x08 || *gameMode == 0x0A) hash.Update(*stageTimer);
   if (*gameMode == 0x05 && *gameSubMode == 0x02) hash.Update(*stageTimer);
   if (*gameSubMode != 0x06) hash.Update(*stageTimer);

   // If grabbing item, add timer to hash
   if (*grabItemTimer > 0) hash.Update(*stageTimer);
  }

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 int simonPosXInt = (uint8_t)*simonPosX;
 int simonPosYInt = *simonPosY;
 int bossPosXInt = *bossPosX;
 int bossPosYInt = *bossPosY;
 int mummy2PosXInt = *mummy2PosX;
 bossSimonDistance = std::abs(simonPosXInt - bossPosXInt) + std::abs(simonPosYInt - bossPosYInt);
 mummiesDistance = std::abs(bossPosXInt - mummy2PosXInt);

 simonRelativePosX = *((uint8_t*)simonPosX);
 batMedusa1AbsolutePosX = *screenOffset - 128 + *batMedusa1PosX;

 // Advancing useless frames
 uint16_t advanceCounter = 0;

 int weapon1PosXInt = *subweapon1PosX;
 int weapon2PosXInt = *subweapon2PosX;
 bossWeaponDistance = (*subweapon1State == 0 ? 255 : std::abs(weapon1PosXInt - bossPosXInt)) + (subweapon2State == 0 ? 255 : std::abs(weapon2PosXInt - bossPosXInt));

 //while ( (advanceCounter < 1024) && (*gameMode == 8) ) { _emu->advanceState(0); advanceCounter++; }

 isCandelabrumBroken = 0;
 for (size_t i = 0x0191; i < 0x1A8; i++) if (_emu->_baseMem[i] > 0) isCandelabrumBroken = 1;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 // If pause enabled, add it to the possible movement list
 if (enablePause) moveList.push_back("S");

 if (allowB == true)
 {

  // Pass 1 - Stage 3-0
  if (*simonState == 0x00) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", "..DU....", ".L......", ".L.....A", ".L....B.", ".L.U....", ".L.U...A", ".LD.....", ".LD....A", ".LD...B.", ".LDU....", "R.......", "R......A", "R.....B.", "R..U....", "R..U...A", "R..U..B.", "R.D.....", "R.D....A", "R.D...B.", "R.DU....", "RL.U....", "RL.U...A", "RLD.....", "RLD....A", "RLD...B.", "RL.U..B."});
  if (*simonState == 0x01) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL.U..B."});
  if (*simonState == 0x03) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "R.D...B."});
  if (*simonState == 0x04) moveList.insert(moveList.end(), { "......B.", ".L......", "R......."});
  if (*simonState == 0x09) moveList.insert(moveList.end(), { "......B.", ".L......", ".L....B.", "R.......", "R.....B."});

  // Pass 2 - Stage 1-0
  if (*simonState == 0x01) moveList.insert(moveList.end(), { "RL....B."});
  if (*simonState == 0x03) moveList.insert(moveList.end(), { "..DU..BA", ".LDU..B.", "R.DU..B."});
  if (*simonState == 0x03) moveList.insert(moveList.end(), { "RLDU..B."});
  if (*simonState == 0x04) moveList.insert(moveList.end(), { "...U..B."});

  // Pass 3 - Stage 4-0

  if (*simonState == 0x09) moveList.insert(moveList.end(), { ".L.U..B.", "R..U..B."});

  // Pass 4 - Stage 14-1

  if (*simonState == 0x02) moveList.insert(moveList.end(), { ".......A", "......B.", "......BA", "...U....", "...U...A", "...U..B.", "...U..BA", "..D.....", "..D....A", "..D...B.", "..D...BA", "..DU....", "..DU...A", "..DU..B.", "..DU..BA"});

  if (*simonState == 0x0000) moveList.insert(moveList.end(), { ".LDU..B.", "RLDU..B."});
  if (*simonState == 0x0001) moveList.insert(moveList.end(), { ".LDU..B."});
  if (*simonState == 0x0007) moveList.insert(moveList.end(), { "RL......"});
 }

  /// PACIFIST

 if (allowB == false)
 {
   // Pass 1 - Stage 3-0
   if (*simonState == 0x00) moveList.insert(moveList.end(), { ".......A", "...U....", "..D.....", "..DU....", ".L......", ".L.....A", ".L.U....", ".L.U...A", ".LD.....", ".LD....A", ".LDU....", "R.......", "R......A", "R..U....", "R..U...A", "R.D.....", "R.D....A", "R.DU....", "RL.U....", "RL.U...A", "RLD.....", "RLD....A"});
   if (*simonState == 0x01) moveList.insert(moveList.end(), { "RL......"});
   if (*simonState == 0x02) moveList.insert(moveList.end(), { ".......A", "...U....", "...U...A", "..D.....", "..D....A", "..DU....", "..DU...A"});
   if (*simonState == 0x03) moveList.insert(moveList.end(), { "..D.....", ".L......", ".LD.....", "R.......", "R.D....."});
   if (*simonState == 0x04) moveList.insert(moveList.end(), { ".L......", "R......."});
   if (*simonState == 0x07) moveList.insert(moveList.end(), { "RL......"});
   if (*simonState == 0x09) moveList.insert(moveList.end(), { ".L......", "R......."});
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
  boundedValue = (float)batMedusa1AbsolutePosX;
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

  // Evaluating skeleton reward on pos x
  boundedValue = (float)*skeletonPosX2;
  boundedValue = std::min(boundedValue, magnets.skeletonHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.skeletonHorizontalMagnet.min);
  diff = std::abs(magnets.skeletonHorizontalMagnet.center - boundedValue);
  reward += magnets.skeletonHorizontalMagnet.intensity * -diff;

  // Evaluating subweapon hit count magnet
  boundedValue = (float)*subweaponHitCount;
  boundedValue = std::min(boundedValue, magnets.subweaponHitCountMagnet.max);
  boundedValue = std::max(boundedValue, magnets.subweaponHitCountMagnet.min);
  diff = std::abs(magnets.subweaponHitCountMagnet.center - boundedValue);
  reward += magnets.subweaponHitCountMagnet.intensity * -diff;

  // Evaluating freeze time magnet
  reward += magnets.freezeTimeMagnet * *freezeTimeTimer;

  // Evaluating simon heart magnet
  reward += magnets.simonHeartMagnet * *simonHeartCount;

  // Evaluating boss timer magnet
  reward += magnets.bossStateTimerMagnet * *bossStateTimer;

  // Evaluating boss health magnet
  reward += magnets.bossHealthMagnet * *bossHealth;

  // Evaluating boss health magnet
  reward += magnets.bossSimonDistanceMagnet * bossSimonDistance;

  // Evaluating boss health magnet
  reward += magnets.bossWeaponDistanceMagnet * bossWeaponDistance;

  // Evaluating boss health magnet
  reward += magnets.mummiesDistanceMagnet * mummiesDistance;

  // Evaluating subweapon active states
  int isSubweapon1Active = *subweapon1State == 23 ? 1 : 0;
  reward += magnets.subweapon1ActiveMagnet * isSubweapon1Active;
  int isSubweapon2Active = *subweapon2State == 23 ? 1 : 0;
  reward += magnets.subweapon2ActiveMagnet * isSubweapon2Active;
  int isSubweapon3Active = *subweapon3State == 23 ? 1 : 0;
  reward += magnets.subweapon3ActiveMagnet * isSubweapon3Active;

  // Evaluating simon's stairs magnet
  if (magnets.simonStairMagnet.mode == *simonStairMode) reward += magnets.simonStairMagnet.reward;

  // Evaluating simon's weapon magnet
  if (magnets.simonWeaponMagnet.weapon == *subweaponNumber) reward += magnets.simonWeaponMagnet.reward;

  // Evaluating tile magnets
  for (const auto& tileMagnet : magnets.scrollTileMagnets) if (_emu->_ppuNameTableMem[tileMagnet.pos] == tileMagnet.value) reward += tileMagnet.reward;

  // Reward boss active
  //reward += *bossIsActive;

  // Reward shot count
  //reward += 10.0f * *subweaponShotCount;

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
  LOG("[Jaffar]  + Timer:                  %u (%02u)\n", *stageTimer, *levelTransitionTimer);
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
  LOG("[Jaffar]  + Game Mode:              %02u-%02u (P: %02u)\n", *gameMode, *gameSubMode, *isPaused);
//  LOG("[Jaffar]  + Screen Offset:          %04u\n", *screenOffset);
  LOG("[Jaffar]  + Is Lag Frame:           %02u\n", *isLagFrame);
//  LOG("[Jaffar]  + Simon Heart Count:      %02u\n", *simonHeartCount);
  LOG("[Jaffar]  + Simon State:            %02u\n", *simonState);
  LOG("[Jaffar]  + Simon Facing Direction: %02u\n", *simonFacingDirection);
//  LOG("[Jaffar]  + Simon Vertical Speed:   %02u-%02u\n", *simonVerticalSpeed, *simonVerticalDirection);
//  LOG("[Jaffar]  + Simon Lives:            %02u\n", *simonLives);
  LOG("[Jaffar]  + Simon Stair Mode:       %02u\n", *simonStairMode);
  LOG("[Jaffar]  + Simon Health:           %02u\n", *simonHealth);
  LOG("[Jaffar]  + Simon Pos X:            %04u (%02u %02u) (%02u)\n", *simonPosX, simonRelativePosX, *(((uint8_t*)simonPosX)+1), *jumpingInertia);
  LOG("[Jaffar]  + Simon Pos Y:            %04u\n", *simonPosY);
//  LOG("[Jaffar]  + Simon Screen Offset:    %02u %02u\n", *simonScreenOffsetX, *screenMotionX);
  LOG("[Jaffar]  + Simon Invulnerability:  %02u\n", *simonInvulnerability);
//  LOG("[Jaffar]  + Simon Kneeling Mode:    %02u\n", *simonKneelingMode);
//  LOG("[Jaffar]  + Subweapon Info:         %02u (%02u), H: %02u, D: %.0f\n", *subweaponNumber, *subweaponShotCount, *subweaponHitCount, bossWeaponDistance);
//  LOG("[Jaffar]  + Subweapon 1:            S: %02u, X: %02u Y: %02u, B: %02u, D: %02u\n", *subweapon1State, *subweapon1PosX, *subweapon1PosY, *subweapon1Bounce, *subweapon1Direction);
//  LOG("[Jaffar]  + Subweapon 2:            S: %02u, X: %02u Y: %02u, B: %02u, D: %02u\n", *subweapon2State, *subweapon2PosX, *subweapon2PosY, *subweapon2Bounce, *subweapon2Direction);
//  LOG("[Jaffar]  + Subweapon 3:            S: %02u, X: %02u Y: %02u, B: %02u, D: %02u\n", *subweapon3State, *subweapon3PosX, *subweapon3PosY, *subweapon3Bounce, *subweapon3Direction);
//  LOG("[Jaffar]  + Whip Length:            %02u\n", *whipLength);
  LOG("[Jaffar]  + Boss H/X/Y/D:           %02u, %02u, %02u, %03.4f\n", *bossHealth, *bossPosX, *bossPosY, bossSimonDistance);
  LOG("[Jaffar]  + Mummies X1/X2/D:        %02u, %02u, %02u\n", *bossPosX, *mummy2PosX, mummiesDistance);
  LOG("[Jaffar]  + Boss Image / State:     %02u / %02u\n", *bossImage, *bossState);
  LOG("[Jaffar]  + Boss Is Active:         %02u (%02u)\n", *bossIsActive, *bossStateTimer);
//  LOG("[Jaffar]  + Enemy S/X:              (%02u, %02u), (%02u, %02u), (%02u, %02u), %02u, %02u \n", *enemy1State, *enemy1PosX, *enemy2State, *enemy2PosX, *enemy3State, *enemy3PosX, *enemy4State, *enemy5State);
  LOG("[Jaffar]  + Bat / Medusa S/X/Y:     (%02u, %02u, %02u, A: %04u) (%02u, %02u, %02u) (%02u, %02u, %02u) \n", *batMedusa1State, *batMedusa1PosX, *batMedusa1PosY, batMedusa1AbsolutePosX, *batMedusa2State, *batMedusa2PosX, *batMedusa2PosY, *batMedusa3State, *batMedusa3PosX, *batMedusa3PosY);
  LOG("[Jaffar]  + Skeleton X/X2/B1/B2/B3:    %02u, %02u, %02u, %02u\n", *skeletonPosX, *skeletonPosX2, *skeletonBone1PosY, *skeletonBone2PosY, *skeletonBone3PosY);
  LOG("[Jaffar]  + Grab Item Timer:        %02u\n", *grabItemTimer);
  LOG("[Jaffar]  + Freeze Time Timer:      %02u\n", *freezeTimeTimer);
  LOG("[Jaffar]  + Item Drop Counter:      %02u\n", *itemDropCounter);
  LOG("[Jaffar]  + Holy Water:             %02u, %02u\n", *enemy1HolyWaterLockState, *holyWaterFire1Timer);

  LOG("[Jaffar]  + Enemy States:           %02u, %02u, %02u, %02u, %02u, %02u, %02u, %02u\n", *enemy0State, *enemy1State, *enemy2State, *enemy3State, *enemy4State, *enemy5State, *enemy6State, *enemy7State);
  LOG("[Jaffar]  + Bat States:             %02u, %02u, %02u, %02u, %02u\n", *batMedusa1State, *batMedusa2State, *batMedusa3State, *batMedusa4State, *batMedusa5State);

  LOG("[Jaffar]  + Enemy X:                %02u, %02u, %02u, %02u, %02u, %02u, %02u\n", *enemy1PosX, *enemy2PosX, *enemy3PosX, *enemy4PosX, *enemy5PosX, *enemy6PosX, *enemy7PosX);

  LOG("[Jaffar]  + Candelabra States: ");
    for (size_t i = 0x0191; i < 0x1A8; i++) LOG("%d", _emu->_baseMem[i] > 0 ? 1 : 0);
  LOG("\n");

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  for (const auto& tilePos : tileHashIncludes )  LOG("[Jaffar] + Tile Magnet [0x%04X]=%02u\n", tilePos, _emu->_ppuNameTableMem[tilePos]);


  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.simonHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Simon Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.simonHorizontalMagnet.intensity, magnets.simonHorizontalMagnet.center, magnets.simonHorizontalMagnet.min, magnets.simonHorizontalMagnet.max);
  if (std::abs(magnets.simonVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Simon Vertical Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.simonVerticalMagnet.intensity, magnets.simonVerticalMagnet.center, magnets.simonVerticalMagnet.min, magnets.simonVerticalMagnet.max);
  if (std::abs(magnets.bossHorizontalMagnet.intensity) > 0.0f)      LOG("[Jaffar]  + Boss Horizontal Magnet         - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossHorizontalMagnet.intensity, magnets.bossHorizontalMagnet.center, magnets.bossHorizontalMagnet.min, magnets.bossHorizontalMagnet.max);
  if (std::abs(magnets.bossVerticalMagnet.intensity) > 0.0f)        LOG("[Jaffar]  + Boss Vertical Magnet           - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossVerticalMagnet.intensity, magnets.bossVerticalMagnet.center, magnets.bossVerticalMagnet.min, magnets.bossVerticalMagnet.max);
  if (std::abs(magnets.batMedusaHorizontalMagnet.intensity) > 0.0f) LOG("[Jaffar]  + Bat / Medusa Horizontal Magnet - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.batMedusaHorizontalMagnet.intensity, magnets.batMedusaHorizontalMagnet.center, magnets.batMedusaHorizontalMagnet.min, magnets.batMedusaHorizontalMagnet.max);
  if (std::abs(magnets.batMedusaVerticalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Bat / Medusa Vertical Magnet   - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.batMedusaVerticalMagnet.intensity, magnets.batMedusaVerticalMagnet.center, magnets.batMedusaVerticalMagnet.min, magnets.batMedusaVerticalMagnet.max);
  if (std::abs(magnets.skeletonHorizontalMagnet.intensity) > 0.0f)  LOG("[Jaffar]  + Skeleton Horizontal Magnet     - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.skeletonHorizontalMagnet.intensity, magnets.skeletonHorizontalMagnet.center, magnets.skeletonHorizontalMagnet.min, magnets.skeletonHorizontalMagnet.max);
  if (std::abs(magnets.subweaponHitCountMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Subweapon Hit Count Magnet     - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.subweaponHitCountMagnet.intensity, magnets.subweaponHitCountMagnet.center, magnets.subweaponHitCountMagnet.min, magnets.subweaponHitCountMagnet.max);
  if (std::abs(magnets.simonHeartMagnet) > 0.0f)                    LOG("[Jaffar]  + Simon Heart Magnet             - Intensity: %.5f\n", magnets.simonHeartMagnet);
  if (std::abs(magnets.freezeTimeMagnet) > 0.0f)                    LOG("[Jaffar]  + Freeze Time Magnet             - Intensity: %.5f\n", magnets.freezeTimeMagnet);
  if (std::abs(magnets.bossHealthMagnet) > 0.0f)                    LOG("[Jaffar]  + Boss Health Magnet             - Intensity: %.5f\n", magnets.bossHealthMagnet);
  if (std::abs(magnets.bossSimonDistanceMagnet) > 0.0f)             LOG("[Jaffar]  + Boss/Simon Distance Magnet     - Intensity: %.5f\n", magnets.bossSimonDistanceMagnet);
  if (std::abs(magnets.bossWeaponDistanceMagnet) > 0.0f)            LOG("[Jaffar]  + Boss/Weapon Distance Magnet    - Intensity: %.5f\n", magnets.bossWeaponDistanceMagnet);
  if (std::abs(magnets.mummiesDistanceMagnet) > 0.0f)               LOG("[Jaffar]  + Mummies Distance Magnet        - Intensity: %.5f\n", magnets.mummiesDistanceMagnet);
  if (std::abs(magnets.bossStateTimerMagnet) > 0.0f)                LOG("[Jaffar]  + Boss State Timer Magnet        - Intensity: %.5f\n", magnets.bossStateTimerMagnet);
  if (std::abs(magnets.subweapon1ActiveMagnet) > 0.0f)              LOG("[Jaffar]  + Subweapon 1 Active Magnet      - Intensity: %.5f\n", magnets.subweapon1ActiveMagnet);
  if (std::abs(magnets.subweapon2ActiveMagnet) > 0.0f)              LOG("[Jaffar]  + Subweapon 2 Active Magnet      - Intensity: %.5f\n", magnets.subweapon2ActiveMagnet);
  if (std::abs(magnets.subweapon3ActiveMagnet) > 0.0f)              LOG("[Jaffar]  + Subweapon 3 Active Magnet      - Intensity: %.5f\n", magnets.subweapon3ActiveMagnet);
  if (std::abs(magnets.simonStairMagnet.reward) > 0.0f)             LOG("[Jaffar]  + Simon Stairs Magnet            - Reward:    %.1f, Mode: %u\n", magnets.simonStairMagnet.reward, magnets.simonStairMagnet.mode);
  if (std::abs(magnets.simonWeaponMagnet.reward) > 0.0f)            LOG("[Jaffar]  + Simon Weapon Magnet            - Reward:    %.1f, Weapon: %u\n", magnets.simonWeaponMagnet.reward, magnets.simonWeaponMagnet.weapon);

  for (const auto& tileMagnet : magnets.scrollTileMagnets) LOG("[Jaffar] + Tile Magnet [0x%04X]=%02u (%02u->%4.2f), %s\n", tileMagnet.pos, _emu->_ppuNameTableMem[tileMagnet.pos], tileMagnet.value, tileMagnet.reward, _emu->_ppuNameTableMem[tileMagnet.pos] == tileMagnet.value ? "True" : "False");
}
