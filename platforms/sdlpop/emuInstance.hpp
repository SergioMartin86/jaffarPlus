#pragma once

#include <emuInstanceBase.hpp>
#include "miniPoP.hpp"
#include <state.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>

class EmuInstance : public EmuInstanceBase
{
 public:

 std::string _sdlPopEnvRoot;
 std::string _levelsFilePath;
 std::string _stateFilePath;

 dword _overrideRNGValue;
 word _overrideLooseSound;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom fil
  if(const char* env_p = std::getenv("SDLPOP_LEVELS_FILE_OVERRIDE")) _levelsFilePath = env_p;
  else
  {
   if (isDefined(config, "Levels File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Levels File' key.\n");
   _levelsFilePath = config["Levels File"].get<std::string>();
  }

  if(const char* env_p = std::getenv("SDLPOP_ROOT_PATH_OVERRIDE")) _sdlPopEnvRoot = env_p;
  else
  {
   if (isDefined(config, "SDLPop Root Path") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'SDLPop Root Path' key.\n");
   _sdlPopEnvRoot = config["SDLPop Root Path"].get<std::string>();
  }

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

  //init_copyprot();
  prandom(1);
  // Setting argument config
  is_validate_mode = true;
  g_argc = 1;

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

  if (isDefined(config, "RNG Value") == true)
   _overrideRNGValue = config["RNG Value"].get<dword>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'RNG Value' was not defined\n");

  if (isDefined(config, "Loose Tile Sound Id") == true)
   _overrideLooseSound = config["Loose Tile Sound Id"].get<word>();
  else EXIT_WITH_ERROR("[Error] Game Configuration 'Loose Tile Sound Id' was not defined\n");

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

 void loadStateFile(const std::string& _stateFilePath) override
 {
  // Loading state data
  std::string sdlPopState;
  loadStringFromFile(sdlPopState, _stateFilePath.c_str());
  uint8_t stateData[_STATE_DATA_SIZE_TRAIN];
  memset(stateData, 0, _STATE_DATA_SIZE_TRAIN);
  memcpy(stateData, (uint8_t*)sdlPopState.data(), sdlPopState.size());
  deserializeState(stateData);

  gameState.random_seed = _overrideRNGValue;
  gameState.last_loose_sound = _overrideLooseSound;
 }

 void saveStateFile(const std::string& _stateFilePath) const override
 {
  // Saving state
  std::string state;
  state.resize(_STATE_DATA_SIZE_TRAIN);
  serializeState((uint8_t*)state.data());
  saveStringToFile(state, _stateFilePath.c_str());
 }

 void serializeState(uint8_t* state) const override
 {
  memcpy(state, &gameState, _STATE_DATA_SIZE_TRAIN);
 }

 void deserializeState(const uint8_t* state) override
 {
  memcpy(&gameState, state, _STATE_DATA_SIZE_TRAIN);
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

 static inline uint8_t moveStringToCode(const std::string& move)
 {
  uint8_t moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'C': moveCode |= 0b01000000; break;
    case 'A': moveCode |= 0b00100000; break;
    case 'L': moveCode |= 0b00010000; break;
    case 'R': moveCode |= 0b00001000; break;
    case 'U': moveCode |= 0b00000100; break;
    case 'D': moveCode |= 0b00000010; break;
    case 'S': moveCode |= 0b00000001; break;
    case '.': break;
    default: EXIT_WITH_ERROR("Move provided cannot be parsed: '%s', unrecognized character: '%c'\n", move.c_str(), move[i]);
  }

  return moveCode;
 }

 static inline std::string moveCodeToString(const uint8_t move)
 {
  std::string moveString;

  if (move & 0b01000000) moveString += 'C'; else moveString += '.';
  if (move & 0b00100000) moveString += 'A'; else moveString += '.';
  if (move & 0b00010000) moveString += 'L'; else moveString += '.';
  if (move & 0b00001000) moveString += 'R'; else moveString += '.';
  if (move & 0b00000100) moveString += 'U'; else moveString += '.';
  if (move & 0b00000010) moveString += 'D'; else moveString += '.';
  if (move & 0b00000001) moveString += 'S'; else moveString += '.';

  return moveString;
 }

 void advanceState(const uint8_t move) override
 {
  key_states[SDL_SCANCODE_UP] = 0;
  key_states[SDL_SCANCODE_DOWN] = 0;
  key_states[SDL_SCANCODE_LEFT] = 0;
  key_states[SDL_SCANCODE_RIGHT] = 0;
  key_states[SDL_SCANCODE_RSHIFT] = 0;

  if (move & 0b01000000 && move & 0b00100000) is_restart_level = 1;
  if (move & 0b00010000) key_states[SDL_SCANCODE_LEFT] = 1;
  if (move & 0b00001000) key_states[SDL_SCANCODE_RIGHT] = 1;
  if (move & 0b00000100) key_states[SDL_SCANCODE_UP] = 1;
  if (move & 0b00000010) key_states[SDL_SCANCODE_DOWN] = 1;
  if (move & 0b00000001) key_states[SDL_SCANCODE_RSHIFT] = 1;

  guardhp_delta = 0;
  hitp_delta = 0;
  timers();
  play_frame();

  if (gameState.rem_tick == 0) gameState.rem_tick = 600;
  gameState.rem_tick--;

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

};
