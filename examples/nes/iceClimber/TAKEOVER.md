# NES Ice Climber — Takeover / Handoff Notes

Working notes for solving Ice Climber with JaffarPlus (QuickerNES). Written to let a
future session resume cold. Game-model changes are committed in `games/nes/iceClimber.hpp`.
The standalone helper tools built during this effort were **removed** on request — their
designs are documented below so they can be recreated.

---

## 1. Goal & status

- **Goal:** full-game TAS beating the reference (which covers up to mountain 6), completing
  all 32 mountains. Global RNG persists across levels and determines each level's layout.
- **Constraints (standing):** State DB and Hash DB each < 10 GB
  (`JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB`, `..._HASHDB_SIZE_MB`); Raw input history
  (not Trie); no full production runs on this machine (research/test runs only); commit only
  when asked; `.nes/.asm/.inc`, `reference/`, `disasm/` are gitignored.
- **Status:**
  - Climb → bonus → grab pipeline works end-to-end for floor-climb mountains.
  - The **bonus stage is fully solved** (a routing tool solved all 5 reference bonuses).
  - **Level injection works** — can start any mountain in isolation.
  - **OPEN / THE WALL:** mountains whose *climb* contains moving clouds (e.g. mtn 6). Neither
    the automated router nor manual co-routing has cracked mtn 6's cloud-climb. Details in §6.

---

## 2. Authoritative RAM map (LRAM offsets)

| Addr   | Meaning |
|--------|---------|
| `$59`  | Current level (0-based). **Mountain N ⇒ `$59 = N-1`.** Skeleton = `$59 & 7`. |
| `$56`  | Mountains completed (difficulty). |
| `$64`  | Player pos X. |
| `$66`  | Player pos Y. **192 = climb bottom; lower value = higher on screen.** |
| `$13`  | Screen scroll Y. **Decreases as you climb, wraps at 240** (one wrap ≈ one screen). |
| `$E0`  | Player airborne/jumping. **0 = on ground.** |
| `$D7`  | Bird/condor state. **`== 21` ⇒ bonus stage active** (condor flying). |
| `$4D`  | Grabbed bird (grab timer; >0 briefly after a grab). |
| `$243` | Bird pos X (`!= 112` used with `$4D>0` to detect a real condor grab). |
| `$364` | Bricks broken (cumulative count). |
| `$382` | **NOT a clean death bit.** =0 on mtn1, =1 during *normal* mtn6 play, =3 at movie-end. Do not use as "dead". |
| `$27`  | Bonus-stage-start flag. |
| **Clouds** (bonus stage AND cloud-climbs), slot `i` in 0..3: | |
| `$786+i` | Cloud active (nonzero). |
| `$682+i` | Cloud left X.  `$6A0+i` = right X (span ≈ 48 px). |
| `$6BE+i` | Cloud raw Y (screen Y = value + 32). |
| `$7B7+i` | Cloud signed velocity. Moved by `sub_E33C`. |
| `$400–$4C0` | Terrain / brick layout (hash this region to detect layout changes / broken holes). |

`$382`, `$D7`, and the packed-savestate quirk (below) are the biggest footguns.

---

## 3. Architecture

Solve a mountain in two phases, then concatenate the input sequences:

1. **Climb** — jaffar beam-search from the climb bottom to the bonus entry
   (win = `Is Condor Active == 1`, i.e. `$D7 == 21`, gated so it fires on the NEW mountain's
   bonus, not a carried-over one — see `Left Bonus` latch in `iceClimber.hpp`).
2. **Bonus** — a **macro-action router** (not a reward beam) from the bonus entry to the
   condor grab.

The bonus stage is a vertical scrolling climb over **moving cloud platforms**, topped by a
condor. It is fully deterministic (cloud positions/velocities fixed; jump physics
deterministic), so it is a **routing** problem. A reward-priority beam CANNOT do it — it
dedups on near-exact pixel state and drops the long, gradient-less ride.

---

## 4. Level injection — start ANY mountain (WORKS)

Chaining through all prior mountains is fragile (the reference ends messily mid-mtn6 with a
frozen level-load and a movie-end `$382=3` artifact). Instead, inject the level number.

- **Boot injection does NOT work:** the initial-sequence replay runs the emulator's
  `advanceStateImpl` directly (see `quickerNES.hpp`), **bypassing the game's per-frame hook**;
  and the layout terrain-gen happens *during the boot's START frame* (frame 8). So a per-frame
  hook can't intercept it at boot.
- **Level-TRANSITION injection WORKS:** a search/tool whose **Initial Sequence ends just
  before a level transition** + pin `$59`/`$56` every frame → the transition's terrain-gen
  reads the pinned `$59` → loads any mountain fresh (verified: terrain hash differs per level).
  The transition is *inside the search*, where the game hook fires.
- Mechanism (committed): env var **`ICE_INJECT_LEVEL`** (0-based `$59`; mtn N ⇒ N-1) — the
  `stateUpdatePostHook` pins `$59`/`$56` each frame.
- Supporting pieces in `iceClimber.hpp`: **`Left Bonus` latch** (`$D7` ever `!= 21`) so an
  injected search starting at the prior mountain's bonus-end doesn't win instantly; and a
  **reset of scroll-wrap count / best-Y / condor tracking on first entering the climb**
  (otherwise the prior mountain's state corrupts realY/reward).
- Ready-made boot file: `mountain_trans.sol` = `reference/full_game_partial.sol` head -2718
  (just before the mtn1→mtn2 transition at frame 2724). Player appears at the climb bottom
  (posX 56, posY 192) a few frames after the transition. Use with `ICE_INJECT_LEVEL=5` for mtn6.

---

## 5. The bonus router (removed — recreate as a game-specific tool)

Convention: build it under `games/nes/iceClimberRouter/` and gate on `-Dgame=iceClimber`
in `meson.build` (cf. `arkanoid2`, `lunarBall`). It solved **all 5 reference bonuses**
(mtn1@550, mtn2@635, mtn3@1186, mtn4@775, mtn5@557 frames), seconds each.

Design that worked:
- Build a `Runner` from a config whose Initial Sequence reaches the bonus entry; frameskip 0.
- Macro-action A* search. Decisions = **how long to RIDE the current cloud** (walk-L / walk-R /
  wait, advanced until the coarse key changes) + **which JUMP** (9 variants: launch
  {A, LA, RA} × air-steer {noop, L, R}; plus air-steer *release timing* so short hops onto
  near platforms are possible, not only the maximal leap that overshoots into a gap).
- Transitions simulated by the real emulator (exact physics/cloud motion) → landings correct.
- Dedup key = `(scroll-wraps, posX/PB, posY/PB, scroll/8, E0, active-cloud X/CB, bricks-broken)`.
  `PB≈2`, `CB≈4`. Finer buckets solve precise high-cloud landings.
- Priority (min-first, A*-ish): `frames - GREED*heightScore`, `heightScore = (wraps*240 -
  scroll)*4 + (144 - posY) + BRICK*bricksBroken`. `GREED≈6`. High GREED = greedy hill-climb
  (fast, but stalls under ceilings needing lateral detours); low = broader/routes around.
- Intro-skip: advance NOOP until a test A-press actually launches a jump.
- `--climb` whole-level mode was added (route cloud-climb → bonus → grab in one search):
  in-play gate broadened from `inBonus()` to "alive" (the macro search only expands on-ground
  nodes and only adds jumps that LAND, so falls drop out via the maxAir timeout — no death flag
  needed, which is good because `$382` is not one). It reached mtn6 screen 1 but not the top (§6).

---

## 6. mtn 6 cloud-climb — THE WALL (unsolved)

Some mountains' **climbs** contain moving clouds, not just brick floors. mtn 6's start has
**two clouds to jump onto, then a destructible-brick ceiling** you must punch a hole in.

What we learned:
- **Automated router dead-ends:** greedy height-max brute-forces straight up the RIGHT side,
  breaking ~16 bricks, into a corner (≈posX 209, "level 8"). Verified 3 ways it is a genuine
  dead-end: any left jump drops to level 1 (center platforms ~45 px away, beyond any jump);
  straight up is a solid ceiling (no break, posY 93 max); and **no cloud ever services that
  corner** (parked off-screen for 320+ frames). Lowering GREED / adding release-timing jumps /
  long cloud-waits did not break the plateau — it's a *reachability/pathing* dead-end, the
  wrong part of level 8.
- **Intended route (per the human player, who only partly remembers post-level-6):**
  jump onto the right cloud → ride it left → jump up to the level-3 brick floor → ride the
  cloud far left → break **one of the two single-brick spots** in the ceiling (the ceiling is
  a two-brick row *except* two single-brick spots) → jump through *just in time* → … →
  jump to a left platform → up a couple times → bonus stage.
- **Co-route progress:** moves 1–2 confirmed (onto the right cloud; ride left + jump up to the
  level-3 floor). **Stalled at move 3.** From the cloud, the player can only land on level 3 at
  the *right* part (≈posX 166); the whole left half is unreachable (up-jumps top out at the
  floor's underside ≈posY 94–97 but the player is always over a gap and falls back; no brick
  breaks anywhere reachable from that cloud, which only drifts left to ≈posX 42). From the
  right-part landing the player is boxed in (ceiling up, gaps L/R).
- **Best hypothesis for next time:** move 2 likely lands the player on the *wrong part* of
  level 3 (a wrong turn we only detect several moves later). Rebuild the co-route from move 1
  with a **screenshot after every single step** so the human catches the exact desync. The
  clouds may also need to be caught at a specific phase, and the second cloud (`slot 2`, high
  at screen Y≈38 / X≈39) may be the real vehicle to the ceiling rather than the low right cloud.

---

## 7. Key gotchas

- **File-based savestates are BROKEN for this game.** The packed serializer writes
  `getStateSize()` bytes (≈2188) but the deserialize path expects the full ≈2200-byte layout
  → `Maximum input data position reached` → core dump. So the emulator **"Initial State File
  Path"** and `jaffar-player --saveStateFile` cannot round-trip here. For checkpoints, use the
  **in-process `Runner::serializeState/deserializeState`** (what the router uses every frame —
  that works fine). This is why the co-route helper kept checkpoints in-process, not on disk.
- `$382` is **not** a death bit (see §2).
- **Boot injection fails; transition injection works** (see §4).
- **Screenshots:** `jaffar-player <cfg> <sol> --reproduce --exitOnEnd --unattended
  --screenshotDir DIR --screenshotSteps N` writes BMPs — **requires rendering (do NOT pass
  `--disableRender`)**. The box has no PIL/ImageMagick; use a small pure-python BMP→PNG
  converter (24-bit BGR, bottom-up rows; zlib + PNG chunks; nearest-neighbor upscale ~3×).
- Emulation is ~90 fps headless here, so replaying a 2700-frame prefix per probe is slow
  (~30–60 s). Checkpoint *after* the long Initial Sequence and probe from there (near-instant).

---

## 8. Files & assets (in this folder)

- `reference/full_game_partial.sol` — reference movie, 12182 frames, boot → mtn6 start; grabs
  mtns 1–5. (gitignored)
- `mountain_trans.sol` — head -2718 of the reference; injection boot point (see §4).
- Scratch configs/sols (`mountain0N.*.jaffar`, `*.sol`, `injector_*.jaffar`, `route*.sol`) are
  experiment artifacts (untracked); safe to delete.
- Base per-level configs (`mountain03.jaffar`, etc.) carry the reward/rules used by the climb
  search.

---

## 9. Suggested next steps

1. **Crack mtn 6's cloud-climb** — rebuild the co-route from move 1 with per-step screenshots
   (§6), or get exact inputs from a human playthrough, or design a richer action model.
2. **Recreate the bonus router** (§5) and the co-route helper (in-process checkpoints, §7) as
   `-Dgame=iceClimber` tools.
3. **Global RNG-DP** — the cross-level RNG (free-running idle-loop LFSR at `$18–$1C`) is the
   speed lever; plan a DP over RNG states to chain all 32 mountains for the real playthrough.
