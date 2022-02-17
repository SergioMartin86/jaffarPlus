#include "gameInstance.hpp"
#include "gameRule.hpp"

GameInstance::GameInstance(EmuInstance* emu, const nlohmann::json& config)
{
  // Setting emulator
  _emu = emu;

  // Parsing SDLPoP-specific configuration
  _hashKidCurrentHp = false;
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
    std::string hashType = entry["Type"].get<std::string>();
    int room = entry["Room"].get<int>();
    int column = entry["Column"].get<int>();
    auto idx = std::make_pair(room, column);
    if (hashType == "Index Only") _hashTypeMobs[idx] = INDEX_ONLY;
    if (hashType == "Full") _hashTypeMobs[idx] = FULL;
   }
  }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Falling Tiles Hash Types' was not defined\n");

  if (isDefined(config, "Active Objects Hash Types") == true)
  {
   for (const auto& entry : config["Active Objects Hash Types"])
   {
    std::string hashType = entry["Type"].get<std::string>();

    int room = -1;
    if (isDefined(entry, "Room") == true)
    {
     if (entry["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Active Objects Hash Types room must be an integer.\n");
     room = entry["Room"].get<int>();
    }

    int tile = -1;
    if (isDefined(entry, "Tile") == true)
    {
     if (entry["Tile"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Active Objects Hash Types tile must be an integer.\n");
     tile = entry["Tile"].get<int>();
    }

    int idx = (room-1)*30 + (tile-1);
    if (hashType == "Index Only") _hashTypeTrobs[idx] = INDEX_ONLY;
    if (hashType == "Full") _hashTypeTrobs[idx] = FULL;
   }

  }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Active Objects Hash Types' was not defined\n");

  if (isDefined(config, "Static Tile Hash Types") == true)
  {
   for (const auto& entry : config["Static Tile Hash Types"])
   {
     int room = -1;
     if (isDefined(entry, "Room") == true)
     {
      if (entry["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Active Objects Hash Types room must be an integer.\n");
      room = entry["Room"].get<int>();
     }

     int tile = -1;
     if (isDefined(entry, "Tile") == true)
     {
      if (entry["Tile"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Active Objects Hash Types tile must be an integer.\n");
      tile = entry["Tile"].get<int>();
     }

     int idx = (room-1)*30 + (tile-1);

    _hashTypeStatic.push_back(idx);
   }
  }
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Static Tile Hash Types' was not defined\n");

};

// This function computes the hash for the current state
uint64_t GameInstance::computeHash() const
{
 // Storage for hash calculation
 MetroHash64 hash;

 // Adding fixed hash elements
 hash.Update(gameState.drawn_room);
 hash.Update(gameState.leveldoor_open);
 hash.Update(gameState.Kid);
 hash.Update(gameState.Guard);
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

 // Manual hashing
 hash.Update(gameState.level.guards_x);
 hash.Update(gameState.level.guards_dir);
 hash.Update(gameState.level.guards_seq_lo);
 hash.Update(gameState.level.guards_seq_hi);
 hash.Update(gameState.level.guards_tile);

 if (_hashKidCurrentHp == true) hash.Update(gameState.hitp_curr);
 if (_hashGuardCurrentHp == true) hash.Update(gameState.guardhp_curr);
 if (_hashTrobCount == true) hash.Update(gameState.trobs_count);

 // Mobs are moving objects (falling tiles only afaik).
 for (int i = 0; i < gameState.mobs_count; i++)
 {
  const auto &mob = gameState.mobs[i];
  const auto idx = std::make_pair(mob.room, mob.xh);
  if (_hashTypeMobs.count(idx))
  {
   const auto hashType = _hashTypeMobs.at(idx);
   if (hashType == INDEX_ONLY) { hash.Update(mob.room); hash.Update(mob.xh); }
   if (hashType == FULL)  hash.Update(mob);
  }
 }

 // Trobs are stationary animated objects. They only change in state, hence we only read BG
 for (int i = 0; i < gameState.trobs_count; i++)
 {
   const auto &trob = gameState.trobs[i];
   const auto idx = (trob.room - 1) * 30 + trob.tilepos;

   if (_hashTypeTrobs.count(idx))
   {
    const auto hashType = _hashTypeTrobs.at(idx);
    if (hashType == INDEX_ONLY) hash.Update(idx * 255);
    if (hashType == FULL) { hash.Update(gameState.level.bg[idx] + idx * 255 ); }
   }
 }

 // Computing hash for static objects. They only change on tile type, hence we only read FG
 for (const auto idx : _hashTypeStatic)  hash.Update(gameState.level.fg[idx] + idx * 255);

 uint64_t result;
 hash.Finalize(reinterpret_cast<uint8_t *>(&result));
 return result;
}

void GameInstance::updateDerivedValues()
{
 next_room = gameState.drawn_room = gameState.Kid.room;
 load_room_links();
}

// Function to determine the current possible moves
std::vector<std::string> GameInstance::getPossibleMoves() const
{
 // If dead, do nothing
 if (gameState.Kid.alive >= 0)
   return {"."};

 // For level 1, if kid touches ground and music plays, try restarting level
 if (gameState.Kid.frame == 109 && gameState.need_level1_music == 33)
   return {".", "CA"};

 // If bumped, nothing to do
 if (gameState.Kid.action == actions_5_bumped)
   return {"."};

 // If in mid air or free fall, hope to grab on to something
 if (gameState.Kid.action == actions_3_in_midair || gameState.Kid.action == actions_4_in_freefall)
   return {".", "S"};

 // Move, sheath, attack, parry
 if (gameState.Kid.sword == sword_2_drawn)
   return {".", "S", "U", "L", "R", "D"};

 // Kid is standing or finishing a turn, try all possibilities
 if (gameState.Kid.frame == frame_15_stand || (gameState.Kid.frame >= frame_50_turn && gameState.Kid.frame < 53))
   return {".", "S", "U", "L", "R", "D", "LU", "LD", "RU", "RD", "SR", "SL"};

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
 return {"."};
}

// Function to get magnet information
magnetInfo_t GameInstance::getKidMagnetValues(const bool* rulesStatus, const int room) const
{
 // Storage for magnet information
 magnetInfo_t magnetInfo;
 magnetInfo.positionX = 0.0f;
 magnetInfo.intensityX = 0.0f;
 magnetInfo.intensityY = 0.0f;

 // Iterating rule vector
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
 {
  if (rulesStatus[ruleId] == true)
  {
    const auto& rule = _rules[ruleId];

    for (size_t i = 0; i < _rules[ruleId]->_kidMagnetPositionX.size(); i++)
     if (rule->_kidMagnetPositionX[i].room == room)
      magnetInfo.positionX = rule->_kidMagnetPositionX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_kidMagnetIntensityX.size(); i++)
     if (rule->_kidMagnetIntensityX[i].room == room)
      magnetInfo.intensityX = rule->_kidMagnetIntensityX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_kidMagnetIntensityY.size(); i++)
     if (rule->_kidMagnetIntensityY[i].room == room)
      magnetInfo.intensityY = rule->_kidMagnetIntensityY[i].value;
  }
 }

 return magnetInfo;
}

// Function to get magnet information
magnetInfo_t GameInstance::getGuardMagnetValues(const bool* rulesStatus, const int room) const
{

 // Storage for magnet information
 magnetInfo_t magnetInfo;
 magnetInfo.positionX = 0.0f;
 magnetInfo.intensityX = 0.0f;
 magnetInfo.intensityY = 0.0f;

 // Iterating rule vector
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
  {
    const auto& rule = _rules[ruleId];

    for (size_t i = 0; i < _rules[ruleId]->_guardMagnetPositionX.size(); i++)
     if (rule->_guardMagnetPositionX[i].room == room)
      magnetInfo.positionX = rule->_guardMagnetPositionX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_guardMagnetIntensityX.size(); i++)
     if (rule->_guardMagnetIntensityX[i].room == room)
      magnetInfo.intensityX = rule->_guardMagnetIntensityX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_guardMagnetIntensityY.size(); i++)
     if (rule->_guardMagnetIntensityY[i].room == room)
      magnetInfo.intensityY = rule->_guardMagnetIntensityY[i].value;
  }

 return magnetInfo;
}

// Obtains the score of a given frame
float GameInstance::getStateReward(const bool* rulesStatus) const
{
 // Getting rewards from rules
 float reward = 0.0;
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (rulesStatus[ruleId] == true)
   reward += _rules[ruleId]->_reward;

 // Getting kid room
 int kidCurrentRoom = gameState.Kid.room;

 // Getting magnet values for the kid
 auto kidMagnet = getKidMagnetValues(rulesStatus, kidCurrentRoom);

 // Getting kid's current frame
 const auto curKidFrame = gameState.Kid.frame;

 // Evaluating kidMagnet's reward on the X axis
 const float kidDiffX = std::abs(gameState.Kid.x - kidMagnet.positionX);
 reward += (float) kidMagnet.intensityX * (256.0f - kidDiffX);

 // For positive Y axis kidMagnet, rewarding climbing frames
 if ((float) kidMagnet.intensityY > 0.0f)
 {
   // Jumphang, because it preludes climbing (Score + 1-20)
   if (curKidFrame >= 67 && curKidFrame <= 80)
    reward += (float) kidMagnet.intensityY * (curKidFrame - 66);

   // Hang, because it preludes climbing (Score +21)
   if (curKidFrame == 91) reward += 21.0f * (float) kidMagnet.intensityY;

   // Climbing (Score +22-38)
   if (curKidFrame >= 135 && curKidFrame <= 149) reward += (float) kidMagnet.intensityY * (22.0f + (curKidFrame - 134));

   // Adding absolute reward for Y position
   reward += (float) kidMagnet.intensityY * (256.0f - gameState.Kid.y);
 }

 // For negative Y axis kidMagnet, rewarding falling/climbing down frames
 if ((float) kidMagnet.intensityY < 0.0f)
 {
   // Turning around, because it generally preludes climbing down
   if (curKidFrame >= 45 && curKidFrame <= 52) reward += -0.5f * (float) kidMagnet.intensityY;

   // Hanging, because it preludes falling
   if (curKidFrame >= 87 && curKidFrame <= 99) reward += -0.5f * (float) kidMagnet.intensityY;

   // Hang drop, because it preludes falling
   if (curKidFrame >= 81 && curKidFrame <= 85) reward += -1.0f * (float) kidMagnet.intensityY;

   // Falling start
   if (curKidFrame >= 102 && curKidFrame <= 105) reward += -1.0f * (float) kidMagnet.intensityY;

   // Falling itself
   if (curKidFrame == 106) reward += -2.0f + (float) kidMagnet.intensityY;

   // Climbing down
   if (curKidFrame == 148) reward += -2.0f + (float) kidMagnet.intensityY;

   // Adding absolute reward for Y position
   reward += (float) -1.0f * kidMagnet.intensityY * (gameState.Kid.y);
 }

 // Getting guard room
 int guardCurrentRoom = gameState.Guard.room;

 // Getting magnet values for the guard
 auto guardMagnet = getGuardMagnetValues(rulesStatus, guardCurrentRoom);

 // Getting guard's current frame
 const auto curGuardFrame = gameState.Guard.frame;

 // Evaluating guardMagnet's reward on the X axis
 const float guardDiffX = std::abs(gameState.Guard.x - guardMagnet.positionX);
 reward += (float) guardMagnet.intensityX * (256.0f - guardDiffX);

 // For positive Y axis guardMagnet
 if ((float) guardMagnet.intensityY > 0.0f)
 {
  // Adding absolute reward for Y position
  reward += (float) guardMagnet.intensityY * (256.0f - gameState.Guard.y);
 }

 // For negative Y axis guardMagnet, rewarding falling/climbing down frames
 if ((float) guardMagnet.intensityY < 0.0f)
 {
   // Falling start
   if (curGuardFrame >= 102 && curGuardFrame <= 105) reward += -1.0f * (float) guardMagnet.intensityY;

   // Falling itself
   if (curGuardFrame == 106) reward += -2.0f + (float) guardMagnet.intensityY;

   // Adding absolute reward for Y position
   reward += (float) -1.0f * guardMagnet.intensityY * (gameState.Guard.y);
 }

 // Apply bonus when kid is inside a non-visible room
 if (kidCurrentRoom == 0 || kidCurrentRoom >= 25) reward += 128.0f;

 // Apply bonus when kid is climbing exit stairs
 if (curKidFrame >= 217 && curKidFrame <= 228)
 {
   reward += 1000.0f;
   reward += 100.0f * ((float)curKidFrame - 217.0f);
 }

 // For some levels, keeping HP allows for later skips
 reward += gameState.hitp_curr * 100.0f;

 // Returning reward
 return reward;
}

void GameInstance::printStateInfo(const bool* rulesStatus) const
{
  LOG("[Jaffar]  + Current/Next Level: %2d / %2d\n", gameState.current_level, gameState.next_level);
  LOG("[Jaffar]  + [Kid]   Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, HP: %d/%d\n", int(gameState.Kid.room), int(gameState.Kid.x), int(gameState.Kid.y), int(gameState.Kid.frame), int(gameState.hitp_curr), int(gameState.hitp_max));
  LOG("[Jaffar]  + [Guard] Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, HP: %d/%d\n", int(gameState.Guard.room), int(gameState.Guard.x), int(gameState.Guard.y), int(gameState.Guard.frame), int(gameState.guardhp_curr), int(gameState.guardhp_max));
  LOG("[Jaffar]  + Exit Room Timer: %d\n", gameState.exit_room_timer);
  LOG("[Jaffar]  + Reached Checkpoint: %s\n", gameState.checkpoint ? "Yes" : "No");
  LOG("[Jaffar]  + Feather Fall: %d\n", gameState.is_feather_fall);
  LOG("[Jaffar]  + RNG State: 0x%08X (Last Loose Tile Sound Id: %d)\n", gameState.random_seed, gameState.last_loose_sound);

  LOG("[Jaffar]  + Rule Status: ");
  for (size_t i = 0; i < _rules.size(); i++) LOG("%d", rulesStatus[i] ? 1 : 0);
  LOG("\n");

  // Getting kid room
  int kidCurrentRoom = gameState.Kid.room;

  // Getting magnet values for the kid
  auto kidMagnet = getKidMagnetValues(rulesStatus, kidCurrentRoom);

  LOG("[Jaffar]  + Kid Horizontal Magnet Intensity / Position: %.1f / %.0f\n", kidMagnet.intensityX, kidMagnet.positionX);
  LOG("[Jaffar]  + Kid Vertical Magnet Intensity: %.1f\n", kidMagnet.intensityY);

  // Getting guard room
  int guardCurrentRoom = gameState.Guard.room;

  // Getting magnet values for the guard
  auto guardMagnet = getGuardMagnetValues(rulesStatus, guardCurrentRoom);

  LOG("[Jaffar]  + Guard Horizontal Magnet Intensity / Position: %.1f / %.0f\n", guardMagnet.intensityX, guardMagnet.positionX);
  LOG("[Jaffar]  + Guard Vertical Magnet Intensity: %.1f\n", guardMagnet.intensityY);

}
