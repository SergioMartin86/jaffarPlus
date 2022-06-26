#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
 // Setting emulator
 _emu = emu;

 // Setting memory positions
 currentStageRaw         = (uint8_t*)   &_emu->_68KRam[0xEC70];
 gameTimer               = (uint8_t*)   &_emu->_68KRam[0xE882];
 lagFrame                = (uint8_t*)   &_emu->_68KRam[0xFF5B];
 inputFrame              = (uint8_t*)   &_emu->_68KRam[0xFFC4];
 animationFrame          = (uint8_t*)   &_emu->_68KRam[0xE800];
 gameMode                = (uint8_t*)   &_emu->_68KRam[0xED9C];

 lesterPosX              = (int16_t*)   &_emu->_68KRam[0xEA86];
 lesterPosY              = (int16_t*)   &_emu->_68KRam[0xEA88];
 lesterRoom              = (uint8_t*)   &_emu->_68KRam[0xEB52];
 lesterFrame             = (uint16_t*)  &_emu->_68KRam[0xECAC];
 lesterState1            = (uint8_t*)   &_emu->_68KRam[0xEB4A];

 lesterHasGun            = (int16_t*)   &_emu->_68KRam[0xECD8];
 lesterGunCharge         = (int16_t*)   &_emu->_68KRam[0xEA90];
 lesterDeadFlag          = (uint8_t*)   &_emu->_68KRam[0xEA8D];

 // Alien
 alienDeadFlag           = (uint8_t*)   &_emu->_68KRam[0xEB5B];
 alienPosX               = (int16_t*)   &_emu->_68KRam[0xEB54];
 alienRoom               = (uint8_t*)   &_emu->_68KRam[0xEB58];

 // Stage 01 Specific values
 stage01AppearTimer       = (uint8_t*)   &_emu->_68KRam[0xEAAD];
 stage01SkipMonsterFlag   = (uint8_t*)   &_emu->_68KRam[0xEB40];
 stage01VineState         = (int16_t*)   &_emu->_68KRam[0xEC90];
 stage01Finish            = (uint8_t*)   &_emu->_68KRam[0xEAE9];

 // Stage 02 Specific values
 stage02AngularMomentum1  = (uint8_t*)   &_emu->_68KRam[0xEAAE];
 stage02AngularMomentum2  = (uint8_t*)   &_emu->_68KRam[0xEAB0];

 // Initialize derivative values
 updateDerivedValues();
}

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash64 hash;

  if (*animationFrame == 1) hash.Update(_emu->_68KRam+0x0000, 0xFFFF);

//  hash.Update(_emu->_68KRam+0xE880, 0x0560);
  hash.Update(_emu->_68KRam+0xEA80, 0x040);
  hash.Update(_emu->_68KRam+0xEC70, 0x090);
//    hash.Update(_emu->_68KRam+0x0000, 0x0080);
//    hash.Update(_emu->_68KRam+0xFF50, 0x00B0);

  hash.Update(*currentStageRaw);
  hash.Update(*gameTimer);
  hash.Update(*gameMode);
  hash.Update(*lagFrame);

  hash.Update(*animationFrame);
  hash.Update(*lesterPosX);
  hash.Update(*lesterPosY);
  hash.Update(*lesterRoom);
  hash.Update(*lesterHasGun);
  hash.Update(*lesterGunCharge);

  hash.Update(*alienPosX);
  hash.Update(*alienRoom);

  if (currentStage == 1)
  {
   hash.Update(*stage01AppearTimer);
   hash.Update(*stage01SkipMonsterFlag);
   hash.Update(*stage01VineState);
   hash.Update(*stage01Finish);
  }

  if (currentStage == 2)
  {
   hash.Update(*stage02AngularMomentum1);
   hash.Update(*stage02AngularMomentum2);
  }


//  hash.Update(_emu->_CRam, 0x40);

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 currentStage = *currentStageRaw+1;

 lesterAbsolutePosX = *lesterPosX > 0 ? *lesterPosX : -*lesterPosX;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

 if (*inputFrame == 255) return moveList;

 // Stage01a

// if (*lesterFrame == 0x41CC) moveList.insert(moveList.end(), { "....A.......", "...R........", "..L.........", ".D..........", "U...........", "...RA.......", "..L.A.......", ".D.R........", ".DL.........", "U...A.......", "U..R........", "U.L.........", "U..RA.......", "U.L.A......."});
// if (*lesterFrame == 0x41D1) moveList.insert(moveList.end(), { "....A.......", "...R........", "..L.........", ".D..........", "U...........", "...RA.......", "..L.A.......", ".D.R........", ".DL.........", "U...A.......", "U..R........", "U.L.........", "U..RA.......", "U.L.A......."});
// if (*lesterFrame == 0x41D6) moveList.insert(moveList.end(), { "....A.......", "...R........", "..L.........", ".D..........", "U...........", "...RA.......", "..L.A.......", ".D.R........", ".DL.........", "U...A.......", "U..R........", "U.L.........", "U..RA.......", "U.L.A......."});
// if (*lesterFrame == 0x41DB) moveList.insert(moveList.end(), { "....A.......", "...R........", "..L.........", ".D..........", "U...........", "...RA.......", "..L.A.......", ".D.R........", "U...A.......", "U..R........", "U.L.........", "U..RA......."});
// if (*lesterFrame == 0xFFFF) moveList.insert(moveList.end(), { "....A.......", "...R........", "..L.........", ".D..........", "U...........", "...RA.......", "..L.A.......", ".D.R........", ".DL.........", "U...A.......", "U..R........", "U.L.........", "U..RA.......", "U.L.A......."});

 // Stage01b
 // moveList.insert(moveList.end(), { "L", "R", "B", "C", "LB", "LC", "RB", "RB", "LBC", "RBC", "LRB", "LRC", "LRBC"});

 // Stage02a
 // moveList.insert(moveList.end(), { "L", "R" });
 moveList.insert(moveList.end(), { "L", "R", "B", "C", "D", "DB", "LR", "LB", "LC", "RB", "RB", "LBC", "RBC", "LRB", "LRC", "DLB", "DLC", "DRB", "DRC", "DBC", "LRBC"});

 return moveList;
}

uint16_t GameInstance::advanceState(const INPUT_TYPE &move)
{
   _emu->advanceState(move);
  uint16_t skippedFrames = 0;

//  while(*inputFrame != 0 || *animationFrame != 0) { _emu->advanceState(0); skippedFrames++; }

  updateDerivedValues();
  return skippedFrames;
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

  // Evaluating lester magnet's reward on position X
  boundedValue = (float)*lesterPosX;
  boundedValue = std::min(boundedValue, magnets.lesterHorizontalMagnet[*lesterRoom].max);
  boundedValue = std::max(boundedValue, magnets.lesterHorizontalMagnet[*lesterRoom].min);
  diff = std::abs(magnets.lesterHorizontalMagnet[*lesterRoom].center - boundedValue);
  reward += magnets.lesterHorizontalMagnet[*lesterRoom].intensity * -diff;

  // Evaluating lester magnet's reward on position Y
  boundedValue = (float)*lesterPosY;
  boundedValue = std::min(boundedValue, magnets.lesterVerticalMagnet[*lesterRoom].max);
  boundedValue = std::max(boundedValue, magnets.lesterVerticalMagnet[*lesterRoom].min);
  diff = std::abs(magnets.lesterVerticalMagnet[*lesterRoom].center - boundedValue);
  reward += magnets.lesterVerticalMagnet[*lesterRoom].intensity * -diff;

  // Evaluating alien magnet's reward on position X
  boundedValue = (float)*alienPosX;
  boundedValue = std::min(boundedValue, magnets.alienHorizontalMagnet[*alienRoom].max);
  boundedValue = std::max(boundedValue, magnets.alienHorizontalMagnet[*alienRoom].min);
  diff = std::abs(magnets.alienHorizontalMagnet[*alienRoom].center - boundedValue);
  reward += magnets.alienHorizontalMagnet[*alienRoom].intensity * -diff;

  // Evaluating Gun Charge Magnet
  reward += magnets.gunChargeMagnet * (float)*lesterGunCharge;

  // Evaluating Stage 01 Vine Magnet
  reward += magnets.stage01VineStateMagnet * (float)*stage01VineState;

  // Evaluating Stage 02 Angular momentum magnet
  reward += magnets.lesterAngularMomentumMagnet * (float)*stage02AngularMomentum1;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Timer:                             %02u (%02u) (%02u)\n", *gameTimer, *lagFrame, *inputFrame);
 LOG("[Jaffar]  + Current Stage:                     %02u\n", currentStage);
 LOG("[Jaffar]  + Reward:                            %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                              0x%lX\n", computeHash());
 LOG("[Jaffar]  + Game Mode:                         %02u\n", *gameMode);
 LOG("[Jaffar]  + Animation Frame:                   %02u\n", *animationFrame);
 LOG("[Jaffar]  + Lester Room:                       %02u\n", *lesterRoom);
 LOG("[Jaffar]  + Lester Pos X:                      %04d\n", *lesterPosX);
 LOG("[Jaffar]  + Lester Pos Y:                      %04d\n", *lesterPosY);
 LOG("[Jaffar]  + Lester State:                      %02u\n", *lesterState1);
 LOG("[Jaffar]  + Lester Frame:                      0x%04X [%02X, %02X]\n", *lesterFrame, *(((uint8_t*)lesterFrame)+0), *(((uint8_t*)lesterFrame)+1));
 LOG("[Jaffar]  + Lester Gun:                        State: %04d, Charge: %04d\n", *lesterHasGun, *lesterGunCharge);
 LOG("[Jaffar]  + Lester Dead Flag                   %02u\n", *lesterDeadFlag);
 LOG("[Jaffar]  + Alien Dead Flag                    %02u\n", *alienDeadFlag);
 LOG("[Jaffar]  + Alien Room:                        %02u\n", *alienRoom);
 LOG("[Jaffar]  + Alien Pos X:                       %04d\n", *alienPosX);

 if (currentStage == 1)
 {
  LOG("[Jaffar]  + Level 1 Values:\n");
  LOG("[Jaffar]  + Appear Timer:                      %02u\n", *stage01AppearTimer);
  LOG("[Jaffar]  + Vine State:                        %04d\n", *stage01VineState);
  LOG("[Jaffar]  + Skip Monster Flag:                 %02u\n", *stage01SkipMonsterFlag);
  LOG("[Jaffar]  + Finished State:                    %02u\n", *stage01Finish);
 }

 if (currentStage == 2)
 {
  LOG("[Jaffar]  + Level 2 Values:\n");
  LOG("[Jaffar]  + Angular Momenta:                   [%02u, %02u]\n", *stage02AngularMomentum1, *stage02AngularMomentum2);
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

  if (std::abs(magnets.lesterHorizontalMagnet[*lesterRoom].intensity) > 0.0f)  LOG("[Jaffar]  + Lester Horizontal Magnet       - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.lesterHorizontalMagnet[*lesterRoom].intensity, magnets.lesterHorizontalMagnet[*lesterRoom].center, magnets.lesterHorizontalMagnet[*lesterRoom].min, magnets.lesterHorizontalMagnet[*lesterRoom].max);
  if (std::abs(magnets.lesterVerticalMagnet[*lesterRoom].intensity) > 0.0f)    LOG("[Jaffar]  + Lester Vertical Magnet         - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.lesterVerticalMagnet[*lesterRoom].intensity, magnets.lesterVerticalMagnet[*lesterRoom].center, magnets.lesterVerticalMagnet[*lesterRoom].min, magnets.lesterVerticalMagnet[*lesterRoom].max);
  if (std::abs(magnets.alienHorizontalMagnet[*alienRoom].intensity) > 0.0f)    LOG("[Jaffar]  + Alien Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.alienHorizontalMagnet[*alienRoom].intensity, magnets.alienHorizontalMagnet[*alienRoom].center, magnets.alienHorizontalMagnet[*alienRoom].min, magnets.alienHorizontalMagnet[*alienRoom].max);
  if (std::abs(magnets.gunChargeMagnet) > 0.0f)                                LOG("[Jaffar]  + Gun Charge Magnet              - Intensity: %.5f\n", magnets.gunChargeMagnet);
  if (std::abs(magnets.lesterAngularMomentumMagnet) > 0.0f)                    LOG("[Jaffar]  + Lester Angular Momentum Magnet - Intensity: %.5f\n", magnets.lesterAngularMomentumMagnet);
  if (std::abs(magnets.stage01VineStateMagnet) > 0.0f)                         LOG("[Jaffar]  + Stage 01 Vine State Magnet     - Intensity: %.5f\n", magnets.stage01VineStateMagnet);
}
