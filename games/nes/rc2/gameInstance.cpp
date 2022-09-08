#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  gameTimer               = (uint8_t*)   &_emu->_baseMem[0x043C];
  player1Speed            = (uint8_t*)   &_emu->_baseMem[0x0596];
  player1Angle            = (uint8_t*)   &_emu->_baseMem[0x059E];
  player1Standing         = (uint8_t*)   &_emu->_baseMem[0x057A];
  player1PosX1            = (uint8_t*)   &_emu->_baseMem[0x0531];
  player1PosX2            = (uint8_t*)   &_emu->_baseMem[0x052C];
  player1PosY1            = (uint8_t*)   &_emu->_baseMem[0x053B];
  player1PosY2            = (uint8_t*)   &_emu->_baseMem[0x0536];

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

  if (timerTolerance > 0) hash.Update(*gameTimer % timerTolerance);

  hash.Update(*player1Speed);
  hash.Update(*player1Angle);
  hash.Update(*player1Standing);
  hash.Update(*player1PosX1);
  hash.Update(*player1PosX2);
  hash.Update(*player1PosY1);
  hash.Update(*player1PosY2);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 player1PosX = (float)*player1PosX1 * 256.0f + (float)*player1PosX2;
 player1PosY = (float)*player1PosY1 * 256.0f + (float)*player1PosY2;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 // Stage 00
// moveList.insert(moveList.end(), { "R", "L", "LA", "RA", "RB", "LB", "RLA", "RLB", "B", "A", "BA", "D" });

 // Stage 12
// moveList.insert(moveList.end(), { "R", "L", "B", "D", "RL", "RB", "RD", "LB", "LD", "BD", "RLB", "RLD", "LBD", "RLBD"  });

 // Stage scroller
// moveList.insert(moveList.end(), { "...U....", "..D.....", ".L......", ".......A", "R.......", "...U...A", "..D....A", ".L.....A", ".L.U....", ".LD.....", "R.D.....", "R......A", "R..U....", "R.D....A", "R..U...A", ".LD....A", ".L.U...A"});

 moveList.insert(moveList.end(), { "......B.", ".L......", "R.......", ".L....B.", "R.....B."});

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

  // Evaluating player1 health  magnet
  reward += magnets.player1SpeedMagnet * (float)*player1Speed;

  // Evaluating player1 health  magnet
  reward += magnets.player1StandingMagnet * (float)*player1Standing;


  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Game Timer:                           %03u\n", *gameTimer);
 LOG("[Jaffar]  + Player 1 Standing:                    %01u / 4\n", *player1Standing);
 LOG("[Jaffar]  + Player 1 Pos X:                       %f (%03u, %03u)\n", player1PosX, *player1PosX1, *player1PosX2);
 LOG("[Jaffar]  + Player 1 Pos Y:                       %f (%03u, %03u)\n", player1PosY, *player1PosY1, *player1PosY2);
 LOG("[Jaffar]  + Player 1 Vel:                         %03u\n", *player1Speed);
 LOG("[Jaffar]  + Player 1 Angle:                       %03u\n", *player1Angle);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.player1SpeedMagnet) > 0.0f)                     LOG("[Jaffar]  + Player 1 Speed Magnet              - Intensity: %.5f\n", magnets.player1SpeedMagnet);
 if (std::abs(magnets.player1StandingMagnet) > 0.0f)                  LOG("[Jaffar]  + Player 1 Standing Magnet           - Intensity: %.5f\n", magnets.player1StandingMagnet);
}

