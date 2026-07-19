# Prince of Persia (SDLPoP) — search notes

Notes for running JaffarPlus on the SDLPoP port. The level folders (`0100`, `0200`, …)
are named `LLSS` = level·segment. These are practical hints — verify against current code.

## Approach: read the game, don't only brute-force
SDLPoP is open source, so the highest-leverage move is to **read the game logic for
oversights and shortcuts** rather than searching blindly. Examples that paid off:
- A pickup object-type read that leaks state.
- Shadow-figure collision being disabled on certain levels.
Finding the mechanic first, then searching to exploit it, is far cheaper than hoping a
blind search stumbles onto it.

## Configuration
- **Disable Non-Gameplay RNG.** The torch-flicker animation churns the RNG every frame;
  with combat present that explodes the combat-RNG hash space. Set the option that
  disables non-gameplay RNG, and pair it with the combat-RNG hash guard on segments that
  actually contain guards.
- Reconstructed room maps and a `LEVELS.DAT` decoder live under `analysis/`; a published
  full-game TAS reconstruction lives under `oldScripts/`.

## Movement tricks discovered (level noted where confirmed)
- **Guard edge-wrap**: herd a guard to a room corner and nudge so its X byte over/
  underflows to the opposite side of the *same* room; crossing then carries into the next
  room. Confirmed on level 4.
- **Wall-clip**: a guard bump into the left wall can clip the character *down* through the
  floor. Key on level 8; note that down-clips elsewhere tend to be dead-ends.
- **Sword lock-and-drag** ("moonwalk"): sheathe/redraw the sword to lock onto a guard at
  zero horizontal distance, then retreat to drag it quickly one direction. Confirmed on
  level 7.

## Segment re-search method
- Organize by `LLSS` segment folders with mid-climb checkpoints. Use an interpolated
  vertical-position climb reward so partial climbs are rewarded smoothly.
- Levels 1 and 2 came out optimal; combat-heavy segments explode the state DB, so hash
  combat state carefully and **always verify a solution visually** — Pos-Y-bucket win
  conditions and inert placeholder tiles have both produced false positives.

## Custom levels & visualization
- You can forge a chamber by injecting into a `LEVELS.DAT` slot (no checksum); copy
  another level's tiles, set the levels-file path, and cold-boot. Guards spawn on room
  transition.
- Headless screenshots (`player --screenshotDir/--screenshotSteps`) plus a video script
  render any solution or single frame for review.
