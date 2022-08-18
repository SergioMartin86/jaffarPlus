#include "gameInstance.hpp"
#include "gameRule.hpp"

#define ENEMY_COUNT 4

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameMode           = (uint8_t*)   &_emu->_baseMem[0x0051];
  gameSubMode        = (uint8_t*)   &_emu->_baseMem[0x0008];
  gameTimer          = (uint8_t*)   &_emu->_baseMem[0x0049];
  heroAction         = (uint8_t*)   &_emu->_baseMem[0x0069];
  screenScroll1      = (uint8_t*)   &_emu->_baseMem[0x00E5];
  screenScroll2      = (uint8_t*)   &_emu->_baseMem[0x00D4];
  bossHP             = (int8_t*)    &_emu->_baseMem[0x04A5];
  heroHP             = (int8_t*)    &_emu->_baseMem[0x04A6];
  currentStage       = (uint8_t*)   &_emu->_baseMem[0x0058];
  score1             = (uint8_t*)   &_emu->_baseMem[0x0535];
  score2             = (uint8_t*)   &_emu->_baseMem[0x0534];
  score3             = (uint8_t*)   &_emu->_baseMem[0x0533];
  score4             = (uint8_t*)   &_emu->_baseMem[0x0532];
  score5             = (uint8_t*)   &_emu->_baseMem[0x0531];
  heroActionTimer    = (uint8_t*)   &_emu->_baseMem[0x0021];
  heroScreenPosX     = (uint8_t*)   &_emu->_baseMem[0x0094];
  heroScreenPosY     = (uint8_t*)   &_emu->_baseMem[0x00B6];
  heroAirMode        = (uint8_t*)   &_emu->_baseMem[0x036A];
  enemyShrugCounter  = (uint8_t*)   &_emu->_baseMem[0x0378];
  enemyGrabCounter   = (uint8_t*)   &_emu->_baseMem[0x0374];
  enemyPosX          = (uint8_t*)   &_emu->_baseMem[0x008E];
  enemyAction        = (uint8_t*)   &_emu->_baseMem[0x0080];
  enemyActionTimer   = (uint8_t*)   &_emu->_baseMem[0x002B];
  bossPosX           = (uint8_t*)   &_emu->_baseMem[0x0093];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  if (*gameMode != 2)  hash.Update(*gameTimer);

  if (timerTolerance > 0) hash.Update(*gameTimer % timerTolerance);
  hash.Update(*gameMode);
  hash.Update(*gameSubMode);
  hash.Update(*heroAction);
  hash.Update(*screenScroll1);
  hash.Update(*screenScroll2);
  hash.Update(*bossHP);
  hash.Update(*heroHP);
  hash.Update(*currentStage);
  hash.Update(*score1);
  hash.Update(*score2);
  hash.Update(*score3);
  hash.Update(*score4);
  hash.Update(*score5);
  hash.Update(*heroActionTimer);
  hash.Update(*heroScreenPosX);
  hash.Update(*heroScreenPosY);
  hash.Update(*heroAirMode);
  hash.Update(*enemyShrugCounter);
  hash.Update(*enemyGrabCounter);
  hash.Update(*bossPosX);

  hash.Update(&_emu->_baseMem[0x04A0], 0x20); // Enemy Alive Flags
  hash.Update(&_emu->_baseMem[0x0300], 0x80 );
  hash.Update(&_emu->_baseMem[0x03B0], 0x150);
  hash.Update(&_emu->_baseMem[0x0700], 0x100);

  hash.Update(&_emu->_baseMem[0x0005], 0x000E);
  hash.Update(&_emu->_baseMem[0x0016], 0x0032);
  hash.Update(&_emu->_baseMem[0x004A], 0x0025);
  hash.Update(&_emu->_baseMem[0x006B], 0x0035);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 heroPosX = (float)*screenScroll1 * 256.0f + (float)*screenScroll2 + (float)*heroScreenPosX;
 heroPosY =  (float)*heroScreenPosY;
 score = (float)*score5 * 100000.0f + (float)*score4 * 10000.0f + (float)*score3 * 1000.0f + (float)*score2 * 100.0f + (float)*score1 * 10.0f;

 enemyPosImbalance = 0;
 for (uint8_t i = 0; i < ENEMY_COUNT; i++)
  if (*(enemyActionTimer+i) != 0)
   if (*(enemyAction+i) == 3) enemyPosImbalance++;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*gameMode != 2) return {"."};

 // First pass stage 00-00
 if (*heroAction == 0x0000) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "R.......", ".......A", "...U...A", "...U..B.", "..DU....", ".L.U....", "RL......", ".L.....A", "R......A", "R.....B.", "R..U....", "R.D.....", "RL.U....", "R.DU....", "R..U..B.", "R..U...A", ".L.U..B.", ".L.U...A"});
 if (*heroAction == 0x0001) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "R.......", ".......A", "R..U....", "R......A", "R.D.....", ".LD.....", "RL......", ".L.U....", ".L....B.", ".L.....A", "..DU....", "...U..B.", "...U...A", "R..U..B.", "RL.U....", "R..U...A", ".LDU....", ".LD...B.", ".L.U..B.", ".L.U...A"});
 if (*heroAction == 0x0002) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".......A", ".L......",  "R.......", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", "R......A", ".L.U....", "...U...A", "..DU....", "..D...B.", "..D....A", "...U..B.", "R..U..B.", "RL.U....", "R.DU....", "R.D...B.", "R.D....A", "..DU..B.", "R..U...A", ".LDU....", "..DU...A", ".L.U..B.", ".L.U...A", "R.DU...A", "R.DU..B."});
 if (*heroAction == 0x0003) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".......A", ".L......",  "R.......", ".L.U....", "R.D.....", "R..U....", ".LD.....", ".L....B.", ".L.....A", "..DU....", "..D...B.", "...U...A", "..D....A", "...U..B.", ".LDU....", "RL.U....", "R.DU....", "R..U..B.", "R..U...A", "..DU..B.", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A", "..DU...A", ".LDU...A", ".LDU..B."});
 if (*heroAction == 0x0004) moveList.insert(moveList.end(), { ".......A", "......B.", ".L......", "R.......", ".L....B.", "R.....B.", ".L.U....", "R..U....", ".L.U..B.", "R..U..B."});
 if (*heroAction == 0x0005) moveList.insert(moveList.end(), { ".......A", "......B.", ".L......", "R.......", ".L....B.", "R.....B.", ".L.U....", "R..U....", ".L.U..B.", "R..U..B."});
 if (*heroAction == 0x0010) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", "...U..B.", "..DU....", "R.....B.", "R..U....", "R.D.....", "RL......", "R..U..B.", "R.DU...."});
 if (*heroAction == 0x0011) moveList.insert(moveList.end(), { "......B.", "...U....", "..D.....", ".L......", "...U..B.", "..DU....", ".L....B.", ".L.U....", ".LD.....", ".L.U..B.", ".LDU...."});
 if (*heroAction == 0x0012) moveList.insert(moveList.end(), { "......B.", "...U....", "...U..B.", "R.....B.", "R..U....", "RL......", "R..U..B."});
 if (*heroAction == 0x0013) moveList.insert(moveList.end(), { "......B.", "...U....", ".L......", "R.......", "...U..B.", ".L....B.", ".L.U....", ".L.U..B."});
 if (*heroAction == 0x0014) moveList.insert(moveList.end(), {  ".L......", "R.......", ".L....B.", "R.....B.", ".L.U..B.", "R..U..B."});
 if (*heroAction == 0x0015) moveList.insert(moveList.end(), { ".......A", "......B.", ".L......", "R.......", ".L....B.", "R.....B.", ".L.U..B.", "R..U..B."});
 if (*heroAction == 0x0020) moveList.insert(moveList.end(), { ".......A", "...U....", ".L......", "R.......", "..D.....", ".L.....A", "...U...A", "..DU....", "R......A", "R..U....", "R.D.....", "RL......", "R..U...A", "R.DU...."});
 if (*heroAction == 0x0021) moveList.insert(moveList.end(), { ".......A", "...U....", "..D.....", ".L......",  "R.......", "...U...A", "..DU....", ".L.....A", ".L.U....", ".LD.....", ".L.U...A", ".LDU...."});
 if (*heroAction == 0x0022) moveList.insert(moveList.end(), { ".......A", "...U....", "...U...A", "R......A", "R..U....", "RL......", "R..U...A"});
 if (*heroAction == 0x0023) moveList.insert(moveList.end(), { ".......A", "...U....", ".L......",  "R.......", "...U...A", ".L.....A", ".L.U....", ".L.U...A"});

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

  // Evaluating hero magnet's reward on position X
  boundedValue = heroPosX;
  boundedValue = std::min(boundedValue, magnets.heroHorizontalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.heroHorizontalMagnet.min);
  diff = std::abs(magnets.heroHorizontalMagnet.center - boundedValue);
  reward += magnets.heroHorizontalMagnet.intensity * -diff;

  // Evaluating hero magnet's reward on position Y
  boundedValue = (float)heroPosY;
  boundedValue = std::min(boundedValue, magnets.heroVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.heroVerticalMagnet.min);
  diff = std::abs(magnets.heroVerticalMagnet.center - boundedValue);
  reward += magnets.heroVerticalMagnet.intensity * -diff;

  // Evaluating boss health magnet
  auto bossHPCorrected = *bossHP;
  if (bossHPCorrected < 0) bossHPCorrected = 0;
  reward += magnets.bossHealthMagnet * bossHPCorrected;

  // Evaluating hero health  magnet
  reward += magnets.heroHealthMagnet * *heroHP;

  // Evaluating hero health  magnet
  reward += magnets.scoreMagnet * score;

  // Evaluating position imbalance magnet
  reward += magnets.positionImbalanceMagnet * enemyPosImbalance;

  // Evaluating position imbalance magnet
  reward += magnets.bossHorizontalMagnet * (float)*bossPosX;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Game Timer:                       %02u\n", *gameTimer);
 LOG("[Jaffar]  + Game Mode:                        %02u-%02u\n", *gameMode, *gameSubMode);
 LOG("[Jaffar]  + Reward:                           %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX\n", computeHash());
 LOG("[Jaffar]  + Score:                            %f\n", score);
 LOG("[Jaffar]  + Hero HP:                          %02d\n", *heroHP);
 LOG("[Jaffar]  + Hero Air Mode:                    %02u\n", *heroAirMode);
 LOG("[Jaffar]  + Hero Action:                      0x%02X (0x%02X)\n", *heroAction, *heroActionTimer);
 LOG("[Jaffar]  + Hero Position X:                  %f\n", heroPosX);
 LOG("[Jaffar]  + Hero Position Y:                  %f\n", heroPosY);
 LOG("[Jaffar]  + Boss HP:                          %02d\n", *bossHP);
 LOG("[Jaffar]  + Boss Pos X:                       %02d\n", *bossPosX);
 LOG("[Jaffar]  + Enemy Position Imbalance:         %02d\n", enemyPosImbalance);
 LOG("[Jaffar]  + Enemy Shrug Counter:              %02u\n", *enemyShrugCounter);
 LOG("[Jaffar]  + Enemy Grab Counter:               %02u\n", *enemyGrabCounter);

 for (uint8_t i = 0; i < ENEMY_COUNT; i++)
  LOG("[Jaffar]    + Enemy %02u - X: (%3u), Action:(%3u), Action Timer:(%3u)\n", i, *(enemyPosX+i), *(enemyAction+i), *(enemyActionTimer+i));

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.heroHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Hero Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.heroHorizontalMagnet.intensity, magnets.heroHorizontalMagnet.center, magnets.heroHorizontalMagnet.min, magnets.heroHorizontalMagnet.max);
 if (std::abs(magnets.heroVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Hero Vertical Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.heroVerticalMagnet.intensity, magnets.heroVerticalMagnet.center, magnets.heroVerticalMagnet.min, magnets.heroVerticalMagnet.max);
 if (std::abs(magnets.bossHealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Boss Health Magnet            - Intensity: %.5f\n", magnets.bossHealthMagnet);
 if (std::abs(magnets.heroHealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Hero Health Magnet            - Intensity: %.5f\n", magnets.heroHealthMagnet);
 if (std::abs(magnets.scoreMagnet) > 0.0f)                        LOG("[Jaffar]  + Score Magnet                  - Intensity: %.5f\n", magnets.scoreMagnet);
 if (std::abs(magnets.positionImbalanceMagnet) > 0.0f)            LOG("[Jaffar]  + Position Imbalance Magnet     - Intensity: %.5f\n", magnets.positionImbalanceMagnet);
 if (std::abs(magnets.bossHorizontalMagnet) > 0.0f)               LOG("[Jaffar]  + Boss Horizontal Magnet        - Intensity: %.5f\n", magnets.bossHorizontalMagnet);
}

