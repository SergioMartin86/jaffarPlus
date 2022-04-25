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
  isLagFrame             = (uint8_t*)   &_emu->_baseMem[0x001B];
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
  enemy1State            = (uint8_t*)   &_emu->_baseMem[0x0438];
  enemy2State            = (uint8_t*)   &_emu->_baseMem[0x0439];
  enemy3State            = (uint8_t*)   &_emu->_baseMem[0x043A];
  freezeTimeTimer        = (uint8_t*)   &_emu->_baseMem[0x057C];
  batMedusa1State        = (uint8_t*)   &_emu->_baseMem[0x04C9];
  batMedusa1PosX         = (uint8_t*)   &_emu->_baseMem[0x0395];
  batMedusa1PosY         = (uint8_t*)   &_emu->_baseMem[0x035D];
  itemDropCounter        = (uint8_t*)   &_emu->_baseMem[0x007B];
  RNGState               = (uint8_t*)   &_emu->_baseMem[0x006F];
  stairAnimationFrame    = (uint8_t*)   &_emu->_baseMem[0x0370];
  bossStateTimer         = (uint8_t*)   &_emu->_baseMem[0x0553];
  simonScreenOffsetX     = (uint8_t*)   &_emu->_baseMem[0x038C];

  enemy1HolyWaterLockState = (uint8_t*)   &_emu->_baseMem[0x056C];
  holyWaterFire1Timer      = (uint8_t*)   &_emu->_baseMem[0x057C];

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
  hash.Update(*isLagFrame);
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
  hash.Update(*bossStateTimer % 4);
  hash.Update(*simonScreenOffsetX);
  hash.Update(*levelTransitionTimer);

  // Conditional hashes
  if (hashIncludes.contains("Subweapon Number")) hash.Update(*subweaponNumber);
  if (hashIncludes.contains("Enemy 1 Pos X")) hash.Update(*enemy1PosX);
  if (hashIncludes.contains("Enemy 2 Pos X")) hash.Update(*enemy2PosX);
  if (hashIncludes.contains("Enemy 3 Pos X")) hash.Update(*enemy3PosX);
  if (hashIncludes.contains("Enemy 1 State")) hash.Update(*enemy1State);
  if (hashIncludes.contains("Enemy 2 State")) hash.Update(*enemy2State);
  if (hashIncludes.contains("Enemy 3 State")) hash.Update(*enemy3State);
  if (hashIncludes.contains("Bat / Medusa 1 State")) hash.Update(*batMedusa1State);
  if (hashIncludes.contains("Bat / Medusa 1 Pos X")) hash.Update(*batMedusa1PosX);
  if (hashIncludes.contains("Bat / Medusa 1 Pos Y")) hash.Update(*batMedusa1PosY);
  if (hashIncludes.contains("Enemy 1 Holy Water Lock State")) hash.Update(*enemy1HolyWaterLockState);
  if (hashIncludes.contains("Holy Water Fire 1 Timer")) hash.Update(*holyWaterFire1Timer);

  // Updating nametable
  hash.Update(_emu->_ppuNameTableMem, 0x1000);

  // Turning invulnerability into a boolean
  hash.Update(*simonInvulnerability % 2);

  if (hashIncludes.contains("Simon Heart Count"))  hash.Update(*simonHeartCount);

  // Freeze timer is around 180 frames. To reduce exploration space, we only take whether it's active or not
  if (hashIncludes.contains("Freeze Time Timer"))  hash.Update(*freezeTimeTimer == 0 ? 0 : 1);

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
 // Advancing useless frames
 uint16_t advanceCounter = 0;

 while ( (advanceCounter < 1024) && (*gameMode == 8) ) { _emu->advanceState(0); advanceCounter++; }
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 // Pass 1 - Stage 3-0
 if (*simonState == 0x00) moveList.insert(moveList.end(), { ".......A", "......B.", "...U....", "..D.....", "..DU....", ".L......", ".L.....A", ".L....B.", ".L.U....", ".L.U...A", ".LD.....", ".LD....A", ".LD...B.", ".LDU....", "R.......", "R......A", "R.....B.", "R..U....", "R..U...A", "R..U..B.", "R.D.....", "R.D....A", "R.D...B.", "R.DU....", "RL.U....", "RL.U...A", "RLD.....", "RLD....A", "RLD...B.", "RL.U.SB."});
 if (*simonState == 0x01) moveList.insert(moveList.end(), { "......B.", "...U..B.", "RL......", "RL.U..B."});
 if (*simonState == 0x03) moveList.insert(moveList.end(), { "..D.....", "..D...B.", ".L......", ".LD.....", ".LD...B.", "R.......", "R.D.....", "R.D...B."});
 if (*simonState == 0x04) moveList.insert(moveList.end(), { "......B.", ".L......", "R......."});
 if (*simonState == 0x09) moveList.insert(moveList.end(), { "......B.", ".L......", ".L....B.", "R.......", "R.....B."});

 // Pass 2 - Stage 1-0
 if (*simonState == 0x00) moveList.insert(moveList.end(), { "RLDU.SB."});
 if (*simonState == 0x01) moveList.insert(moveList.end(), { "RL....B."});
 if (*simonState == 0x03) moveList.insert(moveList.end(), { "..DU..BA", ".LDU..B.", "R.DU..B."});
 if (*simonState == 0x04) moveList.insert(moveList.end(), { "...U..B."});

 // Pass 3 - Stage 4-0

 if (*simonState == 0x09) moveList.insert(moveList.end(), { ".L.U..B.", "R..U..B."});

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

  // Evaluating simon heart magnet
  reward += magnets.simonHeartMagnet * *simonHeartCount;

  // Evaluating boss timer magnet
  reward += magnets.bossStateTimerMagnet * *bossStateTimer;

  // Evaluating boss health magnet
  reward += magnets.bossHealthMagnet * *bossHealth;

  // Evaluating simon's stairs magnet
  if (magnets.simonStairMagnet.mode == *simonStairMode) reward += magnets.simonStairMagnet.reward;

  // Evaluating simon's weapon magnet
  if (magnets.simonWeaponMagnet.weapon == *subweaponNumber) reward += magnets.simonWeaponMagnet.reward;

  // Evaluating tile magnets
  for (const auto& tileMagnet : magnets.scrollTileMagnets) if (_emu->_ppuNameTableMem[tileMagnet.pos] == tileMagnet.value) reward += tileMagnet.reward;

  // Bat fight-specific tiles
  if (*currentStage == 3 && *currentSubStage == 0)
  {
   if (_emu->_ppuNameTableMem[0x0250] == 106) reward += 1000;
   if (_emu->_ppuNameTableMem[0x0251] == 107) reward += 1000;
   if (_emu->_ppuNameTableMem[0x0252] == 106) reward += 1000;
   if (_emu->_ppuNameTableMem[0x0253] == 107) reward += 1000;
  }

  // Medusa fight-specific tiles
  if (*currentStage == 6 && *currentSubStage == 1)
  {
   if (_emu->_ppuNameTableMem[0x02A1] == 105) reward += 10;
   if (_emu->_ppuNameTableMem[0x02A2] == 104) reward += 10;
   if (_emu->_ppuNameTableMem[0x02A3] == 105) reward += 10;
   if (_emu->_ppuNameTableMem[0x02A4] == 104) reward += 100;
   if (_emu->_ppuNameTableMem[0x02A5] == 105) reward += 100;
   if (_emu->_ppuNameTableMem[0x02A6] == 104) reward += 100;
   if (_emu->_ppuNameTableMem[0x02A7] == 105) reward += 100;
   if (_emu->_ppuNameTableMem[0x02A8] == 104) reward += 20;
   if (_emu->_ppuNameTableMem[0x02A9] == 105) reward += 20;
   if (_emu->_ppuNameTableMem[0x02AA] == 104) reward += 20;
   if (_emu->_ppuNameTableMem[0x02AB] == 105) reward += 20;
   if (_emu->_ppuNameTableMem[0x02AC] == 104) reward += 10;
   if (_emu->_ppuNameTableMem[0x02AD] == 105) reward += 10;
   if (_emu->_ppuNameTableMem[0x02AE] == 104) reward += 10;
   if (_emu->_ppuNameTableMem[0x02AF] == 105) reward += 10;
  }

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
  LOG("[Jaffar]  + Timer:                  %u (%02u)\n", *stageTimer, *levelTransitionTimer);
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
  LOG("[Jaffar]  + Simon Screen Offset:    %02u\n", *simonScreenOffsetX);
  LOG("[Jaffar]  + Simon Invulnerability:  %02u\n", *simonInvulnerability);
  LOG("[Jaffar]  + Simon Kneeling Mode:    %02u\n", *simonKneelingMode);
  LOG("[Jaffar]  + Subweapon Number:       %02u\n", *subweaponNumber);
  LOG("[Jaffar]  + Subweapon Shot Count:   %02u\n", *subweaponShotCount);
  LOG("[Jaffar]  + Whip Length:            %02u\n", *whipLength);
  LOG("[Jaffar]  + Boss Health:            %02u\n", *bossHealth);
  LOG("[Jaffar]  + Boss Pos X:             %04u\n", *bossPosX);
  LOG("[Jaffar]  + Boss Pos Y:             %02u\n", *bossPosY);
  LOG("[Jaffar]  + Boss Image / State:     %02u / %02u\n", *bossImage, *bossState);
  LOG("[Jaffar]  + Boss Is Active:         %02u (%02u)\n", *bossIsActive, *bossStateTimer);
  LOG("[Jaffar]  + Enemy State:            %02u, %02u, %02u\n", *enemy1State, *enemy2State, *enemy3State);
  LOG("[Jaffar]  + Enemy Pos X:            %02u, %02u, %02u\n", *enemy1PosX, *enemy2PosX, *enemy3PosX);
  LOG("[Jaffar]  + Bat / Medusa State:     %02u\n", *batMedusa1State);
  LOG("[Jaffar]  + Bat / Medusa Pos X:     %02u\n", *batMedusa1PosX);
  LOG("[Jaffar]  + Bat / Medusa Pos Y:     %02u\n", *batMedusa1PosY);
  LOG("[Jaffar]  + Grab Item Timer:        %02u\n", *grabItemTimer);
  LOG("[Jaffar]  + Freeze Time Timer:      %02u\n", *freezeTimeTimer);
  LOG("[Jaffar]  + Item Drop Counter:      %02u\n", *itemDropCounter);
  LOG("[Jaffar]  + Holy Water:             %02u, %02u\n", *enemy1HolyWaterLockState, *holyWaterFire1Timer);

  // Level Specific stuff
  if ((*currentStage == 5 && *currentSubStage == 1) || (*currentStage == 6 && *currentSubStage == 0))
  {
   LOG("[Jaffar]  + Stage 5-1 Scroll Tile 1:  %02u\n", *stage51ScrollTile1);
   LOG("[Jaffar]  + Stage 5-1 Scroll Tile 2:  %02u\n", *stage51ScrollTile2);
  }

  // Level Specific stuff
  if ((*currentStage == 6 && *currentSubStage == 1))
  {
   LOG("[Jaffar]  + Stage 6-1 Scroll Tiles:  %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n",
     _emu->_ppuNameTableMem[0x02A1],
     _emu->_ppuNameTableMem[0x02A2],
     _emu->_ppuNameTableMem[0x02A3],
     _emu->_ppuNameTableMem[0x02A4],
     _emu->_ppuNameTableMem[0x02A5],
     _emu->_ppuNameTableMem[0x02A6],
     _emu->_ppuNameTableMem[0x02A7],
     _emu->_ppuNameTableMem[0x02A8],
     _emu->_ppuNameTableMem[0x02A9],
     _emu->_ppuNameTableMem[0x02AA],
     _emu->_ppuNameTableMem[0x02AB],
     _emu->_ppuNameTableMem[0x02AC],
     _emu->_ppuNameTableMem[0x02AD],
     _emu->_ppuNameTableMem[0x02AE],
     _emu->_ppuNameTableMem[0x02AF]);
  }

  if ((*currentStage == 3 && *currentSubStage == 0))
  {
   LOG("[Jaffar]  + Stage 3-0 Scroll Tiles:  %02u %02u %02u %02u\n",
     _emu->_ppuNameTableMem[0x0250],
     _emu->_ppuNameTableMem[0x0251],
     _emu->_ppuNameTableMem[0x0252],
     _emu->_ppuNameTableMem[0x0253]);
  }

  LOG("[Jaffar]  + Candelabra States: ");
    for (size_t i = 0x0191; i < 0x1A8; i++) LOG("%d", _emu->_baseMem[i] > 0 ? 1 : 0);
  LOG("\n");

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  LOG("[Jaffar]  + Simon Horizontal Magnet        - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.simonHorizontalMagnet.intensity, magnets.simonHorizontalMagnet.center, magnets.simonHorizontalMagnet.min, magnets.simonHorizontalMagnet.max);
  LOG("[Jaffar]  + Simon Vertical Magnet          - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.simonVerticalMagnet.intensity, magnets.simonVerticalMagnet.center, magnets.simonVerticalMagnet.min, magnets.simonVerticalMagnet.max);
  LOG("[Jaffar]  + Boss Horizontal Magnet         - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossHorizontalMagnet.intensity, magnets.bossHorizontalMagnet.center, magnets.bossHorizontalMagnet.min, magnets.bossHorizontalMagnet.max);
  LOG("[Jaffar]  + Boss Vertical Magnet           - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossVerticalMagnet.intensity, magnets.bossVerticalMagnet.center, magnets.bossVerticalMagnet.min, magnets.bossVerticalMagnet.max);
  LOG("[Jaffar]  + Bat / Medusa Horizontal Magnet - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.batMedusaHorizontalMagnet.intensity, magnets.batMedusaHorizontalMagnet.center, magnets.batMedusaHorizontalMagnet.min, magnets.batMedusaHorizontalMagnet.max);
  LOG("[Jaffar]  + Bat / Medusa Vertical Magnet   - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.batMedusaVerticalMagnet.intensity, magnets.batMedusaVerticalMagnet.center, magnets.batMedusaVerticalMagnet.min, magnets.batMedusaVerticalMagnet.max);
  LOG("[Jaffar]  + Simon Heart Magnet             - Intensity: %.1f\n", magnets.simonHeartMagnet);
  LOG("[Jaffar]  + Freeze Time Magnet             - Intensity: %.1f\n", magnets.freezeTimeMagnet);
  LOG("[Jaffar]  + Boss Health Magnet             - Intensity: %.1f\n", magnets.bossHealthMagnet);
  LOG("[Jaffar]  + Boss State Timer Magnet        - Intensity: %.1f\n", magnets.bossStateTimerMagnet);
  LOG("[Jaffar]  + Simon Stairs Magnet            - Reward:    %.1f, Mode: %u\n", magnets.simonStairMagnet.reward, magnets.simonStairMagnet.mode);
  LOG("[Jaffar]  + Simon Weapon Magnet            - Reward:    %.1f, Weapon: %u\n", magnets.simonWeaponMagnet.reward, magnets.simonWeaponMagnet.weapon);
  LOG("[Jaffar]  + Tile Magnets:  ");
  for (const auto& tileMagnet : magnets.scrollTileMagnets) LOG("0x%4X=%2u->%4.2f ", tileMagnet.pos, tileMagnet.value, tileMagnet.reward);
}
