# Prince of Persia (NES) — examples

Per-level reward/rule scripts for the `NES / Prince Of Persia` game
(`games/nes/princeOfPersia/princeOfPersia.hpp`), ported from the legacy JaffarPlus
implementation (`games/nes/pop`, commit `c62a505`) to the current config format.

## What carried over
The rule bodies (`Label` / `Conditions` / `Satisfies` / `Actions`) are the legacy
room-progression magnet chains, essentially verbatim. Actions in use:
`Set Player Horizontal Magnet`, `Set Player Vertical Magnet`, `Set Player Direction Magnet`
(room-indexed bounded magnets), plus the built-in `Add Reward` / `Trigger Win` /
`Trigger Fail`. Legacy rules that keyed on the emulator render guards
(`Is Bad Render` / `Is Correct Render`, which no longer exist) were dropped, and any
dangling `Satisfies` references to them were pruned.

## Pending: initial sequences
Each `lvlNN.jaffar` points at `Initial Sequence File Path: "lvlNN.initial.sol"` — the
input prefix that drives the game from power-on to the start of that level. These are
**not committed yet**; they are to be sliced from the full-game TAS (level-entry frames
→ input-prefix slices). Until they are present, `--dryRun` validates each config but a
real search needs the corresponding `.initial.sol`.

## ROM
`Prince of Persia (U) [!].nes` (SHA1 `6B58F149F34FA829135619C58700CAAA95B9CDE3`).
ROMs are git-ignored; drop it in this folder to run.
