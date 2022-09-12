#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameMode                    = (uint8_t*)   &_emu->_baseMem[0x0018];
  frameCounter                = (uint8_t*)   &_emu->_baseMem[0x001A];
  currentLevel                = (uint8_t*)   &_emu->_baseMem[0x0030];
  isLagFrame                  = (uint8_t*)   &_emu->_baseMem[0x001B];
  playerInvincibilityTimer    = (uint8_t*)   &_emu->_baseMem[0x00AE];
  playerVelX                  = (uint8_t*)   &_emu->_baseMem[0x0098];
  playerVelY                  = (uint8_t*)   &_emu->_baseMem[0x00C6];
  playerDeadFlag              = (uint8_t*)   &_emu->_baseMem[0x00B4];
  playerWeapon                = (uint8_t*)   &_emu->_baseMem[0x00AA];
  screenScroll1               = (uint8_t*)   &_emu->_baseMem[0x0064];
  screenScroll2               = (uint8_t*)   &_emu->_baseMem[0x0065];
  playerAnimationFrame        = (uint8_t*)   &_emu->_baseMem[0x00D6];
  playerAimDirection          = (uint8_t*)   &_emu->_baseMem[0x00C2];
  playerPosScreenX            = (uint8_t*)   &_emu->_baseMem[0x0334];
  playerPosScreenY            = (uint8_t*)   &_emu->_baseMem[0x031A];
  playerBulletState           = (uint8_t*)   &_emu->_baseMem[0x0368];
  playerBulletPosX            = (uint8_t*)   &_emu->_baseMem[0x03C8];
  playerBulletPosY            = (uint8_t*)   &_emu->_baseMem[0x03B8];
  enemyType                   = (uint8_t*)   &_emu->_baseMem[0x04BE];
  enemyPosX                   = (uint8_t*)   &_emu->_baseMem[0x0344];
  enemyPosY                   = (uint8_t*)   &_emu->_baseMem[0x032A];
  enemyHP                     = (uint8_t*)   &_emu->_baseMem[0x057E];
  score1                      = (uint8_t*)   &_emu->_baseMem[0x07E2];
  score2                      = (uint8_t*)   &_emu->_baseMem[0x07E3];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  hash.Update(*gameMode);
  if (*playerDeadFlag == 1) hash.Update(*frameCounter);
  if (*isLagFrame == 1) hash.Update(*frameCounter);
  hash.Update(*currentLevel);
  hash.Update(*isLagFrame);
  hash.Update(*playerInvincibilityTimer == 0 ? 0 : (*playerInvincibilityTimer % 2)+1);
  hash.Update(*playerVelX);
  hash.Update(*playerVelY);
  hash.Update(*playerDeadFlag);
  hash.Update(*playerWeapon);
  hash.Update(*screenScroll1);
  hash.Update(*screenScroll2);
  hash.Update(*playerAnimationFrame);
  hash.Update(*playerAimDirection);
  hash.Update(*playerPosScreenX);
  hash.Update(*playerPosScreenY);
  hash.Update(playerBulletState, _BULLET_COUNT);
  //hash.Update(playerBulletPosX, _BULLET_COUNT);
  //hash.Update(playerBulletPosY, _BULLET_COUNT);
  hash.Update(enemyType, _ENEMY_COUNT);
  //hash.Update(enemyPosX, _ENEMY_COUNT);
  //hash.Update(enemyPosY, _ENEMY_COUNT);
  hash.Update(enemyHP, _ENEMY_COUNT);
  hash.Update(*score1);
  hash.Update(*score2);

  uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 playerPosX = (float)*screenScroll1 * 256.0f + (float)*screenScroll2 + (float)*playerPosScreenX;
 playerPosY = (float)*playerPosScreenY;
 score = (float)*score1 * 256.0f + (float)*score2;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 // First pass stage 00-00
 if (*playerAnimationFrame == 0x0000) moveList.insert(moveList.end(), { "...U....", "..D.....", "......B.", ".L......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", "RL......", ".L.U....", ".L....B.", "..DU....", "..D...B.", "...U..B.", "R.D...B.", "RL....B.", "R..U..B.", ".LD...B.", ".L.U..B.", "..DU..B."});
 if (*playerAnimationFrame == 0x0002) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", ".R......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0003) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", ".R......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0004) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", ".R......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0005) moveList.insert(moveList.end(), { "......B.", "...U....", ".......A", "..D.....", ".L......", ".R......", "R......A", "R.....B.", ".LD.....", "R..U....", ".L.U....", ".L....B.", ".L.....A", "R.D.....", "..DU....", "RL......", "..D...B.", "..D....A", "...U..B.", "...U...A", "......BA", "R.D....A", "R.D...B.", "R..U..B.", "R..U...A", "R.....BA", "RL....B.", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", ".L....BA", "..DU..B.", "..DU...A", "..D...BA", "...U..BA", ".LD...BA", ".L.U..BA", "R..U..BA", "..DU..BA", "R.D...BA"});
 if (*playerAnimationFrame == 0x0006) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0008) moveList.insert(moveList.end(), { "...U....", "..D.....", "......B.", ".L......", ".R......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", "RL......", ".L.U....", ".L....B.", "..DU....", "..D...B.", "...U..B.", "R.D...B.", "RL....B.", "R..U..B.", ".LD...B.", ".L.U..B.", "..DU..B."});
 if (*playerAnimationFrame == 0x0009) moveList.insert(moveList.end(), { "...U....", "..D.....", "......B.", ".L......", ".R......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", "RL......", ".L.U....", ".L....B.", "..DU....", "..D...B.", "...U..B.", "R.D...B.", "RL....B.", "R..U..B.", ".LD...B.", ".L.U..B.", "..DU..B."});
 if (*playerAnimationFrame == 0x000D) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x000E) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x000F) moveList.insert(moveList.end(), { "......B.", "...U....", ".......A", "..D.....", ".L......", "R......A", "R.....B.", ".LD.....", "R..U....", ".L.U....", ".L....B.", ".L.....A", "R.D.....", "..DU....", "..D...B.", "..D....A", "RL......", "...U..B.", "...U...A", "......BA", "R.D....A", "R.D...B.", "R.DU....", "R..U..B.", "R..U...A", "R.....BA", ".L.U...A", ".LD...B.", ".LD....A", ".L.U..B.", ".L....BA", "..DU..B.", "..DU...A", "..D...BA", "...U..BA", ".LD...BA", ".L.U..BA", "R..U..BA", "..DU..BA", "R.D...BA", "RLD...B."});
 if (*playerAnimationFrame == 0x0010) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0011) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0012) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0013) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0014) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0015) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0016) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});
 if (*playerAnimationFrame == 0x0017) moveList.insert(moveList.end(), { "......B.", ".......A", "...U....", "..D.....", ".L......", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "......BA", "..DU....", "...U...A", "...U..B.", "..D...B.", "..D....A", "..DU..B.", "R.D...B.", "R.D....A", "R..U..B.", "R..U...A", "R.....BA", "...U..BA", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..D...BA", ".L....BA", "..DU...A", ".LD...BA", ".L.U..BA", "R..U..BA", "R.D...BA", "..DU..BA"});

 if (*playerAnimationFrame == 0x0018) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "......B.", "...U..B.", "..DU....", "RL......", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "R.D.....", ".L.U..B.", "R.DU....", "R..U..B.", "..DU..B.", "RLD...B."});
 if (*playerAnimationFrame == 0x0019) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "......B.", "...U..B.", "..DU....", "RL......", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "R.D.....", ".L.U..B.", "R.DU....", "R..U..B.", "..DU..B.", "RLD...B."});
 if (*playerAnimationFrame == 0x001A) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "R.D.....", "RL......", ".L.U..B.", "R..U..B."});
 if (*playerAnimationFrame == 0x001B) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "......B.", "...U..B.", "..DU....", "RL......", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "R.D.....", ".L.U..B.", "R.DU....", "R..U..B.", "..DU..B.", "RLD...B."});
 if (*playerAnimationFrame == 0x001C) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "......B.", "...U..B.", "..DU....", "RL......", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "R.D.....", ".L.U..B.", "R.DU....", "R..U..B.", "..DU..B.", "RLD...B."});
 if (*playerAnimationFrame == 0x001D) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "......B.", "...U..B.", "..DU....", "RL......", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "R.D.....", ".L.U..B.", "R.DU....", "R..U..B.", "..DU..B.", "RLD...B."});
 if (*playerAnimationFrame == 0x0073) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "R.D.....", "RL......", ".L.U..B.", "R..U..B."});

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

  // Evaluating player magnet's reward on position X
  boundedValue = playerPosX;
  boundedValue = std::min(boundedValue, magnets.playerHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.playerHorizontalMagnet.min);
  diff = std::abs(magnets.playerHorizontalMagnet.center - boundedValue);
  reward += magnets.playerHorizontalMagnet.intensity * -diff;

  // Evaluating player magnet's reward on position Y
  boundedValue = playerPosY;
  boundedValue = std::min(boundedValue, magnets.playerVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.playerVerticalMagnet.min);
  diff = std::abs(magnets.playerVerticalMagnet.center - boundedValue);
  reward += magnets.playerVerticalMagnet.intensity * -diff;

  // Evaluating score magnet
  reward += magnets.scoreMagnet * score;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Frame Counter:                    %03u\n", *frameCounter);
 LOG("[Jaffar]  + Current Level:                    %03u\n", *currentLevel);
 LOG("[Jaffar]  + Game Mode:                        %03u\n", *gameMode);
 LOG("[Jaffar]  + Is Flag Frame:                    %03u\n", *isLagFrame);
 LOG("[Jaffar]  + Reward:                           %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX\n", computeHash());
 LOG("[Jaffar]  + Score:                            %f\n", score * 100.0f);
 LOG("[Jaffar]  + Player Position X:                %f (%03u, %03u, %03u)\n", playerPosX, *screenScroll1, *screenScroll2, *playerPosScreenX);
 LOG("[Jaffar]  + Player Position Y:                %f (%03u)\n", playerPosY, *playerPosScreenY);
 LOG("[Jaffar]  + Player Weapon:                    %03u\n", *playerWeapon);
 LOG("[Jaffar]  + Player Dead Flag:                 %03u\n", *playerDeadFlag);
 LOG("[Jaffar]  + Player Invincibility Timer:       %03u\n", *playerInvincibilityTimer);
 LOG("[Jaffar]  + Player Animation Frame:           %03u\n", *playerAnimationFrame);
 LOG("[Jaffar]  + Player Aim Direction:             %03u\n", *playerAimDirection);

 LOG("[Jaffar]  + Player Bullets:\n");
 for (size_t i = 0; i < _BULLET_COUNT; i++)
  if (*(playerBulletState+i) > 0 )
   LOG("[Jaffar]  + Bullet %lu                      State: %03u - X: %03u - Y: %03u\n", i, *(playerBulletState+i), *(playerBulletPosX+i), *(playerBulletPosY+i));

 LOG("[Jaffar]  + Enemies:\n");
  for (size_t i = 0; i < _ENEMY_COUNT; i++)
   if (*(enemyType+i) > 0 )
    LOG("[Jaffar]  + Enemy %lu                       Type: %03u - X: %03u - Y: %03u - HP: %03u\n", i, *(enemyType+i), *(enemyPosX+i), *(enemyPosY+i), *(enemyHP+i));

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.playerHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Player Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.playerHorizontalMagnet.intensity, magnets.playerHorizontalMagnet.center, magnets.playerHorizontalMagnet.min, magnets.playerHorizontalMagnet.max);
 if (std::abs(magnets.playerVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Player Vertical Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.playerVerticalMagnet.intensity, magnets.playerVerticalMagnet.center, magnets.playerVerticalMagnet.min, magnets.playerVerticalMagnet.max);
 if (std::abs(magnets.scoreMagnet) > 0.0f)                          LOG("[Jaffar]  + Score Magnet                    - Intensity: %.5f\n", magnets.scoreMagnet);
}

