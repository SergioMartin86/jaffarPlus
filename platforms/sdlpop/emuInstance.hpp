#pragma once

#include <emuInstanceBase.hpp>
#include "miniPoP.hpp"
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>

#define _STATE_FIXED_SIZE 148
#define _STATE_DIFFERENTIAL_SIZE 2501
#define _STATE_DATA_SIZE (_STATE_DIFFERENTIAL_SIZE + _STATE_FIXED_SIZE)

class EmuInstance : public EmuInstanceBase
{
 public:

 std::string _sdlPopEnvRoot;
 std::string _levelsFilePath;
 std::string _stateFilePath;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Levels File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Levels File' key.\n");
  _levelsFilePath = config["Levels File"].get<std::string>();

  if (isDefined(config, "SDLPop Root Path") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'SDLPop Root Path' key.\n");
  _sdlPopEnvRoot = config["SDLPop Root Path"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  _stateFilePath = config["State File"].get<std::string>();

  // If not found, looking for the SDLPOP_ROOT env variable
  if (dirExists(_sdlPopEnvRoot.c_str()))
  {
    sprintf(exe_dir, "%s", _sdlPopEnvRoot.c_str());
    found_exe_dir = true;
  }

  if (found_exe_dir == false)
  {
    fprintf(stderr, "[ERROR] Could not find the root folder for  Please set the 'SDLPop Root Path' configuration variable to the path where SDLPop is installed.\n");
    exit(-1);
  }

  // Setting levels.dat path
  sprintf(levels_file, "%s", _levelsFilePath.c_str());

  // Setting argument config
  is_validate_mode = true;
  g_argc = 1;

  // Fix feather fall problem when quickload/quicksaving
  init_copyprot();

  // Fix bug: with start_in_blind_mode enabled, moving objects are not displayed
  // until blind mode is toggled off+on??
  need_drects = 1;

  dathandle = open_dat("PRINCE.DAT", 0);

  parse_cmdline_sound();

  //////////////////////////////////////////////
  // init_game_main

  doorlink1_ad = /*&*/ gameState.level.doorlinks1;
  doorlink2_ad = /*&*/ gameState.level.doorlinks2;
  guard_palettes = (byte *)load_from_opendats_alloc(10, "bin", NULL, NULL);

  // Level color variations (1.3)
  level_var_palettes = reinterpret_cast<byte *>(load_from_opendats_alloc(20, "bin", NULL, NULL));

  // PRINCE.DAT: sword
  chtab_addrs[id_chtab_0_sword] = load_sprites_from_file(700, 1 << 2, 1);

  // PRINCE.DAT: flame, sword on floor, potion
  chtab_addrs[id_chtab_1_flameswordpotion] = load_sprites_from_file(150, 1 << 3, 1);

  close_dat(dathandle);

  // start_game

  start_level = 1;

  ///////////////////////////////////////////////////////////////
  // init_game

  text_time_remaining = 0;
  is_show_time = 1;
  gameState.checkpoint = 0;
  resurrect_time = 0;
  gameState.hitp_beg_lev = custom->start_hitp;    // 3
  gameState.current_level = 0;
  startLevel(1);
  gameState.need_level1_music = custom->intro_music_time_initial;

  // Loading state file
  loadStateFile(_stateFilePath);
 }

 void startLevel(const word level)
 {
  ///////////////////////////////////////////////////////////////
   // play_level
   if (level != gameState.current_level) load_lev_spr(level);

   load_kid_sprite();
   load_level();
   pos_guards();
   clear_coll_rooms();
   clear_saved_ctrl();

   gameState.drawn_room = 0;
   gameState.mobs_count = 0;
   gameState.trobs_count = 0;
   next_sound = -1;
   gameState.holding_sword = 0;
   gameState.grab_timer = 0;
   gameState.can_guard_see_kid = 0;
   gameState.united_with_shadow = 0;
   gameState.leveldoor_open = 0;
   gameState.demo_index = 0;
   gameState.demo_time = 0;
   gameState.guardhp_curr = 0;
   hitp_delta = 0;
   gameState.Guard.charid = charid_2_guard;
   gameState.Guard.direction = dir_56_none;
   do_startpos();

   // (level_number != 1)
   gameState.have_sword = (level == 0 || level >= custom->have_sword_from_level);

   find_start_level_door();

  if (gameState.need_level1_music != 0 && gameState.current_level == custom->intro_music_level)
    gameState.need_level1_music = custom->intro_music_time_restart;
 }

 void setSeed(const dword randomSeed)
 {
   gameState.random_seed = randomSeed;
 }

 dword advanceRNGState(const dword randomSeed)
 {
  return randomSeed * 214013 + 2531011;
 }

 dword reverseRNGState(const dword randomSeed)
 {
  return (randomSeed + 4292436285) * 3115528533;
 }

 std::string getSDLPopStateString() const
 {
  // Saving state
  std::string miniPopState;
  miniPopState.resize(_STATE_DATA_SIZE);
  serializeState((uint8_t*)miniPopState.data());
  auto sdlPopState = saveSdlPopState(miniPopState);
  return sdlPopState;
 }

 void loadStateFile(const std::string& _stateFilePath) override
 {
  // Loading state data
  std::string sdlPopState;
  loadStringFromFile(sdlPopState, _stateFilePath.c_str());
  std::string miniPopState = loadSdlPopState(sdlPopState);
  deserializeState((uint8_t*)miniPopState.data());
 }

 void saveStateFile(const std::string& _stateFilePath) const override
 {
  // Saving state
  auto sdlPopState = getSDLPopStateString();
  saveStringToFile(sdlPopState, _stateFilePath.c_str());
 }
 void serializeState(uint8_t* state) const override
 {
  memcpy(state, &gameState, _STATE_DATA_SIZE);
 }

 void deserializeState(const uint8_t* state) override
 {
  memcpy(&gameState, state, _STATE_DATA_SIZE);
 }

 // Controller input bits
 // 0 - L / 1  // Left Arrow
 // 1 - R / 2  // Right Arrow
 // 2 - U / 4  // Up Arrow
 // 3 - D / 8  // Down Arrow
 // 4 - S / 16 // Shift
 // 5 - C / 32 // Ctrl
 // 6 - A / 64 // A
 // 7 - - / 128

 // Move Format:
 // LRUDSCA.
 // ........

 static uint8_t moveStringToCode(const std::string& move)
 {
  uint8_t moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'L': moveCode |= 0b10000000; break;
    case 'R': moveCode |= 0b01000000; break;
    case 'U': moveCode |= 0b00100000; break;
    case 'D': moveCode |= 0b00010000; break;
    case 'S': moveCode |= 0b00001000; break;
    case 'C': moveCode |= 0b00000100; break;
    case 'A': moveCode |= 0b00000010; break;
    case '.': break;
    default: EXIT_WITH_ERROR("Move provided cannot be parsed: '%s', unrecognized character: '%c'\n", move.c_str(), move[i]);
  }

  return moveCode;
 }

 static std::string moveCodeToString(const uint8_t move)
 {
  std::string moveString;

  if (move & 0b10000000) moveString += 'L'; else moveString += '.';
  if (move & 0b01000000) moveString += 'R'; else moveString += '.';
  if (move & 0b00100000) moveString += 'U'; else moveString += '.';
  if (move & 0b00010000) moveString += 'D'; else moveString += '.';
  if (move & 0b00001000) moveString += 'S'; else moveString += '.';
  if (move & 0b00000100) moveString += 'C'; else moveString += '.';
  if (move & 0b00000010) moveString += 'A'; else moveString += '.';

  return moveString;
 }

 void advanceState(const std::string& move) override
 {
  advanceState(moveStringToCode(move));
 }

 void advanceState(const uint8_t move) override
 {
  key_states[SDL_SCANCODE_UP] = 0;
  key_states[SDL_SCANCODE_DOWN] = 0;
  key_states[SDL_SCANCODE_LEFT] = 0;
  key_states[SDL_SCANCODE_RIGHT] = 0;
  key_states[SDL_SCANCODE_RSHIFT] = 0;

  if (move & 0b10000000) key_states[SDL_SCANCODE_LEFT] = 1;
  if (move & 0b01000000) key_states[SDL_SCANCODE_RIGHT] = 1;
  if (move & 0b00100000) key_states[SDL_SCANCODE_UP] = 1;
  if (move & 0b00010000) key_states[SDL_SCANCODE_DOWN] = 1;
  if (move & 0b00001000) key_states[SDL_SCANCODE_RSHIFT] = 1;
  if (move & 0b00000100 && move & 0b00000010) is_restart_level = 1;

  guardhp_delta = 0;
  hitp_delta = 0;
  timers();
  play_frame();

  if (is_restart_level == 1)
  {
   startLevel(gameState.current_level);
   draw_level_first();
  }

  // if we're on lvl 4, check mirror
  if (gameState.current_level == 4)
  {
   if (jumped_through_mirror == -1) gameState.Guard.x = 245;
   check_mirror();
  }

  // If level has changed, then load it
  if (gameState.current_level != gameState.next_level)
  {
   if (gameState.current_level == custom->copyprot_level-1 && gameState.next_level == custom->copyprot_level)
    gameState.next_level = 15;

   if (gameState.current_level == 15)
    gameState.next_level = custom->copyprot_level;

   // Preventing changing level on win
   // startLevel(gameState.next_level);
   gameState.current_level = gameState.next_level;

   // Handle cutscenes
   //if (gameState.next_level == 2) for (size_t i = 0; i < 3; i++) gameState.random_seed = advanceRNGState(gameState.random_seed);
  }

  is_restart_level = 0;
 }

 static std::string loadSdlPopState(const std::string& sdlPopStateData)
 {
  if (sdlPopStateData.size() != sizeof(sdlPopState_t)) EXIT_WITH_ERROR("Error reading or save state not found. Expected %lu bytes, read: %lu\n", sizeof(sdlPopState_t), sdlPopStateData.size());

  sdlPopState_t sdlPopState;
  memcpy(&sdlPopState, sdlPopStateData.data(), sizeof(sdlPopState_t));

  miniPopState_t miniPopState;
  memcpy(&miniPopState.level, &sdlPopState.level, sizeof(miniPopState.level));
  miniPopState.checkpoint = sdlPopState.checkpoint;
  miniPopState.drawn_room = sdlPopState.drawn_room;
  miniPopState.current_level = sdlPopState.current_level;
  miniPopState.next_level = sdlPopState.next_level;
  miniPopState.mobs_count = sdlPopState.mobs_count;
  memcpy(miniPopState.mobs, sdlPopState.mobs, sizeof(sdlPopState.mobs));
  miniPopState.trobs_count = sdlPopState.trobs_count;
  memcpy(miniPopState.trobs, sdlPopState.trobs, sizeof(miniPopState.trobs));
  miniPopState.leveldoor_open = sdlPopState.leveldoor_open;
  memcpy(&miniPopState.Kid, &sdlPopState.Kid, sizeof(miniPopState.Kid));
  miniPopState.hitp_curr = sdlPopState.hitp_curr;
  miniPopState.hitp_max = sdlPopState.hitp_max;
  miniPopState.hitp_beg_lev = sdlPopState.hitp_beg_lev;
  miniPopState.grab_timer = sdlPopState.grab_timer;
  miniPopState.holding_sword = sdlPopState.holding_sword;
  miniPopState.united_with_shadow = sdlPopState.united_with_shadow;
  miniPopState.have_sword = sdlPopState.have_sword;
  miniPopState.kid_sword_strike = sdlPopState.kid_sword_strike;
  miniPopState.pickup_obj_type = sdlPopState.pickup_obj_type;
  miniPopState.offguard = sdlPopState.offguard;
  memcpy(&miniPopState.Guard, &sdlPopState.Guard, sizeof(miniPopState.Guard));
  miniPopState.guardhp_curr = sdlPopState.guardhp_curr;
  miniPopState.guardhp_max = sdlPopState.guardhp_max;
  miniPopState.demo_index = sdlPopState.demo_index;
  miniPopState.demo_time = sdlPopState.demo_time;
  miniPopState.guard_notice_timer = sdlPopState.guard_notice_timer;
  miniPopState.guard_skill = sdlPopState.guard_skill;
  miniPopState.shadow_initialized = sdlPopState.shadow_initialized;
  miniPopState.guard_refrac = sdlPopState.guard_refrac;
  miniPopState.justblocked = sdlPopState.justblocked;
  miniPopState.droppedout = sdlPopState.droppedout;
  memcpy(&miniPopState.curr_row_coll_room, &sdlPopState.curr_row_coll_room, sizeof(miniPopState.curr_row_coll_room));
  memcpy(&miniPopState.curr_row_coll_flags, &sdlPopState.curr_row_coll_flags, sizeof(miniPopState.curr_row_coll_flags));
  memcpy(&miniPopState.below_row_coll_room, &sdlPopState.below_row_coll_room, sizeof(miniPopState.below_row_coll_room));
  memcpy(&miniPopState.below_row_coll_flags, &sdlPopState.below_row_coll_flags, sizeof(miniPopState.below_row_coll_flags));
  memcpy(&miniPopState.above_row_coll_room, &sdlPopState.above_row_coll_room, sizeof(miniPopState.above_row_coll_room));
  memcpy(&miniPopState.above_row_coll_flags, &sdlPopState.above_row_coll_flags, sizeof(miniPopState.above_row_coll_flags));
  miniPopState.prev_collision_row = sdlPopState.prev_collision_row;
  miniPopState.need_level1_music = sdlPopState.need_level1_music;
  miniPopState.is_screaming = sdlPopState.is_screaming;
  miniPopState.is_feather_fall = sdlPopState.is_feather_fall;
  miniPopState.last_loose_sound = sdlPopState.last_loose_sound;
  miniPopState.random_seed = sdlPopState.random_seed;
  miniPopState.ctrl1_forward = sdlPopState.ctrl1_forward;
  miniPopState.ctrl1_backward = sdlPopState.ctrl1_backward;
  miniPopState.ctrl1_up = sdlPopState.ctrl1_up;
  miniPopState.ctrl1_down = sdlPopState.ctrl1_down;
  miniPopState.ctrl1_shift2 = sdlPopState.ctrl1_shift2;
  miniPopState.exit_room_timer = sdlPopState.exit_room_timer;
  miniPopState.is_guard_notice = sdlPopState.is_guard_notice;
  miniPopState.can_guard_see_kid = sdlPopState.can_guard_see_kid;

  std::string stateData;
  stateData.resize(sizeof(miniPopState_t));
  memcpy(stateData.data(), &miniPopState, sizeof(miniPopState_t));
  return stateData;
 }

 static std::string saveSdlPopState(const std::string& miniPopStateData)
 {
  if (miniPopStateData.size() != sizeof(miniPopState_t)) EXIT_WITH_ERROR("Error reading minipop save state. Expected %lu bytes, read: %lu\n", miniPopStateData.size(), sizeof(miniPopStateData));

  miniPopState_t miniPopState;
  memcpy(&miniPopState, miniPopStateData.data(), sizeof(miniPopState_t));

  sdlPopState_t sdlPopState;
  memcpy(&sdlPopState.quick_control, ".........", sizeof(sdlPopState.quick_control));
  memcpy(&sdlPopState.level, &miniPopState.level, sizeof(sdlPopState.level));
  sdlPopState.checkpoint = miniPopState.checkpoint;
  sdlPopState.upside_down = 0;
  sdlPopState.drawn_room = miniPopState.drawn_room;
  sdlPopState.current_level = miniPopState.current_level;
  sdlPopState.next_level = miniPopState.next_level;
  sdlPopState.mobs_count = miniPopState.mobs_count;
  memcpy(sdlPopState.mobs, miniPopState.mobs, sizeof(miniPopState.mobs));
  sdlPopState.trobs_count = miniPopState.trobs_count;
  memcpy(sdlPopState.trobs, miniPopState.trobs, sizeof(sdlPopState.trobs));
  sdlPopState.leveldoor_open = miniPopState.leveldoor_open;
  memcpy(&sdlPopState.Kid, &miniPopState.Kid, sizeof(sdlPopState.Kid));
  sdlPopState.hitp_curr = miniPopState.hitp_curr;
  sdlPopState.hitp_max = miniPopState.hitp_max;
  sdlPopState.hitp_beg_lev = miniPopState.hitp_beg_lev;
  sdlPopState.grab_timer = miniPopState.grab_timer;
  sdlPopState.holding_sword = miniPopState.holding_sword;
  sdlPopState.united_with_shadow = miniPopState.united_with_shadow;
  sdlPopState.have_sword = miniPopState.have_sword;
  sdlPopState.kid_sword_strike = miniPopState.kid_sword_strike;
  sdlPopState.pickup_obj_type = miniPopState.pickup_obj_type;
  sdlPopState.offguard = miniPopState.offguard;
  memcpy(&sdlPopState.Guard, &miniPopState.Guard, sizeof(sdlPopState.Guard));
  memcpy(&sdlPopState.Char, &miniPopState.Kid, sizeof(sdlPopState.Char));
  memcpy(&sdlPopState.Opp, &miniPopState.Guard, sizeof(sdlPopState.Opp));
  sdlPopState.guardhp_curr = miniPopState.guardhp_curr;
  sdlPopState.guardhp_max = miniPopState.guardhp_max;
  sdlPopState.demo_index = miniPopState.demo_index;
  sdlPopState.demo_time = miniPopState.demo_time;
  sdlPopState.curr_guard_color = 0;
  sdlPopState.guard_notice_timer = miniPopState.guard_notice_timer;
  sdlPopState.guard_skill = miniPopState.guard_skill;
  sdlPopState.shadow_initialized = miniPopState.shadow_initialized;
  sdlPopState.guard_refrac = miniPopState.guard_refrac;
  sdlPopState.justblocked = miniPopState.justblocked;
  sdlPopState.droppedout = miniPopState.droppedout;
  memcpy(&sdlPopState.curr_row_coll_room, &miniPopState.curr_row_coll_room, sizeof(sdlPopState.curr_row_coll_room));
  memcpy(&sdlPopState.curr_row_coll_flags, &miniPopState.curr_row_coll_flags, sizeof(sdlPopState.curr_row_coll_flags));
  memcpy(&sdlPopState.below_row_coll_room, &miniPopState.below_row_coll_room, sizeof(sdlPopState.below_row_coll_room));
  memcpy(&sdlPopState.below_row_coll_flags, &miniPopState.below_row_coll_flags, sizeof(sdlPopState.below_row_coll_flags));
  memcpy(&sdlPopState.above_row_coll_room, &miniPopState.above_row_coll_room, sizeof(sdlPopState.above_row_coll_room));
  memcpy(&sdlPopState.above_row_coll_flags, &miniPopState.above_row_coll_flags, sizeof(sdlPopState.above_row_coll_flags));
  sdlPopState.prev_collision_row = miniPopState.prev_collision_row;
  sdlPopState.flash_color = 0;
  sdlPopState.flash_time = 0;
  sdlPopState.need_level1_music = miniPopState.need_level1_music;
  sdlPopState.is_screaming = miniPopState.is_screaming;
  sdlPopState.is_feather_fall = miniPopState.is_feather_fall;
  sdlPopState.last_loose_sound = miniPopState.last_loose_sound;
  sdlPopState.random_seed = miniPopState.random_seed;
  sdlPopState.rem_min = 60;
  sdlPopState.rem_tick = 719;
  sdlPopState.control_x = 0;
  sdlPopState.control_y = 0;
  sdlPopState.control_shift = 0;
  sdlPopState.control_forward = 0;
  sdlPopState.control_backward = 0;
  sdlPopState.control_up = 0;
  sdlPopState.control_down = 0;
  sdlPopState.control_shift2 = 0;
  sdlPopState.ctrl1_forward = miniPopState.ctrl1_forward;
  sdlPopState.ctrl1_backward = miniPopState.ctrl1_backward;
  sdlPopState.ctrl1_up = miniPopState.ctrl1_up;
  sdlPopState.ctrl1_down = miniPopState.ctrl1_down;
  sdlPopState.ctrl1_shift2 = miniPopState.ctrl1_shift2;
  sdlPopState.exit_room_timer = miniPopState.exit_room_timer;
  sdlPopState.replay_curr_tick = 0;
  sdlPopState.is_guard_notice = miniPopState.is_guard_notice;
  sdlPopState.can_guard_see_kid = miniPopState.can_guard_see_kid;

  std::string stateData;
  stateData.resize(sizeof(sdlPopState_t));
  memcpy(stateData.data(), &sdlPopState, sizeof(sdlPopState_t));
  return stateData;
 }

};
