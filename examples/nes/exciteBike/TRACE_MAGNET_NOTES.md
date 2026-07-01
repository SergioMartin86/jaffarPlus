# ExciteBike Trace-Magnet — Reproduce & Beat the Reference TAS (session notes)

Working notes for reproducing (and then trying to beat by ≥1 frame) the ExciteBike lap-1 reference
TAS with JaffarPlus + the from-scratch native engine (ExciteBikeNative / `extern/exciteBot`),
seeded mid-race. Written to hand off to a future session. Last updated 2026-07-01.

---

## 0. TL;DR / where we are

- **The trace magnet works.** It reproduces the reference in a *small* DB by adding a small reward
  penalty for straying from a recorded reference trajectory, so the greedy State DB stops evicting the
  (temporarily lower-reward) reference line.
- **The single most important fix was DECOUPLING the floor from the magnet** (`getFloorReward`).
  Almost every "wall" we chased (step 147, 253, 632) turned out to be a **coupled-floor false-cancel**,
  not a real divergence.
- **Best config so far: `X=5 Y=1 Z=2 V=0.1 F=0` + decoupled floor.** velX (V) helps; flight2 (F)
  actively hurts. This reproduces the reference cleanly from frame 1050 to **~step 873** (of ~1213) in
  a 5–20 GB DB.
- **Remaining blocker:** an airborne sub-pixel drift zone at **old-step ~873–877** that neither velX,
  capacity (5→20 GB), nor a closer seed fixes decisively (wall moved only 873→877). Likely the
  un-anchored **bike angle** (reference lowers 8→7→6 in the descent) driving residual posX drift.
- **Offset trick (Offset:1) validated** as a *local-slack detector*: best−ref goes positive where the
  reference has slack, 0 where it's frame-optimal. The **airborne stretch has ZERO slack** (bike pinned
  at max velX 1544). Next: probe **ground segments** (bounded ~100-step windows) for slack.

---

## 1. Core problem & the trace-magnet idea

A greedy reward-ranked State DB **evicts the non-greedy optimum**. The reference is temporarily
lower-reward (posX) than an "apparent-better route" (a native-engine phantom), so once the DB fills the
reference gets dropped — verified at **exactly step 136** (reference posX 4435.39 fell below the
worst-kept 4435.55). From there the search chases the phantom (peaks ~+14.5 @step145, collapses
~step200). Big DBs just OOM (hash grows with breadth) — a dead end.

**Fix (trace magnet):** record the reference's per-step coordinates; add a SMALL reward penalty for
straying, so reference-adjacent states stay competitive for a DB slot. Reward stays the sole eviction
driver. Modeled on `games/nes/metroid.hpp`'s trace magnet.

---

## 2. Code changes (UNCOMMITTED, on master) — what to re-check / commit

- **`source/game.hpp`**: `virtual getTraceLine()` (default `""`), `virtual getFloorReward()` (default `_reward`).
- **`source/player.cpp`**: `--dumpTrace <file>` (mirrors `--dumpReward`; dumps `Game::getTraceLine` per step).
- **`games/nes/exciteBike.hpp`**:
  - `Trace File Path` config key; `Set Trace Magnet` rule action with `Intensity X/Y/Z/V/F` + `Offset`.
  - `traceEntry_t { posX, posY, posZ2, velX, flight2 }` (5 columns).
  - `ruleUpdatePostHook` computes per-axis distances + `_magnetPenalty`. **flight2 uses CYCLIC distance**
    `min(|d|, 256-|d|)` (it wraps 0–255).
  - `calculateGameSpecificReward = _bikePosX - _magnetPenalty` (RANKING reward → eviction only).
  - `getFloorReward = getReward + _magnetPenalty` (= posX; the FLOOR reward, un-biased).
  - `getTraceLine` emits `posX posY posZ2 velX flight2`; printInfo shows the magnet.
- **`source/driver.hpp`**: `_bestStateFloorReward` = best state's `getFloorReward`; the Reference Reward
  Floor check + the "Reference Reward (Best - Ref)" display now use it (NOT the magnet-biased reward).

Build: `cd build-eb-native && ninja -j16 jaffar jaffar-player` (cap `-j`, never bare ninja on this box).

---

## 3. KEY LESSONS

### Lesson 1 — DECOUPLE the floor from the magnet (the big one)
Putting the magnet penalty in the same reward the Reference Reward Floor checks **false-cancels** any
axis that varies independently of posX. Example: the search reaches posX = floor with a slightly-off
sub-pixel byte (harmless for position), the penalty drops that position-correct state below the tol=0
floor → false cancel. **`getFloorReward` makes the floor compare true posX; the magnet biases eviction
only.** This dissolved the "walls" at step 147 (velX) AND 253/632 (flight2) — they were ALL
coupled-floor false-cancels, not real divergences. Works because the strong X anchor (5) keeps the
top-ranked state position-correct.

### Lesson 2 — velX helps, flight2 hurts; anchor velocity, be wary of extra axes
- **velX (V)** carries the airborne sub-pixel/launch region (velX 1288→1544 downhill launch). velX-only
  + decoupled floor reaches ~step 873.
- **flight2 (F)** the fractional airborne accumulator (0x380) — anchoring it *hurts*: F=0.1 walled at
  259, F=1.0 walled at 258 and let a `+0.4375` posX-ahead phantom line outrank the reference (the "3rd
  airborne phantom"). Reverted to F=0.
- General: extra magnet axes risk a tug-of-war with the phantom. Add them only with clear RAM evidence.

### Lesson 3 — attribute causes with REPLICATION, not single runs
The wall step is (potentially) a random variable — the parallel search is first-seen-wins,
concurrency-ordered. We tested this: position-only (V=F=0) walled at step **255 five times out of five**
→ **deterministic**, concurrency hypothesis rejected for that config. A single run's wall step is not
evidence; replicate before concluding "bigger DB / closer seed helped."
Replication harness left in `race01a.magnet.walls.txt`.

### Lesson 4 — the diagnostic method (reusable)
1. Crank the magnet (X=5 forces reference-following); the exact cancel step pinpoints the transition.
2. RAM-dump the reference at that frame (see §5) and compare which byte the search can't hold.
3. The **gap size at cancel is a tell**: ~0.66 = velX; ~1.0 = a flight2 carry; ~0.11 = posX2 drift; ~1.2
   = accumulated airborne drift.

---

## 4. The offset trick (local-improvement search) — validated, in progress
`Set Trace Magnet` `"Offset":1` → `_traceStep = currentStep + offset` → the magnet targets the
reference's NEXT-step state, so it rewards advancing *further* than the reference. With the NORMAL
decoupled floor (no shift), **best−ref reads local slack directly**: `>0` = the search beat the
reference locally (candidate 1-frame gain, needs QNES validation for phantom risk); `0` = reference
frame-optimal here. **Result so far: the airborne stretch has ZERO slack** (best−ref 0 for 140+ steps;
bike pinned at max velX 1544). **Next: bounded ~100-step GROUND segments** (where the reference is below
the ~832 velX cap = likeliest slack).
NOTE: do NOT shift the floor +1 for this — from a reference-aligned seed the search starts 0-ahead, so a
+1 floor cancels at step 0. Let the lead build up under the normal floor instead.

---

## 5. Recipes (commands)

**Reference solution / configs:**
- `race01a.tas.sol` — full lap-1 reference TAS (frame 0 → race end ~frame 2263).
- `race01a.native.jaffar` — no-seed (frame-0) config to replay the full TAS in the native engine.
- Native binaries: `/home/jaffar/jaffarPlus/build-eb-native/{jaffar,jaffar-player}`.

**Generate the trace (must align to the floor):**
```
jaffar-player race01a.native.jaffar race01a.tas.sol --reproduce --unattended --disableRender --exitOnEnd --dumpTrace /tmp/t
tail -n +1051 /tmp/t > race01a.trace1050          # seed frame 1050 -> step0=frame1050 -> line N = full line 1050+N
# VERIFY: awk '{print $1}' race01a.trace1050 must equal refreward1050.floor exactly
```
Player replay MUST use `--reproduce --unattended --disableRender --exitOnEnd` or it hangs in the
interactive loop.

**RAM-dump the reference at a frame (diagnose a wall):**
```
jaffar-player race01a.native.jaffar race01a.tas.sol --reproduce --unattended --disableRender --exitOnEnd --dumpRam /tmp/full.ram
# LRAM = 0x800 bytes/frame; frame = 1050 + step (for the frame-1050 seed).
# Key addrs: velX=0x94*256+0x90, airMode=0xB0, angle=0xAC, posY=0x8C, posZ2=0xB8, climb=0xBC,
#            flight2=0x380, flight3=0x384, velZ=0xDC, posZ1=0x70, posX2(sub-pixel)=0x394, gameCycle=0x4C.
```

**Carve a mid-race seed (segment / faster iteration):**
```
jaffar-player race01a.native.jaffar race01a.tas.sol --reproduce --unattended --disableRender --exitOnEnd --saveStateStep <FRAME> --saveStateFile frame<FRAME>.state
# Initial Block Transitions = floor(reference_posX_at_FRAME / 256)   (reference posX = refreward1050.floor line FRAME-1050+1)
# Slice floor/trace from that frame:  tail -n +<FRAME-1050+1> refreward1050.floor > refreward<FRAME>.floor  (and same for the trace)
# Config: Emulator "Initial State File Path"=frame<FRAME>.state, Game "Initial Block Transitions"=<IBT>,
#         "Trace File Path"=race01a.trace<FRAME>, Driver Reference Reward Floor "Path"=refreward<FRAME>.floor.
# Step numbering restarts at 0 = the seed frame. (We used frame 1700: old_step = new_step + 650.)
```

**Launch (detached, pinned 256 threads):**
```
setsid env OMP_NUM_THREADS=256 OMP_PROC_BIND=close OMP_PLACES=cores \
  /home/jaffar/jaffarPlus/build-eb-native/jaffar race01a.magnet.jaffar >> race01a.magnet.log 2>&1 </dev/null &
```
Monitor: `Reference Reward (Best - Ref):  <ref> (Best +X.XXX, tol ...)` — Best is now the FLOOR reward
(posX − ref). +0.000 = tracking; negative = fell behind (real, since the floor is decoupled).

---

## 6. Artifacts in this directory
- `race01a.magnet.jaffar` — the working config (currently: seed frame1700, X5 Y1 Z2 V0.1 F0 Offset1,
  decoupled floor, 20 GB, raw history). Reset Offset→0 and re-point the seed/trace/floor to frame 1050
  for a full-race run.
- `frame1050.state` (IBT 14), `frame1700.state` (IBT 30) — seeds.
- `race01a.trace1050`, `race01a.trace1700` — 5-column traces (posX posY posZ2 velX flight2), aligned to
  the floors.
- `refreward1050.floor` (1213 steps), `refreward1700.floor` (563 steps) — reference posX per step.
- `race01a.magnet.walls.txt` — replication tally (5/5 at step 255 for the position-only config).
- `race01a.magnet.run_*.log` — archived per-run logs from the replication set.

---

## 7. Suggested next steps (pick up here)
1. **Beat-the-reference (main goal):** run the **offset trick on bounded ~100-step GROUND segments**
   (seed where reference velX < ~832 cap). Any positive best−ref = candidate local gain → **QNES-validate**
   (replay in the real emulator; the native engine has ≥1 known airborne phantom, so unvalidated gains
   are suspect).
2. **Full reproduction to the win:** to get past the ~873–877 airborne drift, the untested lever is
   anchoring the **bike angle (0xAC)** as a magnet axis (reference lowers 8→7→6 in the descent). Risk:
   like flight2, an extra axis can start a phantom tug-of-war — add it only if RAM at the failing frame
   confirms angle is what diverges, and keep its intensity small.
3. **Or segment the whole reproduction:** stitch short reference-seeded segments so the airborne drift
   never accumulates; capture each segment's inputs from raw history.
4. **Validate the reproduced line in QNES** once a full segment/run reaches the win.
5. **Commit** the trace-magnet code (§2) if keeping it.

See also the auto-memory `excitebike-trace-magnet.md`, `excitebike-native-engine.md`,
`excitebike-tas-beat.md` for related context.
