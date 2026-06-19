# 5. Adding a Game

This chapter is for contributors. It shows how to add a new game (and, briefly, a new emulator core)
to JaffarPlus. Thanks to automatic registration, adding a game is just **writing one header** —
there are no lists to edit and nothing to wire up by hand.

- [How registration works](#how-registration-works)
- [Anatomy of a game class](#anatomy-of-a-game-class)
- [The methods you implement](#the-methods-you-implement)
- [Exposing properties](#exposing-properties)
- [Reward and custom actions (magnets)](#reward-and-custom-actions-magnets)
- [Building and running your game](#building-and-running-your-game)
- [Adding a new emulator core](#adding-a-new-emulator-core)

## How registration works

Games and emulators follow a uniform shape: a class `class <Name> final : public <Base>` inside a
known namespace, compiled behind a per-directory preprocessor guard. At configure time,
[`genRegistry.py`](../genRegistry.py) globs the source tree, finds those classes, and generates two
headers — an `#include` block and a `DETECT_*` block — that `games/gameList.hpp` and
`emulators/emulatorList.hpp` pull in. The generated files live in the build directory, so a
single-game build (`-Dgame=`) never disturbs another build directory.

The practical upshot: **drop a new `.hpp` into the right `games/<platform>/` directory and re-run
`meson setup`** — it is detected automatically. The directory determines the platform guard (e.g.
`games/nes/` → `__JAFFAR_ENABLE_NES`), so your game compiles only for the core(s) that define it.

## Anatomy of a game class

The smallest complete example in the repo is the test game
[`games/test/gridWalker.hpp`](../games/test/gridWalker.hpp). Its shape:

```cpp
#pragma once
#include "testEmulator/testEmulator.hpp"   // the emulator this game runs on
#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus { namespace games { namespace test {   // games::<platform>

class GridWalker final : public jaffarPlus::Game
{
public:
  // The string matched by "Game Name" in the config:
  static __INLINE__ std::string getName() { return "Test / GridWalker"; }

  GridWalker(std::unique_ptr<Emulator> emulator, const nlohmann::json& config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
    // Read any game-specific config keys here:
    _goalX = jaffarCommon::json::getNumber<uint8_t>(config, "Goal X");
    _goalY = jaffarCommon::json::getNumber<uint8_t>(config, "Goal Y");
  }

private:
  // ... the overrides below ...
};

}}} // namespace
```

Two contracts matter for registration:

- The class must be `final : public jaffarPlus::Game` and live in `namespace jaffarPlus::games::<dir>`.
- `static getName()` returns the exact string users put in `Game Configuration` > `Game Name`. The
  "game not recognized" error lists all names compiled into the build, so pick something
  descriptive.

Any custom config keys (like `Goal X` above) are read in the constructor — and they become part of
your game's documented schema, so add them to the [Configuration Reference](02-config-reference.md).

## The methods you implement

`Game` declares seven pure-virtual methods every game must override *(source: `source/game.hpp`)*:

| Method | Purpose |
|--------|---------|
| `registerGameProperties()` | Declare the named properties the config can reference (see below). |
| `advanceStateImpl(input)` | Advance the game by one input/step (usually calls `_emulator->advanceState(input)` plus bookkeeping). |
| `serializeStateImpl(serializer)` | Persist any game-owned state not already in the emulator's save-state. |
| `deserializeStateImpl(deserializer)` | Restore that game-owned state. |
| `calculateGameSpecificReward()` | Return the game's contribution to a state's reward (magnets, distance heuristics). |
| `computeAdditionalHashing(hashEngine)` | Fold any extra game-owned bytes into the state hash (often empty if all dedup keys are registered properties). |
| `parseRuleActionImpl(rule, type, json)` | Parse game-specific rule actions; return `false` for unrecognized types. |
| `printInfoImpl()` | Print game-specific fields in the status banner. |

There are also optional hooks worth knowing:

- `stateUpdatePostHook()` — recompute derived values after every state change/deserialize (GridWalker
  uses it to recompute distance-to-goal).
- `getStateInputHash()` / `getDirectStateHash()` — fine-grained hashing for candidate-input testing
  and for `Bypass Hash Calculation`; keep them keyed on the same identity as your `Hash Properties`.

## Exposing properties

`registerGameProperties()` maps memory into named, typed properties. You typically grab pointers
into emulator memory and register them:

```cpp
void registerGameProperties() override
{
  auto position = (uint8_t*)_emulator->getProperty("Position").pointer;
  registerGameProperty("Pos X", &position[0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
  registerGameProperty("Pos Y", &position[1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
  registerGameProperty("Distance", &_distance, Property::datatype_t::dt_int32, Property::endianness_t::little);
}
```

Once registered, a property name can be used in `Hash Properties`, `Print Properties`, conditions,
and magnets. Datatypes and endianness are listed in the
[Configuration Reference](02-config-reference.md#property-datatypes). The error on an unknown
property name lists every registered property, which is handy while authoring configs.

## Reward and custom actions (magnets)

The engine forms a state's reward from rule `Add Reward` actions **plus** whatever
`calculateGameSpecificReward()` returns. GridWalker just returns `-(distance to goal)` so closer is
better. Richer games sum their magnets here.

To add **magnets** (or any custom action `Type`), implement `parseRuleActionImpl`. It is a chain of
string comparisons; return `true` once you recognize and handle an action, `false` otherwise:

```cpp
bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
{
  if (actionType == "Set Player Pos X Magnet")
  {
    auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
    auto position  = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Position");
    rule.addAction([=, this]() { _playerPosXMagnet = pointMagnet_t{.intensity = intensity, .position = position}; });
    return true;
  }
  return false; // unrecognized -> let the base class report it
}
```

Then in `calculateGameSpecificReward()`, add `intensity * -|current - position|` for each active
magnet (see `games/sdlpop/princeOfPersia.hpp` for the canonical pattern, including room-gating).
Use the consistent naming convention `Set <Thing> Magnet` and document the new action's keys in
the configuration reference / chapter 3.

## Building and running your game

No list edits are needed — just configure and build:

```bash
# Build only your new game on its core:
meson setup build-mygame -Demulator=QuickerNES -Dgame=myGame
ninja -C build-mygame
./build-mygame/jaffar myGame.jaffar --dryRun   # validate the config first
```

`-Dgame=` matches the header file stem (case-insensitive). Omit it to build all games of the core.
If your game isn't detected, confirm the file is in the right `games/<platform>/` directory, the
class is `final : public jaffarPlus::Game`, and you re-ran `meson setup` (the registry is generated
at configure time).

## Adding a new emulator core

The same model applies to emulators: a class `final : public Emulator` in
`namespace jaffarPlus::emulator`, in a directory under `emulators/`, behind a `__JAFFAR_USE_<DIR>`
guard (with a few overrides — see the top of [`genRegistry.py`](../genRegistry.py)). Register it as
a `-Demulator=` choice in [`meson_options.txt`](../meson_options.txt) and wire its compile guard in
`emulators/meson.build`. Implement the emulator interface in `source/emulator.hpp` (load/save state,
advance step, expose memory as properties, and — optionally — `saveScreenshot` for the rendering
tooling). The in-repo [`emulators/testEmulator`](../emulators/testEmulator) is the minimal reference
implementation.
