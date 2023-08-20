#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  player1Continues    = (uint8_t* ) &_emu->_baseMem[0x000E];
  player2Continues    = (uint8_t* ) &_emu->_baseMem[0x000F];
  currentLevel        = (uint8_t* ) &_emu->_baseMem[0x0010];
  player1Lives        = (uint8_t* ) &_emu->_baseMem[0x0011];
  player2Lives        = (uint8_t* ) &_emu->_baseMem[0x0012];
  rng                 = (uint32_t*) &_emu->_baseMem[0x0025];
  cameraPosX          = (int16_t* ) &_emu->_baseMem[0x0087];
  cameraPosY          = (int16_t* ) &_emu->_baseMem[0x0089];
  levelConfigPointer  = (uint8_t* ) &_emu->_baseMem[0x00B7];
  objectSlotStart     = (uint8_t* ) &_emu->_baseMem[0x03C1];
  player1PosX1        = (uint8_t* ) &_emu->_baseMem[0x03EE];
  player1PosX2        = (uint8_t* ) &_emu->_baseMem[0x03FD];
  player1PosX3        = (uint8_t* ) &_emu->_baseMem[0x0448];
  player1PosY1        = (uint8_t* ) &_emu->_baseMem[0x040C];
  player1PosY2        = (uint8_t* ) &_emu->_baseMem[0x041B];
  player1PosY3        = (uint8_t* ) &_emu->_baseMem[0x0457];
  player1PosZ1        = (uint8_t* ) &_emu->_baseMem[0x042A];
  player1PosZ2        = (uint8_t* ) &_emu->_baseMem[0x0439];
  player1PosZ3        = (uint8_t* ) &_emu->_baseMem[0x0466];
  player2PosX1        = (uint8_t* ) &_emu->_baseMem[0x03EF];
  player2PosX2        = (uint8_t* ) &_emu->_baseMem[0x03FE];
  player2PosX3        = (uint8_t* ) &_emu->_baseMem[0x0449];
  player2PosY1        = (uint8_t* ) &_emu->_baseMem[0x040D];
  player2PosY2        = (uint8_t* ) &_emu->_baseMem[0x041C];
  player2PosY3        = (uint8_t* ) &_emu->_baseMem[0x0458];
  player2PosZ1        = (uint8_t* ) &_emu->_baseMem[0x042B];
  player2PosZ2        = (uint8_t* ) &_emu->_baseMem[0x043A];
  player2PosZ3        = (uint8_t* ) &_emu->_baseMem[0x0467];

  keyAddress1        = (uint8_t* ) &_emu->_baseMem[0x0017];
  keyAddress2        = (uint8_t* ) &_emu->_baseMem[0x0018];
  keyAddress3        = (uint8_t* ) &_emu->_baseMem[0x0021];

  keyEvent1Triggered = (uint8_t* ) &_emu->_baseMem[0x0160];
  keyEvent2Triggered = (uint8_t* ) &_emu->_baseMem[0x0161];

  lagFrame = (uint8_t* ) &_emu->_baseMem[0x0009];

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

  hash.Update(*player1Continues  );
  hash.Update(*player2Continues  );
  hash.Update(*currentLevel      );
  hash.Update(*player1Lives      );
  hash.Update(*player2Lives      );
  hash.Update(*rng               );
  hash.Update(*cameraPosX        );
  hash.Update(*cameraPosY        );
  hash.Update(*levelConfigPointer);
  hash.Update(*objectSlotStart   );
  hash.Update(*player1PosX1      );
  hash.Update(*player1PosX2      );
  hash.Update(*player1PosX3      );
  hash.Update(*player1PosY1      );
  hash.Update(*player1PosY2      );
  hash.Update(*player1PosY3      );
  hash.Update(*player1PosZ1      );
  hash.Update(*player1PosZ2      );
  hash.Update(*player1PosZ3      );
  hash.Update(*player2PosX1      );
  hash.Update(*player2PosX2      );
  hash.Update(*player2PosX3      );
  hash.Update(*player2PosY1      );
  hash.Update(*player2PosY2      );
  hash.Update(*player2PosY3      );
  hash.Update(*player2PosZ1      );
  hash.Update(*player2PosZ2      );
  hash.Update(*player2PosZ3      );
  hash.Update(*keyAddress1);
  hash.Update(*keyAddress2);
  hash.Update(*keyAddress3);
  hash.Update(*keyEvent1Triggered);
  hash.Update(*keyEvent2Triggered);
  hash.Update(*lagFrame);

//  hash.Update(&_emu->_baseMem[0x03EE], 0x100);

  hash.Update(objectSlotStart, 0x20D);

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}


void GameInstance::updateDerivedValues()
{
 player1PosX = (float)*player1PosX1 * 256.0f + (float)*player1PosX2 + (float)*player1PosX3 / 256.0f ;
 player1PosY = (float)*player1PosY1 * 256.0f + (float)*player1PosY2 + (float)*player1PosY3 / 256.0f ;
 player1PosZ = (float)*player1PosZ1 * 256.0f + (float)*player1PosZ2 + (float)*player1PosZ3 / 256.0f ;

 player2PosX = (float)*player2PosX1 * 256.0f + (float)*player2PosX2 + (float)*player2PosX3 / 256.0f ;
 player2PosY = (float)*player2PosY1 * 256.0f + (float)*player2PosY2 + (float)*player2PosY3 / 256.0f ;
 player2PosZ = (float)*player2PosZ1 * 256.0f + (float)*player2PosZ2 + (float)*player2PosZ3 / 256.0f ;
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
 std::vector<std::string> moveList = { "." };

 moveList.insert(moveList.end(), { "B", "U", "D", "L", "A", "R", "RA", "UR", "UB", "UA", "UL", "DL", "UD", "DR", "DB", "DA", "BA", "LR", "LB", "LA", "RB", "UDA", "UDB", "ULR", "ULB", "ULA", "UDR", "UDL", "URB", "URA", "DLR", "DLB", "DLA", "DRB", "DRA", "LRB", "LRA", "LBA", "RBA" });
 moveList.insert(moveList.end(), { "s", "S", "Rs", "Us", "LS", "US", "Ls", "RS", "Ds", "DS", "Ss", "SB", "SA", "sB", "sA", "DSs", "DSB", "DRs", "UDS", "DRS", "DLs", "DLS", "UBA", "UsA", "UsB", "USA", "USB", "USs", "URs", "URS", "ULs", "ULS", "UDs", "LsA", "sBA", "SBA", "SsA", "SsB", "RsA", "RsB", "RSA", "RSB", "RSs", "DSA", "LsB", "LSA", "LSB", "LSs", "LRs", "LRS", "DBA", "DsA", "DsB" });

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

  if (*keyEvent1Triggered == 0)
  {
   if (*keyAddress1 == 0x1F) reward += 1000.0;
   if (*keyAddress2 == 0x04) reward += 1000.0;
  }

  if (*keyAddress1 == 0x1F && *keyAddress2 == 0x04 && *player1Lives == 2 && *lagFrame != 0) *keyEvent1Triggered = 1;
  if (*keyAddress3 >= 3 && *player1Lives == 2 && *lagFrame != 0) *keyEvent2Triggered = 1;

  // Returning reward
  return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
 LOG("[Jaffar]  + Reward:                           %.10f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + RNG:                              0x%X\n", *rng);
 LOG("[Jaffar]  + Lag Frame:                        %2u\n", *lagFrame);
 LOG("[Jaffar]  + Current Level:                    %2u\n", *currentLevel);

 LOG("[Jaffar]  + Player 1 Lives / Continues:       %2u / %2u\n", *player1Lives, *player1Continues  );
 LOG("[Jaffar]  + Player 2 Lives / Continues:       %2u / %2u\n", *player2Lives, *player2Continues  );

 LOG("[Jaffar]  + Camera Pos X:                     %2d\n", *cameraPosX );
 LOG("[Jaffar]  + Camera Pos Y:                     %2d\n", *cameraPosY );

 LOG("[Jaffar]  + Level Config Pointer:             %2u\n", *levelConfigPointer );
 LOG("[Jaffar]  + Object Slot Start:                %2u\n", *objectSlotStart );


 LOG("[Jaffar]  + Player 1 Pos X:                   %f (%2u, %2u, %2u)\n", player1PosX, *player1PosX1, *player1PosX2, *player1PosX3);
 LOG("[Jaffar]  + Player 1 Pos Y:                   %f (%2u, %2u, %2u)\n", player1PosY, *player1PosY1, *player1PosY2, *player1PosY3);
 LOG("[Jaffar]  + Player 1 Pos Z:                   %f (%2u, %2u, %2u)\n", player1PosZ, *player1PosZ1, *player1PosZ2, *player1PosZ3);

 LOG("[Jaffar]  + Player 2 Pos X:                   %f (%2u, %2u, %2u)\n", player2PosX, *player2PosX1, *player2PosX2, *player2PosX3);
 LOG("[Jaffar]  + Player 2 Pos Y:                   %f (%2u, %2u, %2u)\n", player2PosY, *player2PosY1, *player2PosY2, *player2PosY3);
 LOG("[Jaffar]  + Player 2 Pos Z:                   %f (%2u, %2u, %2u)\n", player2PosZ, *player2PosZ1, *player2PosZ2, *player2PosZ3);


 LOG("[Jaffar]  + Key Address 1 (0x17):             0x%X\n", *keyAddress1 );
 LOG("[Jaffar]  + Key Address 2 (0x18):             0x%X\n", *keyAddress2 );
 LOG("[Jaffar]  + Key Address 3 (0x21):             0x%X\n", *keyAddress3 );
 LOG("[Jaffar]  + Key Event 1:                      0x%X\n", *keyEvent1Triggered );
 LOG("[Jaffar]  + Key Event 2:                      0x%X\n", *keyEvent2Triggered );

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");
}

