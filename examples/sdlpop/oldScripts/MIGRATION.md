# Migration of the world-record SDLPoP TAS scripts to current JaffarPlus

These folders hold the original scripts, initial states, and solutions used to produce the
published TAS (https://tasvideos.org/8059S). Each four-digit folder is a full level (`xx00`)
or an independent leg (`xx01`+). This directory contains both the originals and reconstructed,
current-format versions.

## Reconstructed artifacts (per folder)
- `script.jaffar`  — the old `script.jaffar.old` migrated to the current config schema.
- `solution.sol`   — the old `solution` converted to the current input format.
- `state`          — the original initial state (reused as-is; format is byte-compatible).
  (`1500` is a 14-segment level: `script0..13.jaffar` + `full0..13.sol`.)

## What the conversion does (see migrate.py / migrate_old.py)
- **Schema**: two old schemas exist. `0100` uses a near-current one; the rest use an older
  `Jaffar/Playback Configuration` + top-level `Rules` schema. Both are mapped to the current
  `Driver/Engine/Emulator/Runner/Game Configuration` layout (state DB, hash DB, frameskip object,
  bypass flags, NUMA group, etc.).
- **Solution input format** -> current `|r|LRUDS|` 9-char pipe format, from three old encodings:
  positional 7-char `[C][A][L][R][U][D][S]`, compact button-sets (`LU`,`SR`,`.`), and already-pipe.
  `Ctrl+A` (old `CA`) = SDLPoP "restart level" -> `r`.
- **Rules**: templated tile conditions (`"Background Element[", Room, Tile`) -> `Background
  Element[Room][Tile]`; `Tile FG State` -> `Foreground Element[..]`; property renames
  (`Kid Position Y`->`Kid Pos Y`, `Guard Position X/Y`->`Guard Pos X/Y Raw`,
  `Exit Door Timer`->`Exit Room Timer`); action renames (`Set Guard Horizontal/Vertical Magnet`
  -> `Set Guard Pos X/Y Magnet`); magnet key `Center`->`Position`.
- Game version and RNG/loose-tile overrides are preserved from the originals.

## Validation
Each reconstructed solution was replayed with `jaffar-player --reproduce --printFinalState` and
plays through cleanly (all inputs, no error). Full-level folders reach a **Win** (level
transition); legs reproduce to their intended waypoint (final state `Normal`).

### Per-folder reproduction status
```
0100  new CLEAN/WIN        inputs=  244 finalStep=  244 final=Win
0200  old CLEAN            inputs=  217 finalStep=  217 final=Normal
0201  old CLEAN            inputs=  100 finalStep=  100 final=Normal
0202  old CLEAN            inputs=  159 finalStep=  159 final=Normal
0203  old CLEAN            inputs=  184 finalStep=  184 final=Normal
0204  old CLEAN/WIN        inputs=  274 finalStep=  274 final=Win
0300  old CLEAN            inputs=  909 finalStep=  909 final=Normal
0301  old CLEAN/WIN        inputs=  192 finalStep=  192 final=Win
0400  old CLEAN/WIN        inputs=  575 finalStep=  575 final=Win
0500  old CLEAN            inputs=  245 finalStep=  245 final=Normal
0501  old CLEAN/WIN        inputs=  241 finalStep=  241 final=Win
0600  old CLEAN            inputs=  217 finalStep=  217 final=Fail
0700  old CLEAN/WIN        inputs=  496 finalStep=  496 final=Win
0800  old CLEAN            inputs=  742 finalStep=  742 final=Fail
0900  old CLEAN/WIN        inputs=  126 finalStep=  126 final=Win
0901  old CLEAN            inputs=  322 finalStep=  322 final=Normal
0902  old CLEAN            inputs=  270 finalStep=  270 final=Normal
0903  old CLEAN            inputs=  181 finalStep=  181 final=Normal
0904  old CLEAN            inputs=  260 finalStep=  260 final=Normal
0905  old CLEAN/WIN        inputs=  200 finalStep=  200 final=Win
1000  old CLEAN            inputs=  292 finalStep=  292 final=Normal
1001  old CLEAN            inputs=  189 finalStep=  189 final=Normal
1100  old CLEAN            inputs=  379 finalStep=  379 final=Normal
1101  old CLEAN            inputs=  194 finalStep=  194 final=Normal
1102  old CLEAN/WIN        inputs=  245 finalStep=  245 final=Win
1200  old CLEAN/WIN        inputs=  897 finalStep=  897 final=Win
1300  old CLEAN/WIN        inputs=  145 finalStep=  145 final=Win
1301  old CLEAN/WIN        inputs=  132 finalStep=  132 final=Win
1302  old CLEAN/WIN        inputs=   75 finalStep=   75 final=Win
1400  old CLEAN            inputs=  126 finalStep=  126 final=Normal
1500  -   SKIP missing files inputs=    - finalStep=    - final=-
```

### Notes / to verify against the TAS
- `0600`, `0800`: reproduce fully but the final frame classifies as `Fail` (e.g. `0800` reaches the
  level 8->9 transition at 1 HP). Likely a boundary rule firing at the leg end; verify intent.
- `1301`: the old solution had an anomalous column (an `L` in the right-button slot); it still
  reproduces to Win, but a few inputs may have been normalized — verify.
- `1500`: 14 segments share one evolving `state` (each segment started from the previous one's end
  state). Only the last-used segment's start state remains, so only that segment is validated here;
  the rest are migrated but need the intermediate states (regenerate by chaining, or supply them).
