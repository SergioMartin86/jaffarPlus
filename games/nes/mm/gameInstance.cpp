#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  gameTimer             = (uint8_t*)   &_emu->_baseMem[0x000A];
  gameCycle             = (uint8_t*)   &_emu->_baseMem[0x07DD];
  winFlag               = (uint8_t*)   &_emu->_baseMem[0x008B];
  marbleState           = (uint8_t*)   &_emu->_baseMem[0x0019];
  marbleFlags           = (uint8_t*)   &_emu->_baseMem[0x0408];
  marblePosX1           = (uint8_t*)   &_emu->_baseMem[0x0398];
  marblePosX2           = (uint8_t*)   &_emu->_baseMem[0x0390];
  marblePosX3           = (uint8_t*)   &_emu->_baseMem[0x0388];
  marblePosY1           = (uint8_t*)   &_emu->_baseMem[0x03B0];
  marblePosY2           = (uint8_t*)   &_emu->_baseMem[0x03A8];
  marblePosY3           = (uint8_t*)   &_emu->_baseMem[0x03A0];
  marblePosZ1           = (uint8_t*)   &_emu->_baseMem[0x03C0];
  marbleAirtime         = (uint8_t*)   &_emu->_baseMem[0x03C8];
  marbleVelX            = (int8_t*)    &_emu->_baseMem[0x03D0];
  marbleVelY            = (int8_t*)    &_emu->_baseMem[0x03E0];
  marbleDeadFlag        = (uint8_t*)   &_emu->_baseMem[0x0400];
  marbleSurfaceAngle    = (uint8_t*)   &_emu->_baseMem[0x0028];

  if (isDefined(config, "Hash Includes") == true)
   for (const auto& entry : config["Hash Includes"])
    hashIncludes.insert(entry.get<std::string>());
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Hash Includes' was not defined\n");

  if (isDefined(config, "Timer Tolerance") == true)  timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  // Updating nametable
  if (hashIncludes.contains("Game Cycle")) hash.Update(*gameCycle);

  // Timer tolerance
  if (timerTolerance > 0) hash.Update(*gameTimer % timerTolerance);

  hash.Update(*winFlag);
  hash.Update(*marbleState);
  hash.Update(*marbleFlags);
  hash.Update(*marblePosX1);
  hash.Update(*marblePosX2);
  hash.Update(*marblePosX3);
  hash.Update(*marblePosY1);
  hash.Update(*marblePosY2);
  hash.Update(*marblePosY3);
  hash.Update(*marblePosZ1);
  hash.Update(*marbleAirtime);
  hash.Update(*marbleVelX);
  hash.Update(*marbleVelY);
  hash.Update(*marbleDeadFlag);
  hash.Update(*marbleSurfaceAngle);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 marblePosX = (float)*marblePosX1 * 256.0f + (float)*marblePosX2 + (float)*marblePosX3 / 256.0f;
 marblePosY = (float)*marblePosY1 * 256.0f + (float)*marblePosY2 + (float)*marblePosY3 / 256.0f;
 marblePosZ = (float)*marblePosZ1;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*winFlag == 0x01) moveList.insert(moveList.end(), { ".......A", "...U....", "...U...A", "..D.....", "..D....A", ".L......", ".L.....A", ".L.U....", ".L.U...A", ".LD.....", ".LD....A", "R.......", "R......A", "R..U....", "R..U...A", "R.D.....", "R.D....A", "RL......", "RL.....A"});

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

  // Evaluating marble reward on position X
  boundedValue = (float)marblePosX;
  boundedValue = std::min(boundedValue, magnets.marbleXMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marbleXMagnet.min);
  diff = std::abs(magnets.marbleXMagnet.center - boundedValue);
  reward += magnets.marbleXMagnet.intensity * -diff;

  // Evaluating marble reward on position Y
  boundedValue = (float)marblePosY;
  boundedValue = std::min(boundedValue, magnets.marbleYMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marbleYMagnet.min);
  diff = std::abs(magnets.marbleYMagnet.center - boundedValue);
  reward += magnets.marbleYMagnet.intensity * -diff;

  // Evaluating marble reward on position Z
  boundedValue = (float)marblePosZ;
  boundedValue = std::min(boundedValue, magnets.marbleZMagnet.max);
  boundedValue = std::max(boundedValue, magnets.marbleZMagnet.min);
  diff = std::abs(magnets.marbleZMagnet.center - boundedValue);
  reward += magnets.marbleZMagnet.intensity * -diff;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{

}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Game Timer:             %02u\n", *gameTimer);
 LOG("[Jaffar]  + Game Cycle:             %02u\n", *gameCycle);
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX\n", computeHash());
 LOG("[Jaffar]  + Marble State:           %02u\n", *marbleState);
 LOG("[Jaffar]  + Marble Flags:           %02u\n", *marbleFlags);
 LOG("[Jaffar]  + Marble Pos X:           %f, (%02u, %02u, %02u)\n", marblePosX, *marblePosX1, *marblePosX2, *marblePosX3);
 LOG("[Jaffar]  + Marble Pos Y:           %f, (%02u, %02u, %02u)\n", marblePosY, *marblePosY1, *marblePosY2, *marblePosY3);
 LOG("[Jaffar]  + Marble Pos Z:           %f\n", marblePosZ);
 LOG("[Jaffar]  + Marble Vel X:           %02d\n", *marbleVelX);
 LOG("[Jaffar]  + Marble Vel Y:           %02d\n", *marbleVelY);
 LOG("[Jaffar]  + Marble Airtime:         %02u\n", *marbleAirtime);
 LOG("[Jaffar]  + Marble Surface Angle:   %02u\n", *marbleSurfaceAngle);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);
 if (std::abs(magnets.marbleXMagnet.intensity) > 0.0f)    LOG("[Jaffar]  + Marble X Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.marbleXMagnet.intensity, magnets.marbleXMagnet.center, magnets.marbleXMagnet.min, magnets.marbleXMagnet.max);
 if (std::abs(magnets.marbleYMagnet.intensity) > 0.0f)    LOG("[Jaffar]  + Marble Y Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.marbleYMagnet.intensity, magnets.marbleYMagnet.center, magnets.marbleYMagnet.min, magnets.marbleYMagnet.max);
 if (std::abs(magnets.marbleZMagnet.intensity) > 0.0f)    LOG("[Jaffar]  + Marble Z Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.marbleZMagnet.intensity, magnets.marbleZMagnet.center, magnets.marbleZMagnet.min, magnets.marbleZMagnet.max);
}

