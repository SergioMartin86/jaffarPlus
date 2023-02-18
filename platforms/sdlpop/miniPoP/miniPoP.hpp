/*
MiniPop, a barebones thread-safe version of SDLPop for routing
Copyright (C) 2021 Sergio Martin

based on

SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2020  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#pragma once
#include "types.h"

extern "C" __thread struct sdlPopState_t gameState;
extern "C" __thread word* copyprot_room;
extern "C" __thread byte* sound_interruptible;
extern "C" __thread custom_options_type *custom;
extern "C" __thread byte is_validate_mode;
extern "C" __thread word text_time_remaining;
extern "C" __thread word text_time_minipop_total;
extern "C" __thread word is_show_time;
extern "C" __thread word resurrect_time;
extern "C" __thread word dont_reset_time;
extern "C" __thread byte sound_flags;
extern "C" __thread word draw_mode;
extern "C" __thread short start_level;
extern "C" __thread byte *guard_palettes;
extern "C" __thread chtab_type *chtab_addrs[10];
extern "C" __thread word copyprot_plac;
extern "C" __thread word copyprot_idx;
extern "C" __thread word cplevel_entr[14];
extern "C" __thread dialog_type *copyprot_dialog;
extern "C" __thread rect_type dialog_rect_1;
extern "C" __thread rect_type dialog_rect_2;
extern "C" __thread byte curr_minipop_tile;
extern "C" __thread byte curr_modifier;
extern "C" __thread tile_and_mod leftroom_[3];
extern "C" __thread tile_and_mod row_below_left_[10];
extern "C" __thread word loaded_room;
extern "C" __thread byte *curr_room_minipop_tiles;
extern "C" __thread byte *curr_room_modif;
extern "C" __thread word draw_xh;
extern "C" __thread byte graphics_mode;
extern "C" __thread word room_L;
extern "C" __thread word room_R;
extern "C" __thread word room_A;
extern "C" __thread word room_B;
extern "C" __thread word room_BR;
extern "C" __thread word room_BL;
extern "C" __thread word room_AR;
extern "C" __thread word room_AL;
extern "C" __thread short table_counts[5];
extern "C" __thread short drects_count;
extern "C" __thread short peels_count;
extern "C" __thread back_table_type foretable[200];
extern "C" __thread back_table_type backtable[200];
extern "C" __thread midtable_type midtable[50];
extern "C" __thread peel_type *peels_minipop_table[50];
extern "C" __thread rect_type drects[30];
extern "C" __thread sbyte obj_direction;
extern "C" __thread short obj_clip_left;
extern "C" __thread short obj_clip_minipop_top;
extern "C" __thread short obj_clip_right;
extern "C" __thread short obj_clip_bottom;
extern "C" __thread wipetable_type wipetable[300];
extern "C" __thread word need_drects;
extern "C" __thread word leveldoor_right;
extern "C" __thread word leveldoor_ybottom;
extern "C" __thread byte palace_wall_colors[44 * 3];
extern "C" __thread word seed_was_init;
extern "C" __thread byte *doorlink2_ad;
extern "C" __thread byte *doorlink1_ad;
extern "C" __thread word is_paused;
extern "C" __thread word is_restart_level;
extern "C" __thread byte sound_mode;
extern "C" __thread byte is_sound_on;
extern "C" __thread short guardhp_delta;
extern "C" __thread word next_room;
extern "C" __thread short hitp_delta;
extern "C" __thread word need_quotes;
extern "C" __thread short roomleave_result;
extern "C" __thread sound_buffer_type *sound_pointers[58];
extern "C" __thread chtab_type *chtab_minipop_title40;
extern "C" __thread chtab_type *chtab_minipop_title50;
extern "C" __thread short hof_count;
extern "C" __thread word demo_mode;
extern "C" __thread short next_sound;
extern "C" __thread short knock;
extern "C" __thread byte wipe_frames[30];
extern "C" __thread sbyte wipe_heights[30];
extern "C" __thread byte redraw_frames_anim[30];
extern "C" __thread byte redraw_frames2[30];
extern "C" __thread byte redraw_frames_floor_overlay[30];
extern "C" __thread byte redraw_frames_full[30];
extern "C" __thread byte redraw_frames_fore[30];
extern "C" __thread byte tile_object_redraw[30];
extern "C" __thread byte redraw_frames_above[10];
extern "C" __thread word need_full_redraw;
extern "C" __thread short n_curr_objs;
extern "C" __thread objtable_type objtable[50];
extern "C" __thread short curr_objs[50];
extern "C" __thread byte obj_xh;
extern "C" __thread byte obj_xl;
extern "C" __thread byte obj_y;
extern "C" __thread byte obj_chtab;
extern "C" __thread byte obj_id;
extern "C" __thread byte obj_minipop_tilepos;
extern "C" __thread short obj_x;
extern "C" __thread frame_type cur_frame;
extern "C" __thread word seamless;
extern "C" __thread trob_type trob;
extern "C" __thread short redraw_height;
extern "C" __thread byte curr_minipop_tilepos;
extern "C" __thread short curr_room;
extern "C" __thread mob_type curmob;
extern "C" __thread short tile_col;
extern "C" __thread byte key_states[SDL_NUM_SCANCODES];
extern "C" __thread short char_col_right;
extern "C" __thread short char_col_left;
extern "C" __thread short char_minipop_top_row;
extern "C" __thread short prev_char_minipop_top_row;
extern "C" __thread short prev_char_col_right;
extern "C" __thread short prev_char_col_left;
extern "C" __thread short char_bottom_row;
extern "C" __thread short jumped_through_mirror;
extern "C" __thread byte curr_minipop_tile2;
extern "C" __thread short tile_row;
extern "C" __thread word char_width_half;
extern "C" __thread word char_height;
extern "C" __thread short char_x_left;
extern "C" __thread short char_x_left_coll;
extern "C" __thread short char_x_right_coll;
extern "C" __thread short char_x_right;
extern "C" __thread short char_minipop_top_y;
extern "C" __thread byte fall_frame;
extern "C" __thread byte through_minipop_tile;
extern "C" __thread sbyte infrontx; // name from Apple II source
extern "C" __thread word current_sound;
extern "C" __thread byte edge_type;
extern "C" __thread char **sound_names;
extern "C" __thread int g_argc;
extern "C" __thread int last_key_scancode;
extern "C" __thread word curmob_index;
extern "C" __thread dat_type *dathandle;
extern "C" __thread word need_redraw_because_flipped;
extern "C" __thread byte *level_var_palettes;
extern "C" __thread word first_start;
extern "C" __thread sbyte distance_mirror;
extern "C" __thread Uint64 last_minipop_transition_counter;
extern "C" __thread sbyte bump_col_left_of_wall;
extern "C" __thread sbyte bump_col_right_of_wall;
extern "C" __thread sbyte right_checked_col;
extern "C" __thread sbyte left_checked_col;
extern "C" __thread short coll_minipop_tile_left_xpos;
extern "C" __thread word curr_minipop_tile_minipop_temp;
extern "C" __thread char exe_dir[POP_MAX_PATH];
extern "C" __thread bool found_exe_dir;
extern "C" __thread word which_quote;
extern "C" __thread dat_type *dat_chain_ptr;
extern "C" __thread char last_minipop_text_input;
extern "C" __thread short drawn_row;
extern "C" __thread short draw_bottom_y;
extern "C" __thread short draw_main_y;
extern "C" __thread short drawn_col;
extern "C" __thread byte tile_left;
extern "C" __thread byte modifier_left;
extern "C" __thread byte kidSeqId;
extern "C" __thread char levels_file[POP_MAX_PATH];

typedef void restore_room_after_quick_load_minipop_t(void);
typedef void load_global_options_minipop_t(void);
typedef void check_mod_param_minipop_t(void);
typedef void load_ingame_settings_minipop_t(void);
typedef void turn_sound_on_off_minipop_t(byte);
typedef void load_mod_options_minipop_t(void);
typedef void apply_seqtbl_patches_minipop_t(void);
typedef dat_type *__pascal open_dat_minipop_t(const char *file, int drive);
typedef int  parse_grmode_minipop_t(void);
typedef void  init_timer_minipop_t(int frequency);
typedef void  parse_cmdline_sound_minipop_t(void);
typedef void  set_hc_pal_minipop_t(void);
typedef void  show_loading_minipop_t(void);
typedef int  set_joy_mode_minipop_t(void);
typedef void  init_copyprot_dialog_minipop_t(void);
typedef void init_record_replay_minipop_t(void);
typedef void init_menu_minipop_t(void);
typedef word  prandom_minipop_t(word max);
typedef void far *__pascal load_from_opendats_alloc_minipop_t(int resource, const char *extension, data_location *out_result, int *out_size);
typedef void  set_pal_256_minipop_t(rgb_type far *source);
typedef void  set_pal_minipop_t(int index, int red, int green, int blue, int vsync);
typedef chtab_type *__pascal load_sprites_from_file_minipop_t(int resource, int palette_bits, int quit_on_error);
typedef void  close_dat_minipop_t(dat_type far *pointer);
typedef void init_lighting_minipop_t(void);
typedef void load_all_sounds_minipop_t(void);
typedef void  hof_read_minipop_t(void);
typedef void  release_minipop_title_images_minipop_t(void);
typedef void  free_optsnd_chtab_minipop_t(void);
typedef void  load_kid_sprite_minipop_t(void);
typedef void  load_lev_spr_minipop_t(int level);
typedef void  load_level_minipop_t(void);
typedef void  pos_guards_minipop_t(void);
typedef void  clear_coll_rooms_minipop_t(void);
typedef void  clear_saved_ctrl_minipop_t(void);
typedef void  do_startpos_minipop_t(void);
typedef void  find_start_level_door_minipop_t(void);
typedef int  check_sound_playing_minipop_t(void);
typedef int  do_paused_minipop_t(void);
typedef void idle_minipop_t(void);
typedef void  stop_sounds_minipop_t(void);
typedef void  show_copyprot_minipop_t(int where);
typedef void reset_timer_minipop_t(int timer_index);
typedef void  free_peels_minipop_t(void);
typedef int  play_level_2_minipop_t(void);
typedef void  timers_minipop_t(void);
typedef void  start_game_minipop_t(void);
typedef void  check_fall_flo_t(void);
typedef void  play_frame_minipop_t(void);
typedef void  draw_game_frame_minipop_t(void);
typedef void update_screen_minipop_t(void);
typedef void __pascal do_simple_wait_minipop_t(int timer_index);
typedef void reset_level_unused_fields_minipop_t(bool loading_clean_level);
typedef void  load_room_links_minipop_t(void);
typedef void set_timer_length_minipop_t(int timer_index, int length);
typedef void  draw_level_first_minipop_t(void);
typedef void  play_level_minipop_t(int level_number);
typedef int save_recorded_replay_minipop_t(const char *full_filename);
typedef void start_recording_minipop_t(void);
typedef void add_replay_move_minipop_t(void);
typedef void  process_minipop_trobs_minipop_t(void);
typedef void  do_mobs_minipop_t(void);
typedef void  check_skel_minipop_t(void);
typedef void  check_can_guard_see_kid_minipop_t(void);
typedef void  check_mirror_minipop_t(void);
typedef void  init_copyprot_minipop_t(void);
typedef void  alter_mods_allrm_minipop_t(void);
typedef void  start_replay_minipop_t(void);
typedef void  display_minipop_text_bottom_minipop_t(const char near *text);
typedef void  redraw_screen_minipop_t(int drawing_different_room);
typedef void  anim_minipop_tile_modif_minipop_t();

extern "C" restore_room_after_quick_load_minipop_t restore_room_after_quick_load;
extern "C" load_global_options_minipop_t load_global_options;
extern "C" check_mod_param_minipop_t check_mod_param;
extern "C" load_ingame_settings_minipop_t load_ingame_settings;
extern "C" turn_sound_on_off_minipop_t turn_sound_on_off;
extern "C" load_mod_options_minipop_t load_mod_options;
extern "C" apply_seqtbl_patches_minipop_t apply_seqtbl_patches;
extern "C" open_dat_minipop_t open_dat;
extern "C" parse_grmode_minipop_t parse_grmode;
extern "C" init_timer_minipop_t init_timer;
extern "C" parse_cmdline_sound_minipop_t parse_cmdline_sound;
extern "C" set_hc_pal_minipop_t set_hc_pal;
extern "C" show_loading_minipop_t show_loading;
extern "C" set_joy_mode_minipop_t set_joy_mode;
extern "C" init_copyprot_dialog_minipop_t init_copyprot_dialog;
extern "C" init_record_replay_minipop_t init_record_replay;
extern "C" init_menu_minipop_t init_menu;
extern "C" prandom_minipop_t prandom;
extern "C" load_from_opendats_alloc_minipop_t load_from_opendats_alloc;
extern "C" set_pal_256_minipop_t set_pal_256;
extern "C" set_pal_minipop_t set_pal;
extern "C" load_sprites_from_file_minipop_t load_sprites_from_file;
extern "C" close_dat_minipop_t close_dat;
extern "C" init_lighting_minipop_t init_lighting;
extern "C" load_all_sounds_minipop_t load_all_sounds;
extern "C" hof_read_minipop_t hof_read;
extern "C" release_minipop_title_images_minipop_t release_minipop_title_images;
extern "C" free_optsnd_chtab_minipop_t free_optsnd_chtab;
extern "C" load_kid_sprite_minipop_t load_kid_sprite;
extern "C" load_lev_spr_minipop_t load_lev_spr;
extern "C" load_level_minipop_t load_level;
extern "C" pos_guards_minipop_t pos_guards;
extern "C" clear_coll_rooms_minipop_t clear_coll_rooms;
extern "C" clear_saved_ctrl_minipop_t clear_saved_ctrl;
extern "C" do_startpos_minipop_t do_startpos;
extern "C" find_start_level_door_minipop_t find_start_level_door;
extern "C" check_sound_playing_minipop_t check_sound_playing;
extern "C" do_paused_minipop_t do_paused;
extern "C" idle_minipop_t idle;
extern "C" stop_sounds_minipop_t stop_sounds;
extern "C" show_copyprot_minipop_t show_copyprot;
extern "C" reset_timer_minipop_t reset_timer;
extern "C" free_peels_minipop_t free_peels;
extern "C" play_level_2_minipop_t play_level_2;
extern "C" timers_minipop_t timers;
extern "C" play_frame_minipop_t play_frame;
extern "C" draw_game_frame_minipop_t draw_game_frame;
extern "C" update_screen_minipop_t update_screen;
extern "C" do_simple_wait_minipop_t do_simple_wait;
extern "C" reset_level_unused_fields_minipop_t reset_level_unused_fields;
extern "C" load_room_links_minipop_t load_room_links;
extern "C" set_timer_length_minipop_t set_timer_length;
extern "C" draw_level_first_minipop_t draw_level_first;
extern "C" play_level_minipop_t play_level;
extern "C" save_recorded_replay_minipop_t save_recorded_replay;
extern "C" start_recording_minipop_t start_recording;
extern "C" add_replay_move_minipop_t add_replay_move;
extern "C" process_minipop_trobs_minipop_t process_minipop_trobs;
extern "C" do_mobs_minipop_t do_mobs;
extern "C" check_skel_minipop_t check_skel;
extern "C" check_can_guard_see_kid_minipop_t check_can_guard_see_kid;
extern "C" check_mirror_minipop_t check_mirror;
extern "C" init_copyprot_minipop_t init_copyprot;
extern "C" alter_mods_allrm_minipop_t alter_mods_allrm;
extern "C" start_replay_minipop_t start_replay;
extern "C" start_game_minipop_t start_game;
extern "C" display_minipop_text_bottom_minipop_t display_minipop_text_bottom;
extern "C" redraw_screen_minipop_t redraw_screen;
extern "C" anim_minipop_tile_modif_minipop_t anim_minipop_tile_modif;
extern "C" void gen_palace_wall_colors_randomizer();
extern "C" void check_fall_flo();

