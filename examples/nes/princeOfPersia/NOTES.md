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

## Search notes / gotchas

- **Terminology:** in this game refer to the character as the **"Player"** (not "Kid",
  which is the SDLPoP term).
- **Serialize the PPU nametables.** The root cause of replay/search divergence here is
  that PoP reads the *drawn* room background (the PPU nametables) for game logic. If the
  nametables aren't serialized, a loaded state and a live run diverge. Set the
  **Nametable Block Size** to the full `4096` (it defaults to `0`). Pair this with
  transition base-compression, a dedup hash, and climb magnets for the room-to-room seams.
- **Resyncing a full-game movie** onto QuickerNES: a recorded movie can be re-aligned with
  small **phase-shift null-input edits** at the seams (insert/remove idle frames until the
  trajectory re-locks). This turned a desynced level-1 reward into a clean replay.
- **Level 1 result:** with the nametable fix, faithful floor-trace reward, lag-counter
  handling, tolerance-0 search, and a large state DB, a search **beat the reference TAS on
  level 1** (well under its frame count). The productive workflow was *replicate the
  reference faithfully first, then let the search improve on it* — not brute force from
  scratch.

