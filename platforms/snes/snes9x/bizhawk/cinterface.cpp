#include <emulibc.h>
#include <waterboxcore.h>

#include "snes9x.h"
#include "memmap.h"
#include "srtc.h"
#include "apu/apu.h"
#include "apu/bapu/snes/snes.hpp"
#include "gfx.h"
#include "snapshot.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "logger.h"
#include "display.h"
#include "conffile.h"
#include <stdio.h>
#ifndef __WIN32__
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define RETRO_MEMORY_SNES_BSX_RAM ((1 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_BSX_PRAM ((2 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM ((3 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM ((4 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RAM ((5 << 8) | RETRO_MEMORY_SAVE_RAM)
#define RETRO_MEMORY_SNES_GAME_BOY_RTC ((6 << 8) | RETRO_MEMORY_RTC)

#define RETRO_GAME_TYPE_BSX 0x101
#define RETRO_GAME_TYPE_BSX_SLOTTED 0x102
#define RETRO_GAME_TYPE_SUFAMI_TURBO 0x103
#define RETRO_GAME_TYPE_SUPER_GAME_BOY 0x104

#define RETRO_LOG_ERROR 1
#define RETRO_LOG_INFO 0

#define EXPORT extern "C" ECL_EXPORT

static void log_cb(int level, const char *fmt, ...)
{
	_debug_puts(fmt);
}

static bool use_overscan = false;
static bool rom_loaded = false;

EXPORT void biz_set_sound_channels(int channels)
{
	// bits 0..7: channels 1..8
	S9xSetSoundControl(channels & 0xff);
}

EXPORT void biz_set_layers(int layers)
{
	// bits 0..3: bg 0..3
	// bit 4: NOT USED (check bits 8..11)
	Settings.BG_Forced = ~layers & 0x0f;
	// bit 5: window
	Settings.DisableGraphicWindows = !(layers & 0x20);
	// bit 6: transparency
	Settings.Transparency = !!(layers & 0x40);

	Settings.OBJ_Displayed = (layers >> 8) & 0xF;
}

/*void retro_get_system_info(struct retro_system_info *info)
{
	 memset(info,0,sizeof(retro_system_info));

	 info->library_name = "Snes9x";
	 info->library_version = VERSION;
	 info->valid_extensions = "smc|sfc|swc|fig|bs";
	 info->need_fullpath = false;
	 info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	 memset(info,0,sizeof(retro_system_av_info));

	 info->geometry.base_width = SNES_WIDTH;
	 info->geometry.base_height = use_overscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT;
	 info->geometry.max_width = MAX_SNES_WIDTH;
	 info->geometry.max_height = MAX_SNES_HEIGHT;
	 info->geometry.aspect_ratio = 4.0f / 3.0f;
	 info->timing.sample_rate = 32040;
	 info->timing.fps = retro_get_region() == RETRO_REGION_NTSC ? 21477272.0 / 357366.0 : 21281370.0 / 425568.0;
}*/

EXPORT void biz_soft_reset()
{
	S9xSoftReset();
}

EXPORT void biz_hard_reset()
{
	S9xReset();
}

/*

available devices for each port
	const struct retro_controller_description port_1[] = {
		{"SNES Joypad", RETRO_DEVICE_JOYPAD},
		{"SNES Mouse", RETRO_DEVICE_MOUSE},
		{"Multitap", RETRO_DEVICE_JOYPAD_MULTITAP},
	};

	const struct retro_controller_description port_2[] = {
		{"SNES Joypad", RETRO_DEVICE_JOYPAD},
		{"SNES Mouse", RETRO_DEVICE_MOUSE},
		{"Multitap", RETRO_DEVICE_JOYPAD_MULTITAP},
		{"SuperScope", RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE},
		{"Justifier", RETRO_DEVICE_LIGHTGUN_JUSTIFIER},
	};

*/

#define RETRO_DEVICE_NONE 0
#define RETRO_DEVICE_JOYPAD 1
#define RETRO_DEVICE_JOYPAD_MULTITAP 2
#define RETRO_DEVICE_MOUSE 3
#define RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE 4
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIER 5

static unsigned snes_devices[2];

static void retro_set_controller_port_device(unsigned port, unsigned device)
{
	if (port < 2)
	{
		int offset = snes_devices[0] == RETRO_DEVICE_JOYPAD_MULTITAP ? 4 : 1;
		switch (device)
		{
		case RETRO_DEVICE_JOYPAD:
			S9xSetController(port, CTL_JOYPAD, port * offset, 0, 0, 0);
			snes_devices[port] = RETRO_DEVICE_JOYPAD;
			break;
		case RETRO_DEVICE_JOYPAD_MULTITAP:
			S9xSetController(port, CTL_MP5, port * offset, port * offset + 1, port * offset + 2, port * offset + 3);
			snes_devices[port] = RETRO_DEVICE_JOYPAD_MULTITAP;
			break;
		case RETRO_DEVICE_MOUSE:
			S9xSetController(port, CTL_MOUSE, 0, 0, 0, 0);
			snes_devices[port] = RETRO_DEVICE_MOUSE;
			break;
		case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE:
			S9xSetController(port, CTL_SUPERSCOPE, 0, 0, 0, 0);
			snes_devices[port] = RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE;
			break;
		case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:
			S9xSetController(port, CTL_JUSTIFIER, 0, 0, 0, 0);
			snes_devices[port] = RETRO_DEVICE_LIGHTGUN_JUSTIFIER;
			break;
		default:
			log_cb(RETRO_LOG_ERROR, "[libretro]: Invalid device (%d).\n", device);
			break;
		}
		//if (!port)
		//	retro_set_controller_port_device(1, snes_devices[1]);
	}
}

EXPORT void biz_set_port_devices(unsigned left, unsigned right)
{
	retro_set_controller_port_device(0, left);
	retro_set_controller_port_device(1, right);
}

/*static void init_descriptors(void)
{
	struct retro_input_descriptor desc[] = {
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start"},

		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select"},
		{1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start"},

		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select"},
		{2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start"},

		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select"},
		{3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start"},

		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select"},
		{4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start"},

		{0},
	};

	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}*/

EXPORT int biz_load_rom(const void *data, int size)
{
	rom_loaded = Memory.LoadROMMem((const uint8_t *)data, size);

	int pixel_format = RGB565;

	S9xGraphicsDeinit();
	S9xSetRenderPixelFormat(pixel_format);
	S9xGraphicsInit();

	return rom_loaded;
}

/*bool retro_load_game_special(unsigned game_type,
							 const struct retro_game_info *info, size_t num_info)
{

	init_descriptors();
	switch (game_type)
	{
	case RETRO_GAME_TYPE_BSX:

		if (num_info == 1)
		{
			rom_loaded = Memory.LoadROMMem((const uint8_t *)info[0].data, info[0].size);
		}
		else if (num_info == 2)
		{
			memcpy(Memory.BIOSROM, (const uint8_t *)info[0].data, info[0].size);
			rom_loaded = Memory.LoadROMMem((const uint8_t *)info[1].data, info[1].size);
		}

		if (!rom_loaded && log_cb)
			log_cb(RETRO_LOG_ERROR, "[libretro]: BSX ROM loading failed...\n");

		break;

	case RETRO_GAME_TYPE_BSX_SLOTTED:

		if (num_info == 2)
			rom_loaded = Memory.LoadMultiCartMem((const uint8_t *)info[0].data, info[0].size,
												 (const uint8_t *)info[1].data, info[1].size, NULL, 0);

		if (!rom_loaded && log_cb)
			log_cb(RETRO_LOG_ERROR, "[libretro]: Multirom loading failed...\n");

		break;

	case RETRO_GAME_TYPE_SUFAMI_TURBO:

		if (num_info == 3)
			rom_loaded = Memory.LoadMultiCartMem((const uint8_t *)info[1].data, info[1].size,
												 (const uint8_t *)info[2].data, info[2].size, (const uint8_t *)info[0].data, info[0].size);

		if (!rom_loaded && log_cb)
			log_cb(RETRO_LOG_ERROR, "[libretro]: Sufami Turbo ROM loading failed...\n");

		break;

	default:
		rom_loaded = false;
		break;
	}

	return rom_loaded;
}*/

static void map_buttons();

EXPORT int biz_init()
{
	memset(&Settings, 0, sizeof(Settings));
	Settings.MouseMaster = TRUE;
	Settings.SuperScopeMaster = TRUE;
	Settings.JustifierMaster = TRUE;
	Settings.MultiPlayer5Master = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.SixteenBitSound = TRUE;
	Settings.Stereo = TRUE;
	Settings.SoundPlaybackRate = 44100;
	Settings.SoundInputRate = 32040;
	Settings.SoundSync = TRUE;
	Settings.SupportHiRes = TRUE;
	Settings.Transparency = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.InitialInfoStringTimeout = 120;
	Settings.HDMATimingHack = 100;
	Settings.BlockInvalidVRAMAccessMaster = TRUE;
	Settings.WrongMovieStateProtection = TRUE;
	Settings.DumpStreamsMaxFrames = -1;
	Settings.StretchScreenshots = 0;
	Settings.SnapshotScreenshots = FALSE;
	Settings.CartAName[0] = 0;
	Settings.CartBName[0] = 0;
	Settings.AutoSaveDelay = 1;
	Settings.DontSaveOopsSnapshot = TRUE;

	CPU.Flags = 0;

	if (!Memory.Init() || !S9xInitAPU())
	{
		log_cb(RETRO_LOG_ERROR, "[libretro]: Failed to init Memory or APU.\n");
		return 0;
	}

	S9xInitSound(24, 0); // 16, 0)
	S9xSetSoundMute(FALSE);
	//S9xSetSamplesAvailableCallback(S9xAudioCallback, NULL);

	GFX.Pitch = MAX_SNES_WIDTH * sizeof(uint16);
	GFX.Screen = (uint16 *)alloc_invisible(GFX.Pitch * MAX_SNES_HEIGHT);
	S9xGraphicsInit();

	S9xInitInputDevices();
	for (int i = 0; i < 2; i++)
	{
		S9xSetController(i, CTL_JOYPAD, i, 0, 0, 0);
		snes_devices[i] = RETRO_DEVICE_JOYPAD;
	}

	S9xUnmapAllControls();
	map_buttons();

	return 1;
}

EXPORT void biz_post_load_state()
{
	memset(IPPU.TileCached[TILE_2BIT], 0, MAX_2BIT_TILES);
	memset(IPPU.TileCached[TILE_4BIT], 0, MAX_4BIT_TILES);
	memset(IPPU.TileCached[TILE_8BIT], 0, MAX_8BIT_TILES);
	memset(IPPU.TileCached[TILE_2BIT_EVEN], 0, MAX_2BIT_TILES);
	memset(IPPU.TileCached[TILE_2BIT_ODD], 0, MAX_2BIT_TILES);
	memset(IPPU.TileCached[TILE_4BIT_EVEN], 0, MAX_4BIT_TILES);
	memset(IPPU.TileCached[TILE_4BIT_ODD], 0, MAX_4BIT_TILES);
}

#define MAP_BUTTON(id, name) S9xMapButton((id), S9xGetCommandT((name)), false)
#define MAKE_BUTTON(pad, btn) (((pad) << 4) | (btn))

#define PAD_1 1
#define PAD_2 2
#define PAD_3 3
#define PAD_4 4
#define PAD_5 5

#define BTN_B 0
#define BTN_Y 1
#define BTN_SELECT 2
#define BTN_START 3
#define BTN_UP 4
#define BTN_DOWN 5
#define BTN_LEFT 6
#define BTN_RIGHT 7
#define BTN_A 8
#define BTN_X 9
#define BTN_L 10
#define BTN_R 11
#define BTN_FIRST 0
#define BTN_LAST 11

#define MOUSE_X 0
#define MOUSE_Y 1
#define MOUSE_LEFT 2
#define MOUSE_RIGHT 3
#define MOUSE_FIRST 0
#define MOUSE_LAST 3

#define SCOPE_X 0
#define SCOPE_Y 1
#define SCOPE_TRIGGER 2
#define SCOPE_CURSOR 3
#define SCOPE_TURBO 4
#define SCOPE_PAUSE 5
#define SCOPE_FIRST 0
#define SCOPE_LAST 5

#define JUSTIFIER_X 0
#define JUSTIFIER_Y 1
#define JUSTIFIER_TRIGGER 2
#define JUSTIFIER_OFFSCREEN 3
#define JUSTIFIER_START 4
#define JUSTIFIER_FIRST 0
#define JUSTIFIER_LAST 4

#define BTN_POINTER (BTN_LAST + 1)
#define BTN_POINTER2 (BTN_POINTER + 1)

static void map_buttons()
{
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_A), "Joypad1 A");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_B), "Joypad1 B");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_X), "Joypad1 X");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_Y), "Joypad1 Y");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_SELECT), "{Joypad1 Select,Mouse1 L}");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_START), "{Joypad1 Start,Mouse1 R}");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_L), "Joypad1 L");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_R), "Joypad1 R");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_LEFT), "Joypad1 Left");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_RIGHT), "Joypad1 Right");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_UP), "Joypad1 Up");
	MAP_BUTTON(MAKE_BUTTON(PAD_1, BTN_DOWN), "Joypad1 Down");
	S9xMapPointer((BTN_POINTER), S9xGetCommandT("Pointer Mouse1+Superscope+Justifier1"), false);
	S9xMapPointer((BTN_POINTER2), S9xGetCommandT("Pointer Mouse2"), false);

	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_A), "Joypad2 A");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_B), "Joypad2 B");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_X), "Joypad2 X");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_Y), "Joypad2 Y");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_SELECT), "{Joypad2 Select,Mouse2 L,Superscope Fire,Justifier1 Trigger}");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_START), "{Joypad2 Start,Mouse2 R,Superscope Cursor,Justifier1 Start}");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_L), "Joypad2 L");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_R), "Joypad2 R");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_LEFT), "Joypad2 Left");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_RIGHT), "Joypad2 Right");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_UP), "{Joypad2 Up,Superscope ToggleTurbo,Justifier1 AimOffscreen}");
	MAP_BUTTON(MAKE_BUTTON(PAD_2, BTN_DOWN), "{Joypad2 Down,Superscope Pause}");

	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_A), "Joypad3 A");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_B), "Joypad3 B");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_X), "Joypad3 X");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_Y), "Joypad3 Y");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_SELECT), "Joypad3 Select");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_START), "Joypad3 Start");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_L), "Joypad3 L");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_R), "Joypad3 R");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_LEFT), "Joypad3 Left");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_RIGHT), "Joypad3 Right");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_UP), "Joypad3 Up");
	MAP_BUTTON(MAKE_BUTTON(PAD_3, BTN_DOWN), "Joypad3 Down");

	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_A), "Joypad4 A");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_B), "Joypad4 B");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_X), "Joypad4 X");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_Y), "Joypad4 Y");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_SELECT), "Joypad4 Select");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_START), "Joypad4 Start");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_L), "Joypad4 L");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_R), "Joypad4 R");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_LEFT), "Joypad4 Left");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_RIGHT), "Joypad4 Right");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_UP), "Joypad4 Up");
	MAP_BUTTON(MAKE_BUTTON(PAD_4, BTN_DOWN), "Joypad4 Down");

	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_A), "Joypad5 A");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_B), "Joypad5 B");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_X), "Joypad5 X");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_Y), "Joypad5 Y");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_SELECT), "Joypad5 Select");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_START), "Joypad5 Start");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_L), "Joypad5 L");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_R), "Joypad5 R");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_LEFT), "Joypad5 Left");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_RIGHT), "Joypad5 Right");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_UP), "Joypad5 Up");
	MAP_BUTTON(MAKE_BUTTON(PAD_5, BTN_DOWN), "Joypad5 Down");
}

//#define RETRO_DEVICE_LIGHTGUN 32

#define RETRO_DEVICE_ID_MOUSE_X 0
#define RETRO_DEVICE_ID_MOUSE_Y 1
#define RETRO_DEVICE_ID_LIGHTGUN_X 0
#define RETRO_DEVICE_ID_LIGHTGUN_Y 1

static int16_t *input_state_values;

static int16_t input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id)
{
	// port: 0, 1, or up to 4 for multitap

	// devices:

	// RETRO_DEVICE_JOYPAD (1) - regular joypad inputs, multitap or otherwise
	// id: B = 0, Y, Select, Start, Up, Down, Left, Right, A, X, L, R = 11

	// RETRO_DEVICE_MOUSE (3) - everything a mouse does
	// id: xpos = 0, ypos, Left Button, Right Button = 3

	// RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE (4) - everything a lightgun does
	// id: xpos = 0, ypos, trigger, cursor, turbo, pause = 5

	// RETRO_DEVICE_LIGHTGUN_JUSTIFIER (5) - everything a justifier does
	// id: xpos = 0, ypos, trigger, offscreen, start = 4

	// index: always 0

	//char buff[512];
	//sprintf(buff, "port %u device %u index %u id %u", port, device, index, id);
	//_debug_puts(buff);

	return input_state_values[port * 16 + id];
}

// libretro uses relative values for analogue devices.
// S9x seems to use absolute values, but do convert these into relative values in the core. (Why?!)
// Hack around it. :)
static int16_t snes_mouse_state[2][2] = {{0}, {0}};
static int16_t snes_scope_state[2] = {0};
static int16_t snes_justifier_state[2][2] = {{0}, {0}};
static void report_buttons()
{
	int _x, _y;
	int offset = snes_devices[0] == RETRO_DEVICE_JOYPAD_MULTITAP ? 4 : 1;
	for (int port = 0; port <= 1; port++)
	{
		switch (snes_devices[port])
		{
		case RETRO_DEVICE_JOYPAD:
			for (int i = BTN_FIRST; i <= BTN_LAST; i++)
				S9xReportButton(MAKE_BUTTON(port * offset + 1, i), input_state_cb(port * offset, RETRO_DEVICE_JOYPAD, 0, i));
			break;

		case RETRO_DEVICE_JOYPAD_MULTITAP:
			for (int j = 0; j < 4; j++)
				for (int i = BTN_FIRST; i <= BTN_LAST; i++)
					S9xReportButton(MAKE_BUTTON(port * offset + j + 1, i), input_state_cb(port * offset + j, RETRO_DEVICE_JOYPAD, 0, i));
			break;

		case RETRO_DEVICE_MOUSE:
			_x = input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
			_y = input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
			snes_mouse_state[port][0] += _x;
			snes_mouse_state[port][1] += _y;
			S9xReportPointer(BTN_POINTER + port, snes_mouse_state[port][0], snes_mouse_state[port][1]);
			for (int i = MOUSE_LEFT; i <= MOUSE_LAST; i++)
				S9xReportButton(MAKE_BUTTON(port + 1, i), input_state_cb(port, RETRO_DEVICE_MOUSE, 0, i));
			break;

		case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE:
			snes_scope_state[0] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE, 0, RETRO_DEVICE_ID_LIGHTGUN_X);
			snes_scope_state[1] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE, 0, RETRO_DEVICE_ID_LIGHTGUN_Y);
			if (snes_scope_state[0] < 0)
				snes_scope_state[0] = 0;
			else if (snes_scope_state[0] > (SNES_WIDTH - 1))
				snes_scope_state[0] = SNES_WIDTH - 1;
			if (snes_scope_state[1] < 0)
				snes_scope_state[1] = 0;
			else if (snes_scope_state[1] > (SNES_HEIGHT - 1))
				snes_scope_state[1] = SNES_HEIGHT - 1;
			S9xReportPointer(BTN_POINTER, snes_scope_state[0], snes_scope_state[1]);
			for (int i = SCOPE_TRIGGER; i <= SCOPE_LAST; i++)
				S9xReportButton(MAKE_BUTTON(2, i), input_state_cb(port, RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE, 0, i));
			break;

		case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:
			snes_justifier_state[port][0] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_JUSTIFIER, 0, RETRO_DEVICE_ID_LIGHTGUN_X);
			snes_justifier_state[port][1] += input_state_cb(port, RETRO_DEVICE_LIGHTGUN_JUSTIFIER, 0, RETRO_DEVICE_ID_LIGHTGUN_Y);
			if (snes_justifier_state[port][0] < 0)
				snes_justifier_state[port][0] = 0;
			else if (snes_justifier_state[port][0] > (SNES_WIDTH - 1))
				snes_justifier_state[port][0] = SNES_WIDTH - 1;
			if (snes_justifier_state[port][1] < 0)
				snes_justifier_state[port][1] = 0;
			else if (snes_justifier_state[port][1] > (SNES_HEIGHT - 1))
				snes_justifier_state[port][1] = SNES_HEIGHT - 1;
			S9xReportPointer(BTN_POINTER, snes_justifier_state[port][0], snes_justifier_state[port][1]);
			for (int i = JUSTIFIER_TRIGGER; i <= JUSTIFIER_LAST; i++)
				S9xReportButton(MAKE_BUTTON(2, i), input_state_cb(port, RETRO_DEVICE_LIGHTGUN_JUSTIFIER, 0, i));
			break;

		default:
			log_cb(RETRO_LOG_ERROR, "[libretro]: Unknown device...\n");
		}
	}
}

static void (*InputCallback)();
extern bool8 pad_read;
void S9xOnSNESPadRead()
{
	if (InputCallback)
		InputCallback();
}

EXPORT void SetInputCallback(void (*callback)())
{
	InputCallback = callback;
}

static int actual_width;
static int actual_height;

static void Blit(const uint16_t *src, uint8_t *dst)
{
	const int vinc = GFX.Pitch / sizeof(uint16_t) - actual_width;
	for (int j = 0; j < actual_height; j++)
	{
		for (int i = 0; i < actual_width; i++)
		{
			auto c = *src++;
			*dst++ = c << 3 & 0xf8 | c >> 2 & 7;
			*dst++ = c >> 3 & 0xfa | c >> 9 & 3;
			*dst++ = c >> 8 & 0xf8 | c >> 13 & 7;
			*dst++ = 0xff;
		}
		src += vinc;
	}
}

EXPORT void SetButtons(int16_t* buttons)
{
	input_state_values = buttons;
	report_buttons();
	input_state_values = nullptr;
}

EXPORT void FrameAdvance(FrameInfo *frame)
{
	pad_read = FALSE;
	S9xMainLoop();
	S9xFinalizeSamples();
	frame->Lagged = !pad_read;

	size_t avail = S9xGetSampleCount();
	S9xMixSamples((uint8 *)frame->SoundBuffer, avail);
	frame->Samples = avail / 2;

	frame->Width = actual_width;
	frame->Height = actual_height;
	Blit(GFX.Screen, (uint8_t*)frame->VideoBuffer);
}

EXPORT int biz_is_ntsc()
{
	return !Settings.PAL;
}


EXPORT void GetMemoryAreas(MemoryArea* m)
{
	m[0].Data = Memory.SRAM; // sram, or sufami A sram
	m[0].Name = "CARTRAM";
	m[0].Size = (unsigned)(Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0);
	if (m[0].Size > 0x20000)
		m[0].Size = 0x20000;
	m[0].Flags = MEMORYAREA_FLAGS_WRITABLE | MEMORYAREA_FLAGS_WORDSIZE2 | MEMORYAREA_FLAGS_SAVERAMMABLE;

	m[1].Data = Multi.sramB; // sufami B sram
	m[1].Name = "CARTRAM B";
	m[1].Size = (unsigned)(Multi.cartType && Multi.sramSizeB ? (1 << (Multi.sramSizeB + 3)) * 128 : 0);
	m[1].Flags = MEMORYAREA_FLAGS_WRITABLE | MEMORYAREA_FLAGS_WORDSIZE2 | MEMORYAREA_FLAGS_SAVERAMMABLE;

	m[2].Data = RTCData.reg;
	m[2].Name = "RTC";
	m[2].Size = (Settings.SRTC || Settings.SPC7110RTC) ? 20 : 0;
	m[2].Flags = MEMORYAREA_FLAGS_WRITABLE | MEMORYAREA_FLAGS_WORDSIZE1 | MEMORYAREA_FLAGS_SAVERAMMABLE;

	m[3].Data = Memory.RAM;
	m[3].Name = "WRAM";
	m[3].Size = 128 * 1024;
	m[3].Flags = MEMORYAREA_FLAGS_WRITABLE | MEMORYAREA_FLAGS_WORDSIZE2 | MEMORYAREA_FLAGS_PRIMARY;

	m[4].Data = Memory.VRAM;
	m[4].Name = "VRAM";
	m[4].Size = 64 * 1024;
	m[4].Flags = MEMORYAREA_FLAGS_WRITABLE | MEMORYAREA_FLAGS_WORDSIZE2;

	m[5].Data = Memory.ROM;
	m[5].Name = "CARTROM";
	m[5].Size = Memory.CalculatedSize;
	m[5].Flags = MEMORYAREA_FLAGS_WORDSIZE2;
}

bool8 S9xDeinitUpdate(int width, int height)
{
	if (!use_overscan)
	{
		if (height >= SNES_HEIGHT << 1)
		{
			height = SNES_HEIGHT << 1;
		}
		else
		{
			height = SNES_HEIGHT;
		}
	}
	else
	{
		if (height > SNES_HEIGHT_EXTENDED)
		{
			if (height < SNES_HEIGHT_EXTENDED << 1)
				memset(GFX.Screen + (GFX.Pitch >> 1) * height, 0, GFX.Pitch * ((SNES_HEIGHT_EXTENDED << 1) - height));
			height = SNES_HEIGHT_EXTENDED << 1;
		}
		else
		{
			if (height < SNES_HEIGHT_EXTENDED)
				memset(GFX.Screen + (GFX.Pitch >> 1) * height, 0, GFX.Pitch * (SNES_HEIGHT_EXTENDED - height));
			height = SNES_HEIGHT_EXTENDED;
		}
	}

	actual_width = width;
	actual_height = height;
	return TRUE;
}

bool8 S9xContinueUpdate(int width, int height)
{
	return true;
}

// Dummy functions that should probably be implemented correctly later.
void S9xParsePortConfig(ConfigFile &, int) {}
void S9xSyncSpeed() {}
const char *S9xStringInput(const char *in) { return in; }
const char *S9xGetFilename(const char *in, s9x_getdirtype) { return in; }
const char *S9xGetDirectory(s9x_getdirtype) { return ""; }
void S9xInitInputDevices() {}
const char *S9xChooseFilename(unsigned char) { return ""; }
void S9xHandlePortCommand(s9xcommand_t, short, short) {}
bool S9xPollButton(unsigned int, bool *) { return false; }
void S9xToggleSoundChannel(int) {}
const char *S9xGetFilenameInc(const char *in, s9x_getdirtype) { return ""; }
const char *S9xBasename(const char *in) { return in; }
bool8 S9xInitUpdate() { return TRUE; }
void S9xExtraUsage() {}
bool8 S9xOpenSoundDevice() { return TRUE; }
void S9xMessage(int, int, const char *) {}
bool S9xPollAxis(unsigned int, short *) { return FALSE; }
void S9xSetPalette() {}
void S9xParseArg(char **, int &, int) {}
void S9xExit() {}
bool S9xPollPointer(unsigned int, short *, short *) { return false; }
const char *S9xChooseMovieFilename(unsigned char) { return NULL; }

bool8 S9xOpenSnapshotFile(const char *filepath, bool8 read_only, STREAM *file)
{
	if (read_only)
	{
		if ((*file = OPEN_STREAM(filepath, "rb")) != 0)
		{
			return (TRUE);
		}
	}
	else
	{
		if ((*file = OPEN_STREAM(filepath, "wb")) != 0)
		{
			return (TRUE);
		}
	}
	return (FALSE);
}

void S9xCloseSnapshotFile(STREAM file)
{
	CLOSE_STREAM(file);
}

void S9xAutoSaveSRAM()
{
	return;
}

#ifndef __WIN32__
// S9x weirdness.
void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	*drive = 0;

	const char *slash = strrchr(path, SLASH_CHAR),
			   *dot = strrchr(path, '.');

	if (dot && slash && dot < slash)
		dot = NULL;

	if (!slash)
	{
		*dir = 0;

		strcpy(fname, path);

		if (dot)
		{
			fname[dot - path] = 0;
			strcpy(ext, dot + 1);
		}
		else
			*ext = 0;
	}
	else
	{
		strcpy(dir, path);
		dir[slash - path] = 0;

		strcpy(fname, slash + 1);

		if (dot)
		{
			fname[dot - slash - 1] = 0;
			strcpy(ext, dot + 1);
		}
		else
			*ext = 0;
	}
}

void _makepath(char *path, const char *, const char *dir, const char *fname, const char *ext)
{
	if (dir && *dir)
	{
		strcpy(path, dir);
		strcat(path, SLASH_STR);
	}
	else
		*path = 0;

	strcat(path, fname);

	if (ext && *ext)
	{
		strcat(path, ".");
		strcat(path, ext);
	}
}
#endif // __WIN32__

int main(void)
{
	return 0;
}
