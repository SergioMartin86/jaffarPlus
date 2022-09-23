#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameTimer               = (uint8_t*)   &_emu->_baseMem[0x0027];
  cycleTimer              = (uint8_t*)   &_emu->_baseMem[0x0020];

  playerPosX             = (uint8_t*)   &_emu->_baseMem[0x0207];
  playerPosY             = (uint8_t*)   &_emu->_baseMem[0x0208];
  playerFrame1           = (uint8_t*)   &_emu->_baseMem[0x0211];
  playerFrame2           = (uint8_t*)   &_emu->_baseMem[0x0217];
  playerFrame3           = (uint8_t*)   &_emu->_baseMem[0x020B];
  playerAnimation        = (uint8_t*)   &_emu->_baseMem[0x0306];
  ballPosX               = (uint8_t*)   &_emu->_baseMem[0x022B];
  ballPosY               = (uint8_t*)   &_emu->_baseMem[0x022C];
  ballPosZ               = (uint8_t*)   &_emu->_baseMem[0x006F];
  oppPosX                = (uint8_t*)   &_emu->_baseMem[0x0233];
  oppPosY                = (uint8_t*)   &_emu->_baseMem[0x0234];
  ballDirection          = (int8_t*)    &_emu->_baseMem[0x0087];
  playerTurn             = (uint8_t*)   &_emu->_baseMem[0x035B];
  serveFault             = (uint8_t*)   &_emu->_baseMem[0x007A];
  normalFault            = (uint8_t*)   &_emu->_baseMem[0x0079];
  pointEnd               = (uint8_t*)   &_emu->_baseMem[0x0072];
  playerScore            = (uint8_t*)   &_emu->_baseMem[0x0058];
  playerGames            = (uint8_t*)   &_emu->_baseMem[0x005A];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  if (timerTolerance > 0)
  {
   int timerPhase = *gameTimer % timerTolerance;
   hash.Update(&timerPhase);
  }

  if (*pointEnd == 1) hash.Update(*gameTimer);

//  hash.Update(*cycleTimer);
  if (*playerAnimation == 0x0011) hash.Update(*gameTimer);

  hash.Update(*playerPosX);
  hash.Update(*playerPosY);
//  hash.Update(*playerFrame1);
//  hash.Update(*playerFrame2);
//  hash.Update(*playerFrame3);
  hash.Update(*playerAnimation);
  hash.Update(*ballPosX);
  hash.Update(*ballPosY);
  hash.Update(*ballPosZ);
  hash.Update(*oppPosX);
  hash.Update(*oppPosY);
  hash.Update(*ballDirection);
  hash.Update(*playerTurn);
  hash.Update(*serveFault);
  hash.Update(*normalFault);
  hash.Update(*pointEnd);
  hash.Update(*playerScore);
  hash.Update(*playerGames);

  uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 playerBallDistanceX = (float)std::abs(*ballPosX - *playerPosX);
 playerBallDistanceY = (float)std::abs(*ballPosY - *playerPosY);
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*pointEnd == 1) return moveList;

// if (*playerAnimation == 0x0000) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".......A", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", ".L.U....", ".L....B.", "..DU....", "..D....A", "...U..B.", ".L.U...A", ".LD...B.", "R..U..B.", "R.D...B.", "RL.U....", "RLD....."});
// if (*playerAnimation == 0x0001) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0002) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0003) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0004) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0005) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".......A", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", ".L.U....", ".L....B.", "..DU....", "..D....A", "...U..B.", ".L.U...A", ".LD...B.", "R..U..B.", "R.D...B.", "RL.U....", "RLD....."});
// if (*playerAnimation == 0x0006) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0007) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0008) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".L.U....", ".LD.....", "R..U....", "R.D....."});
// if (*playerAnimation == 0x0009) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x000A) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".......A", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", ".L.U....", ".L....B.", "..DU....", "..D....A", "...U..B.", ".L.U...A", ".LD...B.", "R..U..B.", "R.D...B.", "RL.U....", "RLD....."});
// if (*playerAnimation == 0x000C) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x000D) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".......A", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", ".L.U....", ".L....B.", "..DU....", "..D....A", "...U..B.", ".L.U...A", ".LD...B.", "R..U..B.", "R.D...B.", "RL.U....", "RLD....."});
// if (*playerAnimation == 0x000E) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".......A", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", ".L.U....", ".L....B.", "..DU....", "..D....A", "...U..B.", ".L.U...A", ".LD...B.", "R..U..B.", "R.D...B.", "RL.U....", "RLD....."});
// if (*playerAnimation == 0x000F) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".......A", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", ".L.U....", ".L....B.", "..DU....", "..D....A", "...U..B.", ".L.U...A", ".LD...B.", "R..U..B.", "R.D...B.", "RL.U....", "RLD....."});
// if (*playerAnimation == 0x0010) moveList.insert(moveList.end(), { "..D.....", ".L......", "...U....", "R.......", "...U..B.", "..D....A", "..DU....", ".L....B.", ".L.U....", ".LD.....", "R.....B.", "R..U....", "RL......", "R.D.....", "RLD.....", "RL.U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0011) moveList.insert(moveList.end(), { ".......A", ".L......", "R......."});
// if (*playerAnimation == 0x0012) moveList.insert(moveList.end(), { ".......A"});
// if (*playerAnimation == 0x0013) moveList.insert(moveList.end(), { ".L......", "R......."});
// if (*playerAnimation == 0x0014) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".L.U....", ".LD.....", "R..U....", "R.D....."});
// if (*playerAnimation == 0x0015) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".L.U....", ".LD.....", "R..U....", "R.D....."});
// if (*playerAnimation == 0x0016) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x0024) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U..B.", "..D....A", ".L....B.", ".L.U....", ".LD.....", "R.D.....", "R.....B.", "R..U....", "R.D...B.", "R..U..B.", ".LD...B.", ".L.U...A"});
// if (*playerAnimation == 0x003A) moveList.insert(moveList.end(), { "...U....", ".......A", "..D.....", ".L......", "R.......", "RL......", "R.D.....", "R..U....", "R.....B.", ".LD.....", ".L.U....", ".L....B.", "..D....A", "...U..B.", ".LD...B.", ".L.U...A", "R..U..B.", "R.D...B."});
//
// if (*playerAnimation == 0x0000) moveList.insert(moveList.end(), { "..D.....", "R.......", ".L......", "...U....", "R.D.....", "R..U....", "R.....B.", "R......A", "RL......", ".LD.....", ".L.U....", ".L....B.", ".L.....A", "..DU....", "..D...B.", "..D....A", "...U..B.", "...U...A", "RL.U....", "R.D...B.", "R.D....A", "RLD.....", "R..U..B.", "R..U...A", ".LD...B.", ".LD....A", ".L.U..B.", ".L.U...A"});
// if (*playerAnimation == 0x0001) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0002) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0003) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0004) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0005) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", "...U...A", "..D...B.", ".L.....A", ".L.U....", ".LD.....", "R......A", "R..U....", "R.D.....", ".L.U..B.", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0006) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0007) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0009) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x000A) moveList.insert(moveList.end(), { "...U...A", "..D...B.", ".L.....A", "R......A", ".L.U..B.", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x000B) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", "R.......", ".......A", ".LD.....", "RL......", "R.D.....", "R..U....", "R.....B.", ".L.U....", ".L....B.", "..DU....", "..D....A", "...U..B.", ".L.U...A", ".LD...B.", "R.D...B.", "RL.U....", "RLD....."});
// if (*playerAnimation == 0x000C) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x000D) moveList.insert(moveList.end(), { "...U...A", "..D...B.", ".L.....A", "R......A", ".L.U..B.", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x000E) moveList.insert(moveList.end(), { "...U...A", "..D...B.", ".L.....A", "R......A", ".L.U..B.", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x000F) moveList.insert(moveList.end(), { "...U...A", "..D...B.", ".L.....A", "R......A", ".L.U..B.", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0010) moveList.insert(moveList.end(), { "...U...A", "..D...B.", ".L.....A", "R......A", ".L.U..B.", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x0013) moveList.insert(moveList.end(), { "...U....", "..D.....", ".L.U....", ".LD.....", "R..U....", "R.D....."});
// if (*playerAnimation == 0x0024) moveList.insert(moveList.end(), { "......B.", "...U...A", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});
// if (*playerAnimation == 0x003A) moveList.insert(moveList.end(), { "......B.", "...U...A", "..D...B.", "..DU....", ".L.....A", "R......A", ".LD....A", "R..U...A", "R.D....A"});

 moveList.insert(moveList.end(), { "......B.", ".......A", "..D.....", "...U....", ".L......", "R......."});

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

  // Evaluating magnets
  reward += magnets.playerTurnMagnet * *playerTurn;
  reward += magnets.ballDirectionMagnet * *ballDirection;
  reward += magnets.ballPosYMagnet * *ballPosY;
  reward += magnets.playerScoreMagnet * *playerScore;
  reward += magnets.playerBallDistanceXMagnet * (float)playerBallDistanceX;
  reward += magnets.playerBallDistanceYMagnet * (float)playerBallDistanceY;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                             %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                               0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:                         %03u %03u\n", *gameTimer, *cycleTimer);

 LOG("[Jaffar]  + Player Pos:                         (%03u, %03u)\n", *playerPosX, *playerPosY);
 LOG("[Jaffar]  + Player Frame:                       (%03u, %03u, %03u, %03u)\n", *playerFrame1, *playerFrame2, *playerFrame3, *playerAnimation);
 LOG("[Jaffar]  + Opp Pos:                            (%03u, %03u)\n", *oppPosX, *oppPosY);
 LOG("[Jaffar]  + Ball Pos:                           (%03u, %03u, %03u)\n", *ballPosX, *ballPosY, *ballPosZ);
 LOG("[Jaffar]  + Ball Direction:                     (%03d)\n", *ballDirection);
 LOG("[Jaffar]  + Player Turn:                        (%03u)\n", *playerTurn);
 LOG("[Jaffar]  + Serve Fault:                        (%03u)\n", *serveFault);
 LOG("[Jaffar]  + Normal Fault:                       (%03u)\n", *normalFault);
 LOG("[Jaffar]  + Point End:                          (%03u)\n", *pointEnd);
 LOG("[Jaffar]  + Player Score:                       (%03u)\n", *playerScore);
 LOG("[Jaffar]  + Player Games:                       (%03u)\n", *playerGames);
 LOG("[Jaffar]  + Player / Ball Distance:             (X: %.2f, Y: %.2f)\n", playerBallDistanceX, playerBallDistanceY);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.playerTurnMagnet) > 0.0f)                 LOG("[Jaffar]  + Player Turn Magnet                - Intensity: %.5f\n", magnets.playerTurnMagnet);
 if (std::abs(magnets.ballDirectionMagnet) > 0.0f)              LOG("[Jaffar]  + Ball Direction Magnet             - Intensity: %.5f\n", magnets.ballDirectionMagnet);
 if (std::abs(magnets.ballPosYMagnet) > 0.0f)                   LOG("[Jaffar]  + Ball Pos Y Magnet                 - Intensity: %.5f\n", magnets.ballPosYMagnet);
 if (std::abs(magnets.playerScoreMagnet) > 0.0f)                LOG("[Jaffar]  + Player Score Magnet               - Intensity: %.5f\n", magnets.playerScoreMagnet);
 if (std::abs(magnets.playerBallDistanceXMagnet) > 0.0f)        LOG("[Jaffar]  + Player / Ball Distance X Magnet   - Intensity: %.5f\n", magnets.playerBallDistanceXMagnet);
 if (std::abs(magnets.playerBallDistanceYMagnet) > 0.0f)        LOG("[Jaffar]  + Player / Ball Distance Y Magnet   - Intensity: %.5f\n", magnets.playerBallDistanceYMagnet);
}

