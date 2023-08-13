#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;
  gameMode                          = (uint8_t*) &_emu->_baseMem[0x00F2];
  globalTimer                       = (uint8_t*) &_emu->_baseMem[0x0092];
  duckPosX1                        = (uint8_t*) &_emu->_baseMem[0x006F];
  duckPosX2                        = (uint8_t*) &_emu->_baseMem[0x006E];
  duckPosY                        = (uint8_t*)  &_emu->_baseMem[0x0440];
  duckFrameCycle                   = (uint8_t*) &_emu->_baseMem[0x0430];
  grabbedWeapon                    = (uint8_t*) &_emu->_baseMem[0x0054];

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

  hash.Update(*gameMode                 );
  hash.Update(*duckPosX1               );
  hash.Update(*duckPosX2               );
  hash.Update(*duckPosY               );
  hash.Update(*duckFrameCycle          );

  // Duck Stats Array
  hash.Update(&_emu->_baseMem[0x0400], 0x140);
  hash.Update(&_emu->_baseMem[0x0040], 0x30);

  // Adding time tolerance
  if (timerTolerance > 0) hash.Update(currentStep % (timerTolerance+1));


  _uint128_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

void GameInstance::updateDerivedValues()
{

 duckPosX = (float)*duckPosX1 * 256.0 + (float)*duckPosX2;

 // Calculating trace position
 float minDistance = std::numeric_limits<float>::infinity();
 for (size_t i = 0; i < trace.size(); i++)
 {
  float tracePointDistance =  std::sqrt(std::abs(duckPosX - trace[i].first) + std::abs((float)*duckPosY - trace[i].second));
  if (tracePointDistance <= traceTolerance)
   if (tracePointDistance < minDistance)
   { minDistance = tracePointDistance; tracePos = i; }
 }
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 std::vector<std::string> moveList = {"."};

// if (*gameMode == 0x0001) moveList.insert(moveList.end(), { "s", "B", "R", "L", "U", "A", "D", "DA", "UL", "UR", "UB", "UA", "DL", "DR", "DB", "LR", "BA", "LB", "LA", "RB", "RA", "RBA", "ULB", "ULA", "URB", "URA", "UBA", "LRB", "DLB", "DRB", "LBA", "LRA" });
// if (*gameMode == 0x000E) moveList.insert(moveList.end(), { "s", "A", "B", "R", "L", "D", "U", "BA", "RA", "LA", "UA", "RBA", "LBA", "URA", "ULA" });

 if (*gameMode == 0x0001) moveList.insert(moveList.end(), { "s", "R", "L", "U", "A", "D", "DA", "UL", "UR", "UA", "DL", "DR", "LR", "LA", "RA", "ULA", "URA", "LRA" });
 if (*gameMode == 0x000E) moveList.insert(moveList.end(), { "s", "A",  "R", "L", "D", "U", "RA", "LA", "UA", "URA", "ULA" });


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

  // Getting rewards from rules
  float reward = 0.0;
  for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
   if (rulesStatus[ruleId] == true)
    reward += _rules[ruleId]->_reward;

  // Getting magnet values for the kid
  auto magnets = getMagnetValues(rulesStatus);

  // Container for bounded value and difference with center
  float diff = 0.0;

  // Evaluating duck magnet's reward on position X
  diff = std::abs(magnets.duckHorizontalMagnet.center - duckPosX);
  reward += magnets.duckHorizontalMagnet.intensity * -diff;

  // Evaluating duck magnet's reward on position Y
  diff = std::abs(magnets.duckVerticalMagnet.center - (float)*duckPosY);
  reward += magnets.duckVerticalMagnet.intensity * -diff;

  // Evaluating trace distance magnet
  reward += magnets.traceMagnet * (float)tracePos;

  // Returning reward
  return reward;
}



void GameInstance::printStateInfo(const bool* rulesStatus) const
{

 LOG("[Jaffar]  + Global Timer:                     %02u (Tolerance: %02u)\n", *globalTimer, timerTolerance);
 LOG("[Jaffar]  + Reward:                           %f\n", getStateReward(rulesStatus));
 LOG("[Jaffar]  + Hash:                             0x%lX%lX\n", computeHash().first, computeHash().second);
 LOG("[Jaffar]  + Game Mode:                        %02u\n", *gameMode);
 LOG("[Jaffar]  + Duck Frame Cycle:                 %02u\n", *duckFrameCycle);
 LOG("[Jaffar]  + Duck Position X:                  %f: %02u + %02u\n", duckPosX, *duckPosX1, *duckPosX2);
 LOG("[Jaffar]  + Duck Position Y:                  %02u\n", *duckPosY);
 LOG("[Jaffar]  + Grabbed Weapon:                   %02u\n", *grabbedWeapon);

 if (useTrace == true)
 {
  LOG("[Jaffar]  + Current Trace Pos:                %05lu / %05lu (%s)\n", tracePos, trace.size(), traceFile.c_str());
 }

 LOG("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
 LOG("\n");

 auto magnets = getMagnetValues(rulesStatus);

 if (std::abs(magnets.duckHorizontalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Duck Horizontal Magnet        - Intensity: %.5f, Center: %3.3f\n", magnets.duckHorizontalMagnet.intensity, magnets.duckHorizontalMagnet.center);
 if (std::abs(magnets.duckVerticalMagnet.intensity) > 0.0f)       LOG("[Jaffar]  + Duck Vertical Magnet          - Intensity: %.5f, Center: %3.3f\n", magnets.duckVerticalMagnet.intensity, magnets.duckVerticalMagnet.center);
 if (std::abs(magnets.traceMagnet) > 0.0f)                        LOG("[Jaffar]  + Trace Magnet                   - Intensity: %.5f\n", magnets.traceMagnet);
}

std::string GameInstance::getFrameTrace() const
{
 return std::to_string(duckPosX) + std::string(" ") + std::to_string(*duckPosY) + std::string("\n");
}

