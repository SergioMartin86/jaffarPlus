#pragma once

#include <array>
#include <cstdio>
#include <cstring>
#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <set>
#include <unordered_map>
#include <vector>

// Glitch-hunt input alphabet (U+X impossible-combo probes) is orthogonal to the QuickerNES-only
// bad-access DETECTOR: the probes are also needed by the NesHawk build for the free-exploration run
// (run 3). So the alphabet is guarded by _POP_GLITCH_HUNT (definable for any emulator build) while
// the detector (property/hashing/trace) stays under _QUICKERNES_DETECT_BAD_ACCESS. The detector
// build implies the alphabet.
// Either core's bad-access detector implies both the unified detector guard (_POP_BAD_ACCESS, used
// for the "CPU Bad Access" property/hashing/trace) and the glitch-hunt input alphabet.
#if defined(_QUICKERNES_DETECT_BAD_ACCESS) || defined(_NESHAWK_DETECT_BAD_ACCESS)
#ifndef _POP_BAD_ACCESS
#define _POP_BAD_ACCESS
#endif
#endif
#if defined(_POP_BAD_ACCESS) && !defined(_POP_GLITCH_HUNT)
#define _POP_GLITCH_HUNT
#endif

namespace jaffarPlus
{

namespace games
{

namespace nes
{

// Number of distinct rooms the level-magnet tables index over.
#define POP_ROOM_COUNT 256

class PrinceOfPersia final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Prince Of Persia"; }

  PrinceOfPersia(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Whether pressing Start (pause) is a meaningful game action to explore.
    _enablePause = jaffarCommon::json::popBoolean(_gameConfigRemaining, "Enable Pause");

    // Global-timer hashing tolerance: the timer is folded into the state hash modulo (tolerance + 1),
    // so states that differ only by up to this many timer ticks are treated as equivalent.
    _timerTolerance = jaffarCommon::json::popNumber<uint8_t>(_gameConfigRemaining, "Timer Tolerance");

    // Optional ledge-grab/climb bias: reward for being in a grab/climb-up animation frame while in one of
    // the listed rooms (see calculateGameSpecificReward). Biases the search to catch a ledge and climb
    // out rather than fall into a pit. Both keys optional; absent => disabled.
    if (_gameConfigRemaining.contains("Climb Bonus")) _climbBonus = jaffarCommon::json::popNumber<float>(_gameConfigRemaining, "Climb Bonus");
    if (_gameConfigRemaining.contains("Climb Bonus Rooms"))
      for (const auto r : jaffarCommon::json::popArray<int>(_gameConfigRemaining, "Climb Bonus Rooms")) _climbBonusRooms.insert((uint8_t)r);

#ifdef _POP_BAD_ACCESS
    // Optional: minimum reward jump for a derail to count as "progress-advancing" (see _glitchAdvanced).
    if (_gameConfigRemaining.contains("Glitch Reward Threshold")) _glitchRewardThreshold = jaffarCommon::json::popNumber<float>(_gameConfigRemaining, "Glitch Reward Threshold");
    if (_gameConfigRemaining.contains("Glitch Settle Frames")) _glitchSettleFrames = jaffarCommon::json::popNumber<uint8_t>(_gameConfigRemaining, "Glitch Settle Frames");
    if (_gameConfigRemaining.contains("Glitch Settle Window")) _glitchSettleWindow = jaffarCommon::json::popNumber<uint8_t>(_gameConfigRemaining, "Glitch Settle Window");
#endif
#ifdef _POP_GLITCH_HUNT
    // Optional override for the phase-pruning ceiling (see getAdditionalAllowedInputs).
    if (_gameConfigRemaining.contains("Max Input Phase")) _maxInputPhase = jaffarCommon::json::popNumber<uint8_t>(_gameConfigRemaining, "Max Input Phase");
#endif

    // All recognized game-configuration keys have now been consumed; the framework rejects any leftover.
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties (addresses per the Prince of Persia (U) [!] RAM map)
    registerGameProperty("Global Timer", &_lowMem[0x04EF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Current Level", &_lowMem[0x0070], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("RNG State", &_lowMem[0x0060], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Phase", &_lowMem[0x002C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bottom Text Timer", &_lowMem[0x06E5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game State", &_lowMem[0x001C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Password Timer", &_lowMem[0x007D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    // Next-level index consumed by the level loader (0-based, part of the 0x0766-0x076A load-parameter
    // cluster). Unlike zero-page 0x0070 ("Current Level"), which the loaders reuse as scratch and can hold
    // transient garbage mid-sequence, this lives in the stable game-variable region: it is 0 for the whole
    // level and flips to the destination index exactly when the level-end is confirmed (or a level-skip
    // glitch triggers a load). A death-reload of the same level keeps it 0. Use it for win detection.
    registerGameProperty("Next Level", &_lowMem[0x0767], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Emulator CPU halt latch: 1 once a KIL/JAM opcode executed (real hardware freezes there; only reset
    // recovers). The U+D input family jumps through a bad action pointer and CAN land on such opcodes --
    // those lineages are emulator artifacts, not valid play, and must FAIL (rule on this property).
    registerGameProperty("CPU Halted", _emulator->getProperty("CPU Halt Latch").pointer, Property::datatype_t::dt_uint8, Property::endianness_t::little);

#ifdef _POP_BAD_ACCESS
    // Glitch-investigation detector (only in -D_QUICKERNES_DETECT_BAD_ACCESS QuickerNES builds): a
    // per-frame flag, 1 iff this frame's advance executed an unofficial opcode or fetched code from
    // RAM -- i.e. the U+X action-pointer dispatch derailed into data-as-code. Legit play never sets
    // it (validated). This is the WIN signal for the QuickerNES "find where a bad access is reachable"
    // search; the room/position it fires in is then transplanted to the NesHawk ground-truth core.
    registerGameProperty("CPU Bad Access", _emulator->getProperty("CPU Bad Access").pointer, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    // "Glitch Advanced" == 1 marks a state whose parent was a derail that RAISED the reward (see the
    // one-frame-lag detector in advanceStateImpl). A win rule on this catches any progress-advancing
    // U+X glitch -- teleport forward, room skip, level skip -- without hardcoding specific effects,
    // because the reward already encodes progress. Normal play never sets it (a non-derail frame has
    // _prevBadAccess 0; a benign derail does not raise the reward past the threshold).
    registerGameProperty("Glitch Advanced", &_glitchAdvanced, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Teleported", &_teleported, Property::datatype_t::dt_uint8, Property::endianness_t::little);
#endif

    registerGameProperty("Player Pos X", &_lowMem[0x060F], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x0611], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Direction", &_lowMem[0x0615], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Frame", &_lowMem[0x0617], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Movement", &_lowMem[0x0613], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Fall Wait", &_lowMem[0x06E4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player HP", &_lowMem[0x06CF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Room", &_lowMem[0x04DE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Jumping State", &_lowMem[0x0616], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Fight Mode", &_lowMem[0x06EE], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Guard Pos X", &_lowMem[0x061D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard HP", &_lowMem[0x06D0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Notice", &_lowMem[0x0601], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Frame", &_lowMem[0x0621], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Movement", &_lowMem[0x0625], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Present", &_lowMem[0x06E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Disappear Mode", &_lowMem[0x04F8], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Drawn Room", &_lowMem[0x0051], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Transition", &_lowMem[0x04AC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Transition 2", &_lowMem[0x01E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Drawn", &_lowMem[0x0732], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Is Paused", &_lowMem[0x06D1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Buffered Command", &_lowMem[0x04C9], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Door Opening Timer", &_lowMem[0x04CD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Current Door State", &_lowMem[0x073A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Exit Door State", &_lowMem[0x0400], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Level-specific tiles / gates
    registerGameProperty("Lvl1 First Tile BG", &_lowMem[0x06A4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Lvl1 First Tile FG", &_lowMem[0x06B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Lvl2 Last Tile FG", &_lowMem[0x0665], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 2 Exit Door State", &_lowMem[0x0708], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 3 Checkpoint Gate Timer", &_lowMem[0x05E9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Lvl3 Skeleton Loose Tile", &_lowMem[0x0502], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 4 Exit Door State", &_lowMem[0x06F7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 5 Gate Timer", &_lowMem[0x0538], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 7 Slow Fall Potion State", &_lowMem[0x0708], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 9 Room 15 Door State", &_lowMem[0x05E9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 10 Room 0 Door State", &_lowMem[0x04B8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 10 Room 4 Door State", &_lowMem[0x0541], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Caching pointers used by the game logic
    _globalTimer  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _currentLevel = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Level")]->getPointer();
    _rngState     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("RNG State")]->getPointer();
    _cpuHalted    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("CPU Halted")]->getPointer();
#ifdef _POP_BAD_ACCESS
    _cpuBadAccess = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("CPU Bad Access")]->getPointer();
#endif
    _framePhase         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Phase")]->getPointer();
    _bottomTextTimer    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bottom Text Timer")]->getPointer();
    _gameState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game State")]->getPointer();
    _passwordTimer      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Password Timer")]->getPointer();
    _playerPosX         = (int16_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _playerDirection    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Direction")]->getPointer();
    _playerFrame        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Frame")]->getPointer();
    _playerMovement     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Movement")]->getPointer();
    _playerFallWait     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Fall Wait")]->getPointer();
    _playerHP           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player HP")]->getPointer();
    _playerRoom         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Room")]->getPointer();
    _playerJumpingState = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Jumping State")]->getPointer();
    _playerFightMode    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Fight Mode")]->getPointer();
    _guardPosX          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Pos X")]->getPointer();
    _guardHP            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard HP")]->getPointer();
    _guardNotice        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Notice")]->getPointer();
    _guardFrame         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Frame")]->getPointer();
    _guardMovement      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Movement")]->getPointer();
    _guardPresent       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Present")]->getPointer();
    _guardDisappearMode = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Disappear Mode")]->getPointer();
    _drawnRoom          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Drawn Room")]->getPointer();
    _screenTransition   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Transition")]->getPointer();
    _screenTransition2  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Transition 2")]->getPointer();
    _screenDrawn        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Drawn")]->getPointer();
    _isPaused           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Is Paused")]->getPointer();
    _bufferedCommand    = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Buffered Command")]->getPointer();
    _doorOpeningTimer   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Door Opening Timer")]->getPointer();
    _currentDoorState   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Door State")]->getPointer();
    _exitDoorState      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Exit Door State")]->getPointer();

    _lvl1FirstTileBG       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Lvl1 First Tile BG")]->getPointer();
    _lvl1FirstTileFG       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Lvl1 First Tile FG")]->getPointer();
    _lvl2LastTileFG        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Lvl2 Last Tile FG")]->getPointer();
    _lvl2ExitDoorState     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 2 Exit Door State")]->getPointer();
    _lvl3GateTimer         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 3 Checkpoint Gate Timer")]->getPointer();
    _lvl3SkeletonLooseTile = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Lvl3 Skeleton Loose Tile")]->getPointer();
    _lvl4ExitDoorState     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 4 Exit Door State")]->getPointer();
    _lvl5GateTimer         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 5 Gate Timer")]->getPointer();
    _lvl7SlowfallState     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 7 Slow Fall Potion State")]->getPointer();
    _lvl9Room15DoorState   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 9 Room 15 Door State")]->getPointer();
    _lvl10Room0DoorState   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 10 Room 0 Door State")]->getPointer();
    _lvl10Room4DoorState   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level 10 Room 4 Door State")]->getPointer();

    // Derived / artificial properties (usable in rule conditions)
    registerGameProperty("Player Pos Y Actual", &_playerPosYActual, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Frame Diff", &_playerFrameDiff, Property::datatype_t::dt_int8, Property::endianness_t::little);
    // Aliases matching the legacy rule vocabulary
    registerGameProperty("Player Position X", &_lowMem[0x060F], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Player Position Y", &_playerPosYActual, Property::datatype_t::dt_float32, Property::endianness_t::little);

    // Registering the full input alphabet and building the per-frame move table
    registerInputs();

    // Priming derived values
    _playerPrevFrame = *_playerFrame;
    stateUpdatePostHook();
  }

  // Registers every controller input the game can issue and maps each Player animation frame to the set
  // of inputs that are meaningful in it (a direct port of the legacy getPossibleMoves table).
  __INLINE__ void registerInputs()
  {
    _in_null = _emulator->registerInput("|..|........|");
    _in_U    = _emulator->registerInput("|..|U.......|");
    _in_D    = _emulator->registerInput("|..|.D......|");
    _in_L    = _emulator->registerInput("|..|..L.....|");
    _in_R    = _emulator->registerInput("|..|...R....|");
    _in_S    = _emulator->registerInput("|..|....S...|");
    _in_A    = _emulator->registerInput("|..|.......A|");
    _in_B    = _emulator->registerInput("|..|......B.|");
    _in_LA   = _emulator->registerInput("|..|..L....A|");
    _in_RA   = _emulator->registerInput("|..|...R...A|");
    _in_DB   = _emulator->registerInput("|..|.D....B.|");
    _in_LB   = _emulator->registerInput("|..|..L...B.|");
    _in_RB   = _emulator->registerInput("|..|...R..B.|");
    _in_UB   = _emulator->registerInput("|..|U.....B.|");
    _in_UD   = _emulator->registerInput("|..|UD......|");
    _in_DL   = _emulator->registerInput("|..|.DL.....|");
    _in_DR   = _emulator->registerInput("|..|.D.R....|");
    _in_BA   = _emulator->registerInput("|..|......BA|");
    _in_UBA  = _emulator->registerInput("|..|U.....BA|");
    _in_UDB  = _emulator->registerInput("|..|UD....B.|");
    _in_UDBA = _emulator->registerInput("|..|UD....BA|");

#ifdef _POP_GLITCH_HUNT
    // Glitch-hunt probe alphabet: physically-impossible pad combinations (opposed D-pad directions,
    // U+X families) that the action-pointer dispatch can index past the end of its table with. These
    // are offered in EVERY actionable state (see getAdditionalAllowedInputs) so the search can look
    // for the multi-frame setup that arms a bad access anywhere it can act. Combos that are inert in
    // a given state produce the same machine state as null and are pruned by dedup, so this broad
    // alphabet is self-limiting -- it only branches where an impossible combo actually changes state.
    _in_UL        = _emulator->registerInput("|..|U.L.....|");
    _in_UR        = _emulator->registerInput("|..|U..R....|");
    _in_ULR       = _emulator->registerInput("|..|U.LR....|");
    _in_LR        = _emulator->registerInput("|..|..LR....|");
    _in_UDL       = _emulator->registerInput("|..|UDL.....|");
    _in_UDR       = _emulator->registerInput("|..|UD.R....|");
    _in_UDLR      = _emulator->registerInput("|..|UDLR....|");
    _in_UDA       = _emulator->registerInput("|..|UD.....A|");
    _in_ULA       = _emulator->registerInput("|..|U.L....A|");
    _in_URA       = _emulator->registerInput("|..|U..R...A|");
    _glitchProbes = {_in_UD, _in_UDB, _in_UDBA, _in_UBA, _in_UL, _in_UR, _in_ULR, _in_LR, _in_UDL, _in_UDR, _in_UDLR, _in_UDA, _in_ULA, _in_URA};
#endif

    using V  = std::vector<InputSet::inputIndex_t>;
    auto set = [&](uint8_t frame, V moves) { _frameMoves[frame] = std::move(moves); };

    set(0x00, {_in_A}); // post-exit-walk pose: A confirms the level end (increments Current Level = the win)
    for (uint8_t f = 0x01; f <= 0x04; f++) set(f, {_in_L, _in_R});
    set(0x05, {_in_A, _in_L, _in_R});
    for (uint8_t f = 0x06; f <= 0x0D; f++) set(f, {_in_L, _in_R, _in_LA, _in_RA});
    set(0x0E, {_in_L, _in_R});
    set(0x0F, {_in_A, _in_B, _in_D, _in_L, _in_R, _in_U, _in_DB, _in_LB, _in_RB, _in_UB, _in_UD, _in_UBA, _in_UDB, _in_UDBA});
    set(0x14, {_in_A, _in_L, _in_R});
    set(0x17, {_in_A, _in_R, _in_L});
    set(0x18, {_in_A, _in_R, _in_L});
    set(0x1A, {_in_A, _in_L, _in_R});
    set(0x23, {_in_A, _in_L, _in_R});
    for (uint8_t f = 0x25; f <= 0x2C; f++) set(f, {_in_A, _in_L, _in_R});
    for (uint8_t f = 0x35; f <= 0x37; f++) set(f, {_in_A, _in_L, _in_R});
    set(0x39, {_in_A, _in_L, _in_R});
    set(0x3A, {_in_A, _in_R, _in_L});
    set(0x4E, {_in_A, _in_U});
    set(0x4F, {_in_A, _in_U});
    set(0x50, {_in_A, _in_U});
    set(0x53, {_in_L, _in_R});
    set(0x54, {_in_L, _in_R});
    set(0x55, {_in_L, _in_R});
    set(0x57, {_in_A, _in_U});
    set(0x58, {_in_A, _in_U});
    set(0x59, {_in_A, _in_U});
    set(0x5A, {_in_A, _in_L, _in_R, _in_U});
    set(0x5B, {_in_A, _in_L, _in_R, _in_U});
    set(0x5C, {_in_A, _in_U});
    set(0x5D, {_in_A, _in_L, _in_R, _in_U});
    for (uint8_t f = 0x5E; f <= 0x63; f++) set(f, {_in_A, _in_U});
    for (uint8_t f = 0x66; f <= 0x69; f++) set(f, {_in_A});
    set(0x6A, {_in_A, _in_B, _in_L, _in_R});
    set(0x6B, {_in_L, _in_R});
    set(0x6C, {_in_A, _in_L, _in_R});
    set(0x6D, {_in_D, _in_L, _in_R, _in_DL, _in_DR});
    for (uint8_t f = 0x6E; f <= 0x77; f++) set(f, {_in_L, _in_R});
    set(0x7D, {_in_A, _in_L, _in_R});
    for (uint8_t f = 0x96; f <= 0x9E; f++) set(f, {_in_A, _in_L, _in_R});
    set(0xA0, {_in_A, _in_L, _in_R});
    set(0xA4, {_in_A});
    set(0xA5, {_in_A});
    set(0xAA, {_in_A, _in_L, _in_R, _in_BA});
    set(0xAB, {_in_A, _in_B, _in_L, _in_R});
    set(0xAC, {_in_A, _in_L, _in_R});
    set(0xAD, {_in_A, _in_L, _in_R});
    set(0xAE, {_in_A, _in_L, _in_R});
    set(0xD8, {_in_A, _in_L, _in_R});
  }

  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    std::set<std::string> inputs = {"|..|........|", "|..|U.......|", "|..|.D......|", "|..|..L.....|", "|..|...R....|", "|..|....S...|", "|..|.......A|",
                                    "|..|......B.|", "|..|..L....A|", "|..|...R...A|", "|..|.D....B.|", "|..|..L...B.|", "|..|...R..B.|", "|..|U.....B.|",
                                    "|..|UD......|", "|..|.DL.....|", "|..|.D.R....|", "|..|......BA|", "|..|U.....BA|", "|..|UD....B.|", "|..|UD....BA|"};
#ifdef _POP_GLITCH_HUNT
    // pre-register the glitch-hunt probe alphabet (must include every input the search can emit)
    inputs.insert(
        {"|..|U.L.....|", "|..|U..R....|", "|..|U.LR....|", "|..|..LR....|", "|..|UDL.....|", "|..|UD.R....|", "|..|UDLR....|", "|..|UD.....A|", "|..|U.L....A|", "|..|U..R...A|"});
#endif
    return inputs;
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    // The null (wait) input is always available
    allowedInputSet.push_back(_in_null);

#ifdef _POP_GLITCH_HUNT
    // Phase pruning (USER directive, empirically bounded): the game only EXECUTES an input at certain
    // frame phases; at the rest the input is a no-op, so offer ONLY null there. The user's model is a
    // single active phase (2 normally / 3 in the display view), but measured against the legit level
    // the meaningful inputs actually span current-state phases {0,1,2} -- a strict single-phase rule
    // loses real inputs and stalls the search. Phases {3,4} are NEVER used by any meaningful input
    // (validated: nulling every off-{0,1,2} input replays the level identically). So we prune non-null
    // branching at phases {3,4} -- lossless, and it removes ~40% of the frame-by-frame branching. The
    // ceiling is configurable via _maxInputPhase.
    if (*_framePhase > _maxInputPhase) return; // phases 3,4: only null is meaningful
#endif

    // Pausing, when enabled, is a valid action in any state
    if (_enablePause) allowedInputSet.push_back(_in_S);

    // Frame-specific moves
    const auto it = _frameMoves.find(*_playerFrame);
    if (it != _frameMoves.end()) allowedInputSet.insert(allowedInputSet.end(), it->second.begin(), it->second.end());

#ifdef _POP_GLITCH_HUNT
    // Glitch hunt: the U+X action-pointer glitch ONLY arms from the standing-idle pose (USER: no other
    // player frame activates it). Player Frame 0x0F is that standing frame (confirmed: the teleport
    // fires at frame 0x0F). So offer the impossible-combo probes ONLY when standing -- a large branching
    // cut vs. offering them in every actionable frame.
    if (*_playerFrame == 0x0F) allowedInputSet.insert(allowedInputSet.end(), _glitchProbes.begin(), _glitchProbes.end());
#endif
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Remembering the frame before advancing so the derived frame-diff can be computed
    _playerPrevFrame = *_playerFrame;

#ifdef _POP_BAD_ACCESS
    // DELAYED reward-improve-after-glitch detection (one-frame lag; see member docs). The glitch frame
    // itself is a TRANSIENT -- a crash/garbage posX (e.g. 0x6012 / 65519). The game must run a few
    // frames to SETTLE. So we do NOT win on the glitch; instead a glitch opens a window [K, K+W] frames
    // later, and we flag a state's CHILDREN if, inside that window and once the screen is stable (no
    // active transition), the reward has risen above the recorded pre-glitch reward. A glitch that
    // crashes/dies or lands somewhere no better never reaches an improved settled state, so it never
    // wins. At this point getReward() holds the reward of the state we are expanding.
    const float _rewardBeforeAdvance = getReward();
    _glitchAdvanced =
        (_framesSinceGlitch != 255 && _framesSinceGlitch >= _glitchSettleFrames && *_screenTransition == 0 && _rewardBeforeAdvance > _preGlitchReward + _glitchRewardThreshold) ? 1
                                                                                                                                                                                : 0;
    const int _posXBefore = (int)*_playerPosX;
#endif

    _emulator->advanceState(input);

#ifdef _POP_BAD_ACCESS
    // "Glitch this frame" = a detected derail (unofficial opcode / RAM exec) OR a POSITION TELEPORT:
    // an in-room posX jump with NO active screen transition. The teleport arm is essential because the
    // real U+X teleport reaches the entity-reposition routine (legit code) fed garbage via DOCUMENTED
    // opcodes, so the bad-access flag never fires (bad=0) -- yet posX jumps to a wild value (e.g.
    // 0x6012). The transition gate excludes normal room-edge wraps (which are big posX jumps too but
    // occur during a transition). Validated: 0 false-positives across the legit level.
    const int  _posXAfter = (int)*_playerPosX;
    const int  _posXDelta = _posXAfter > _posXBefore ? _posXAfter - _posXBefore : _posXBefore - _posXAfter;
    const bool _teleport  = (_posXDelta > 40) && (*_screenTransition == 0);
    _prevBadAccess        = *_cpuBadAccess;                             // pure derail (for the CPU Bad Access rules)
    _prevGlitch           = (*_cpuBadAccess != 0 || _teleport) ? 1 : 0; // derail OR teleport (opens the settle window)
    _teleported           = _teleport ? 1 : 0;                          // deterministic teleport flag (diagnostic)

    // Advance the post-glitch settle window for the child state. Open it ONLY on a FRESH glitch (no
    // window already open): the settling of a teleport is itself a series of big posX jumps that each
    // re-trigger the glitch flag, and resetting the window on those would overwrite the true pre-glitch
    // reward with the high TRANSIENT reward, hiding the real improvement. Once open, the counter just
    // ages until the window closes (255 = no open window).
    if (_prevGlitch == 1 && _framesSinceGlitch == 255)
    {
      _preGlitchReward   = _rewardBeforeAdvance;
      _framesSinceGlitch = 0;
    }
    else if (_framesSinceGlitch != 255)
    {
      _framesSinceGlitch++;
      if (_framesSinceGlitch > _glitchSettleFrames + _glitchSettleWindow) _framesSinceGlitch = 255;
    }
#endif

    // Count frames elapsed inside the current room transition (reset outside one). Used to reward
    // transition progress so the reward-guided DB keeps later-transition states instead of evicting them.
    _transitionFrames = (*_screenTransition != 0) ? (uint16_t)(_transitionFrames + 1) : 0;

    // Count consecutive lag frames (engine-busy frames, Is Paused == 5). During a lag burst the entire
    // game RAM is bit-identical frame to frame (the progress lives in PPU timing, outside LRAM), so
    // without this counter consecutive lag frames hash identically and dedup would prune the wait.
    _lagFrames = (*_isPaused == 5) ? (uint8_t)(_lagFrames + 1) : 0;

    _currentStep++;
    stateUpdatePostHook();
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    if (_timerTolerance > 0) hashEngine.Update((uint8_t)(*_globalTimer % (_timerTolerance + 1)));

    hashEngine.Update(*_currentLevel);
    hashEngine.Update(*_framePhase);
    hashEngine.Update(*_doorOpeningTimer);
    hashEngine.Update(*_currentDoorState);
    hashEngine.Update(*_exitDoorState);
    hashEngine.Update(*_bottomTextTimer);
    hashEngine.Update(*_bufferedCommand);
    hashEngine.Update(*_gameState);

    hashEngine.Update(*_playerPosX);
    hashEngine.Update(*_playerPosY);
    hashEngine.Update(*_playerFrame);
    hashEngine.Update(*_playerDirection);
    hashEngine.Update(*_playerMovement);
    hashEngine.Update(*_playerFallWait);
    hashEngine.Update(*_playerHP);
    hashEngine.Update(*_playerRoom);
    hashEngine.Update(*_playerFightMode);
    hashEngine.Update(*_playerJumpingState);

    hashEngine.Update(*_guardPosX);
    hashEngine.Update(*_guardHP);
    hashEngine.Update(*_guardNotice);
    hashEngine.Update(*_guardFrame);
    hashEngine.Update(*_guardMovement);
    hashEngine.Update(*_guardPresent);
    hashEngine.Update(*_guardDisappearMode);

    hashEngine.Update(*_drawnRoom);
    hashEngine.Update(*_screenTransition);
    hashEngine.Update(*_screenTransition2);
    hashEngine.Update(*_screenDrawn);
    hashEngine.Update(*_isPaused);
    hashEngine.Update(*_lvl4ExitDoorState);

    // Screen-transition scratch area (part of normal per-frame state; always hashed).
    hashEngine.Update(&_lowMem[0x01E0], 0x20);

    // Room-transition scroll progress. A screen transition scrolls over a VARIABLE number of frames,
    // tracked by counters the game repurposes for it: 0x0791/0x0792 form a 16-bit scroll position that
    // advances every frame, 0x07F2/0x06F8 are transition timers. Hashing them keeps each mid-scroll
    // frame DISTINCT so the BFS can traverse the whole transition instead of dedup-collapsing it to a
    // single state and never completing it. Hashed ONLY while transitioning: outside a transition
    // 0x0791/0x0792 hold the ever-ticking Global Timer, which would flood the state DB with per-frame
    // dupes during normal play.
    if (*_screenTransition != 0)
    {
      // Fine per-frame scroll/screen-draw progress: 0x078B-0x0790 decrement once per transition frame
      // (0x078E is strictly one-value-per-frame), and 0x0791/0x0792 are the ever-advancing global timer.
      // The coarser transition counters alone (0x07F2 is CYCLIC ~11, 0x06F8 is slow) collide across
      // differently-timed entries into the same scroll -- first-seen-wins then DROPS the state that would
      // complete the transition. These fine counters break that tie so the whole transition can advance.
      hashEngine.Update(&_lowMem[0x078B], 8); // 0x078B .. 0x0792 (draw counters + global timer)
      hashEngine.Update(_lowMem[0x07F2]);
      hashEngine.Update(_lowMem[0x06F8]);
      // The RNG churns during some transitions (e.g. 5->7 it seeds the destination room's guard). If it
      // isn't distinguished, differently-seeded crossings collide and get dropped -- draining the frontier
      // and merging paths whose kept state and stored history disagree. Hashed transition-only so it does
      // not explode normal play. (Combat-RNG in-room is handled separately by the guard fields above.)
      hashEngine.Update(*_rngState);
    }

    // Lag frames (Is Paused == 5) repeat the entire game RAM bit-identically while the engine catches up,
    // so consecutive lag frames are indistinguishable to the hash. The software lag-frame counter breaks
    // the tie so a solution can wait through a lag burst without dedup pruning it.
    hashEngine.Update(_lagFrames);

    // A CPU-halted state must never hash-merge with a healthy state that happens to share its RAM.
    hashEngine.Update(*_cpuHalted);

#ifdef _POP_BAD_ACCESS
    // A GLITCH frame (derail into data-as-code, OR a position teleport) must not hash-merge with a
    // normal frame that happens to share its RAM (many glitches self-heal to normal-looking RAM).
    // Without this, first-seen-wins dedup keeps the NORMAL path for that hash slot, and the saved
    // "win" solution reaches the state via ordinary play -- so it replays with no glitch. Hashing
    // all three glitch flags keeps the glitch states (and their input histories, incl. the arming/
    // teleport input and the reward-jump win flag) distinct, so the winning solution reproduces.
    hashEngine.Update(*_cpuBadAccess);
    hashEngine.Update(_prevGlitch);
    hashEngine.Update(_glitchAdvanced);
    hashEngine.Update(_teleported);
#endif

    if (*_gameState == 8) hashEngine.Update(*_passwordTimer);
    if (*_gameState == 8 && *_passwordTimer == 0) hashEngine.Update(*_globalTimer);
    if (*_gameState == 1 && *_playerFrame == 0 && *_playerMovement == 91) hashEngine.Update(*_globalTimer);

    // Level-specific tiles / gates
    if (*_currentLevel == 0)
    {
      hashEngine.Update(*_lvl1FirstTileBG);
      hashEngine.Update(*_lvl1FirstTileFG);
    }
    if (*_currentLevel == 1)
    {
      hashEngine.Update(*_lvl2LastTileFG);
      hashEngine.Update(*_lvl2ExitDoorState);
    }
    if (*_currentLevel == 2)
    {
      hashEngine.Update(*_lvl3GateTimer);
      hashEngine.Update(*_exitDoorState);
      hashEngine.Update(*_lvl3SkeletonLooseTile);
    }
    if (*_currentLevel == 4) hashEngine.Update(*_lvl5GateTimer);
    if (*_currentLevel == 6) hashEngine.Update(*_lvl7SlowfallState);
    if (*_currentLevel == 7) hashEngine.Update(*_lvl9Room15DoorState);
    if (*_currentLevel == 9)
    {
      hashEngine.Update(*_lvl10Room0DoorState);
      hashEngine.Update(*_lvl10Room4DoorState);
    }
  }

  // Recomputes the interpolated vertical position (the legacy playerPosYActual), which the game uses to
  // rank climbing/hanging/jumping frames more finely than the raw byte allows.
  __INLINE__ void stateUpdatePostHook() override
  {
    _playerFrameDiff  = (int8_t)(*_playerFrame - _playerPrevFrame);
    _playerPosYActual = (float)*_playerPosY;
    const auto f      = *_playerFrame;

    // The game snaps Player Pos Y discretely during a climb (still on the lower platform, then suddenly
    // up on the higher one), which makes a raw-Y magnet flat-then-jump with no gradient across the climb.
    // We interpolate: every frame of a climb-UP action is assigned a monotonically DECREASING (=higher)
    // Y, and every climb-DOWN frame a monotonically increasing Y, so the reward rises smoothly frame by
    // frame. The offsets below are tuned (against the reference climb) to bridge the raw-Y jumps WITHOUT
    // overshooting past the destination platform, and the completion frames continue the descent monotonically.

    // Climb-UP, lower pull-up (0x43-0x4F): raw Y already steps ~1/frame; nudge it down smoothly.
    if (f >= 0x43 && f <= 0x4F) _playerPosYActual -= 16.0f - (float)(0x4F - f);

    // Climb-UP, hang / re-grip (0x50, 0x57-0x5B): bridge the raw-Y jump at the grip point.
    if (f == 0x50) _playerPosYActual -= 20.0f;
    if (f >= 0x57 && f <= 0x5B) _playerPosYActual -= 25.0f - (float)(0x5B - f);

    // Climb-UP, upper handhold pull (0x87-0x8C): constant offset so interp lands JUST above the platform
    // (~73) instead of overshooting below it (the old 39-(0x8D-f) drove it to ~52, causing a reward dip).
    if (f >= 0x87 && f <= 0x8C) _playerPosYActual -= 17.0f;

    // Climb-UP completion (0x8D-0x93, frames INCREASING): the player is settling onto the platform; leave
    // interp = raw (~platform Y 65) so it continues the monotonic descent. (Frames DECREASING here are a
    // climb-DOWN -- handled below -- which is how up vs down is disambiguated on this shared frame range.)

    // Climb-DOWN (0x8D-0x94, frames DECREASING): interp rises smoothly as the player lowers itself.
    if (f >= 0x8D && f <= 0x94 && _playerFrameDiff < 0) _playerPosYActual += (float)(0x94 - f);

    // Running jump (0x27-0x2C): the leap arc transiently RAISES the player (raw Y dips ~14) which a
    // downward vertical magnet punishes -- the mid-air states sink below their non-jumping peers and get
    // evicted exactly where the route REQUIRES the jump (e.g. room 7's gap). Report the last grounded Y
    // for the whole arc so a forward jump is pure horizontal progress, never a reward valley.
    if (f >= 0x27 && f <= 0x2C)
      _playerPosYActual = (float)_lastGroundedPosY;
    else
      _lastGroundedPosY = *_playerPosY;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting all magnets ahead of rule re-evaluation
    _playerHorizontalMagnet.fill(genericMagnet_t{});
    _playerVerticalMagnet.fill(genericMagnet_t{});
    _playerDirectionMagnet = 0.0f;
    _nextRoomTarget        = 255; // no transition target until a satisfied rule sets one
    _nextRoomReward        = 0.0f;
    _nextRoomBase          = 0.0f;
  }

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_playerPrevFrame, sizeof(_playerPrevFrame));
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_transitionFrames, sizeof(_transitionFrames));
    serializer.pushContiguous(&_lagFrames, sizeof(_lagFrames));
    serializer.pushContiguous(&_lastGroundedPosY, sizeof(_lastGroundedPosY));
#ifdef _POP_BAD_ACCESS
    serializer.pushContiguous(&_preGlitchReward, sizeof(_preGlitchReward));
    serializer.pushContiguous(&_framesSinceGlitch, sizeof(_framesSinceGlitch));
    serializer.pushContiguous(&_prevBadAccess, sizeof(_prevBadAccess));
    serializer.pushContiguous(&_prevGlitch, sizeof(_prevGlitch));
    serializer.pushContiguous(&_glitchAdvanced, sizeof(_glitchAdvanced));
    serializer.pushContiguous(&_teleported, sizeof(_teleported));
#endif
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializer.popContiguous(&_playerPrevFrame, sizeof(_playerPrevFrame));
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_transitionFrames, sizeof(_transitionFrames));
    deserializer.popContiguous(&_lagFrames, sizeof(_lagFrames));
    deserializer.popContiguous(&_lastGroundedPosY, sizeof(_lastGroundedPosY));
#ifdef _POP_BAD_ACCESS
    deserializer.popContiguous(&_preGlitchReward, sizeof(_preGlitchReward));
    deserializer.popContiguous(&_framesSinceGlitch, sizeof(_framesSinceGlitch));
    deserializer.popContiguous(&_prevBadAccess, sizeof(_prevBadAccess));
    deserializer.popContiguous(&_prevGlitch, sizeof(_prevGlitch));
    deserializer.popContiguous(&_glitchAdvanced, sizeof(_glitchAdvanced));
    deserializer.popContiguous(&_teleported, sizeof(_teleported));
#endif
  }

  __INLINE__ float calculateGameSpecificReward() const override
  {
    float reward = 0.0;

    // Room-transition shaping. A room-progression rule declares its "next room" target (via the
    // "Set Next Room Reward" action). While the transition toward that target is in progress -- the
    // drawn/next room (0x0051) already equals the target but the player room (0x04DE) has not yet caught
    // up -- grant a SMALL reward that grows with transition frames, so the reward-guided DB is drawn to
    // trigger and push the correct transition to completion instead of parking on a positional magnet.
    // The term is active for the ENTIRE scroll (Screen Transition != 0), gated to the desired direction
    // (either room pointer equals the target; a backtrack transition matches neither). It makes the whole
    // exit->scroll->entry sequence monotonically increasing:
    //   - The base term compensates for the room magnet switching OFF the instant a transition starts
    //     (magnets require room==drawn): base > max magnet (~613) keeps the scroll entry strictly above
    //     the pre-transition reward.
    //   - The per-frame term then favours later-scroll states through to completion.
    //   - The room rules' full Add-Reward (+10000) is gated on Screen Transition == 0, so it fires at
    //     scroll COMPLETION, on top of the restored magnet -- always above the scroll peak (~3700), never
    //     mid-scroll (firing at the Player-Room flip used to make the vanishing scroll term a ~1150 dip).
    if (_nextRoomTarget != 255 && *_screenTransition != 0 && (*_drawnRoom == _nextRoomTarget || *_playerRoom == _nextRoomTarget))
      reward += _nextRoomBase + _nextRoomReward * (float)_transitionFrames;

    // Magnets only take effect when the player's room matches the drawn room (i.e. no active transition)
    if (*_playerRoom == *_drawnRoom)
    {
      const auto& hMagnet = _playerHorizontalMagnet[*_drawnRoom];
      const auto& vMagnet = _playerVerticalMagnet[*_drawnRoom];

      // Horizontal magnet on Player Pos X
      {
        float boundedValue = (float)*_playerPosX;
        boundedValue       = std::min(boundedValue, hMagnet.max);
        boundedValue       = std::max(boundedValue, hMagnet.min);
        float diff         = -358.0f + std::abs(hMagnet.center - boundedValue);
        reward += hMagnet.intensity * -diff;
      }

      // Vertical magnet on the interpolated Player Pos Y
      {
        float boundedValue = _playerPosYActual;
        boundedValue       = std::min(boundedValue, vMagnet.max);
        boundedValue       = std::max(boundedValue, vMagnet.min);
        float diff         = -255.0f + std::abs(vMagnet.center - boundedValue);
        reward += vMagnet.intensity * -diff;
      }
    }

    // Ledge-grab / climb bias for rooms with a pickup-over-a-pit (e.g. room 14's sword). The search
    // otherwise falls straight into the pit, grabs the item at the bottom, and gets stuck in the draw
    // animation (a dead-end). Rewarding the grab/climb-up animation frames biases it to catch the ledge
    // and climb out instead of falling. Gated to the configured rooms so it never perturbs normal play.
    if (_climbBonusRooms.count(*_playerRoom) != 0)
    {
      const auto f = *_playerFrame;
      if ((f >= 0x43 && f <= 0x5B) || (f >= 0x87 && f <= 0x93)) reward += _climbBonus;
    }

    // Rewarding reaching the end-of-game state
    if (*_currentLevel == 12 && *_isPaused == 2) reward += (float)*_currentLevel * 5000000.0f;

    // Player direction magnet
    reward += (*_playerDirection == 0 ? -1.0f : 1.0f) * _playerDirectionMagnet;

    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]   + Current Level:            %02u\n", *_currentLevel);
    jaffarCommon::logger::log("[J+]   + Global Timer:             %02u\n", *_globalTimer);
    jaffarCommon::logger::log("[J+]   + RNG State:                0x%02X\n", *_rngState);
    jaffarCommon::logger::log("[J+]   + Frame Phase:              %02u\n", *_framePhase);
    jaffarCommon::logger::log("[J+]   + Is Paused:                %02u\n", *_isPaused);
    jaffarCommon::logger::log("[J+]   + Screen Trans / Drawn:     %02u / %02u\n", *_screenTransition, *_screenDrawn);
    jaffarCommon::logger::log("[J+]   + Game State:               %02u\n", *_gameState);
    jaffarCommon::logger::log("[J+]   + Current Door / Timer:     %02u / %02u\n", *_currentDoorState, *_doorOpeningTimer);
    jaffarCommon::logger::log("[J+]   + Drawn Room / Exit Door:   %02u / %02u\n", *_drawnRoom, *_exitDoorState);
    jaffarCommon::logger::log("[J+]   + Player Room:                 %02u\n", *_playerRoom);
    jaffarCommon::logger::log("[J+]   + Player Frame:                0x%02X (Prev: 0x%02X, Diff: %d)\n", *_playerFrame, _playerPrevFrame, _playerFrameDiff);
    jaffarCommon::logger::log("[J+]   + Player Direction:            %s\n", *_playerDirection == 0 ? "Left" : "Right");
    jaffarCommon::logger::log("[J+]   + Player Pos X:                %04d\n", *_playerPosX);
    jaffarCommon::logger::log("[J+]   + Player Pos Y:                %.3f (Base: %02u)\n", _playerPosYActual, *_playerPosY);
    jaffarCommon::logger::log("[J+]   + Player Movement:             %02u\n", *_playerMovement);
    jaffarCommon::logger::log("[J+]   + Player HP:                   %02u\n", *_playerHP);
    jaffarCommon::logger::log("[J+]   + Player Fight Mode:           %02u\n", *_playerFightMode);
    jaffarCommon::logger::log("[J+]   + Guard Pos X / HP:         %02u / %02u\n", *_guardPosX, *_guardHP);
    jaffarCommon::logger::log("[J+]   + Guard Present / Mode:     %02u / %02u\n", *_guardPresent, *_guardDisappearMode);

    if (*_playerRoom == *_drawnRoom)
    {
      const auto& h = _playerHorizontalMagnet[*_drawnRoom];
      const auto& v = _playerVerticalMagnet[*_drawnRoom];
      if (std::abs(h.intensity) > 0.0f)
        jaffarCommon::logger::log("[J+]  + Player Horizontal Magnet   - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", h.intensity, h.center, h.min, h.max);
      if (std::abs(v.intensity) > 0.0f)
        jaffarCommon::logger::log("[J+]  + Player Vertical Magnet     - Intensity: %.1f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", v.intensity, v.center, v.min, v.max);
    }
    if (std::abs(_playerDirectionMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Player Direction Magnet    - Intensity: %.1f\n", _playerDirectionMagnet);
  }

  // Per-step diagnostic line (dumped via jaffar-player --dumpTrace); used to spot movie desyncs during
  // re-synchronization: level/room progression, HP (a drop to 0 = death = desync), position and frame.
  __INLINE__ std::string getTraceLine() const override
  {
    char buf[280];
#ifdef _POP_BAD_ACCESS
    const unsigned badAccess = *_cpuBadAccess;
    const unsigned gAdv      = _glitchAdvanced;
    const unsigned pGlitch   = _prevGlitch;
    const float    rew       = getReward();
#else
    const unsigned badAccess = 0;
    const unsigned gAdv = 0, pGlitch = 0;
    const float    rew = 0.0f;
#endif
    snprintf(buf, sizeof(buf),
             "lvl=%u room=%u drawn=%u hp=%u fight=%u posX=%d posY=%.1f frame=0x%02X trans=%u paused=%u phase=%u rng=0x%02X gpres=%u gpx=%u ghp=%u gnot=%u halt=%u bad=%u rew=%.0f "
             "pGl=%u gAdv=%u",
             *_currentLevel, *_playerRoom, *_drawnRoom, *_playerHP, *_playerFightMode, (int)*_playerPosX, _playerPosYActual, *_playerFrame, *_screenTransition, *_isPaused,
             *_framePhase, *_rngState, *_guardPresent, *_guardPosX, *_guardHP, *_guardNotice, *_cpuHalted, badAccess, rew, pGlitch, gAdv);
    return std::string(buf);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Player Horizontal Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
      auto min       = jaffarCommon::json::getNumber<float>(actionJs, "Min");
      auto max       = jaffarCommon::json::getNumber<float>(actionJs, "Max");
      rule.addAction([=, this]() { this->_playerHorizontalMagnet[room] = genericMagnet_t{.intensity = intensity, .center = center, .min = min, .max = max}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Vertical Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
      auto min       = jaffarCommon::json::getNumber<float>(actionJs, "Min");
      auto max       = jaffarCommon::json::getNumber<float>(actionJs, "Max");
      rule.addAction([=, this]() { this->_playerVerticalMagnet[room] = genericMagnet_t{.intensity = intensity, .center = center, .min = min, .max = max}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Direction Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_playerDirectionMagnet = intensity; });
      recognizedActionType = true;
    }

    // Declares the "next room" for the current room-progression step and the per-transition-frame reward
    // for progressing toward it (see calculateGameSpecificReward). Set by the rule for the CURRENT room
    // so the reward fires while transitioning into the NEXT one.
    if (actionType == "Set Next Room Reward")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      // Base compensation for the magnet lost at the transition entry (default 700 > max magnet ~613).
      float base = actionJs.contains("Base") ? jaffarCommon::json::getNumber<float>(actionJs, "Base") : 700.0f;
      rule.addAction(
          [=, this]()
          {
            this->_nextRoomTarget = room;
            this->_nextRoomReward = intensity;
            this->_nextRoomBase   = base;
          });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // Discriminate candidate states by the player's current animation frame (its available moves)
    return jaffarCommon::hash::hash_t({0, *_playerFrame});
  }

  // Datatype describing a bounded position magnet (attraction toward a center within [min,max])
  struct genericMagnet_t
  {
    float intensity = 0.0;
    float center    = 0.0;
    float min       = 0.0;
    float max       = 0.0;
  };

  // Per-room magnets (indexed by the drawn room)
  std::array<genericMagnet_t, POP_ROOM_COUNT> _playerHorizontalMagnet;
  std::array<genericMagnet_t, POP_ROOM_COUNT> _playerVerticalMagnet;
  float                                       _playerDirectionMagnet = 0.0f;

  // Configuration
  bool    _enablePause    = false;
  uint8_t _timerTolerance = 0;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;

  // Cached game-value pointers
  uint8_t* _globalTimer;
  uint8_t* _currentLevel;
  uint8_t* _rngState;
  uint8_t* _cpuHalted; // emulator CPU halt latch (KIL/JAM executed); see registerGameProperties
#ifdef _POP_BAD_ACCESS
  uint8_t* _cpuBadAccess; // per-frame bad-access flag (glitch investigation); see registerGameProperties
#endif
  uint8_t*  _framePhase;
  uint8_t*  _bottomTextTimer;
  uint8_t*  _gameState;
  uint8_t*  _passwordTimer;
  int16_t*  _playerPosX;
  uint8_t*  _playerPosY;
  uint8_t*  _playerDirection;
  uint8_t*  _playerFrame;
  uint8_t*  _playerMovement;
  uint8_t*  _playerFallWait;
  uint8_t*  _playerHP;
  uint8_t*  _playerRoom;
  uint8_t*  _playerJumpingState;
  uint8_t*  _playerFightMode;
  uint8_t*  _guardPosX;
  uint8_t*  _guardHP;
  uint8_t*  _guardNotice;
  uint8_t*  _guardFrame;
  uint8_t*  _guardMovement;
  uint8_t*  _guardPresent;
  uint8_t*  _guardDisappearMode;
  uint8_t*  _drawnRoom;
  uint8_t*  _screenTransition;
  uint8_t*  _screenTransition2;
  uint8_t*  _screenDrawn;
  uint8_t*  _isPaused;
  uint16_t* _bufferedCommand;
  uint8_t*  _doorOpeningTimer;
  uint8_t*  _currentDoorState;
  uint8_t*  _exitDoorState;

  uint8_t* _lvl1FirstTileBG;
  uint8_t* _lvl1FirstTileFG;
  uint8_t* _lvl2LastTileFG;
  uint8_t* _lvl2ExitDoorState;
  uint8_t* _lvl3GateTimer;
  uint8_t* _lvl3SkeletonLooseTile;
  uint8_t* _lvl4ExitDoorState;
  uint8_t* _lvl5GateTimer;
  uint8_t* _lvl7SlowfallState;
  uint8_t* _lvl9Room15DoorState;
  uint8_t* _lvl10Room0DoorState;
  uint8_t* _lvl10Room4DoorState;

  // Derived values
  float    _playerPosYActual = 0.0f;
  int8_t   _playerFrameDiff  = 0;
  uint8_t  _playerPrevFrame  = 0;
  uint32_t _currentStep      = 0;

  // Frames elapsed inside the current room transition (0 when not transitioning). Serialized; used to
  // both hash-distinguish mid-scroll frames and reward transition progress.
  uint16_t _transitionFrames = 0;

  // Consecutive lag frames so far (Is Paused == 5; 0 outside a lag burst). Serialized; hashed to
  // distinguish frames whose entire game RAM repeats bit-identically while the engine catches up.
  uint8_t _lagFrames = 0;

  // Player Pos Y on the most recent grounded (non-running-jump) frame. Serialized; used to keep the
  // vertical-magnet reward flat across a running jump's arc (see stateUpdatePostHook).
  uint8_t _lastGroundedPosY = 0;

#ifdef _POP_BAD_ACCESS
  // Reward-increasing-derail detector (glitch hunt). The reward already encodes "progress" (room
  // magnets + per-room bonuses), so a garbage execution that RAISES the reward is progress-advancing
  // by construction -- it teleported/skipped forward, not just derailed benignly. jaffar decides a
  // state's win-ness (evaluateRules) BEFORE computing its reward (updateReward), so this is detected
  // with a one-frame lag: each state carries its parent's reward and whether it was born from a
  // derail; when the state is expanded, we check (born-from-derail AND reward > parentReward+thresh)
  // and flag its children via "Glitch Advanced", which a win rule watches. All serialized.
  float   _preGlitchReward       = 0.0f;   // reward recorded just before the most recent glitch opened the window
  uint8_t _framesSinceGlitch     = 255;    // frames since the last glitch (255 = no open settle window)
  uint8_t _prevBadAccess         = 0;      // was this state produced by a bad-access (derailing) frame?
  uint8_t _prevGlitch            = 0;      // was this state produced by a GLITCH frame (derail OR position teleport)?
  uint8_t _glitchAdvanced        = 0;      // 1 iff a post-glitch settled state improved reward (delayed win signal)
  uint8_t _teleported            = 0;      // this frame was an in-room posX teleport (diagnostic)
  float   _glitchRewardThreshold = 100.0f; // min reward improvement over pre-glitch to count as progress
  uint8_t _glitchSettleFrames    = 8;      // K: frames to let a glitch settle before checking reward improvement
  uint8_t _glitchSettleWindow    = 40;     // W: how many further frames the post-glitch win window stays open
#endif
  // Current room-progression target: the "next room" (0x0051 value) the search should transition into,
  // and the per-transition-frame reward for progressing toward it. Set each rule-eval by "Set Next Room
  // Reward"; not serialized (re-derived from the satisfied rules every step).
  uint8_t           _nextRoomTarget = 255;
  float             _nextRoomReward = 0.0f;
  float             _nextRoomBase   = 0.0f; // base reward compensating the magnet loss at the transition entry
  float             _climbBonus     = 0.0f; // reward for a grab/climb-up frame in a climb-bonus room
  std::set<uint8_t> _climbBonusRooms;       // rooms where the climb bonus applies (e.g. 14, the sword pit)

  // Input alphabet
  InputSet::inputIndex_t _in_null, _in_U, _in_D, _in_L, _in_R, _in_S, _in_A, _in_B;
  InputSet::inputIndex_t _in_LA, _in_RA, _in_DB, _in_LB, _in_RB, _in_UB, _in_UD, _in_DL, _in_DR, _in_BA, _in_UBA, _in_UDB, _in_UDBA;
#ifdef _POP_GLITCH_HUNT
  InputSet::inputIndex_t              _in_UL, _in_UR, _in_ULR, _in_LR, _in_UDL, _in_UDR, _in_UDLR, _in_UDA, _in_ULA, _in_URA;
  std::vector<InputSet::inputIndex_t> _glitchProbes;
  // Phase pruning ceiling: non-null inputs offered only when Frame Phase <= this. Default 2 (prune the
  // never-used phases 3,4); overridable via game config "Max Input Phase".
  uint8_t _maxInputPhase = 2;
#endif

  // Per-frame allowed-move table
  std::unordered_map<uint8_t, std::vector<InputSet::inputIndex_t>> _frameMoves;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
