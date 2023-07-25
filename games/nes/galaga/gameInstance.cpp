#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  gameMode    = (uint8_t*)  &_emu->_baseMem[0x0018];
  gameTimer   = (uint8_t*)  &_emu->_baseMem[0x0048];
  shot1PosX   = (uint8_t*)  &_emu->_baseMem[0x02E2];
  shot1PosY   = (uint8_t*)  &_emu->_baseMem[0x02E1];
  shot1Active = (uint8_t*)  &_emu->_baseMem[0x02E0];
  shot2PosX   = (uint8_t*)  &_emu->_baseMem[0x02EA];
  shot2PosY   = (uint8_t*)  &_emu->_baseMem[0x02E9];
  shot2Active = (uint8_t*)  &_emu->_baseMem[0x02E8];
  shipPosX    = (uint8_t*)  &_emu->_baseMem[0x0014];
  score10e1   = (uint8_t*)  &_emu->_baseMem[0x00E6];
  score10e2   = (uint8_t*)  &_emu->_baseMem[0x00E5];
  score10e3   = (uint8_t*)  &_emu->_baseMem[0x00E4];
  score10e4   = (uint8_t*)  &_emu->_baseMem[0x00E3];
  score10e5   = (uint8_t*)  &_emu->_baseMem[0x00E2];
  score10e6   = (uint8_t*)  &_emu->_baseMem[0x00E1];
  currentStage  = (uint8_t*)  &_emu->_baseMem[0x0482];
  enemiesRemaining  = (uint8_t*)  &_emu->_baseMem[0x0494];
  activeEnemies  = (uint8_t*)  &_emu->_baseMem[0x0495];

  endLevelTimer  = (uint8_t*)  &_emu->_baseMem[0x01FB];
  endLevelState  = (uint8_t*)  &_emu->_baseMem[0x01FC];

  movingEnemiesState = (uint8_t*)  &_emu->_baseMem[0x0220];
  movingEnemiesPosY  = (uint8_t*)  &_emu->_baseMem[0x0222];
  movingEnemiesPosX  = (uint8_t*)  &_emu->_baseMem[0x0223];

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  // If timer tolerance is set, use the game tick for hashing
  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));

//  hash.Update(*gameTimer % 2);
  hash.Update(*gameMode  );
  hash.Update(*shot1PosX  );
  hash.Update(*shot1PosY  );
  hash.Update(*shot2PosX  );
  hash.Update(*shot2PosY  );
  hash.Update(*shot1Active  );
  hash.Update(*shot2Active  );
  hash.Update(*shipPosX  );
  hash.Update(*score10e1 );
  hash.Update(*score10e2 );
  hash.Update(*score10e3 );
  hash.Update(*score10e4 );
  hash.Update(*score10e5 );
  hash.Update(*score10e6 );
  hash.Update(*currentStage );
  hash.Update(*enemiesRemaining );
  hash.Update(*endLevelTimer);
  hash.Update(*endLevelState);
  hash.Update(*activeEnemies);

  for (size_t i = 0; i < _MOVING_ENEMY_SLOTS; i++)
  {
//   hash.Update(movingEnemiesState[i * _MOVING_ENEMY_OFFSET]);
//   hash.Update(movingEnemiesPosY[i * _MOVING_ENEMY_OFFSET]);
//   hash.Update(movingEnemiesPosX[i * _MOVING_ENEMY_OFFSET]);
  }

  // Game Cycle
//  hash.Update(_emu->_baseMem[0x0072]);
//  hash.Update(&_emu->_baseMem[0x0000], 0x10);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 double _score10e1 = *score10e1 == 16 ? 0 : (double)*score10e1 * 10.0;
 double _score10e2 = *score10e2 == 16 ? 0 : (double)*score10e2 * 100.0;
 double _score10e3 = *score10e3 == 16 ? 0 : (double)*score10e3 * 1000.0;
 double _score10e4 = *score10e4 == 16 ? 0 : (double)*score10e4 * 10000.0;
 double _score10e5 = *score10e5 == 16 ? 0 : (double)*score10e5 * 100000.0;
 double _score10e6 = *score10e6 == 16 ? 0 : (double)*score10e6 * 1000000.0;
 score = _score10e1 + _score10e2 + _score10e3 + _score10e4 + _score10e5 + _score10e6;

 shotCount = 0;
 if (*shot1Active == 0x80 ) shotCount++;
 if (*shot2Active == 0x80 ) shotCount++;

 closestMovingEnemyIdx = -1;
 closestMovingEnemyPosX = 0;
 closestMovingEnemyPosY = 0;

 for (int i = 0; i < _MOVING_ENEMY_SLOTS; i++)
 {
  auto state = movingEnemiesState[i * _MOVING_ENEMY_OFFSET];
  auto posY = movingEnemiesPosY[i * _MOVING_ENEMY_OFFSET];
  auto posX = movingEnemiesPosX[i * _MOVING_ENEMY_OFFSET];

  if (state != 0x80)
  {
   if (posY > closestMovingEnemyPosY)
   {
    closestMovingEnemyIdx = i;
    closestMovingEnemyPosY = posY;
    closestMovingEnemyPosX = posX;
   }
  }
 }

}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 moves.push_back(move);
 updateDerivedValues();

 return moves;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
  if (shotCount > 0)
  {
   if (*gameTimer % 2 == 0) return { "L", "R", "LA", "RA" };
   if (*gameTimer % 2 == 1) return { "L", "R", "LB", "RB" };
  }
  return { "L", "R" };
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

  reward -= (double)*enemiesRemaining;

  uint8_t realEndLevelTimer = *endLevelTimer;
  if (realEndLevelTimer > 120) realEndLevelTimer = 0;

  if (*endLevelState == 75) reward += 521.0 - (double)realEndLevelTimer;
  if (*endLevelState == 78) reward += 522.0;
  reward += 10000.0 * (double)*currentStage;
  if (*endLevelState == 127) reward += 121.0 - (double)realEndLevelTimer;
  if (*endLevelState == 132) reward += 122.0;
  if (*endLevelState == 145) reward += 123.0;
  if (*endLevelState == 91) reward += 124.0;

  if (*endLevelState == 181) reward += 121.0 - (double)realEndLevelTimer;
  if (*endLevelState == 190) reward += 322.0 - (double)realEndLevelTimer;
  if (*endLevelState == 193) reward += 323.0;
  if (*endLevelState == 210) reward += 324.0;

  if ( closestMovingEnemyIdx != -1)
  {
    reward -= 0.001 * closestMovingEnemyPosY;
    reward -= 0.0001 * (float)std::abs(closestMovingEnemyPosX - *shot1PosX);
    reward -= 0.0001 * (float)std::abs(closestMovingEnemyPosX - *shot2PosX);
  }

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                           %.10f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Mode:                        %2u\n", *gameMode  );
 LOG("[Jaffar]  + Current Stage:                    %2u\n", *currentStage );
 LOG("[Jaffar]  + Score:                            %f\n",  score );
 LOG("[Jaffar]  + Active Enemies:                   %u\n",  *activeEnemies );
 LOG("[Jaffar]  + Enemies Remaining:                %u\n",  *enemiesRemaining );
 LOG("[Jaffar]  + Shot Count:                       %u\n",  shotCount );
 LOG("[Jaffar]  + Shot 1 Pos X:                     %2u\n", *shot1PosX  );
 LOG("[Jaffar]  + Shot 1 Pos Y:                     %2u\n", *shot1PosY  );
 LOG("[Jaffar]  + Shot 1 Active:                    %2u\n", *shot1Active  );
 LOG("[Jaffar]  + Shot 2 Pos X:                     %2u\n", *shot2PosX  );
 LOG("[Jaffar]  + Shot 2 Pos Y:                     %2u\n", *shot2PosY  );
 LOG("[Jaffar]  + Shot 2 Active:                    %2u\n", *shot2Active  );
 LOG("[Jaffar]  + Ship Pos X:                       %2u\n", *shipPosX  );
 LOG("[Jaffar]  + End Level Timer:                  %2u\n", *endLevelTimer  );
 LOG("[Jaffar]  + End Level State:                  %2u\n", *endLevelState  );

 LOG("[Jaffar]  + Moving Enemies:\n");
 for (int i = 0; i < _MOVING_ENEMY_SLOTS; i++)
 {
  auto state = movingEnemiesState[i * _MOVING_ENEMY_OFFSET];
  auto posY = movingEnemiesPosY[i * _MOVING_ENEMY_OFFSET];
  auto posX = movingEnemiesPosX[i * _MOVING_ENEMY_OFFSET];
  if (state != 0x80)
  {
   LOG("[Jaffar]    + Enemy %2u, State: %2u, PosX: %3u, PosY: %3u", i, state, posX, posY);
   if (i == closestMovingEnemyIdx) LOG(" (Closest) ");
   LOG("\n");
  }
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");
}

