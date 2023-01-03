#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Parsing SDLPoP-specific configuration
  _hashKidCurrentHp = false;
  _hashGuardCurrentHp = false;
  _hashTrobCount = false;

  levelTileHashes.resize(16);

  if (isDefined(config, "Property Hash Types") == true)
  {
   for (const auto& entry : config["Property Hash Types"])
   {
    if (entry == "Kid Current HP") _hashKidCurrentHp = true;
    if (entry == "Guard Current HP") _hashGuardCurrentHp = true;
    if (entry == "Trob Count") _hashTrobCount = true;
   }
  }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Property Hash Types' was not defined\n");

  if (isDefined(config, "Falling Tiles Hash Types") == true)
  {
   for (const auto& entry : config["Falling Tiles Hash Types"])
   {
    int level = -1;
    if (isDefined(entry, "Level") == true)
    {
     if (entry["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Falling Tiles Hash Types level must be an integer.\n");
     level = entry["Level"].get<int>();
    }
    else EXIT_WITH_ERROR("[Error] Game Configuration 'Falling Tiles Hash Types', 'Level' entry was not defined\n");

    std::string hashType = entry["Type"].get<std::string>();
    int room = entry["Room"].get<int>();
    int column = entry["Column"].get<int>();
    auto idx = std::make_pair(room, column);
    if (hashType == "Index Only") levelTileHashes[level]._hashTypeMobs[idx] = INDEX_ONLY;
    if (hashType == "Full") levelTileHashes[level]._hashTypeMobs[idx] = FULL;
   }
  }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Falling Tiles Hash Types' was not defined\n");

  if (isDefined(config, "Active Objects Hash Types") == true)
  {
   for (const auto& entry : config["Active Objects Hash Types"])
   {
    std::string hashType = entry["Type"].get<std::string>();

    int level = -1;
    if (isDefined(entry, "Level") == true)
    {
     if (entry["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Active Objects Hash Types level must be an integer.\n");
     level = entry["Level"].get<int>();
    }
    else EXIT_WITH_ERROR("[Error] Game Configuration 'Active Objects Hash Types', 'Level' entry was not defined\n");

    int room = -1;
    if (isDefined(entry, "Room") == true)
    {
     if (entry["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Active Objects Hash Types room must be an integer.\n");
     room = entry["Room"].get<int>();
    }
    else EXIT_WITH_ERROR("[Error] Game Configuration 'Active Objects Hash Types', 'Room' entry was not defined\n");

    int tile = -1;
    if (isDefined(entry, "Tile") == true)
    {
     if (entry["Tile"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Active Objects Hash Types tile must be an integer.\n");
     tile = entry["Tile"].get<int>();
    }
    else EXIT_WITH_ERROR("[Error] Game Configuration 'Active Objects Hash Types', 'Tile' entry was not defined\n");

    int idx = (room-1)*30 + (tile-1);
    if (hashType == "Index Only") levelTileHashes[level]._hashTypeTrobs[idx] = INDEX_ONLY;
    if (hashType == "Full") levelTileHashes[level]._hashTypeTrobs[idx] = FULL;
   }

  }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Active Objects Hash Types' was not defined\n");

  if (isDefined(config, "Static Tile Hash Types") == true)
  {
   for (const auto& entry : config["Static Tile Hash Types"])
   {
    int level = -1;
    if (isDefined(entry, "Level") == true)
    {
     if (entry["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Static Tile Hash Types level must be an integer.\n");
     level = entry["Level"].get<int>();
    }
    else EXIT_WITH_ERROR("[Error] Game Configuration 'Static Tile Hash Types', 'Level' entry was not defined\n");

     int room = -1;
     if (isDefined(entry, "Room") == true)
     {
      if (entry["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Static Tile Hash Types room must be an integer.\n");
      room = entry["Room"].get<int>();
     }
     else EXIT_WITH_ERROR("[Error] Game Configuration 'Static Tile Hash Types', 'Room' entry was not defined\n");

     int tile = -1;
     if (isDefined(entry, "Tile") == true)
     {
      if (entry["Tile"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Static Tile Hash Types tile must be an integer.\n");
      tile = entry["Tile"].get<int>();
     }
     else EXIT_WITH_ERROR("[Error] Game Configuration 'Static Tile Hash Types', 'Tile' entry was not defined\n");

     int idx = (room-1)*30 + (tile-1);

     levelTileHashes[level]._hashTypeStatic.push_back(idx);
   }
  }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Static Tile Hash Types' was not defined\n");

  // Timer tolerance
  if (isDefined(config, "Timer Tolerance") == true)
   timerTolerance = config["Timer Tolerance"].get<uint8_t>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Timer Tolerance' was not defined\n");
};

// This function computes the hash for the current state
_uint128_t GameInstance::computeHash() const
{
 // Storage for hash calculation
 MetroHash128 hash;

 // If timer tolerance is set, use the game tick for hashing
 if (timerTolerance > 0) hash.Update(gameState.globalStepCounter % (timerTolerance+1));

 // Adding fixed hash elements
 hash.Update(gameState.drawn_room);
 hash.Update(gameState.leveldoor_open);
 hash.Update(gameState.Kid);
 if (gameState.Kid.room == gameState.Guard.room) hash.Update(gameState.Guard);
 if (gameState.Kid.room == gameState.Char.room) hash.Update(gameState.Char);
// if (gameState.Kid.room == gameState.Opp.room) hash.Update(gameState.Opp);
 hash.Update(gameState.grab_timer);
 hash.Update(gameState.holding_sword);
 hash.Update(gameState.united_with_shadow);
 hash.Update(gameState.have_sword);
 hash.Update(gameState.kid_sword_strike);
 hash.Update(gameState.offguard);
 hash.Update(gameState.guard_notice_timer);
 hash.Update(gameState.guard_refrac);
 hash.Update(gameState.justblocked);
 hash.Update(gameState.droppedout);
 hash.Update(gameState.need_level1_music);
 hash.Update(gameState.is_screaming);
 hash.Update(gameState.is_feather_fall);
 hash.Update(gameState.last_loose_sound);

 hash.Update(gameState.is_guard_notice);
 hash.Update(gameState.can_guard_see_kid);
 hash.Update(gameState.collision_row);
 hash.Update(gameState.prev_coll_room);
 hash.Update(gameState.prev_coll_flags);
 hash.Update(gameState.jumped_through_mirror);

 hash.Update(gameState.current_level);
 hash.Update(gameState.next_level);
 hash.Update(gameState.mobs_count);
 hash.Update(gameState.pickup_obj_type);
 hash.Update(gameState.demo_index);
 hash.Update(gameState.demo_time);
 hash.Update(gameState.curr_guard_color);
 hash.Update(gameState.guard_skill);
 hash.Update(gameState.shadow_initialized);

 hash.Update(gameState.curr_row_coll_room);
 hash.Update(gameState.curr_row_coll_flags);
 hash.Update(gameState.below_row_coll_room);
 hash.Update(gameState.below_row_coll_flags);
 hash.Update(gameState.above_row_coll_room);
 hash.Update(gameState.above_row_coll_flags);
 hash.Update(gameState.prev_collision_row);
 hash.Update(gameState.exit_room_timer);

 // Manual hashing
// hash.Update(gameState.level.guards_x);
// hash.Update(gameState.level.guards_dir);
// hash.Update(gameState.level.guards_seq_lo);
// hash.Update(gameState.level.guards_seq_hi);
// hash.Update(gameState.level.guards_tile);

 // Artificial items
 hash.Update(gameState.currentCutsceneDelay);
 hash.Update(gameState.cumulativeCutsceneDelay);
 hash.Update(gameState.kidPrevframe);

 if (_hashKidCurrentHp == true) hash.Update(gameState.hitp_curr);
 if (_hashGuardCurrentHp == true) hash.Update(gameState.guardhp_curr);
 if (_hashTrobCount == true) hash.Update(gameState.trobs_count);

 // Mobs are moving objects (falling tiles only afaik).
 for (int i = 0; i < gameState.mobs_count; i++)
 {
  const auto &mob = gameState.mobs[i];
  const auto idx = std::make_pair(mob.room, mob.xh);
  if (levelTileHashes[gameState.current_level]._hashTypeMobs.count(idx))
  {
   const auto hashType = levelTileHashes[gameState.current_level]._hashTypeMobs.at(idx);
   if (hashType == INDEX_ONLY) { hash.Update(mob.room); hash.Update(mob.xh); }
   if (hashType == FULL)  hash.Update(mob);
  }
 }

 // Trobs are stationary animated objects. They only change in state, hence we only read BG
 for (int i = 0; i < gameState.trobs_count; i++)
 {
   const auto &trob = gameState.trobs[i];
   const auto idx = (trob.room - 1) * 30 + trob.tilepos;

   if (levelTileHashes[gameState.current_level]._hashTypeTrobs.count(idx))
   {
    const auto hashType = levelTileHashes[gameState.current_level]._hashTypeTrobs.at(idx);
    if (hashType == INDEX_ONLY) hash.Update(idx * 255);
    if (hashType == FULL) { hash.Update(gameState.level.bg[idx] + idx * 255 ); }
   }
 }

 // Computing hash for static objects. They only change on tile type, hence we only read FG
 for (const auto idx : levelTileHashes[gameState.current_level]._hashTypeStatic)  hash.Update(gameState.level.fg[idx] + idx * 255);

 _uint128_t result;
 hash.Finalize(reinterpret_cast<uint8_t *>(&result));
 return result;
}

std::vector<INPUT_TYPE> GameInstance::advanceGameState(const INPUT_TYPE &move)
{
 gameState.kidPrevframe = gameState.Kid.frame;

 std::vector<INPUT_TYPE> moves;

 _emu->advanceState(move);
 moves.push_back(move);

 return moves;
}

void GameInstance::updateDerivedValues()
{
 kidFrameDiff = gameState.Kid.frame - gameState.kidPrevframe;
 kidPosY = (float)gameState.Kid.y;

 // If climbing down, add pos y. Otherwise subtract
 if (gameState.Kid.frame >= 0x8D && gameState.Kid.frame <= 0x94 && kidFrameDiff < 0) kidPosY += (0x94 - gameState.Kid.frame);
 if (gameState.Kid.frame >= 0x8D && gameState.Kid.frame <= 0x94 && kidFrameDiff > 0) kidPosY += 7.0f - (gameState.Kid.frame - 0x8D);

 // If jumpclimb up, subtract pos y
 if (gameState.Kid.frame >= 0x43 && gameState.Kid.frame <= 0x4F) kidPosY -= 16.0f - (0x4F - gameState.Kid.frame);

 // If hanging, subtract pos y
 if (gameState.Kid.frame == 0x50) kidPosY -= 20.0f;
 if (gameState.Kid.frame >= 0x57 && gameState.Kid.frame <= 0x5B) kidPosY -= 25.0f - (0x5B - gameState.Kid.frame);

 // Climbing up
 if (gameState.Kid.frame >= 0x87 && gameState.Kid.frame < 0x8D) kidPosY -= 32.0f + 7.0f - (0x8D - gameState.Kid.frame);
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves(const bool* rulesStatus) const
{
 // If cutscene, then only allow pressing shift
 if (gameState.current_level != gameState.next_level)
  return {".", "S"};

 // For level 1, if kid touches ground and music plays, try restarting level
 if (gameState.Kid.frame == 109 && gameState.need_level1_music == 33)
   return {".", "CA"};

 // If bumped, nothing to do
 if (gameState.Kid.action == actions_5_bumped)
   return {".", "S"};

 // If in mid air or free fall, hope to grab on to something
 if (gameState.Kid.action == actions_3_in_midair || gameState.Kid.action == actions_4_in_freefall)
   return {".", "S", "U" };

 // Move, sheath, attack, parry
 if (gameState.Kid.sword == sword_2_drawn)
   return {".", "S", "U", "L", "R", "D", "US", "DS" };

 // Kid is standing or finishing a turn, try all possibilities
 if (gameState.Kid.frame == frame_15_stand || (gameState.Kid.frame >= frame_50_turn && gameState.Kid.frame < 53))
   return {".", "S", "U", "L", "R", "D", "LU", "LD", "RU", "RD", "SR", "SL", "DS", "US", "RUS", "LUS"};

 // Turning frame, try all possibilities
 if (gameState.Kid.frame == frame_48_turn)
   return {".", "S", "U", "L", "R", "D", "LU", "LD", "RU", "RD", "SR", "SL"};

 // Start running animation, all movement without shift
 if (gameState.Kid.frame < 4)
   return {".", "U", "L", "R", "D", "LU", "LD", "RU", "RD"};

 // Starting jump up, check directions, jump and shift
 if (gameState.Kid.frame >= frame_67_start_jump_up_1 && gameState.Kid.frame < frame_70_jumphang)
   return {".", "S", "U", "L", "R", "D", "LU", "RU", "SU"};

 // Running, all movement without shift
 if (gameState.Kid.frame < 15)
   return {".", "U", "L", "R", "D", "LU", "LD", "RU", "RD"};

 // Hanging, up and shift are only options
 if (gameState.Kid.frame >= frame_87_hanging_1 && gameState.Kid.frame < 100)
   return {".", "S", "U", "SU"};

 // Crouched, can only stand, drink, or hop
 if (gameState.Kid.frame == frame_109_crouch)
   return {".", "S", "L", "R", "D", "LD", "RD", "SD"};

 // Default, do nothing
 return {".", "S"};
}

// Function to get magnet information
magnetSet_t GameInstance::getMagnetValues(const bool* rulesStatus) const
{
 // Storage for magnet information
 magnetSet_t magnets;
 bool ruleFound = false;
 size_t lastRuleFound = 0;

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++) if (rulesStatus[ruleId] == true) { ruleFound = true; lastRuleFound = ruleId; }

 if (ruleFound == true)
  {
    if (_rules[lastRuleFound]->_magnets[gameState.Kid.room].kidHorizontalMagnet.active == true) magnets.kidHorizontalMagnet = _rules[lastRuleFound]->_magnets[gameState.Kid.room].kidHorizontalMagnet;
    if (_rules[lastRuleFound]->_magnets[gameState.Kid.room].kidVerticalMagnet.active == true) magnets.kidVerticalMagnet = _rules[lastRuleFound]->_magnets[gameState.Kid.room].kidVerticalMagnet;
    if (_rules[lastRuleFound]->_magnets[gameState.Kid.room].guardHorizontalMagnet.active == true) magnets.guardHorizontalMagnet = _rules[lastRuleFound]->_magnets[gameState.Kid.room].guardHorizontalMagnet;
    if (_rules[lastRuleFound]->_magnets[gameState.Kid.room].guardVerticalMagnet.active == true) magnets.guardVerticalMagnet = _rules[lastRuleFound]->_magnets[gameState.Kid.room].guardVerticalMagnet;
    magnets.kidDirectionMagnet = _rules[lastRuleFound]->_magnets[gameState.Kid.room].kidDirectionMagnet;
  }

 return magnets;
}

// Obtains the score of a given frame
float GameInstance::getStateReward(const bool* rulesStatus) const
{
 // We calculate a different reward if this is a winning frame
 auto stateType = getStateType(rulesStatus);
 if (stateType == f_win) return ((gameState.rem_min-1) * 720 + gameState.rem_tick);

 // Getting rewards from rules
 float reward = 0.0;
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
   reward += _rules[ruleId]->_reward;

 // Getting magnet value
 auto magnets = getMagnetValues(rulesStatus);

 // Container for bounded value and difference with center
 float diff = 0.0;

 // Evaluating kid magnet's reward on position X
 diff = -255.0f + std::abs(magnets.kidHorizontalMagnet.center - (float)gameState.Kid.x);
 reward += magnets.kidHorizontalMagnet.intensity * -diff;

 // Evaluating guard magnet's reward on position X
 diff = -255.0f + std::abs(magnets.guardHorizontalMagnet.center - (float)gameState.Guard.x);
 reward += magnets.guardHorizontalMagnet.intensity * -diff;

 // Evaluating kid magnet's reward on position Y
 diff = -255.0f + std::abs(magnets.kidVerticalMagnet.center - (float)gameState.Kid.y);
 reward += magnets.kidVerticalMagnet.intensity * -diff;

 // Evaluating guard magnet's reward on position Y
 diff = -255.0f + std::abs(magnets.guardVerticalMagnet.center - (float)gameState.Guard.y);
 reward += magnets.guardVerticalMagnet.intensity * -diff;

 // Kid Direction Magnet
 reward += gameState.Kid.direction == 0 ? 1.0 : -1.0  * magnets.kidDirectionMagnet;

 // Returning reward
 return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{

  // Calculating timing
  size_t remMins = gameState.rem_min-1;
  size_t remSecs = gameState.rem_tick / 12;
  size_t remMilliSecs = floor((double)(gameState.rem_tick % 12) / 0.012);
  char remainingIGTText[512];
  sprintf(remainingIGTText, "%02lu:%02lu.%03lu", remMins, remSecs, remMilliSecs);

  size_t cumMins = 60 - gameState.rem_min;
  size_t cumSecs = (720 - gameState.rem_tick) / 12;
  size_t cumMilliSecs = floor((double)( (720 - gameState.rem_tick) % 12) / 0.012);
  char cumulativeIGTText[512];
  sprintf(cumulativeIGTText, "%02lu:%02lu.%03lu", cumMins, cumSecs, cumMilliSecs);

  LOG("[Jaffar]  + Global Step Counter:  %05d, (Tol: %02u)\n", gameState.globalStepCounter, timerTolerance);
  LOG("[Jaffar]  + Current/Next Level:   %2d / %2d\n", gameState.current_level, gameState.next_level);
  LOG("[Jaffar]  + Reward:               %f\n", getStateReward(rulesStatus));
  LOG("[Jaffar]  + Hash:                 0x%lX%lX\n", computeHash().first, computeHash().second);
  LOG("[Jaffar]  + [Kid]                 Room: %d, Pos.x: %3d, Pos.y: %f (%3d), Frame: %3d, Action: %2d, HP: %d/%d\n", int(gameState.Kid.room), int(gameState.Kid.x), kidPosY, int(gameState.Kid.y), int(gameState.Kid.frame), int(gameState.Kid.action), int(gameState.hitp_curr), int(gameState.hitp_max));
  LOG("[Jaffar]  + [Guard]               Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, Action: %2d, HP: %d/%d\n", int(gameState.Guard.room), int(gameState.Guard.x), int(gameState.Guard.y), int(gameState.Guard.frame), int(gameState.Guard.action), int(gameState.guardhp_curr), int(gameState.guardhp_max));
  LOG("[Jaffar]  + Cumulative IGT:       %s (%03lu %03u -> %05lu)\n", cumulativeIGTText, cumMins, (720 - gameState.rem_tick), cumMins * 720 + (720 - gameState.rem_tick));
  LOG("[Jaffar]  + Remaining IGT:        %s (%03lu %03u -> %05lu)\n", remainingIGTText, remMins, gameState.rem_tick, remMins * 720 + gameState.rem_tick);
  LOG("[Jaffar]  + Cutscene Delay        %03u, Total: %03u\n", gameState.currentCutsceneDelay, gameState.cumulativeCutsceneDelay);
  LOG("[Jaffar]  + Exit Room Timer:      %d\n", gameState.exit_room_timer);
  LOG("[Jaffar]  + Reached Checkpoint:   %s\n", gameState.checkpoint ? "Yes" : "No");
  LOG("[Jaffar]  + Feather Fall:         %d\n", gameState.is_feather_fall);
  LOG("[Jaffar]  + Last Sound Id:        %d\n", gameState.last_loose_sound);

  auto prevRNG1 = _emu->reverseRNGState(gameState.random_seed);
  auto prevRNG2 = _emu->reverseRNGState(prevRNG1);
  auto prevRNG3 = _emu->reverseRNGState(prevRNG2);
  auto prevRNG4 = _emu->reverseRNGState(prevRNG3);
  auto prevRNG5 = _emu->reverseRNGState(prevRNG4);

  auto postRNG1 = _emu->advanceRNGState(gameState.random_seed);
  auto postRNG2 = _emu->advanceRNGState(postRNG1);
  auto postRNG3 = _emu->advanceRNGState(postRNG2);
  auto postRNG4 = _emu->advanceRNGState(postRNG3);
  auto postRNG5 = _emu->advanceRNGState(postRNG4);

  LOG("[Jaffar]  + RNG: [ 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X ] 0x%08X [ 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X ]\n", prevRNG5, prevRNG4, prevRNG3, prevRNG2, prevRNG1, gameState.random_seed, postRNG1, postRNG2, postRNG3, postRNG4, postRNG5);
  LOG("[Jaffar]  + Copy Protection       Place: %02u, Index: %02u\n", copyprot_plac, copyprot_idx);

  LOG("[Jaffar]  + Moving Objects:\n");
  for (int i = 0; i < gameState.mobs_count; ++i)
  {
    const auto &mob = (gameState.mobs)[i];
    LOG("[Jaffar]    + Room: %d, X: %d, Y: %d, Speed: %d, Type: %d, Row: %d\n", mob.room, mob.xh, mob.y, mob.speed, mob.type, mob.row);
  }

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  auto magnets = getMagnetValues(rulesStatus);
  if (std::abs(magnets.kidHorizontalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Kid Horizontal Magnet     - Intensity: %.1f, Center: %3.3f\n", magnets.kidHorizontalMagnet.intensity, magnets.kidHorizontalMagnet.center);
  if (std::abs(magnets.kidVerticalMagnet.intensity) > 0.0f)     LOG("[Jaffar]  + Kid Vertical Magnet       - Intensity: %.1f, Center: %3.3f\n", magnets.kidVerticalMagnet.intensity, magnets.kidVerticalMagnet.center);
  if (std::abs(magnets.kidDirectionMagnet) > 0.0f)              LOG("[Jaffar]  + Kid Direction Magnet      - Intensity: %.1f\n", magnets.kidDirectionMagnet);
  if (std::abs(magnets.guardHorizontalMagnet.intensity) > 0.0f) LOG("[Jaffar]  + Guard Horizontal Magnet   - Intensity: %.1f, Center: %3.3f\n", magnets.guardHorizontalMagnet.intensity, magnets.guardHorizontalMagnet.center);
  if (std::abs(magnets.guardVerticalMagnet.intensity) > 0.0f)   LOG("[Jaffar]  + Guard Vertical Magnet     - Intensity: %.1f, Center: %3.3f\n", magnets.guardVerticalMagnet.intensity, magnets.guardVerticalMagnet.center);
}

void GameInstance::setRNGState(const uint64_t RNGState)
{
 gameState.random_seed = (uint64_t) RNGState;
}
