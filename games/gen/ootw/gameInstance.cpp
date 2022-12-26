#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
 // Setting emulator
 _emu = emu;

 // Setting memory positions
 currentStage            = (uint8_t*)   &_emu->_68KRam[0xEA84];
 gameRNG                 = (uint16_t*)  &_emu->_68KRam[0xEAE6];
 gameTimer               = (uint8_t*)   &_emu->_68KRam[0xE882];
 lagFrame                = (uint8_t*)   &_emu->_68KRam[0xFF5B];
 inputFrame              = (uint8_t*)   &_emu->_68KRam[0xFFC4];
 animationFrame          = (uint8_t*)   &_emu->_68KRam[0xE800];
 gameMode                = (uint8_t*)   &_emu->_68KRam[0xED9C];

 lesterPosX              = (int16_t*)   &_emu->_68KRam[0xEA86];
 lesterPosY              = (int16_t*)   &_emu->_68KRam[0xEA88];
 lesterRoom              = (uint8_t*)   &_emu->_68KRam[0xEB50];
 lesterNextRoom          = (uint8_t*)   &_emu->_68KRam[0xEB52];
 lesterFrame             = (uint16_t*)  &_emu->_68KRam[0xECAC];
 lesterState1            = (uint8_t*)   &_emu->_68KRam[0xEB4A];

 lesterHasGun            = (int16_t*)   &_emu->_68KRam[0xECD8];
 lesterGunCharge         = (int16_t*)   &_emu->_68KRam[0xEA90];
 lesterGunPowerLoadFrame = (int16_t*)   &_emu->_68KRam[0xECB2];
 lesterGunPowerLoad      = (uint8_t*)   &_emu->_68KRam[0xEAA2];
 lesterDeadFlag          = (uint8_t*)   &_emu->_68KRam[0xEA8D];

 shield1PosX             = (int16_t*)   &_emu->_68KRam[0xEBC4];

 shield1Duration         = (int16_t*)   &_emu->_68KRam[0xEBC6];
 shield2Duration         = (int16_t*)   &_emu->_68KRam[0xEBCC];
 shield3Duration         = (int16_t*)   &_emu->_68KRam[0xEBD2];
 shield4Duration         = (int16_t*)   &_emu->_68KRam[0xEBD8];
 shield5Duration         = (int16_t*)   &_emu->_68KRam[0xEBDE];

 liftStatus              = (int16_t*)   &_emu->_68KRam[0xEAAC];

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
 stage02BreakDoorState    = (uint8_t*)   &_emu->_68KRam[0xEBE8];
 stage02TunnerSmokerState1 = (uint8_t*)   &_emu->_68KRam[0xEC54];
 stage02TunnerSmokerState2 = (uint8_t*)   &_emu->_68KRam[0xEC56];
 stage02TunnerSmokerState3 = (uint8_t*)   &_emu->_68KRam[0xEC90];

 // Stage 31 Specific Values
 stage31TriDoorState = (uint16_t*)   &_emu->_68KRam[0xEB16];
 stage31WallState = (uint16_t*)   &_emu->_68KRam[0xEBE6];

 // Stage 33-37 Specific Values
 stage33PoolWallState   = (uint16_t*)   &_emu->_68KRam[0xEBE4];
 stage33BatActive       = (uint16_t*)   &_emu->_68KRam[0xEBE6];
 stage33BigRockState    = (uint16_t*)   &_emu->_68KRam[0xCD80];
 stage33Room204Vine1PosY = (uint16_t*)   &_emu->_68KRam[0xEB62];
 stage33WaterWall        = (uint16_t*)   &_emu->_68KRam[0xEBE4];
 stage33WaterPush       = (uint16_t*)   &_emu->_68KRam[0xECB0];

 // Stage 37 Specific Values
 stage37ProgressState = (uint16_t*)   &_emu->_68KRam[0xEBE6];

 // Stage 64 Specific Values
 stage64ProgressState = (uint16_t*)   &_emu->_68KRam[0xEBF6];
 stage64Enemy1DeadState = (uint16_t*)   &_emu->_68KRam[0xEB6A];
 stage64Enemy2DeadState = (uint16_t*)   &_emu->_68KRam[0xEB7A];
 stage64GuardDoorState = (uint16_t*)   &_emu->_68KRam[0xEBF6];

 // Stage 50 Specific Values
 stage50PanelProgress = (uint16_t*)   &_emu->_68KRam[0xEB52];
 stage50Escape        = (uint16_t*)   &_emu->_68KRam[0xEB4A];
 stage50Button1       = (uint16_t*)   &_emu->_68KRam[0xEB68];
 stage50Button2       = (uint16_t*)   &_emu->_68KRam[0xEB66];
 stage50Button3       = (uint16_t*)   &_emu->_68KRam[0xEB6C];
 stage50Button4       = (uint16_t*)   &_emu->_68KRam[0xEB6E];

 // Stage 60 Specific Values
 lesterCrawlAnimation = (uint16_t*)   &_emu->_68KRam[0xECB0];

 // Initialize derivative values
 updateDerivedValues();
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
  // Storage for hash calculation
  MetroHash128 hash;

  hash.Update(_emu->_68KRam+0x0000, 0xFFFF);

//  hash.Update(_emu->_68KRam+0xE880, 0x0560);
  hash.Update(_emu->_68KRam+0xEA80, 0x64);
//  hash.Update(_emu->_68KRam+0xEAE8, 0x40);
  hash.Update(_emu->_68KRam+0xEB36, 0x050);
//  hash.Update(_emu->_68KRam+0xEC70, 0x090);
//    hash.Update(_emu->_68KRam+0x0000, 0x0080);
//    hash.Update(_emu->_68KRam+0xFF50, 0x00B0);

  hash.Update(*currentStage);
  hash.Update(*gameTimer);
  hash.Update(*inputFrame);
  hash.Update(*gameMode);
  hash.Update(*lagFrame);

  hash.Update(*shield1PosX);
  hash.Update(*shield1Duration == -1 ? -1 : 1);
  hash.Update(*shield2Duration == -1 ? -1 : 1);
  hash.Update(*shield3Duration == -1 ? -1 : 1);
  hash.Update(*shield4Duration == -1 ? -1 : 1);
  hash.Update(*shield5Duration == -1 ? -1 : 1);

  hash.Update(*animationFrame);
  hash.Update(*lesterPosX);
  hash.Update(*lesterPosY);
  hash.Update(*lesterRoom);
  hash.Update(*lesterNextRoom);
  hash.Update(*lesterHasGun);
  hash.Update(*lesterGunCharge % 2);
  hash.Update(*lesterGunPowerLoad);
  hash.Update(*lesterGunPowerLoadFrame);

  hash.Update(*alienPosX);
  hash.Update(*alienRoom);

//  if (*currentStage == 1)
//  {
//   hash.Update(*stage01AppearTimer);
//   hash.Update(*stage01SkipMonsterFlag);
//   hash.Update(*stage01VineState);
//   hash.Update(*stage01Finish);
//  }
//
//  if (*currentStage == 2)
//  {
//   hash.Update(*stage02AngularMomentum1);
//   hash.Update(*stage02AngularMomentum2);
//   hash.Update(*stage02BreakDoorState);
//   hash.Update(*stage02TunnerSmokerState1);
//   hash.Update(*stage02TunnerSmokerState2);
//   hash.Update(*stage02TunnerSmokerState3);
//  }

//  if (*currentStage == 31)
//  {
//   hash.Update(*stage31TriDoorState);
//   hash.Update(*stage31WallState);
//  }

  // Stages 33-37
//  hash.Update(*stage33PoolWallState);
//  hash.Update(*stage33BatActive);
//  hash.Update(*stage33BigRockState);
//  hash.Update(*stage33Room204Vine1PosY);
//  hash.Update(*stage33WaterWall);
//  hash.Update(*stage33WaterPush);

// Stage 37
//  hash.Update(*stage37ProgressState);

  // Stage 64
//  hash.Update(*stage64ProgressState);
//  hash.Update(*stage64Enemy1DeadState);
//  hash.Update(*stage64GuardDoorState);

  // Stage 50
//  hash.Update(*stage50PanelProgress);
//  hash.Update(*stage50Escape);
//  hash.Update(*stage50Button1);
//  hash.Update(*stage50Button2);
//  hash.Update(*stage50Button3);
//  hash.Update(*stage50Button4);

//  hash.Update(_emu->_CRam, 0x40);

  // Stage 60
//  hash.Update(*lesterCrawlAnimation);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 lesterAbsolutePosX = *lesterPosX > 0 ? *lesterPosX : -*lesterPosX;
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 std::vector<std::string> moveList = {"."};

// if (*inputFrame == 255) return moveList;

 if (*lagFrame != 3) return moveList;

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
 moveList.insert(moveList.end(), { "L", "R", "B", "C", "D", "DB", "LR", "LB", "LC", "RB", "RB", "LBC", "RBC", "LRB", "LRC", "DLB", "DLC", "DRB", "DRC", "DBC", "LRBC" });

 return moveList;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
  std::vector<INPUT_TYPE> moves;

  _emu->advanceState(move);
  moves.push_back(move);

  return moves;
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
  boundedValue = (float)*lesterPosX + ((float)*lesterCrawlAnimation / 100000.0f);
  boundedValue = std::min(boundedValue, magnets.lesterHorizontalMagnet[*lesterRoom].max);
  boundedValue = std::max(boundedValue, magnets.lesterHorizontalMagnet[*lesterRoom].min);
  diff = std::abs(magnets.lesterHorizontalMagnet[*lesterRoom].center - boundedValue);
  reward += magnets.lesterHorizontalMagnet[*lesterRoom].intensity * (-diff + 1024.0f);

  // Evaluating lester magnet's reward on position Y
  boundedValue = (float)*lesterPosY;
  boundedValue = std::min(boundedValue, magnets.lesterVerticalMagnet[*lesterRoom].max);
  boundedValue = std::max(boundedValue, magnets.lesterVerticalMagnet[*lesterRoom].min);
  diff = std::abs(magnets.lesterVerticalMagnet[*lesterRoom].center - boundedValue);
  reward += magnets.lesterVerticalMagnet[*lesterRoom].intensity * (-diff + 1024.0f);

  // Evaluating alien magnet's reward on position X
  boundedValue = (float)*alienPosX;
  boundedValue = std::min(boundedValue, magnets.alienHorizontalMagnet[*alienRoom].max);
  boundedValue = std::max(boundedValue, magnets.alienHorizontalMagnet[*alienRoom].min);
  diff = std::abs(magnets.alienHorizontalMagnet[*alienRoom].center - boundedValue);
  reward += magnets.alienHorizontalMagnet[*alienRoom].intensity * (-diff + 1024.0f);

  // Evaluating Gun Charge Magnet
  reward += magnets.gunChargeMagnet * (float)*lesterGunCharge;

  // Evaluating Gun Power Load Magnet
  reward += magnets.gunPowerLoadMagnet * ((float)((double)*lesterGunPowerLoad + (0.000000001 * (double)*lesterGunPowerLoadFrame)));

  // Evaluating Stage 01 Vine Magnet
  reward += magnets.stage01VineStateMagnet * (float)*stage01VineState;

  // Evaluating Stage 02 Angular momentum magnet
  reward += magnets.lesterAngularMomentumMagnet * (float)*stage02AngularMomentum1;

  // Evaluating Shield 1 horizontal magnet
  reward += magnets.shield1HorizontalMagnet * (float)*shield1PosX;

  // Returning reward
  return reward;
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
 *gameRNG = (uint16_t) RNGState;
// *lesterPosX = (int16_t) RNGState;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Timer:                             %02u (%02u) (%02u)\n", *gameTimer, *lagFrame, *inputFrame);
 LOG("[Jaffar]  + RNG:                               %04u\n", *gameRNG);
 LOG("[Jaffar]  + Current Stage:                     %02u\n", *currentStage);
 LOG("[Jaffar]  + Reward:                            %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                              0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Mode:                         %02u\n", *gameMode);
 LOG("[Jaffar]  + Animation Frame:                   %02u\n", *animationFrame);
 LOG("[Jaffar]  + Lester Room:                       %02u, Next: %02u\n", *lesterRoom, *lesterNextRoom);
 LOG("[Jaffar]  + Lester Pos X:                      %04d\n", *lesterPosX);
 LOG("[Jaffar]  + Lester Pos Y:                      %04d\n", *lesterPosY);
 LOG("[Jaffar]  + Lester State:                      %02u\n", *lesterState1);
 LOG("[Jaffar]  + Lester Frame:                      0x%04X [%02X, %02X]\n", *lesterFrame, *(((uint8_t*)lesterFrame)+0), *(((uint8_t*)lesterFrame)+1));
 LOG("[Jaffar]  + Lester Gun:                        State: %04d, Charge: %04d, Power: [%02u, %04d]\n", *lesterHasGun, *lesterGunCharge, *lesterGunPowerLoad, *lesterGunPowerLoadFrame);
 LOG("[Jaffar]  + Lester Dead Flag                   %02u\n", *lesterDeadFlag);
 LOG("[Jaffar]  + Alien Dead Flag                    %02u\n", *alienDeadFlag);
 LOG("[Jaffar]  + Alien Room:                        %02u\n", *alienRoom);
 LOG("[Jaffar]  + Alien Pos X:                       %04d\n", *alienPosX);
 LOG("[Jaffar]  + Lift Status:                       %04d\n", *liftStatus);
 LOG("[Jaffar]  + Shield Pos X:                      [ %04d ]\n", *shield1PosX);
 LOG("[Jaffar]  + Shield Status:                     [ %04d, %04d, %04d, %04d, %04d ]\n", *shield1Duration, *shield2Duration, *shield3Duration, *shield4Duration, *shield5Duration);

// if (*currentStage == 1)
// {
//  LOG("[Jaffar]  + Level 1 Values:\n");
//  LOG("[Jaffar]  + Appear Timer:                      %02u\n", *stage01AppearTimer);
//  LOG("[Jaffar]  + Vine State:                        %04d\n", *stage01VineState);
//  LOG("[Jaffar]  + Skip Monster Flag:                 %02u\n", *stage01SkipMonsterFlag);
//  LOG("[Jaffar]  + Finished State:                    %02u\n", *stage01Finish);
// }
//
// if (*currentStage == 2)
// {
//  LOG("[Jaffar]  + Level 2 Values:\n");
//  LOG("[Jaffar]  + Angular Momenta:                   [%02u, %02u]\n", *stage02AngularMomentum1, *stage02AngularMomentum2);
//  LOG("[Jaffar]  + Break Door State:                  %02u\n", *stage02BreakDoorState);
//  LOG("[Jaffar]  + Tunnel Smoker States:              [ %02u, %02u, %02u ]\n", *stage02TunnerSmokerState1, *stage02TunnerSmokerState2, *stage02TunnerSmokerState3);
// }

//  if (*currentStage == 31)
//  {
//   LOG("[Jaffar]  + Level 31 Values:\n");
//   LOG("[Jaffar]  + Tridoor State:                   [%04u]\n", *stage31TriDoorState);
//   LOG("[Jaffar]  + Wall State:                      [%04u]\n", *stage31WallState);
//  }


// LOG("[Jaffar]  + Level 33-37 Values:\n");
// LOG("[Jaffar]  + Pool Wall State:                    %04u\n", *stage33PoolWallState);
// LOG("[Jaffar]  + Bat Active:                         %04u\n", *stage33BatActive);
// LOG("[Jaffar]  + Big Rock State:                     %04u\n", *stage33BigRockState);
// LOG("[Jaffar]  + Room 204 Vine 1 State:              %04u\n", *stage33Room204Vine1PosY);
// LOG("[Jaffar]  + Water Wall State:                   %04u\n", *stage33WaterWall);
// LOG("[Jaffar]  + Water Push State:                   %04u\n", *stage33WaterPush);

// LOG("[Jaffar]  + Level 37 Values:\n");
// LOG("[Jaffar]  + Progress State:                   %04u\n", *stage37ProgressState);

// LOG("[Jaffar]  + Level 64 Values:\n");
// LOG("[Jaffar]  + Progress State:                   %04u\n", *stage64ProgressState);
// LOG("[Jaffar]  + Enemy 1 Dead State:               %04u\n", *stage64Enemy1DeadState);
// LOG("[Jaffar]  + Enemy 2 Dead State:               %04u\n", *stage64Enemy2DeadState);
// LOG("[Jaffar]  + Guard Door State:                 %04u\n", *stage64GuardDoorState);

// LOG("[Jaffar]  + Level 50 Values:\n");
// LOG("[Jaffar]  + Panel Progress:                    %04u\n", *stage50PanelProgress);
// LOG("[Jaffar]  + Panel Button 1:                    %04u\n", *stage50Button1);
// LOG("[Jaffar]  + Panel Button 2:                    %04u\n", *stage50Button2);
// LOG("[Jaffar]  + Panel Button 3:                    %04u\n", *stage50Button3);
// LOG("[Jaffar]  + Panel Button 4:                    %04u\n", *stage50Button4);
// LOG("[Jaffar]  + Panel Escape:                      %04u\n", *stage50Escape);

  LOG("[Jaffar]  + Lester Crawl Animation:                      %04u\n", *lesterCrawlAnimation);

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

  if (std::abs(magnets.lesterHorizontalMagnet[*lesterRoom].intensity) > 0.0f)  LOG("[Jaffar]  + Lester Horizontal Magnet       - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.lesterHorizontalMagnet[*lesterRoom].intensity, magnets.lesterHorizontalMagnet[*lesterRoom].center, magnets.lesterHorizontalMagnet[*lesterRoom].min, magnets.lesterHorizontalMagnet[*lesterRoom].max);
  if (std::abs(magnets.lesterVerticalMagnet[*lesterRoom].intensity) > 0.0f)    LOG("[Jaffar]  + Lester Vertical Magnet         - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.lesterVerticalMagnet[*lesterRoom].intensity, magnets.lesterVerticalMagnet[*lesterRoom].center, magnets.lesterVerticalMagnet[*lesterRoom].min, magnets.lesterVerticalMagnet[*lesterRoom].max);
  if (std::abs(magnets.alienHorizontalMagnet[*alienRoom].intensity) > 0.0f)    LOG("[Jaffar]  + Alien Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.alienHorizontalMagnet[*alienRoom].intensity, magnets.alienHorizontalMagnet[*alienRoom].center, magnets.alienHorizontalMagnet[*alienRoom].min, magnets.alienHorizontalMagnet[*alienRoom].max);
  if (std::abs(magnets.gunChargeMagnet) > 0.0f)                                LOG("[Jaffar]  + Gun Charge Magnet              - Intensity: %.5f\n", magnets.gunChargeMagnet);
  if (std::abs(magnets.gunPowerLoadMagnet) > 0.0f)                             LOG("[Jaffar]  + Gun Power Load Magnet          - Intensity: %.5f\n", magnets.gunPowerLoadMagnet);
  if (std::abs(magnets.lesterAngularMomentumMagnet) > 0.0f)                    LOG("[Jaffar]  + Lester Angular Momentum Magnet - Intensity: %.5f\n", magnets.lesterAngularMomentumMagnet);
  if (std::abs(magnets.stage01VineStateMagnet) > 0.0f)                         LOG("[Jaffar]  + Stage 01 Vine State Magnet     - Intensity: %.5f\n", magnets.stage01VineStateMagnet);
  if (std::abs(magnets.shield1HorizontalMagnet) > 0.0f)                        LOG("[Jaffar]  + Shield 1 Horizontal Magnet     - Intensity: %.5f\n", magnets.shield1HorizontalMagnet);
}

