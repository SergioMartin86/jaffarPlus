#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
 // Setting emulator
 _emu = emu;

 gameTimer        = (uint16_t*)  &_emu->_baseMem[0x0002];
 gameFrame        = (uint8_t*)   &_emu->_baseMem[0x0000];
 isLagFrame       = (uint8_t*)   &_emu->_baseMem[0x020C];

 kidRoom          = (uint8_t*)   &_emu->_baseMem[0x0472];
 kidPosX          = (uint8_t*)   &_emu->_baseMem[0x0458];
 kidPosY          = (uint8_t*)   &_emu->_baseMem[0x0459];
 kidDirection     = (uint8_t*)   &_emu->_baseMem[0x045A];
 kidHP            = (uint8_t*)   &_emu->_baseMem[0x0508];
 kidFrame         = (uint8_t*)   &_emu->_baseMem[0x0457];
 kidAction        = (uint8_t*)   &_emu->_baseMem[0x045D];
 kidCol           = (uint8_t*)   &_emu->_baseMem[0x045B];
 kidRow           = (uint8_t*)   &_emu->_baseMem[0x045C];

 // Initialize derivative values
 updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  hash.Update(*gameFrame);

  hash.Update(*isLagFrame);
  if (*isLagFrame == 143)
  {
   hash.Update(*gameTimer);
   hash.Update( &_emu->_baseMem[0x0000], 0x100);
  }

  hash.Update(*kidRoom);
  hash.Update(*kidPosX);
  hash.Update(*kidPosY);
  hash.Update(*kidDirection);
  hash.Update(*kidHP);
  hash.Update(*kidFrame);
  hash.Update(*kidAction);
  hash.Update(*kidCol);
  hash.Update(*kidRow);

  hash.Update( &_emu->_baseMem[0x0010], 0x1F0);
  hash.Update( &_emu->_baseMem[0x0400], 0x200);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*gameFrame == 3) return { ".", "L", "R", "U", "A", "D", "B", "LA", "RA", "DA", "DB", "LDA", "RDA" };

 return moveList;
}

uint16_t GameInstance::advanceState(const INPUT_TYPE &move)
{
  uint16_t skippedFrames = 0;

  _emu->advanceState(move);

  return skippedFrames;
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;
 memset(&magnets, 0, sizeof(magnets));

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
  {
    if (_rules[ruleId]->_magnets[*kidRoom].kidHorizontalMagnet.active == true) magnets.kidHorizontalMagnet = _rules[ruleId]->_magnets[*kidRoom].kidHorizontalMagnet;
    if (_rules[ruleId]->_magnets[*kidRoom].kidVerticalMagnet.active == true) magnets.kidVerticalMagnet = _rules[ruleId]->_magnets[*kidRoom].kidVerticalMagnet;
  }

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

 // Evaluating kid magnet's reward on position X
 boundedValue = (float)*kidPosX;
 boundedValue = std::min(boundedValue, magnets.kidHorizontalMagnet.max);
 boundedValue = std::max(boundedValue, magnets.kidHorizontalMagnet.min);
 diff = -255.0f + std::abs(magnets.kidHorizontalMagnet.center - boundedValue);
 reward += magnets.kidHorizontalMagnet.intensity * -diff;

 // Evaluating kid magnet's reward on position Y
 boundedValue = (float)*kidPosY;
 boundedValue = std::min(boundedValue, magnets.kidVerticalMagnet.max);
 boundedValue = std::max(boundedValue, magnets.kidVerticalMagnet.min);
 diff = -255.0f + std::abs(magnets.kidVerticalMagnet.center - boundedValue);
 reward += magnets.kidVerticalMagnet.intensity * -diff;

 // Returning reward
 return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                 %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                   0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Timer:             %05u\n", *gameTimer);
 LOG("[Jaffar]  + Game Frame:             %02u\n", *gameFrame);
 LOG("[Jaffar]  + Is Lag Frame:           %02u\n", *isLagFrame);

 LOG("[Jaffar]  + Kid Room:               %02u\n", *kidRoom);
 LOG("[Jaffar]  + Kid Pos X:              %02u\n", *kidPosX);
 LOG("[Jaffar]  + Kid Pos Y:              %02u\n", *kidPosY);
 LOG("[Jaffar]  + Kid Direction:          %02u\n", *kidDirection);
 LOG("[Jaffar]  + Kid HP:                 %02u\n", *kidHP);
 LOG("[Jaffar]  + Kid Frame:              %02u\n", *kidFrame);
 LOG("[Jaffar]  + Kid Action:             %02u\n", *kidAction);
 LOG("[Jaffar]  + Kid Col:                %02u\n", *kidCol);
 LOG("[Jaffar]  + Kid Row:                %02u\n", *kidRow);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

}

