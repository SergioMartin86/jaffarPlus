#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;
  gameMode                          = (uint8_t*) &_emu->_baseMem[0x001F];
  currentStage                      = (uint8_t*) &_emu->_baseMem[0x005F];
  globalTimer                       = (uint8_t*) &_emu->_baseMem[0x005E];
  ninjaAnimation1                   = (uint8_t*) &_emu->_baseMem[0x00FA];
  ninjaAnimation2                   = (uint8_t*) &_emu->_baseMem[0x00FF];
  ninjaAction                       = (uint8_t*) &_emu->_baseMem[0x037E];
  ninjaFrame                        = (uint8_t*) &_emu->_baseMem[0x0057];
  ninjaWeapon                       = (uint8_t*) &_emu->_baseMem[0x009F];
  ninjaPower                        = (uint8_t*) &_emu->_baseMem[0x00CD];
  ninjaPowerMax                     = (uint8_t*) &_emu->_baseMem[0x00CE];
  ninjaHP                           = (uint8_t*) &_emu->_baseMem[0x00A7];
  ninjaPosX1                        = (uint8_t*) &_emu->_baseMem[0x00FD];
  ninjaPosX2                        = (uint8_t*) &_emu->_baseMem[0x04FE];
  ninjaSpeedX1                      = (int8_t* ) &_emu->_baseMem[0x055E];
  ninjaSpeedX2                      = (uint8_t*) &_emu->_baseMem[0x0546];
  ninjaPosY1                        = (uint8_t*) &_emu->_baseMem[0x058E];
  ninjaPosY2                        = (uint8_t*) &_emu->_baseMem[0x0576];
  ninjaSpeedY1                      = (int8_t* ) &_emu->_baseMem[0x05D6];
  ninjaSpeedY2                      = (uint8_t*) &_emu->_baseMem[0x05BE];
  ninjaDirection                    = (uint8_t*) &_emu->_baseMem[0x00FC];
  screenPosX1                       = (uint8_t*) &_emu->_baseMem[0x00DC];
  screenPosX2                       = (uint8_t*) &_emu->_baseMem[0x00DB];
  screenPosX3                       = (uint8_t*) &_emu->_baseMem[0x00DA];
  screenPosY1                       = (uint8_t*) &_emu->_baseMem[0x0085];
  screenPosY2                       = (uint8_t*) &_emu->_baseMem[0x0084];
  screenPosY3                       = (uint8_t*) &_emu->_baseMem[0x0059];
  bossHP                            = (uint8_t*) &_emu->_baseMem[0x00A8];
  bossPosY                          = (uint8_t*) &_emu->_baseMem[0x05A5];
  bossPosX                          = (uint8_t*) &_emu->_baseMem[0x052D];
  ninjaInvincibilityTimer           = (uint8_t*) &_emu->_baseMem[0x00AD];
  ninjaInvincibilityState           = (uint8_t*) &_emu->_baseMem[0x007B];
  ninjaSwordType                    = (uint8_t*) &_emu->_baseMem[0x00A9];
  ninjaVerticalCollision            = (uint8_t*) &_emu->_baseMem[0x0074];
  ninjaHorizontalCollision          = (uint8_t*) &_emu->_baseMem[0x0076];
  orbStateVector                    = (uint8_t*) &_emu->_baseMem[0x0381];
  enemyStateVector                  = (uint8_t*) &_emu->_baseMem[0x038E];
  bufferedMovement                  = (uint8_t*) &_emu->_baseMem[0x0089];
  levelExitFlag                     = (uint8_t*) &_emu->_baseMem[0x002C];
  levelExitFlag2                    = (uint8_t*) &_emu->_baseMem[0x01FE];


  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");

  // Trace to Follow
  if (isDefined(config, "Trace File") == true)
   traceFile = config["Trace File"].get<std::string>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Trace File' was not defined\n");

  // Trace tolerance
  if (isDefined(config, "Trace Tolerance") == true)
   traceTolerance = config["Trace Tolerance"].get<float>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Trace Tolerance' was not defined\n");

  // Enable B
  if (isDefined(config, "Enable B") == true)
   enableB = config["Enable B"].get<bool>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Enable B' was not defined\n");

  // Loading trace
  if (traceFile != "")
  {
   useTrace = true;
   std::string traceRaw;
   if (loadStringFromFile(traceRaw, traceFile.c_str()) == false) EXIT_WITH_ERROR("Could not find/read trace file: %s\n", traceFile.c_str());

   std::istringstream f(traceRaw);
   std::string line;
   while (std::getline(f, line))
   {
    auto coordinates = split(line, ' ');
    trace.push_back(std::make_pair(std::atof(coordinates[0].c_str()), std::atof(coordinates[1].c_str())));
   }
  }

  // Initialize derivative values
  updateDerivedValues();
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 std::vector<INPUT_TYPE> moves;
 _emu->advanceState(move); moves.push_back(move);
 updateDerivedValues();
 return moves;
}

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash(const uint16_t currentStep) const
{
  // Storage for hash calculation
  MetroHash128 hash;

  // if in transition, take frame counter as hash value
//  if (*gameMode != 0) hash.Update(*globalTimer);

  hash.Update(*gameMode                 );
  hash.Update(*currentStage             );
  hash.Update(*globalTimer              );
  hash.Update(*ninjaAnimation1          );
  hash.Update(*ninjaAnimation2          );
  hash.Update(*ninjaAction              );
  hash.Update(*ninjaFrame               );
  hash.Update(*ninjaWeapon              );
  hash.Update(*ninjaPower               );
  hash.Update(*ninjaPowerMax            );
  hash.Update(*ninjaHP                  );
  hash.Update(*ninjaPosX1               );
  hash.Update(*ninjaPosX2               );
  hash.Update(*ninjaSpeedX1             );
  hash.Update(*ninjaSpeedX2             );
  hash.Update(*ninjaPosY1               );
  hash.Update(*ninjaPosY2               );
  hash.Update(*ninjaSpeedY1             );
  hash.Update(*ninjaSpeedY2             );
  hash.Update(*ninjaDirection           );
  hash.Update(*screenPosX1              );
  hash.Update(*screenPosX2              );
  hash.Update(*screenPosX3              );
  hash.Update(*screenPosY1              );
  hash.Update(*screenPosY2              );
  hash.Update(*screenPosY3              );
  hash.Update(*bossHP                   );
  hash.Update(*bossPosY                 );
  hash.Update(*bossPosX                 );
  hash.Update(*ninjaInvincibilityTimer  );
  hash.Update(*ninjaInvincibilityState  );
  hash.Update(*ninjaSwordType           );
  hash.Update(*ninjaVerticalCollision   );
  hash.Update(*ninjaHorizontalCollision );
  hash.Update(*levelExitFlag);
  hash.Update(*levelExitFlag2);
  hash.Update(*bufferedMovement);
  hash.Update(orbStateVector, ORB_COUNT);
  hash.Update(enemyStateVector, ENEMY_COUNT);

  // Animation Array
  hash.Update(&_emu->_baseMem[0x0060], 0x40);

  // Adding time tolerance
  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));

  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

void GameInstance::updateDerivedValues()
{

 // If game mode is 40, then it's a vertical level
 if (*gameMode == 40)
 {
  absolutePosX = ((float)*screenPosX2 + (float)*ninjaPosX1) + ((float)*screenPosX3 + (float)*ninjaPosX2)/ 256.0;
  absolutePosY = (float)*screenPosY1 * 256.0 + ((float)*screenPosY2 + (float)*ninjaPosY1) + ((float)*screenPosY3 + (float)*ninjaPosY2)/ 256.0;
 }

 // If game mode is 42, then it's a horizontal level
 if (*gameMode == 42)
 {
  absolutePosX = (float)*screenPosX1 * 256.0 + ((float)*screenPosX2 + (float)*ninjaPosX1) + ((float)*screenPosX3 + (float)*ninjaPosX2)/ 256.0;
  absolutePosY = ((float)*screenPosY2 + (float)*ninjaPosY1) + ((float)*screenPosY3 + (float)*ninjaPosY2)/ 256.0;
 }

 ninjaBossDistance = std::abs((float)*ninjaPosX1 - (float)*bossPosX) + std::abs((float)*ninjaPosY1 - (float)*bossPosY);

 // Calculating trace position
 float minDistance = std::numeric_limits<float>::infinity();
 for (size_t i = 0; i < trace.size(); i++)
 {
  float tracePointDistance =  std::abs(absolutePosX - trace[i].first) + std::abs(absolutePosY - trace[i].second);
  if (tracePointDistance < minDistance) { minDistance = tracePointDistance; tracePos = i; }
 }
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

 if (*ninjaAction == 0x0000) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "RB", "LA", "LB", "DA", "DB", "UA", "UB" });
 if (*ninjaAction == 0x0001) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "RB", "LA", "LB", "DA", "DB", "UA", "UB" });
 if (*ninjaAction == 0x0002) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DA", "DB", "UA", "UB" });
 if (*ninjaAction == 0x0003) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "RB", "LA", "LB", "DA", "DB", "UA", "UB" });
 if (*ninjaAction == 0x0004) moveList.insert(moveList.end(), { "B", "D", "U", "RA", "RB", "LA", "UA", "UB" });
 if (*ninjaAction == 0x0005) moveList.insert(moveList.end(), { "R", "L", "B", "D", "U", "RA", "RB", "LA" });
 if (*ninjaAction == 0x0006) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "RB", "LA", "LB", "DA", "DB", "UA", "UB" });
 if (*ninjaAction == 0x0007) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "RB", "LA", "LB", "DA", "DB", "UA", "UB" });
 if (*ninjaAction == 0x0008) moveList.insert(moveList.end(), { "A", "B", "R", "L", "RA", "RB", "LA", "LB", "DA", "UA", "UB" });
 if (*ninjaAction == 0x0009) moveList.insert(moveList.end(), { "A", "B", "R", "L", "RA", "RB", "LA", "LB", "DA", "UA", "UB" });
 if (*ninjaAction == 0x000A) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "RA", "LA", "DB", "UA", "UB" });
 if (*ninjaAction == 0x000B) moveList.insert(moveList.end(), { "A", "RA", "LA", "DA", "UA" });
 if (*ninjaAction == 0x000C) moveList.insert(moveList.end(), { "A", "R", "L", "D", "U", "RA", "LA", "DA", "UA" });
 if (*ninjaAction == 0x000D) moveList.insert(moveList.end(), { "A", "RA", "LA" });
 if (*ninjaAction == 0x000E) moveList.insert(moveList.end(), { "A", "RA", "LA" });
 if (*ninjaAction == 0x000F) moveList.insert(moveList.end(), { "R", "L", "RA", "LA", "UA" });
 if (*ninjaAction == 0x0010) moveList.insert(moveList.end(), { "A", "RA", "LA", "DA", "UA" });
 if (*ninjaAction == 0x0012) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "RB", "LA", "DA", "UA" });
 if (*ninjaAction == 0x0014) moveList.insert(moveList.end(), { "A", "RA", "LA", "DA", "UA" });

 if (*ninjaAction == 0x0004) moveList.insert(moveList.end(), { "LB", "RB" });
 if (*ninjaAction == 0x0005) moveList.insert(moveList.end(), { "LB", "RB" });
 if (*ninjaAction == 0x000A) moveList.insert(moveList.end(), { "DA" });
 if (*ninjaAction == 0x000C) moveList.insert(moveList.end(), { "B" });
 if (*ninjaAction == 0x0011) moveList.insert(moveList.end(), { "A", "RA", "LA", "DA", "UA" });
 if (*ninjaAction == 0x0012) moveList.insert(moveList.end(), { "LB", "RB" });
 if (*ninjaAction == 0x0013) moveList.insert(moveList.end(), { "A", "B", "R", "L", "D", "U", "RA", "LA", "LB", "RB", "DA", "UA" });
 if (*ninjaAction == 0x0015) moveList.insert(moveList.end(), { "A", "RA", "LA", "DA", "UA" });

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
  // We calculate a different reward if this is a winning frame
  auto stateType = getStateType(rulesStatus);

  if (stateType == f_win) return (float)*ninjaHP;

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

  // Evaluating ninja magnet's reward on position X
  diff = std::abs(magnets.ninjaHorizontalMagnet.center - absolutePosX);
  reward += magnets.ninjaHorizontalMagnet.intensity * -diff;

  // Evaluating ninja magnet's reward on position Y
  diff = std::abs(magnets.ninjaVerticalMagnet.center - absolutePosY);
  reward += magnets.ninjaVerticalMagnet.intensity * -diff;

  // Evaluating ninja power magnet
  boundedValue = (float)*ninjaPower;
  boundedValue = std::min(boundedValue, magnets.ninjaPowerMagnet.max);
  boundedValue = std::max(boundedValue, magnets.ninjaPowerMagnet.min);
  diff = std::abs(magnets.ninjaPowerMagnet.center - boundedValue);
  reward += magnets.ninjaPowerMagnet.intensity * -diff;

  // Evaluating ninja HP magnet
  boundedValue = (float)*ninjaHP;
  boundedValue = std::min(boundedValue, magnets.ninjaHPMagnet.max);
  boundedValue = std::max(boundedValue, magnets.ninjaHPMagnet.min);
  diff = std::abs(magnets.ninjaHPMagnet.center - boundedValue);
  reward += magnets.ninjaHPMagnet.intensity * -diff;

  // Evaluating boss HP magnet
  boundedValue = (float)*bossHP;
  boundedValue = std::min(boundedValue, magnets.bossHPMagnet.max);
  boundedValue = std::max(boundedValue, magnets.bossHPMagnet.min);
  diff = std::abs(magnets.bossHPMagnet.center - boundedValue);
  reward += magnets.bossHPMagnet.intensity * -diff;

  // Evaluating ninja/boss distance magnet
  reward += magnets.ninjaBossDistanceMagnet * ninjaBossDistance;

  // Evaluating trace distance magnet
  reward += magnets.traceMagnet * (float)tracePos;

  // Evaluating ninja's weapon magnet
  if (magnets.ninjaWeaponMagnet.weapon == *ninjaWeapon) reward += magnets.ninjaWeaponMagnet.reward;

  // Returning reward
  return reward;
}



void GameInstance::printStateInfo(const bool* rulesStatus) const
{

 LOG("[Jaffar]  + Global Timer:                      %02u (Tolerance: %02u)\n", *globalTimer, timerTolerance);
 LOG("[Jaffar]  + Reward:                            %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                              0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Current Stage:                     %02u\n", *currentStage);
 LOG("[Jaffar]  + Game Mode:                         %02u\n", *gameMode);
 LOG("[Jaffar]  + Ninja Animation                    0x%02X / 0x%02X\n", *ninjaAnimation1, *ninjaAnimation2);
 LOG("[Jaffar]  + Ninja Action                       0x%02X\n", *ninjaAction);
 LOG("[Jaffar]  + Ninja Frame:                       %02u\n", *ninjaFrame);
 LOG("[Jaffar]  + Ninja Weapon:                      %02u\n", *ninjaWeapon);
 LOG("[Jaffar]  + Ninja Sword Type:                  %02u\n", *ninjaSwordType);
 LOG("[Jaffar]  + Ninja Power:                       %02u (Max: %02u)\n", *ninjaPower, *ninjaPowerMax);
 LOG("[Jaffar]  + Ninja HP:                          %02u\n", *ninjaHP);
 LOG("[Jaffar]  + Ninja Position X:                  %f: %02u + %02u\n", absolutePosX, *ninjaPosX1, *ninjaPosX2);
 LOG("[Jaffar]  + Ninja Position Y:                  %f: %02u + %02u\n", absolutePosY, *ninjaPosY1, *ninjaPosY2);
 LOG("[Jaffar]  + Ninja Speed X:                     %02d + %02u\n", *ninjaSpeedX1, *ninjaSpeedX2);
 LOG("[Jaffar]  + Ninja Speed Y:                     %02d + %02u\n", *ninjaSpeedY1, *ninjaSpeedY2);
 LOG("[Jaffar]  + Screen Pos X:                      %02u + %02u + %02u\n", *screenPosX1, *screenPosX2, *screenPosX3);
 LOG("[Jaffar]  + Screen Pos Y:                      %02u + %02u + %02u\n", *screenPosY1, *screenPosY2, *screenPosY3);
 LOG("[Jaffar]  + Ninja Invincibility:               %02u %02u\n", *ninjaInvincibilityState, *ninjaInvincibilityTimer);
 LOG("[Jaffar]  + Boss HP:                           %02u\n", *bossHP);
 LOG("[Jaffar]  + Boss Pos X:                        %02u\n", *bossPosX);
 LOG("[Jaffar]  + Boss Pos Y:                        %02u\n", *bossPosY);
 LOG("[Jaffar]  + Ninja/Boss Distance:               %.3f\n", ninjaBossDistance);

 if (useTrace == true)
 {
  LOG("[Jaffar]  + Current Trace Pos:                %05lu / %05lu (%s)\n", tracePos, trace.size(), traceFile.c_str());
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.ninjaHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Ninja Horizontal Magnet        - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaHorizontalMagnet.intensity, magnets.ninjaHorizontalMagnet.center, magnets.ninjaHorizontalMagnet.min, magnets.ninjaHorizontalMagnet.max);
 if (std::abs(magnets.ninjaVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Ninja Vertical Magnet          - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaVerticalMagnet.intensity, magnets.ninjaVerticalMagnet.center, magnets.ninjaVerticalMagnet.min, magnets.ninjaVerticalMagnet.max);
 if (std::abs(magnets.ninjaPowerMagnet.intensity) > 0.0f)          LOG("[Jaffar]  + Ninja Power Magnet             - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaPowerMagnet.intensity, magnets.ninjaPowerMagnet.center, magnets.ninjaPowerMagnet.min, magnets.ninjaPowerMagnet.max);
 if (std::abs(magnets.ninjaHPMagnet.intensity) > 0.0f)             LOG("[Jaffar]  + Ninja HP Magnet                - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.ninjaHPMagnet.intensity, magnets.ninjaHPMagnet.center, magnets.ninjaHPMagnet.min, magnets.ninjaHPMagnet.max);
 if (std::abs(magnets.bossHPMagnet.intensity) > 0.0f)              LOG("[Jaffar]  + Boss HP Magnet                 - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", magnets.bossHPMagnet.intensity, magnets.bossHPMagnet.center, magnets.bossHPMagnet.min, magnets.bossHPMagnet.max);
 if (std::abs(magnets.ninjaBossDistanceMagnet) > 0.0f)             LOG("[Jaffar]  + Ninja/Boss Distance Magnet     - Intensity: %.5f\n", magnets.ninjaBossDistanceMagnet);
 if (std::abs(magnets.ninjaWeaponMagnet.reward) > 0.0f)            LOG("[Jaffar]  + Ninja Weapon Magnet            - Reward:    %.1f, Weapon: %u\n", magnets.ninjaWeaponMagnet.reward, magnets.ninjaWeaponMagnet.weapon);
 if (std::abs(magnets.traceMagnet) > 0.0f)                         LOG("[Jaffar]  + Trace Magnet                   - Intensity: %.5f\n", magnets.traceMagnet);
}

std::string GameInstance::getFrameTrace() const
{
 return std::to_string(absolutePosX) + std::string(" ") + std::to_string(absolutePosX) + std::string("\n");
}

