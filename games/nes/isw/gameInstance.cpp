#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Container for game-specific values
  RNGValue       = (uint8_t*)   &_emu->_baseMem[0x00F9];
  heroPosX1      = (uint8_t*)   &_emu->_baseMem[0x0226];
  heroPosX2      = (uint8_t*)   &_emu->_baseMem[0x0213];
  heroPosY1      = (uint8_t*)   &_emu->_baseMem[0x025F];
  heroPosY2      = (uint8_t*)   &_emu->_baseMem[0x024C];
  heroLife       = (uint8_t*)   &_emu->_baseMem[0x006A];
  heroMagic      = (uint8_t*)   &_emu->_baseMem[0x006B];
  heroKeys       = (uint8_t*)   &_emu->_baseMem[0x006C];
  heroMoney1     = (uint8_t*)   &_emu->_baseMem[0x008D];
  heroMoney2     = (uint8_t*)   &_emu->_baseMem[0x008C];
  heroScore1     = (uint8_t*)   &_emu->_baseMem[0x0096];
  heroScore2     = (uint8_t*)   &_emu->_baseMem[0x0095];
  heroScore3     = (uint8_t*)   &_emu->_baseMem[0x0094];
  heroScore4     = (uint8_t*)   &_emu->_baseMem[0x0093];
  heroScore5     = (uint8_t*)   &_emu->_baseMem[0x0092];
  heroScore6     = (uint8_t*)   &_emu->_baseMem[0x0091];
  heroState1     = (uint8_t*)   &_emu->_baseMem[0x000B];
  heroState2     = (uint8_t*)   &_emu->_baseMem[0x000F];
  heroState3     = (uint8_t*)   &_emu->_baseMem[0x0013];
  heroState4     = (uint8_t*)   &_emu->_baseMem[0x0014];
  heroState5     = (uint8_t*)   &_emu->_baseMem[0x02BE];
  heroState6     = (uint8_t*)   &_emu->_baseMem[0x030A];
  heroJumpState  = (uint8_t*)   &_emu->_baseMem[0x007D];
  bossHealth1    = (uint8_t*)   &_emu->_baseMem[0x00CB];
  bossHealth2    = (uint8_t*)   &_emu->_baseMem[0x00C8];
  bossHealth3    = (uint8_t*)   &_emu->_baseMem[0x00C9];
  bossHealth4    = (uint8_t*)   &_emu->_baseMem[0x00CA];
  attackX1       = (uint8_t*)   &_emu->_baseMem[0x0227];
  attackX2       = (uint8_t*)   &_emu->_baseMem[0x0214];
  attackY1       = (uint8_t*)   &_emu->_baseMem[0x0260];
  attackY2       = (uint8_t*)   &_emu->_baseMem[0x024D];

  // Initialize derivative values
  updateDerivedValues();
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

//  hash.Update(*RNGValue      );
  hash.Update(&_emu->_baseMem[0x0200], 0x100);
  hash.Update(*heroLife      );
  hash.Update(*heroMagic     );
  hash.Update(*heroKeys      );
  hash.Update(*heroMoney2    );
  hash.Update(*heroMoney1    );
  hash.Update(*heroScore1    );
  hash.Update(*heroScore2    );
  hash.Update(*heroScore3    );
  hash.Update(*heroScore4    );
  hash.Update(*heroScore5    );
  hash.Update(*heroScore6    );
  hash.Update(*heroState1    );
  hash.Update(*heroState2    );
  hash.Update(*heroState3    );
  hash.Update(*heroState4    );
  hash.Update(*heroState5    );
  hash.Update(*heroState6    );
  hash.Update(*heroJumpState );
  hash.Update(*bossHealth1   );
  hash.Update(*bossHealth2   );
  hash.Update(*bossHealth3   );
  hash.Update(*bossHealth4   );

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 heroPosX = (float)*heroPosX1 * 256.0f + (float)*heroPosX2;
 heroPosY = (float)*heroPosY1 * 256.0f + (float)*heroPosY2;
 heroMoney = (float)*heroMoney2 * 100.0f + (float)*heroMoney1 * 50.0f;
 heroScore = (float)*heroScore6 * 100000.0f + (float)*heroScore5 * 10000.0f + (float)*heroScore4 * 1000.0f + (float)*heroScore3 * 100.0f + (float)*heroScore2 * 10.0f + (float)*heroScore1 * 1.0f;
 bossHealth = (float)*bossHealth1 + (float)*bossHealth2 + (float)*bossHealth3 + (float)*bossHealth4;
 attackX = (float)*attackX1 * 256.0f + (float)*attackX2;
 attackY = (float)*attackY1 * 256.0f + (float)*attackY2;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 // First pass stage 00-00
 moveList.insert(moveList.end(), { "...U....", ".......A", "......B.", "...U..B.", "..D.....", "..D...B.", ".L......", ".L.....A", ".L....B.", ".LDU....", "R.......", "R......A", "R.....B.", ".R.U....", ".LU....", ".RDU....", "RL......", "R.....BA", ".L....BA", "URB", "ULB", "LD", "RD", "..DU..BA", ".LD...BA", ".LDU..B.", "R.DU..BA", "RLDU..B."});

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
  boundedValue = heroPosY;
  boundedValue = std::min(boundedValue, magnets.heroVerticalMagnet.max);
  boundedValue = std::max(boundedValue, magnets.heroVerticalMagnet.min);
  diff = std::abs(magnets.heroVerticalMagnet.center - boundedValue);
  reward += magnets.heroVerticalMagnet.intensity * -diff;

  // Evaluating boss health magnet
  reward += magnets.bossHealthMagnet * bossHealth;

  // Evaluating hero magic magnet
  reward += magnets.heroMagicMagnet * *heroMagic;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + RNG Value:                        %02u\n", *RNGValue);
 LOG("[Jaffar]  + Reward:                           %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX\n", computeHash());
 LOG("[Jaffar]  + Hero Life:                        %02u\n", *heroLife);
 LOG("[Jaffar]  + Hero Magic:                       %02u\n", *heroMagic);
 LOG("[Jaffar]  + Hero Jump State:                  %02u\n", *heroJumpState);
 LOG("[Jaffar]  + Hero Position X:                  %f (%02u, %02u)\n", heroPosX, *heroPosX1, *heroPosX2);
 LOG("[Jaffar]  + Hero Position Y:                  %f (%02u, %02u)\n", heroPosY, *heroPosY1, *heroPosY2);
 LOG("[Jaffar]  + Attack Position X:                %f\n", attackX);
 LOG("[Jaffar]  + Attack Position Y:                %f\n", attackY);
 LOG("[Jaffar]  + Boss HP:                          %f (%02u, %02u, %02u, %02u,)\n", bossHealth, *bossHealth1, *bossHealth2, *bossHealth3, *bossHealth4);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.heroHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Hero Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.heroHorizontalMagnet.intensity, magnets.heroHorizontalMagnet.center, magnets.heroHorizontalMagnet.min, magnets.heroHorizontalMagnet.max);
 if (std::abs(magnets.heroVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Hero Vertical Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.heroVerticalMagnet.intensity, magnets.heroVerticalMagnet.center, magnets.heroVerticalMagnet.min, magnets.heroVerticalMagnet.max);
 if (std::abs(magnets.bossHealthMagnet) > 0.0f)                   LOG("[Jaffar]  + Boss Health Magnet            - Intensity: %.5f\n", magnets.bossHealthMagnet);
 if (std::abs(magnets.heroMagicMagnet) > 0.0f)                    LOG("[Jaffar]  + Hero Magic Magnet             - Intensity: %.5f\n", magnets.heroMagicMagnet);
}

