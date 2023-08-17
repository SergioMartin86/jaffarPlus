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

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 moves.push_back(move);
 updateDerivedValues();

 return moves;
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (*gameMode != 2)  hash.Update(currentStep);

  if (timerTolerance > 0) hash.Update(currentStep % timerTolerance);

    uint8_t emuState[_STATE_DATA_SIZE_PLAY];
   _emu->serializeState(emuState);
   _emu->advanceState(0);

//  hash.Update(*gameMode);
//  hash.Update(*gameSubMode);
//  hash.Update(*heroAction);
//  hash.Update(*screenScroll1);
//  hash.Update(*screenScroll2);
//  hash.Update(*bossHP);
//  hash.Update(*heroHP);
//  hash.Update(*currentStage);
//  hash.Update(*score1);
//  hash.Update(*score2);
//  hash.Update(*score3);
//  hash.Update(*score4);
//  hash.Update(*score5);
//  hash.Update(*heroActionTimer);
//  hash.Update(*heroScreenPosX);
//  hash.Update(*heroScreenPosY);
//  hash.Update(*heroAirMode);
//  hash.Update(*enemyShrugCounter);
//  hash.Update(*enemyGrabCounter);
//  hash.Update(*bossPosX);
//
//  hash.Update(&_emu->_baseMem[0x0100], 0x6FF);
////  hash.Update(&_emu->_baseMem[0x04A0], 0x20); // Enemy Alive Flags
////  hash.Update(&_emu->_baseMem[0x0300], 0x80 );
////  hash.Update(&_emu->_baseMem[0x03B0], 0x150);
////  hash.Update(&_emu->_baseMem[0x0700], 0x100);
//
//  hash.Update(&_emu->_baseMem[0x0005], 0x000E);
//  hash.Update(&_emu->_baseMem[0x0016], 0x0032);
//  hash.Update(&_emu->_baseMem[0x004A], 0x0025);
//  hash.Update(&_emu->_baseMem[0x006B], 0x0035);

    for (size_t i = 0; i < 0x800; i++)
//     if (i != 0x0000)
//     if (i != 0x0001)
//     if (i != 0x0002)
//     if (i != 0x0003)
//     if (i != 0x0004)
//     if (i != 0x0012)
     if (i != 0x0018)
     if (i != 0x0019)
     if (i != 0x001A)
     if (i != 0x001B)
     if (i != 0x001C)
     if (i != 0x001D)
     if (i != 0x001F)
     if (i != 0x0020)
     if (i != 0x0014)
     if (i != 0x0015)
//     if (i != 0x0033)
//      if (i != 0x0049)
//     if (i != 0x0052)
//     if (i != 0x0066)
//     if (i != 0x0067)
       if (!(i > 0x0300 && i < 0x0400))
//     if (!(i > 0x03A0 && i < 0x03A8))
//     if (!(i > 0x0360 && i < 0x0370))
//     if (i != 0x0068)
//     if (i != 0x0069)
//     if (i != 0x006A)
//     if (i != 0x036E)
//     if (i != 0x036F)
//      if (i != 0x04DF)
//      if (i != 0x07DF)
//      if (i != 0x049B)
//      if (i != 0x03AB)
//      if (i != 0x0393)
      hash.Update(_emu->_baseMem[i]);

//       Reload game state
  _emu->deserializeState(emuState);

  _uint128_t result;
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
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

 if (*gameMode != 2) return {"."};

 // Getting mini-hash
 auto stateMiniHash = getStateMoveHash();

 if (stateMiniHash == 0x0000) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "RA", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0001) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "RA", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0002) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "RA", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0003) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "RA", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0004) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "RA", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0005) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "RA", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0010) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "LA", "LB", "UL", "DL", "LR" });
 if (stateMiniHash == 0x0011) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "LA", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0012) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "LA", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0013) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "LA", "LB", "UL", "DL", "LR" });
 if (stateMiniHash == 0x0014) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "LA", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0015) moveList.insert(moveList.end(), { "A", "B", "U", "D", "L", "R", "LA", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0020) moveList.insert(moveList.end(), { "B", "U", "D", "R", "DA", "DB", "DL", "UR", "DR", "DRA", "DRB" });
 if (stateMiniHash == 0x0030) moveList.insert(moveList.end(), { "B", "U", "D", "L", "DA", "DB", "UL", "DL", "DR", "DLA", "DLB" });
 if (stateMiniHash == 0x0041) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0042) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0043) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0044) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0045) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0046) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0047) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0048) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0049) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x004A) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x004B) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0051) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0052) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0053) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0054) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0055) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0056) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0057) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0058) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0059) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x005A) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x005B) moveList.insert(moveList.end(), { "A", "B" });
 if (stateMiniHash == 0x0101) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0102) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0103) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0104) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0105) moveList.insert(moveList.end(), { "B", "U", "D", "R", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0106) moveList.insert(moveList.end(), { "B", "U", "D", "R", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0107) moveList.insert(moveList.end(), { "B", "U", "D", "R", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0108) moveList.insert(moveList.end(), { "B", "U", "D", "R", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0109) moveList.insert(moveList.end(), { "B", "U", "D", "R", "RB", "UR", "DR" });
 if (stateMiniHash == 0x010A) moveList.insert(moveList.end(), { "B", "U", "D", "R", "RB", "UR", "DR" });
 if (stateMiniHash == 0x010B) moveList.insert(moveList.end(), { "B", "U", "D", "R", "RB", "UR", "DR" });
 if (stateMiniHash == 0x0111) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0112) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0113) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0114) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0115) moveList.insert(moveList.end(), { "B", "U", "D", "L", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0116) moveList.insert(moveList.end(), { "B", "U", "D", "L", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0117) moveList.insert(moveList.end(), { "B", "U", "D", "L", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0118) moveList.insert(moveList.end(), { "B", "U", "D", "L", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0119) moveList.insert(moveList.end(), { "B", "U", "D", "L", "LB", "UL", "DL" });
 if (stateMiniHash == 0x011A) moveList.insert(moveList.end(), { "B", "U", "D", "L", "LB", "UL", "DL" });
 if (stateMiniHash == 0x011B) moveList.insert(moveList.end(), { "B", "U", "D", "L", "LB", "UL", "DL" });
 if (stateMiniHash == 0x0121) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0122) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0123) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0124) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0125) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0126) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0127) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0128) moveList.insert(moveList.end(), { "B", "U", "R", "RB", "UR" });
 if (stateMiniHash == 0x0129) moveList.insert(moveList.end(), { "B", "U", "R", "RB", "UR" });
 if (stateMiniHash == 0x012A) moveList.insert(moveList.end(), { "B", "U", "R", "RB", "UR" });
 if (stateMiniHash == 0x012B) moveList.insert(moveList.end(), { "B", "U", "R", "RB", "UR" });
 if (stateMiniHash == 0x0131) moveList.insert(moveList.end(), { "L" });
 if (stateMiniHash == 0x0132) moveList.insert(moveList.end(), { "L" });
 if (stateMiniHash == 0x0133) moveList.insert(moveList.end(), { "B", "U", "L" });
 if (stateMiniHash == 0x0134) moveList.insert(moveList.end(), { "B", "L" });
 if (stateMiniHash == 0x0135) moveList.insert(moveList.end(), { "B", "U", "L" });
 if (stateMiniHash == 0x0136) moveList.insert(moveList.end(), { "B", "U", "L" });
 if (stateMiniHash == 0x0137) moveList.insert(moveList.end(), { "B", "U", "L" });
 if (stateMiniHash == 0x0138) moveList.insert(moveList.end(), { "B", "U", "L", "LB", "UL" });
 if (stateMiniHash == 0x0139) moveList.insert(moveList.end(), { "B", "U", "L", "LB", "UL" });
 if (stateMiniHash == 0x013A) moveList.insert(moveList.end(), { "B", "U", "L", "LB", "UL" });
 if (stateMiniHash == 0x013B) moveList.insert(moveList.end(), { "B", "U", "L", "LB", "UL" });
 if (stateMiniHash == 0x0201) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0202) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0203) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0204) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0205) moveList.insert(moveList.end(), { "A", "U", "D", "R", "RA", "UR", "DR" });
 if (stateMiniHash == 0x0206) moveList.insert(moveList.end(), { "A", "U", "D", "R", "RA", "UR", "DR" });
 if (stateMiniHash == 0x0207) moveList.insert(moveList.end(), { "A", "U", "D", "R", "RA", "UR", "DR" });
 if (stateMiniHash == 0x0208) moveList.insert(moveList.end(), { "A", "U", "D", "R", "RA", "UR", "DR" });
 if (stateMiniHash == 0x0209) moveList.insert(moveList.end(), { "A", "U", "D", "R", "RA", "UR", "DR" });
 if (stateMiniHash == 0x020A) moveList.insert(moveList.end(), { "A", "U", "D", "R", "RA", "UR", "DR" });
 if (stateMiniHash == 0x020B) moveList.insert(moveList.end(), { "A", "U", "D", "R", "RA", "UR", "DR" });
 if (stateMiniHash == 0x0211) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0212) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0213) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0214) moveList.insert(moveList.end(), { "D", "L" });
 if (stateMiniHash == 0x0215) moveList.insert(moveList.end(), { "A", "U", "D", "L", "LA", "UL", "DL" });
 if (stateMiniHash == 0x0216) moveList.insert(moveList.end(), { "A", "U", "D", "L", "LA", "UL", "DL" });
 if (stateMiniHash == 0x0217) moveList.insert(moveList.end(), { "A", "U", "D", "L", "LA", "UL", "DL" });
 if (stateMiniHash == 0x0218) moveList.insert(moveList.end(), { "A", "U", "D", "L", "LA", "UL", "DL" });
 if (stateMiniHash == 0x0219) moveList.insert(moveList.end(), { "A", "U", "D", "L", "LA", "UL", "DL" });
 if (stateMiniHash == 0x021A) moveList.insert(moveList.end(), { "A", "U", "D", "L", "LA", "UL", "DL" });
 if (stateMiniHash == 0x021B) moveList.insert(moveList.end(), { "A", "U", "D", "L", "LA", "UL", "DL" });
 if (stateMiniHash == 0x0221) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0222) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0223) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0224) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0225) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0226) moveList.insert(moveList.end(), { "R" });
 if (stateMiniHash == 0x0227) moveList.insert(moveList.end(), { "A", "U", "R", "RA", "UR" });
 if (stateMiniHash == 0x0228) moveList.insert(moveList.end(), { "A", "U", "R", "RA", "UR" });
 if (stateMiniHash == 0x0229) moveList.insert(moveList.end(), { "A", "U", "R", "RA", "UR" });
 if (stateMiniHash == 0x022A) moveList.insert(moveList.end(), { "A", "U", "R", "RA", "UR" });
 if (stateMiniHash == 0x022B) moveList.insert(moveList.end(), { "A", "U", "R", "RA", "UR" });
 if (stateMiniHash == 0x0231) moveList.insert(moveList.end(), { "A", "U", "L" });
 if (stateMiniHash == 0x0232) moveList.insert(moveList.end(), { "A", "U", "L" });
 if (stateMiniHash == 0x0233) moveList.insert(moveList.end(), { "A", "U", "L" });
 if (stateMiniHash == 0x0234) moveList.insert(moveList.end(), { "A", "U", "L" });
 if (stateMiniHash == 0x0235) moveList.insert(moveList.end(), { "A", "U", "L" });
 if (stateMiniHash == 0x0236) moveList.insert(moveList.end(), { "A", "U", "L" });
 if (stateMiniHash == 0x0237) moveList.insert(moveList.end(), { "A", "U", "L", "LA", "UL" });
 if (stateMiniHash == 0x0238) moveList.insert(moveList.end(), { "A", "U", "L", "LA", "UL" });
 if (stateMiniHash == 0x0239) moveList.insert(moveList.end(), { "A", "U", "L", "LA", "UL" });
 if (stateMiniHash == 0x023A) moveList.insert(moveList.end(), { "A", "U", "L", "LA", "UL" });
 if (stateMiniHash == 0x023B) moveList.insert(moveList.end(), { "A", "U", "L", "LA", "UL" });

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

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Game Timer:                       %02u\n", *gameTimer);
 LOG("[Jaffar]  + Game Mode:                        %02u-%02u\n", *gameMode, *gameSubMode);
 LOG("[Jaffar]  + Reward:                           %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX0x%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Move Hash:                        0x%lX\n", getStateMoveHash());
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

uint64_t GameInstance::getStateMoveHash() const
{
 return *heroAction * 16 + *heroActionTimer;
}

std::set<INPUT_TYPE> GameInstance::getCandidateMoves() const
{
 std::set<INPUT_TYPE> candidateMoves;

 for (INPUT_TYPE i = 0; i < 0b11111111; i++)
 {
   if ((i & 0b00001000) == 0) // NES Start
   if ((i & 0b00000100) == 0) // NES Select
   if (countButtonsPressedNumber(i) > 3 == false)
   candidateMoves.insert(i);
 }

 return candidateMoves;
}


