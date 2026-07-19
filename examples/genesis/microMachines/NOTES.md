# Micro Machines (Genesis) — search notes

Notes for running JaffarPlus (QuickerGPGX core) on Micro Machines race tracks. Practical
hints and reverse-engineered mechanics — verify against current code.

## Racer state (player 1)
Racer struct base `0xFFA60C`, stride `0xEA` per racer. Work RAM is stored **little-endian**
in the emulator's RAM array. Key fields (absolute address):

| Field | Address | Notes |
|---|---|---|
| pos (integrator axis A) | `0xA60C` | 16.16; drives grid gy |
| pos (integrator axis B) | `0xA610` | drives grid gx |
| velocity X | `0xA614` | int part; `pos += vel` each frame |
| velocity Y | `0xA618` | |
| attribute byte | `0xA648` | surface-contact state (0–3), **not** a static tile id |
| surface tile byte | `0xA649` | tile *category* (decorative — not the effect) |
| grid gy / gx | `0xA656` / `0xA658` | cell = `(posInt>>3)/12 & 0x1f` (96-unit cells) |
| surface-drag effect flag | `0xA655` | 0 only while a surface is actively dragging the car |
| cumulative checkpoint | `0xA69C` | `lap*36 + within-lap index`; 36 checkpoints/lap |
| laps remaining | `0xA698` | counts down to 0 |
| race-finished flag | `0xA69E` | 0→1 on the winning frame — use this for the win rule |

## Checkpoint / skip (shortcut) mechanic
- The checkpoint is derived from **position** via a grid/tilemap lookup, not line-crossing.
- Transition logic compares the new vs previous within-lap index. **Forward gap ≤ 18 is
  accepted for free** (no penalty); forward ≥ 19 is ambiguous and rejected unless a finish/
  recovery state applies; backward > 18 decrements the lap (the "punishment" wrap).
- So you can skip up to **18 checkpoints in one frame** for free by driving a chord across
  the track. There are **no mandatory key checkpoints** — a lap can complete with very few
  registrations if each gap ≤ 18. Real reference routes use skips like +13 and +9.
- The **track interior is a "syncless dead zone"**: chords across the center lose (the
  checkpoint doesn't re-sync there and the terrain is a slow patchwork). Usable shortcuts
  are corner cuts within the drivable annulus, not center crossings.

## The slowdown surface ("honey")
- The honey drag is **not** identified by position, speed, the surface tile byte
  (`0xA649`), or the attribute byte (`0xA648`) alone — those false-fire on collisions and
  corners and miss the real honey (the attribute differs between crossings).
- Reliable signal: **`0xA649 == 1` (on the honey surface) AND `0xA655 == 0` (drag effect
  active)**. This pair fires only while the car is actually being dragged by the honey,
  never on a start-line opponent collision or normal cornering. Found by diffing the full
  64 KB work RAM between a honey crossing and other frames.
- Terrain reality on race01, from real driving (a teleport-placement probe mislabels it):
  **no holes and no ramps/jumps anywhere** (`0xA66C` fall-state and `0xA64C` Z stay 0);
  the "solid" tiles are just the off-track boundary.

## Reference-driven search (see also `examples/.../jaffar-reference-reward-floor`)
- The **Checkpoint Guide** magnet is the un-gameable progress backbone: build its
  checkpoint list from the first-arrival position at each distinct `0xA69C` value of the
  reference you are using. The reference's skips become **gaps** in the id list, so landing
  a shortcut banks the same reward as the ring-crawler but many frames sooner. **A guide
  built from a different reference/segment makes the search crawl the ring and fall behind.**
- **Reference Reward Floor** is a *cancel* signal, not a pace-keeper: it stops the run when
  the best line falls below the reference. Size its tolerance above the reference's largest
  single-step reward jump, or it false-cancels on the reference's own skip.
- **Reward de-inflation:** a Checkpoint Guide at `1e6`/cp drives totals to ~`1e8` where
  float32 precision drowns the finer Speed/Point terms. Scale all reward magnets uniformly
  (e.g. ÷100) — ranking is unchanged, precision is restored.
- Guards worth keeping as fail rules: honey-bog (above), off-track/recovery (`0xA66C`),
  backward track-position drop, and wrong-lap (laps-remaining increased).

## Seeding from a BizHawk savestate
The reference movies were recorded in BizHawk (Genesis Plus GX). Two ways to seed a
QuickerGPGX search from a mid-race point:
- **Native (preferred) when the point branches off a QuickerGPGX-replayable line:** replay
  that line to the branch frame and `player --saveStateStep N --saveStateFile …`. The state
  is self-consistent (correct video, no interrupt/timing desync). Find the branch by
  matching the 234-byte racer struct (`work_ram + 0xA60C`) against each frame of the
  candidate solution — an exact (0-byte) match is the branch.
- **Transplant when it does not branch off a native line:** lift the dynamic regions from
  the BizHawk waterbox state into a QuickerGPGX `state_save` blob. Both are Genesis Plus GX
  1.7.6, so this is a state lift, not an input match. The QuickerGPGX block offsets are
  dumpable at runtime via the `GPGX_STATE_OFFSETS=1` env var. **Caveat:** transplanting into
  a base captured at a *different* moment desyncs, because the non-lifted regions (VDP
  interrupt/timing, IO) won't match — and those can't be naively copied (different byte
  layout). Prefer the native route whenever the point branches off a replayable line.
