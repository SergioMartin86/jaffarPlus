#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Setting relevant SMB pointers

  // Thanks to https://datacrystal.romhacking.net/wiki/Castlevania:RAM_map for helping me find some of these items

  RNGState               = (uint8_t*)   &_emu->_baseMem[0x0004];
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
  grabItemTimer          = (uint8_t*)   &_emu->_baseMem[0x007E];
  enemy1PosX             = (uint8_t*)   &_emu->_baseMem[0x0390];
  enemy2PosX             = (uint8_t*)   &_emu->_baseMem[0x0391];
  enemy3PosX             = (uint8_t*)   &_emu->_baseMem[0x0392];

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
  hash.Update(*simonInvulnerability);
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
  hash.Update(*enemy1PosX);
  hash.Update(*enemy2PosX);
  hash.Update(*enemy3PosX);

  // Counting health below a certain point is not important
  uint8_t limitedSimonHealth = std::min(*simonHealth, (uint8_t)30);
  hash.Update(limitedSimonHealth);

  // Counting heart counts beyond a certain amount is not important
  uint8_t limitedSimonHeartCount = std::max(*simonHeartCount, (uint8_t)15);
  hash.Update(limitedSimonHeartCount);

  // Conditional hashes
  if (hashIncludes.contains("Subweapon Number")) hash.Update(*subweaponNumber);

  // Only hash boss position if active
  if (*bossIsActive > 0)
  {
   hash.Update(*bossPosX);
   hash.Update(*bossPosY);
  }

  // Unused hashes
  //hash.Update(*simonLives);

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
 // On the floor, try all possible combinations, prioritize jumping and right direction
 return { ".", "R", "RA", "DR", "RB", "LR", "UR", "URB", "B", "L", "LB", "D", "LA", "ULB", "UL", "DL", "UDLB", "UB", "A" };
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;

 magnets.simonHorizontalMagnet.intensity = 0.0f;
 magnets.simonHorizontalMagnet.max = 0.0f;

 magnets.simonVerticalMagnet.intensity = 0.0f;
 magnets.simonVerticalMagnet.max = 0.0f;

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
   if (_rules[ruleId]->_containsMagnets == true)
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

  // Container for bounding calculations
  float boundedValue = 0.0;

  // Evaluating simon magnet's reward on position X
  boundedValue = (float)*simonPosX;
  boundedValue = std::min(boundedValue, magnets.simonHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.simonHorizontalMagnet.min);
  reward += magnets.simonHorizontalMagnet.intensity * boundedValue;

  // Evaluating simon magnet's reward on position Y
  boundedValue = (float)*simonPosY;
  boundedValue = std::min(boundedValue, magnets.simonVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.simonVerticalMagnet.min);
  reward += magnets.simonVerticalMagnet.intensity * boundedValue;

  // Favor accumulating hearts
  reward += 10.0f * *simonHeartCount;

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
  LOG("[Jaffar]  + Timer:                  %u\n", *stageTimer);
  LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
  LOG("[Jaffar]  + RNG State:              0x%X\n", *RNGState);
  LOG("[Jaffar]  + Game Mode:              %02u-%02u\n", *gameMode, *gameSubMode);
  LOG("[Jaffar]  + Screen Offset:          %04u\n", *screenOffset);
  LOG("[Jaffar]  + Simon State:            %02u-%02u\n", *simonState, *simonSubState);
  LOG("[Jaffar]  + Simon Heart Count:      %02u\n", *simonHeartCount);
  LOG("[Jaffar]  + Simon Image:            %02u\n", *simonImage);
  LOG("[Jaffar]  + Simon Facing Direction: %02u\n", *simonFacingDirection);
  LOG("[Jaffar]  + Simon Vertical Speed:   %02u-%02u\n", *simonVerticalSpeed, *simonVerticalDirection);
  LOG("[Jaffar]  + Simon Lives:            %02u\n", *simonLives);
  LOG("[Jaffar]  + Simon Stair Mode:       %02u\n", *simonStairMode);
  LOG("[Jaffar]  + Simon Health:           %02u\n", *simonHealth);
  LOG("[Jaffar]  + Simon Pos X:            %04u\n", *simonPosX);
  LOG("[Jaffar]  + Simon Pos Y:            %04u\n", *simonPosY);
  LOG("[Jaffar]  + Simon Invulnerability:  %02u\n", *simonInvulnerability);
  LOG("[Jaffar]  + Simon Kneeling Mode:    %02u\n", *simonKneelingMode);
  LOG("[Jaffar]  + Subweapon Number:       %02u\n", *subweaponNumber);
  LOG("[Jaffar]  + Subweapon Shot Count:   %02u\n", *subweaponShotCount);
  LOG("[Jaffar]  + Whip Length:            %02u\n", *whipLength);
  LOG("[Jaffar]  + Boss Health:            %02u\n", *bossHealth);
  LOG("[Jaffar]  + Boss Pos X:             %04u\n", *bossPosX);
  LOG("[Jaffar]  + Boss Pos Y:             %02u\n", *bossPosY);
  LOG("[Jaffar]  + Boss Is Active:         %02u\n", *bossIsActive);
  LOG("[Jaffar]  + Enemy Pos X:            %02u, %02u, %02u\n", *enemy1PosX, *enemy2PosX, *enemy3PosX);
  LOG("[Jaffar]  + Grab Item Timer:        %02u\n", *grabItemTimer);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  LOG("[Jaffar]  + Simon Horizontal Magnet    - Intensity: %.1f, Min: %3.3f, Max: %3.3f\n", magnets.simonHorizontalMagnet.intensity, magnets.simonHorizontalMagnet.min, magnets.simonHorizontalMagnet.max);
  LOG("[Jaffar]  + Simon Vertical Magnet      - Intensity: %.1f, Min: %3.3f, Max: %3.3f\n", magnets.simonVerticalMagnet.intensity, magnets.simonVerticalMagnet.min, magnets.simonVerticalMagnet.max);


}
