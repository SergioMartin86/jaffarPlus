#pragma once

#include "config.h"
#include "types.h"
#include <utils.hpp>
#include <string>
#include <playbackInstanceBase.hpp>
#include <dlfcn.h>
#include <filesystem>

char quick_control[] = "........";
float replay_curr_tick = 0.0;

enum ItemType
{
  PER_FRAME_STATE,
  HASHABLE,
  HASHABLE_MANUAL,
};

struct Item
{
  void *ptr;
  size_t size;
  ItemType type;
};

template <class T>
void AddItem(std::vector<Item> *dest, T &val, ItemType type)
{
  dest->push_back({&val, sizeof(val), type});
}

// Function types
typedef void (*restore_room_after_quick_load_t)(void);
typedef void (*load_global_options_t)(void);
typedef void (*check_mod_param_t)(void);
typedef void (*load_ingame_settings_t)(void);
typedef void (*turn_sound_on_off_t)(byte);
typedef void (*load_mod_options_t)(void);
typedef void (*apply_seqtbl_patches_t)(void);
typedef dat_type *(*__pascal open_dat_t)(const char *file, int drive);
typedef int (*__pascal far parse_grmode_t)(void);
typedef void (*__pascal far init_timer_t)(int frequency);
typedef void (*__pascal far parse_cmdline_sound_t)(void);
typedef void (*__pascal far set_hc_pal_t)(void);
typedef surface_type far *(*__pascal rect_sthg_t)(surface_type *surface, const rect_type far *rect);
typedef void (*__pascal far show_loading_t)(void);
typedef int (*__pascal far set_joy_mode_t)(void);
typedef void (*__pascal far init_copyprot_dialog_t)(void);
typedef void (*init_record_replay_t)(void);
typedef void (*init_menu_t)(void);
typedef word (*__pascal far prandom_t)(word max);
typedef void far *(*__pascal load_from_opendats_alloc_t)(int resource, const char *extension, data_location *out_result, int *out_size);
typedef void (*__pascal far set_pal_256_t)(rgb_type far *source);
typedef void (*__pascal far set_pal_t)(int index, int red, int green, int blue, int vsync);
typedef chtab_type *(*__pascal load_sprites_from_file_t)(int resource, int palette_bits, int quit_on_error);
typedef void (*__pascal far close_dat_t)(dat_type far *pointer);
typedef void (*init_lighting_t)(void);
typedef void (*load_all_sounds_t)(void);
typedef void (*__pascal far hof_read_t)(void);
typedef void (*__pascal far release_title_images_t)(void);
typedef void (*__pascal far free_optsnd_chtab_t)(void);
typedef surface_type far *(*__pascal make_offscreen_buffer_t)(const rect_type far *rect);
typedef void (*__pascal far load_kid_sprite_t)(void);
typedef void (*__pascal far load_lev_spr_t)(int level);
typedef void (*__pascal far load_level_t)(void);
typedef void (*__pascal far pos_guards_t)(void);
typedef void (*__pascal far clear_coll_rooms_t)(void);
typedef void (*__pascal far clear_saved_ctrl_t)(void);
typedef void (*__pascal far do_startpos_t)(void);
typedef void (*__pascal far find_start_level_door_t)(void);
typedef int (*__pascal far check_sound_playing_t)(void);
typedef int (*__pascal far do_paused_t)(void);
typedef void (*idle_t)(void);
typedef void (*__pascal far stop_sounds_t)(void);
typedef void (*__pascal far show_copyprot_t)(int where);
typedef void (*reset_timer_t)(int timer_index);
typedef void (*__pascal far free_peels_t)(void);
typedef int (*__pascal far play_level_2_t)(void);
typedef void (*__pascal far timers_t)(void);
typedef void (*__pascal far start_game_t)(void);
typedef void (*__pascal far play_frame_t)(void);
typedef void (*__pascal far draw_game_frame_t)(void);
typedef void (*update_screen_t)(void);
typedef void (*__pascal do_simple_wait_t)(int timer_index);
typedef void (*reset_level_unused_fields_t)(bool loading_clean_level);
typedef void (*__pascal far load_room_links_t)(void);
typedef void (*set_timer_length_t)(int timer_index, int length);
typedef void (*__pascal far draw_level_first_t)(void);
typedef void (*__pascal far play_level_t)(int level_number);
typedef int (*save_recorded_replay_t)(const char *full_filename);
typedef void (*start_recording_t)(void);
typedef void (*add_replay_move_t)(void);
typedef void (*__pascal far process_trobs_t)(void);
typedef void (*__pascal far do_mobs_t)(void);
typedef void (*__pascal far check_skel_t)(void);
typedef void (*__pascal far check_can_guard_see_kid_t)(void);
typedef void (*__pascal far check_mirror_t)(void);
typedef void (*__pascal far init_copyprot_t)(void);
typedef void (*__pascal far alter_mods_allrm_t)(void);
typedef void (*__pascal far start_replay_t)(void);
typedef void (*__pascal far display_text_bottom_t)(const char near *text);
typedef void (*__pascal far redraw_screen_t)(int drawing_different_room);
typedef void (*__pascal far draw_image_transp_vga_t)(image_type far *image,int xpos,int ypos);
typedef SDL_Surface* (*__pascal far get_final_surface_t)(void);

typedef chtab_type *chtab_addrs_t[10];
typedef mob_type mobs_t[14];
typedef trob_type trobs_t[30];
typedef sbyte curr_row_coll_room_t[10];
typedef sbyte below_row_coll_room_t[10];
typedef sbyte above_row_coll_room_t[10];
typedef byte curr_row_coll_flags_t[10];
typedef byte below_row_coll_flags_t[10];
typedef byte above_row_coll_flags_t[10];
typedef char exe_dir_t[256];
typedef byte key_states_t[SDL_NUM_SCANCODES];
typedef long int cachedFilePointerTable_t[MAX_CACHED_FILES];
typedef char* cachedFileBufferTable_t[MAX_CACHED_FILES];
typedef size_t cachedFileBufferSizes_t[MAX_CACHED_FILES];
typedef char cachedFilePathTable_t[MAX_CACHED_FILES][POP_MAX_PATH];
typedef size_t cachedFileCounter_t;
typedef char levels_file_t[POP_MAX_PATH];



class PlaybackInstance : public PlaybackInstanceBase
{
 private:

 std::vector<Item> _items;

 void *_dllHandle;

 size_t _IGTMins;
 size_t _IGTSecs;
 size_t _IGTMillisecs;
 std::string _overlayPath;
 bool _outputImagesEnabled;
 std::string _outputImagesPath;

 SDL_Surface* _downSurface;
 SDL_Surface* _upSurface;
 SDL_Surface* _leftSurface;
 SDL_Surface* _rightSurface;
 SDL_Surface* _shiftSurface;
 SDL_Surface* _down2Surface;
 SDL_Surface* _up2Surface;
 SDL_Surface* _left2Surface;
 SDL_Surface* _right2Surface;
 SDL_Surface* _shift2Surface;

 // SDLPop Functions
 restore_room_after_quick_load_t restore_room_after_quick_load;
 load_global_options_t load_global_options;
 check_mod_param_t check_mod_param;
 load_ingame_settings_t load_ingame_settings;
 turn_sound_on_off_t turn_sound_on_off;
 load_mod_options_t load_mod_options;
 apply_seqtbl_patches_t apply_seqtbl_patches;
 open_dat_t open_dat;
 parse_grmode_t parse_grmode;
 init_timer_t init_timer;
 parse_cmdline_sound_t parse_cmdline_sound;
 set_hc_pal_t set_hc_pal;
 rect_sthg_t rect_sthg;
 show_loading_t show_loading;
 set_joy_mode_t set_joy_mode;
 init_copyprot_dialog_t init_copyprot_dialog;
 init_record_replay_t init_record_replay;
 init_menu_t init_menu;
 prandom_t prandom;
 load_from_opendats_alloc_t load_from_opendats_alloc;
 set_pal_256_t set_pal_256;
 set_pal_t set_pal;
 load_sprites_from_file_t load_sprites_from_file;
 close_dat_t close_dat;
 init_lighting_t init_lighting;
 load_all_sounds_t load_all_sounds;
 hof_read_t hof_read;
 release_title_images_t release_title_images;
 free_optsnd_chtab_t free_optsnd_chtab;
 make_offscreen_buffer_t make_offscreen_buffer;
 load_kid_sprite_t load_kid_sprite;
 load_lev_spr_t load_lev_spr;
 load_level_t load_level;
 pos_guards_t pos_guards;
 clear_coll_rooms_t clear_coll_rooms;
 clear_saved_ctrl_t clear_saved_ctrl;
 do_startpos_t do_startpos;
 find_start_level_door_t find_start_level_door;
 check_sound_playing_t check_sound_playing;
 do_paused_t do_paused;
 idle_t idle;
 stop_sounds_t stop_sounds;
 show_copyprot_t show_copyprot;
 reset_timer_t reset_timer;
 free_peels_t free_peels;
 play_level_2_t play_level_2;
 timers_t timers;
 play_frame_t play_frame;
 draw_game_frame_t draw_game_frame;
 update_screen_t update_screen;
 do_simple_wait_t do_simple_wait;
 reset_level_unused_fields_t reset_level_unused_fields;
 load_room_links_t load_room_links;
 set_timer_length_t set_timer_length;
 draw_level_first_t draw_level_first;
 play_level_t play_level;
 save_recorded_replay_t save_recorded_replay;
 start_recording_t start_recording;
 add_replay_move_t add_replay_move;
 process_trobs_t process_trobs;
 do_mobs_t do_mobs;
 check_skel_t check_skel;
 check_can_guard_see_kid_t check_can_guard_see_kid;
 check_mirror_t check_mirror;
 init_copyprot_t init_copyprot;
 alter_mods_allrm_t alter_mods_allrm;
 start_replay_t start_replay;
 start_game_t start_game;
 display_text_bottom_t display_text_bottom;
 redraw_screen_t redraw_screen;
 draw_image_transp_vga_t draw_image_transp_vga;
 get_final_surface_t get_final_surface;

 // SDLPop State variables
 char_type *Kid;     //
 char_type *Guard;   //
 char_type *Char;    //
 char_type *Opp;     //
 short *trobs_count; //
 trobs_t *trobs;     //
 short *mobs_count;  //
 mobs_t *mobs;       //
 level_type *level;  //
 word *drawn_room;
 word *leveldoor_open;
 word *hitp_curr;
 word *guardhp_curr;
 word *current_level;
 word *next_level;
 word *checkpoint;
 word *upside_down;
 word *exit_room_timer;
 word *hitp_max;
 word *hitp_beg_lev;
 word *grab_timer;
 word *holding_sword;
 short *united_with_shadow; //
 short *pickup_obj_type;    //
 word *kid_sword_strike;
 word *offguard;
 word *have_sword;
 word *guardhp_max;
 word *demo_index;
 short *demo_time; //
 word *curr_guard_color;
 short *guard_notice_timer; //
 word *guard_skill;
 word *shadow_initialized;
 word *guard_refrac;
 word *justblocked;
 word *droppedout;
 curr_row_coll_room_t *curr_row_coll_room;
 curr_row_coll_flags_t *curr_row_coll_flags;
 below_row_coll_room_t *below_row_coll_room;
 below_row_coll_flags_t *below_row_coll_flags;
 above_row_coll_room_t *above_row_coll_room;
 above_row_coll_flags_t *above_row_coll_flags;
 sbyte *prev_collision_row;
 word *flash_color;
 word *flash_time;
 word *need_level1_music;
 word *is_screaming;
 word *is_feather_fall;
 word *last_loose_sound;
 dword *random_seed;
 dword *preserved_seed;
 short *rem_min;
 word *rem_tick;
 sbyte *control_x;
 sbyte *control_y;
 sbyte *control_shift;
 sbyte *control_forward;
 sbyte *control_backward;
 sbyte *control_up;
 sbyte *control_down;
 sbyte *control_shift2;
 sbyte *ctrl1_forward;
 sbyte *ctrl1_backward;
 sbyte *ctrl1_up;
 sbyte *ctrl1_down;
 sbyte *ctrl1_shift2;
 dword *curr_tick;
 dat_type **dathandle;
 byte **level_var_palettes;
 word *is_blind_mode;
 word *seed_was_init;
 word *need_drects;
 int *g_argc;
 char ***g_argv;
 surface_type **current_target_surface;
 SDL_Surface **onscreen_surface_;
 rect_type *screen_rect;
 word *cheats_enabled;
 word *draw_mode;
 word *demo_mode;
 int *play_demo_level;
 byte **doorlink1_ad;
 byte **doorlink2_ad;
 byte **guard_palettes;
 chtab_addrs_t *chtab_addrs;
 short *start_level; //
 surface_type **offscreen_surface;
 rect_type *rect_top;
 word *text_time_remaining;    //
 word *text_time_total;        //
 word *is_show_time;           //
 word *resurrect_time;         //
 custom_options_type **custom; //
 short *next_sound;            //
 short *can_guard_see_kid;     //
 short *hitp_delta;            //
 short *guardhp_delta;         //
 word *different_room;         //
 word *next_room;              //
 word *is_guard_notice;        //
 word *need_full_redraw;
 byte *is_validate_mode;
 exe_dir_t *exe_dir;
 bool *found_exe_dir;
 key_states_t *key_states;
 word *is_cutscene;
 bool isExitDoorOpen;
 byte *enable_quicksave_penalty;
 word *is_restart_level;
 SDL_Window **window_;
 byte* enable_copyprot;
 fixes_options_type** fixes;
 word* copyprot_plac;
 SDL_Renderer** renderer_;
 SDL_Texture** target_texture;
 short *jumped_through_mirror;
 levels_file_t* levels_file;

 int8_t _prevDrawnRoom;

 public:

  // Initializes the playback module instance
 PlaybackInstance(GameInstance* game, const nlohmann::json& config) : PlaybackInstanceBase(game, config)
 {
  std::string libraryFile;
  if(const char* env_p = std::getenv("SDLPOP_LIBRARY_PATH_OVERRIDE")) libraryFile = env_p;
  else
  {
   if (isDefined(config, "SDLPop Library File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'SDLPop Library File' key.\n");
   libraryFile = config["SDLPop Library File"].get<std::string>();
  }

  // Opening SDLPoP shared library
  _dllHandle = dlopen (libraryFile.c_str(), RTLD_NOW);
  if (!_dllHandle) EXIT_WITH_ERROR("Could not load %s. Check that this library's path is included in the LD_LIBRARY_PATH environment variable. \n", libraryFile.c_str());

  if (isDefined(config, "Overlay Path") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Overlay Path' key.\n");
  _overlayPath = config["Overlay Path"].get<std::string>();

  if (isDefined(config, "Output Images") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Overlay Images' key.\n");
  if (isDefined(config["Output Images"], "Enabled") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Overlay Images / Enabled' key.\n");
  _outputImagesEnabled = config["Output Images"]["Enabled"].get<bool>();
  if (isDefined(config["Output Images"], "Path") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Overlay Images / Enabled' key.\n");
  _outputImagesPath = config["Output Images"]["Path"].get<std::string>();

  if (_outputImagesEnabled) std::filesystem::create_directories(_outputImagesPath);
  // Functions
  restore_room_after_quick_load = (restore_room_after_quick_load_t)dlsym(_dllHandle, "restore_room_after_quick_load");
  load_global_options = (load_global_options_t)dlsym(_dllHandle, "load_global_options");
  check_mod_param = (check_mod_param_t)dlsym(_dllHandle, "check_mod_param");
  load_ingame_settings = (load_ingame_settings_t)dlsym(_dllHandle, "load_ingame_settings");
  turn_sound_on_off = (turn_sound_on_off_t)dlsym(_dllHandle, "turn_sound_on_off");
  load_mod_options = (load_mod_options_t)dlsym(_dllHandle, "load_mod_options");
  apply_seqtbl_patches = (apply_seqtbl_patches_t)dlsym(_dllHandle, "apply_seqtbl_patches");
  open_dat = (open_dat_t)dlsym(_dllHandle, "open_dat");
  parse_grmode = (parse_grmode_t)dlsym(_dllHandle, "parse_grmode");
  init_timer = (init_timer_t)dlsym(_dllHandle, "init_timer");
  parse_cmdline_sound = (parse_cmdline_sound_t)dlsym(_dllHandle, "parse_cmdline_sound");
  set_hc_pal = (set_hc_pal_t)dlsym(_dllHandle, "set_hc_pal");
  rect_sthg = (rect_sthg_t)dlsym(_dllHandle, "rect_sthg");
  show_loading = (show_loading_t)dlsym(_dllHandle, "show_loading");
  set_joy_mode = (set_joy_mode_t)dlsym(_dllHandle, "set_joy_mode");
  init_copyprot_dialog = (init_copyprot_dialog_t)dlsym(_dllHandle, "init_copyprot_dialog");
  init_record_replay = (init_record_replay_t)dlsym(_dllHandle, "init_record_replay");
  init_menu = (init_menu_t)dlsym(_dllHandle, "init_menu");
  prandom = (prandom_t)dlsym(_dllHandle, "prandom");
  load_from_opendats_alloc = (load_from_opendats_alloc_t)dlsym(_dllHandle, "load_from_opendats_alloc");
  set_pal_256 = (set_pal_256_t)dlsym(_dllHandle, "set_pal_256");
  set_pal = (set_pal_t)dlsym(_dllHandle, "set_pal");
  load_sprites_from_file = (load_sprites_from_file_t)dlsym(_dllHandle, "load_sprites_from_file");
  close_dat = (close_dat_t)dlsym(_dllHandle, "close_dat");
  init_lighting = (init_lighting_t)dlsym(_dllHandle, "init_lighting");
  load_all_sounds = (load_all_sounds_t)dlsym(_dllHandle, "load_all_sounds");
  release_title_images = (release_title_images_t)dlsym(_dllHandle, "release_title_images");
  free_optsnd_chtab = (free_optsnd_chtab_t)dlsym(_dllHandle, "free_optsnd_chtab");
  make_offscreen_buffer = (make_offscreen_buffer_t)dlsym(_dllHandle, "make_offscreen_buffer");
  load_lev_spr = (load_lev_spr_t)dlsym(_dllHandle, "load_lev_spr");
  load_level = (load_level_t)dlsym(_dllHandle, "load_level");
  pos_guards = (pos_guards_t)dlsym(_dllHandle, "pos_guards");
  clear_coll_rooms = (clear_coll_rooms_t)dlsym(_dllHandle, "clear_coll_rooms");
  clear_saved_ctrl = (clear_saved_ctrl_t)dlsym(_dllHandle, "clear_saved_ctrl");
  do_startpos = (do_startpos_t)dlsym(_dllHandle, "do_startpos");
  find_start_level_door = (find_start_level_door_t)dlsym(_dllHandle, "find_start_level_door");
  check_sound_playing = (check_sound_playing_t)dlsym(_dllHandle, "check_sound_playing");
  do_paused = (check_sound_playing_t)dlsym(_dllHandle, "do_paused");
  load_kid_sprite = (load_kid_sprite_t)dlsym(_dllHandle, "load_kid_sprite");
  idle = (idle_t)dlsym(_dllHandle, "idle");
  hof_read = (hof_read_t)dlsym(_dllHandle, "hof_read");
  stop_sounds = (stop_sounds_t)dlsym(_dllHandle, "stop_sounds");
  show_copyprot = (show_copyprot_t)dlsym(_dllHandle, "show_copyprot");
  reset_timer = (reset_timer_t)dlsym(_dllHandle, "reset_timer");
  free_peels = (free_peels_t)dlsym(_dllHandle, "free_peels");
  play_level_2 = (play_level_2_t)dlsym(_dllHandle, "play_level_2");
  timers = (timers_t)dlsym(_dllHandle, "timers");
  play_frame = (play_frame_t)dlsym(_dllHandle, "play_frame");
  draw_game_frame = (draw_game_frame_t)dlsym(_dllHandle, "draw_game_frame");
  update_screen = (update_screen_t)dlsym(_dllHandle, "update_screen");
  do_simple_wait = (do_simple_wait_t)dlsym(_dllHandle, "do_simple_wait");
  reset_level_unused_fields = (reset_level_unused_fields_t)dlsym(_dllHandle, "reset_level_unused_fields");
  load_room_links = (load_room_links_t)dlsym(_dllHandle, "load_room_links");
  set_timer_length = (set_timer_length_t)dlsym(_dllHandle, "set_timer_length");
  draw_level_first = (draw_level_first_t)dlsym(_dllHandle, "draw_level_first");
  play_level = (play_level_t)dlsym(_dllHandle, "play_level");
  save_recorded_replay = (save_recorded_replay_t)dlsym(_dllHandle, "save_recorded_replay");
  start_recording = (start_recording_t)dlsym(_dllHandle, "start_recording");
  add_replay_move = (add_replay_move_t)dlsym(_dllHandle, "add_replay_move");
  process_trobs = (process_trobs_t)dlsym(_dllHandle, "process_trobs");
  do_mobs = (do_mobs_t)dlsym(_dllHandle, "do_mobs");
  check_skel = (check_skel_t)dlsym(_dllHandle, "check_skel");
  check_can_guard_see_kid = (check_can_guard_see_kid_t)dlsym(_dllHandle, "check_can_guard_see_kid");
  open_dat = (open_dat_t)dlsym(_dllHandle, "open_dat");
  check_mirror = (check_mirror_t)dlsym(_dllHandle, "check_mirror");
  init_copyprot = (init_copyprot_t)dlsym(_dllHandle, "init_copyprot");
  alter_mods_allrm = (alter_mods_allrm_t) dlsym(_dllHandle, "alter_mods_allrm");
  start_replay = (start_replay_t) dlsym(_dllHandle, "start_replay");
  start_game = (start_game_t) dlsym(_dllHandle, "start_game");
  display_text_bottom = (display_text_bottom_t) dlsym(_dllHandle, "display_text_bottom");
  redraw_screen = (redraw_screen_t) dlsym(_dllHandle, "redraw_screen");
  draw_image_transp_vga = (draw_image_transp_vga_t) dlsym(_dllHandle, "draw_image_transp_vga");
  get_final_surface = (get_final_surface_t) dlsym(_dllHandle, "get_final_surface");

  // State variables
  Kid = (char_type *)dlsym(_dllHandle, "Kid");
  Guard = (char_type *)dlsym(_dllHandle, "Guard");
  Char = (char_type *)dlsym(_dllHandle, "Char");
  Opp = (char_type *)dlsym(_dllHandle, "Opp");
  trobs_count = (short *)dlsym(_dllHandle, "trobs_count");
  trobs = (trobs_t *)dlsym(_dllHandle, "trobs");
  mobs_count = (short *)dlsym(_dllHandle, "mobs_count");
  mobs = (mobs_t *)dlsym(_dllHandle, "mobs");
  level = (level_type *)dlsym(_dllHandle, "level");
  drawn_room = (word *)dlsym(_dllHandle, "drawn_room");
  leveldoor_open = (word *)dlsym(_dllHandle, "leveldoor_open");
  hitp_curr = (word *)dlsym(_dllHandle, "hitp_curr");
  guardhp_curr = (word *)dlsym(_dllHandle, "guardhp_curr");
  current_level = (word *)dlsym(_dllHandle, "current_level");
  next_level = (word *)dlsym(_dllHandle, "next_level");
  checkpoint = (word *)dlsym(_dllHandle, "checkpoint");
  upside_down = (word *)dlsym(_dllHandle, "upside_down");
  exit_room_timer = (word *)dlsym(_dllHandle, "exit_room_timer");
  hitp_max = (word *)dlsym(_dllHandle, "hitp_max");
  hitp_beg_lev = (word *)dlsym(_dllHandle, "hitp_beg_lev");
  grab_timer = (word *)dlsym(_dllHandle, "grab_timer");
  holding_sword = (word *)dlsym(_dllHandle, "holding_sword");
  united_with_shadow = (short *)dlsym(_dllHandle, "united_with_shadow");
  pickup_obj_type = (short *)dlsym(_dllHandle, "pickup_obj_type");
  kid_sword_strike = (word *)dlsym(_dllHandle, "kid_sword_strike");
  offguard = (word *)dlsym(_dllHandle, "offguard");
  have_sword = (word *)dlsym(_dllHandle, "have_sword");
  guardhp_max = (word *)dlsym(_dllHandle, "guardhp_max");
  demo_index = (word *)dlsym(_dllHandle, "demo_index");
  demo_time = (short *)dlsym(_dllHandle, "demo_time");
  curr_guard_color = (word *)dlsym(_dllHandle, "curr_guard_color");
  guard_notice_timer = (short *)dlsym(_dllHandle, "guard_notice_timer");
  guard_skill = (word *)dlsym(_dllHandle, "guard_skill");
  shadow_initialized = (word *)dlsym(_dllHandle, "shadow_initialized");
  guard_refrac = (word *)dlsym(_dllHandle, "guard_refrac");
  justblocked = (word *)dlsym(_dllHandle, "justblocked");
  droppedout = (word *)dlsym(_dllHandle, "droppedout");
  curr_row_coll_room = (curr_row_coll_room_t *)dlsym(_dllHandle, "curr_row_coll_room");
  curr_row_coll_flags = (curr_row_coll_flags_t *)dlsym(_dllHandle, "curr_row_coll_flags");
  above_row_coll_room = (above_row_coll_room_t *)dlsym(_dllHandle, "above_row_coll_room");
  above_row_coll_flags = (above_row_coll_flags_t *)dlsym(_dllHandle, "above_row_coll_flags");
  below_row_coll_room = (below_row_coll_room_t *)dlsym(_dllHandle, "below_row_coll_room");
  below_row_coll_flags = (below_row_coll_flags_t *)dlsym(_dllHandle, "below_row_coll_flags");
  prev_collision_row = (sbyte *)dlsym(_dllHandle, "prev_collision_row");
  flash_color = (word *)dlsym(_dllHandle, "flash_color");
  flash_time = (word *)dlsym(_dllHandle, "flash_time");
  need_level1_music = (word *)dlsym(_dllHandle, "need_level1_music");
  is_screaming = (word *)dlsym(_dllHandle, "is_screaming");
  is_feather_fall = (word *)dlsym(_dllHandle, "is_feather_fall");
  last_loose_sound = (word *)dlsym(_dllHandle, "last_loose_sound");
  random_seed = (dword *)dlsym(_dllHandle, "random_seed");
  preserved_seed = (dword *)dlsym(_dllHandle, "preserved_seed");
  rem_min = (short *)dlsym(_dllHandle, "rem_min");
  rem_tick = (word *)dlsym(_dllHandle, "rem_tick");
  control_x = (sbyte *)dlsym(_dllHandle, "control_x");
  control_y = (sbyte *)dlsym(_dllHandle, "control_y");
  control_shift = (sbyte *)dlsym(_dllHandle, "control_shift");
  control_forward = (sbyte *)dlsym(_dllHandle, "control_forward");
  control_backward = (sbyte *)dlsym(_dllHandle, "control_backward");
  control_up = (sbyte *)dlsym(_dllHandle, "control_up");
  control_down = (sbyte *)dlsym(_dllHandle, "control_down");
  control_shift2 = (sbyte *)dlsym(_dllHandle, "control_shift2");
  ctrl1_forward = (sbyte *)dlsym(_dllHandle, "ctrl1_forward");
  ctrl1_backward = (sbyte *)dlsym(_dllHandle, "ctrl1_backward");
  ctrl1_up = (sbyte *)dlsym(_dllHandle, "ctrl1_up");
  ctrl1_down = (sbyte *)dlsym(_dllHandle, "ctrl1_down");
  ctrl1_shift2 = (sbyte *)dlsym(_dllHandle, "ctrl1_shift2");
  curr_tick = (dword *)dlsym(_dllHandle, "curr_tick");
  dathandle = (dat_type **)dlsym(_dllHandle, "dathandle");
  level_var_palettes = (byte **)dlsym(_dllHandle, "level_var_palettes");
  is_blind_mode = (word *)dlsym(_dllHandle, "is_blind_mode");
  seed_was_init = (word *)dlsym(_dllHandle, "seed_was_init");
  dathandle = (dat_type **)dlsym(_dllHandle, "dathandle");
  need_drects = (word *)dlsym(_dllHandle, "need_drects");
  g_argc = (int *)dlsym(_dllHandle, "g_argc");
  g_argv = (char ***)dlsym(_dllHandle, "g_argv");
  current_target_surface = (surface_type **)dlsym(_dllHandle, "current_target_surface");
  onscreen_surface_ = (SDL_Surface **)dlsym(_dllHandle, "onscreen_surface_");
  screen_rect = (rect_type *)dlsym(_dllHandle, "screen_rect");
  cheats_enabled = (word *)dlsym(_dllHandle, "cheats_enabled");
  draw_mode = (word *)dlsym(_dllHandle, "draw_mode");
  demo_mode = (word *)dlsym(_dllHandle, "demo_mode");
  play_demo_level = (int *)dlsym(_dllHandle, "play_demo_level");
  doorlink1_ad = (byte **)dlsym(_dllHandle, "doorlink1_ad");
  doorlink2_ad = (byte **)dlsym(_dllHandle, "doorlink2_ad");
  guard_palettes = (byte **)dlsym(_dllHandle, "guard_palettes");
  chtab_addrs = (chtab_addrs_t *)dlsym(_dllHandle, "chtab_addrs");
  start_level = (short *)dlsym(_dllHandle, "start_level");
  offscreen_surface = (surface_type **)dlsym(_dllHandle, "offscreen_surface");
  rect_top = (rect_type *)dlsym(_dllHandle, "rect_top");
  text_time_remaining = (word *)dlsym(_dllHandle, "text_time_remaining");
  text_time_total = (word *)dlsym(_dllHandle, "text_time_total");
  is_show_time = (word *)dlsym(_dllHandle, "is_show_time");
  resurrect_time = (word *)dlsym(_dllHandle, "resurrect_time");
  custom = (custom_options_type **)dlsym(_dllHandle, "custom");
  next_sound = (short *)dlsym(_dllHandle, "next_sound");
  can_guard_see_kid = (short *)dlsym(_dllHandle, "can_guard_see_kid");
  hitp_delta = (short *)dlsym(_dllHandle, "hitp_delta");
  guardhp_delta = (short *)dlsym(_dllHandle, "guardhp_delta");
  different_room = (word *)dlsym(_dllHandle, "different_room");
  next_room = (word *)dlsym(_dllHandle, "next_room");
  is_guard_notice = (word *)dlsym(_dllHandle, "is_guard_notice");
  need_full_redraw = (word *)dlsym(_dllHandle, "need_full_redraw");
  is_validate_mode = (byte *)dlsym(_dllHandle, "is_validate_mode");
  exe_dir = (exe_dir_t *)dlsym(_dllHandle, "exe_dir");
  found_exe_dir = (bool *)dlsym(_dllHandle, "found_exe_dir");
  key_states = (key_states_t *)dlsym(_dllHandle, "key_states");
  is_cutscene = (word *)dlsym(_dllHandle, "is_cutscene");
  enable_quicksave_penalty = (byte *)dlsym(_dllHandle, "enable_quicksave_penalty");
  is_restart_level = (word *)dlsym(_dllHandle, "is_restart_level");
  last_loose_sound = (word *)dlsym(_dllHandle, "last_loose_sound");
  window_ = (SDL_Window **)dlsym(_dllHandle, "window_");
  enable_copyprot = (byte *)dlsym(_dllHandle, "enable_copyprot");
  fixes = (fixes_options_type**)dlsym(_dllHandle, "fixes");
  copyprot_plac = (word *) dlsym(_dllHandle, "copyprot_plac");
  renderer_ = (SDL_Renderer**) dlsym(_dllHandle, "renderer_");
  target_texture = (SDL_Texture**) dlsym(_dllHandle, "target_texture");
  jumped_through_mirror = (short*) dlsym(_dllHandle, "jumped_through_mirror");
  can_guard_see_kid = (short*) dlsym(_dllHandle, "can_guard_see_kid");
  levels_file = (levels_file_t*) dlsym(_dllHandle, "levels_file");

  // Initializing items map
  _items = GenerateItemsMap();

  // Initializing game

  _IGTMins = 0;
  _IGTSecs = 0;
  _IGTMillisecs = 0;

   if (dirExists(_game->_emu->_sdlPopEnvRoot.c_str()))
   {
     sprintf(*exe_dir, "%s", _game->_emu->_sdlPopEnvRoot.c_str());
     *found_exe_dir = true;
   }

   if (*found_exe_dir == false)
   {
     fprintf(stderr, "[ERROR] Could not find the root folder for  Please set the SDLPOP_ROOT environment variable to the path where SDLPop is installed.\n");
     exit(-1);
   }

   // Setting levels.dat path
   sprintf(*levels_file, "%s", _game->_emu->_levelsFilePath.c_str());

   // Setting argument config
   *is_validate_mode = byte(0);
   *g_argc = 0;
   *g_argv = NULL;

   if (game->_emu->_initializeCopyProt) init_copyprot();

   (*fixes)->fix_quicksave_during_feather = 1;
   (*fixes)->fix_quicksave_during_lvl1_music = 1;

   // debug only: check that the sequence table deobfuscation did not mess things up
   load_global_options();
   check_mod_param();
   turn_sound_on_off(1);
   load_mod_options();

   // CusPop option
   *is_blind_mode = 0;
   *enable_quicksave_penalty = 0;

   // Fix bug: with start_in_blind_mode enabled, moving objects are not displayed
   // until blind mode is toggled off+on??
   *need_drects = 1;

   apply_seqtbl_patches();
   *dathandle = open_dat("PRINCE.DAT", 0);

   /*video_mode =*/
   parse_grmode();

   init_timer(BASE_FPS);
   parse_cmdline_sound();

   set_hc_pal();

   *current_target_surface = rect_sthg(*onscreen_surface_, screen_rect);

   show_loading();
   set_joy_mode();

   *cheats_enabled = 0;
   *draw_mode = 0;
   *demo_mode = 0;

   //init_copyprot_dialog();

   *play_demo_level = 0;


   //////////////////////////////////////////////
   // init_game_main

   *doorlink1_ad = /*&*/ level->doorlinks1;
   *doorlink2_ad = /*&*/ level->doorlinks2;
   *guard_palettes = (byte *)load_from_opendats_alloc(10, "bin", NULL, NULL);

   // (blood, hurt flash) #E00030 = red
   set_pal(12, 0x38, 0x00, 0x0C, 1);

   // (palace wall pattern) #C09850 = light brown
   set_pal(6, 0x30, 0x26, 0x14, 0);

   // Level color variations (1.3)
   *level_var_palettes = reinterpret_cast<byte *>(load_from_opendats_alloc(20, "bin", NULL, NULL));

   // PRINCE.DAT: sword
   (*chtab_addrs)[id_chtab_0_sword] = load_sprites_from_file(700, 1 << 2, 1);

   // PRINCE.DAT: flame, sword on floor, potion
   (*chtab_addrs)[id_chtab_1_flameswordpotion] = load_sprites_from_file(150, 1 << 3, 1);

   close_dat(*dathandle);
   load_all_sounds();
   hof_read();

   ///////////////////////////////////////////////////
   // start_game

   release_title_images(); // added
   free_optsnd_chtab();    // added
   *start_level = 1;

   ///////////////////////////////////////////////////////////////
   // init_game

   *offscreen_surface = make_offscreen_buffer(rect_top);

   *text_time_remaining = 0;
   *text_time_total = 0;
   *is_show_time = 1;
   *checkpoint = 0;
   *upside_down = 0; // N.B. upside_down is also reset in set_start_pos()
   *resurrect_time = 0;
   *rem_min = (*custom)->start_minutes_left; // 60
   *rem_tick = (*custom)->start_ticks_left;  // 719
   *hitp_beg_lev = (*custom)->start_hitp;    // 3
   *current_level = 0;
   startLevel(1);
   *need_level1_music = (*custom)->intro_music_time_initial;


    std::string imagePath;

    imagePath = _overlayPath + std::string("/down.png");
    _downSurface = IMG_Load(imagePath.c_str());
    if (_downSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/up.png");
    _upSurface = IMG_Load(imagePath.c_str());
    if (_upSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/left.png");
    _leftSurface = IMG_Load(imagePath.c_str());
    if (_leftSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/right.png");
    _rightSurface = IMG_Load(imagePath.c_str());
    if (_rightSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/shift.png");
    _shiftSurface = IMG_Load(imagePath.c_str());
    if (_shiftSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/down2.png");
    _down2Surface = IMG_Load(imagePath.c_str());
    if (_down2Surface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/up2.png");
    _up2Surface = IMG_Load(imagePath.c_str());
    if (_up2Surface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/left2.png");
    _left2Surface = IMG_Load(imagePath.c_str());
    if (_left2Surface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/right2.png");
    _right2Surface = IMG_Load(imagePath.c_str());
    if (_right2Surface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

    imagePath = _overlayPath + std::string("/shift2.png");
    _shift2Surface = IMG_Load(imagePath.c_str());
    if (_shift2Surface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  // Loading state from underlying miniPop
  std::string state;
  state.resize(_STATE_DATA_SIZE_PLAY);
  _game->_emu->serializeState((uint8_t*)state.data());
  loadState(state);

  restore_room_after_quick_load();
  draw_game_frame();

  // Calculating timing
  size_t curMins = currentStep / 720;
  size_t curSecs = (currentStep % 720) / 12;
  size_t curMilliSecs = floor((double)(currentStep % 12) / 0.012);

  char IGTText[512];
  sprintf(IGTText, "IGT %2lu:%02lu.%03lu", curMins, curSecs, curMilliSecs);
//  sprintf(IGTText, "Cutscene: %02u / %04u", gameState.currentCutsceneDelay, gameState.cumulativeCutsceneDelay);
  display_text_bottom(IGTText);

  SDL_Surface* downSurface = _downSurface;
  SDL_Surface* upSurface = _upSurface;
  SDL_Surface* leftSurface = _leftSurface;
  SDL_Surface* rightSurface = _rightSurface;
  SDL_Surface* shiftSurface = _shiftSurface;

  if (move.find("D") != std::string::npos) downSurface = _down2Surface;
  if (move.find("U") != std::string::npos) upSurface = _up2Surface;
  if (move.find("L") != std::string::npos) leftSurface = _left2Surface;
  if (move.find("R") != std::string::npos) rightSurface = _right2Surface;
  if (move.find("S") != std::string::npos) shiftSurface = _shift2Surface;

  draw_image_transp_vga(downSurface, 280, 170);
  draw_image_transp_vga(upSurface, 280, 150);
  draw_image_transp_vga(leftSurface, 260, 170);
  draw_image_transp_vga(rightSurface, 300, 170);
  draw_image_transp_vga(shiftSurface, 260, 150);

  update_screen();

  if (Kid->sword == sword_2_drawn) set_timer_length(timer_1, 6);
  else set_timer_length(timer_1, 5);
  do_simple_wait(timer_1);

//    if (Kid->sword == sword_2_drawn) set_timer_length(timer_1, 3);
//    else set_timer_length(timer_1, 2);
//    do_simple_wait(timer_1);

  SDL_RenderClear(*renderer_);
  SDL_RenderCopy(*renderer_, *target_texture, NULL, NULL);
  SDL_RenderPresent(*renderer_);

  // Saving image file
  if (_outputImagesEnabled)
  {
   char fileName[512];
   sprintf(fileName, "%s/frame%05u.bmp", _outputImagesPath.c_str(), currentStep);
   auto surface = get_final_surface();
   SDL_SaveBMP(surface, fileName);
  }
 }

 void startLevel(const word level)
 {
  ///////////////////////////////////////////////////////////////
   // play_level
   if (level != *current_level) load_lev_spr(level);

   load_kid_sprite();
   load_level();
   pos_guards();
   clear_coll_rooms();
   clear_saved_ctrl();

   *drawn_room = 0;
   *mobs_count = 0;
   *trobs_count = 0;
   *next_sound = -1;
   *holding_sword = 0;
   *grab_timer = 0;
   *can_guard_see_kid = 0;
   *united_with_shadow = 0;
   *flash_time = 0;
   *leveldoor_open = 0;
   *demo_index = 0;
   *demo_time = 0;
   *guardhp_curr = 0;
   *hitp_delta = 0;
   Guard->charid = charid_2_guard;
   Guard->direction = dir_56_none;
   do_startpos();

   // (level_number != 1)
   *have_sword = (level == 0 || level >= (*custom)->have_sword_from_level);

   find_start_level_door();

   // busy waiting?
   while (check_sound_playing() && !do_paused()) idle();

   stop_sounds();

   restore_room_after_quick_load();
   draw_level_first();

   show_copyprot(0);
   *enable_copyprot = 1;
   reset_timer(timer_1);

   _prevDrawnRoom = *drawn_room;

   set_timer_length(timer_1, 12);
   // Setting exit door status
   isExitDoorOpen = isLevelExitDoorOpen();

   if (*need_level1_music != 0 && *current_level == (*custom)->intro_music_level)
    if ((*fixes)->fix_quicksave_during_lvl1_music)
     *need_level1_music = (*custom)->intro_music_time_restart;
 }

 void loadState(const std::string &data)
 {
   if (data.size() != _STATE_DATA_SIZE_PLAY)
     EXIT_WITH_ERROR("[Error] Wrong state size. Expected %lu, got: %lu\n", _STATE_DATA_SIZE_PLAY, data.size());

   size_t curPos = 0;
   for (const auto &item : _items)
   {
     memcpy(item.ptr, &data.c_str()[curPos], item.size);
     curPos += item.size;
   }

   isExitDoorOpen = isLevelExitDoorOpen();
   *different_room = 1;
   // Show the room where the prince is, even if the player moved the view away
   // from it (with the H,J,U,N keys).
   *next_room = *drawn_room = Kid->room;
   load_room_links();
 }

 bool isLevelExitDoorOpen()
 {
   bool door_open = *leveldoor_open;

   if (!door_open)
   {
     for (int i = 0; i < *trobs_count; ++i)
     {
       const auto &trob = (*trobs)[i];
       const auto idx = (trob.room - 1) * 30 + trob.tilepos;
       const auto type = level->fg[idx] & 0x1f;
       if (type == tiles_16_level_door_left)
       {
         door_open = true;
         break;
       }
     }
   }

   return door_open;
 }

 std::vector<Item> GenerateItemsMap()
 {
   std::vector<Item> dest;
   AddItem(&dest, quick_control, PER_FRAME_STATE);
   AddItem(&dest, *level, HASHABLE_MANUAL);
   AddItem(&dest, *checkpoint, PER_FRAME_STATE);
   AddItem(&dest, *upside_down, PER_FRAME_STATE);
   AddItem(&dest, *drawn_room, HASHABLE);
   AddItem(&dest, *current_level, PER_FRAME_STATE);
   AddItem(&dest, *next_level, PER_FRAME_STATE);
   AddItem(&dest, *mobs_count, HASHABLE_MANUAL);
   AddItem(&dest, *mobs, HASHABLE_MANUAL);
   AddItem(&dest, *trobs_count, HASHABLE_MANUAL);
   AddItem(&dest, *trobs, HASHABLE_MANUAL);
   AddItem(&dest, *leveldoor_open, HASHABLE);
   AddItem(&dest, *Kid, HASHABLE);
   AddItem(&dest, *hitp_curr, PER_FRAME_STATE);
   AddItem(&dest, *hitp_max, PER_FRAME_STATE);
   AddItem(&dest, *hitp_beg_lev, PER_FRAME_STATE);
   AddItem(&dest, *grab_timer, HASHABLE);
   AddItem(&dest, *holding_sword, HASHABLE);
   AddItem(&dest, *united_with_shadow, HASHABLE);
   AddItem(&dest, *have_sword, HASHABLE);
   /*AddItem(&dest, *ctrl1_forward, HASHABLE);
   AddItem(&dest, *ctrl1_backward, HASHABLE);
   AddItem(&dest, *ctrl1_up, HASHABLE);
   AddItem(&dest, *ctrl1_down, HASHABLE);
   AddItem(&dest, *ctrl1_shift2, HASHABLE);*/
   AddItem(&dest, *kid_sword_strike, HASHABLE);
   AddItem(&dest, *pickup_obj_type, HASHABLE);
   AddItem(&dest, *offguard, HASHABLE);
   // guard
   AddItem(&dest, *Guard, PER_FRAME_STATE);
   AddItem(&dest, *Char, PER_FRAME_STATE);
   AddItem(&dest, *Opp, PER_FRAME_STATE);
   AddItem(&dest, *guardhp_curr, PER_FRAME_STATE);
   AddItem(&dest, *guardhp_max, PER_FRAME_STATE);
   AddItem(&dest, *demo_index, PER_FRAME_STATE);
   AddItem(&dest, *demo_time, PER_FRAME_STATE);
   AddItem(&dest, *curr_guard_color, PER_FRAME_STATE);
   AddItem(&dest, *guard_notice_timer, HASHABLE);
   AddItem(&dest, *guard_skill, PER_FRAME_STATE);
   AddItem(&dest, *shadow_initialized, PER_FRAME_STATE);
   AddItem(&dest, *guard_refrac, HASHABLE);
   AddItem(&dest, *justblocked, HASHABLE);
   AddItem(&dest, *droppedout, HASHABLE);
   // collision
   AddItem(&dest, *curr_row_coll_room, PER_FRAME_STATE);
   AddItem(&dest, *curr_row_coll_flags, PER_FRAME_STATE);
   AddItem(&dest, *below_row_coll_room, PER_FRAME_STATE);
   AddItem(&dest, *below_row_coll_flags, PER_FRAME_STATE);
   AddItem(&dest, *above_row_coll_room, PER_FRAME_STATE);
   AddItem(&dest, *above_row_coll_flags, PER_FRAME_STATE);
   AddItem(&dest, *prev_collision_row, PER_FRAME_STATE);
   // flash
   AddItem(&dest, *flash_color, PER_FRAME_STATE);
   AddItem(&dest, *flash_time, PER_FRAME_STATE);
   // sounds
   AddItem(&dest, *need_level1_music, HASHABLE);
   AddItem(&dest, *is_screaming, HASHABLE);
   AddItem(&dest, *is_feather_fall, HASHABLE);
   AddItem(&dest, *last_loose_sound, HASHABLE);
   // AddItem(&dest, *next_sound, HASHABLE);
   // AddItem(&dest, *current_sound, HASHABLE);
   // random
   AddItem(&dest, *random_seed, PER_FRAME_STATE);
   // remaining time
   AddItem(&dest, *rem_min, PER_FRAME_STATE);
   AddItem(&dest, *rem_tick, PER_FRAME_STATE);
   // saved controls
   AddItem(&dest, *control_x, PER_FRAME_STATE);
   AddItem(&dest, *control_y, PER_FRAME_STATE);
   AddItem(&dest, *control_shift, PER_FRAME_STATE);
   AddItem(&dest, *control_forward, PER_FRAME_STATE);
   AddItem(&dest, *control_backward, PER_FRAME_STATE);
   AddItem(&dest, *control_up, PER_FRAME_STATE);
   AddItem(&dest, *control_down, PER_FRAME_STATE);
   AddItem(&dest, *control_shift2, PER_FRAME_STATE);
   AddItem(&dest, *ctrl1_forward, PER_FRAME_STATE);
   AddItem(&dest, *ctrl1_backward, PER_FRAME_STATE);
   AddItem(&dest, *ctrl1_up, PER_FRAME_STATE);
   AddItem(&dest, *ctrl1_down, PER_FRAME_STATE);
   AddItem(&dest, *ctrl1_shift2, PER_FRAME_STATE);
   // Support for overflow glitch
   AddItem(&dest, *exit_room_timer, PER_FRAME_STATE);
   // replay recording state
   AddItem(&dest, replay_curr_tick, PER_FRAME_STATE);
   AddItem(&dest, *is_guard_notice, PER_FRAME_STATE);
   AddItem(&dest, *can_guard_see_kid, PER_FRAME_STATE);
   return dest;
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {

    return true;
 }

};
