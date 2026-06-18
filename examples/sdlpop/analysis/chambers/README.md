# SDLPoP synthetic test chambers

Custom single-room scenarios (forged into a copy of LEVELS.DAT) to isolate one mechanic and
exhaustively search the FULL 18-input alphabet for glitches / uncontemplated inputs. Tiny state
space => full-alphabet brute force is cheap (impossible on real levels).

## Pipeline (validated 2026-06-17)
1. `gate_forge.py <gateCol> <gateBG>` writes `chamber.dat` = copy of real LEVELS.DAT with level-1
   room-1 replaced by a corridor (row1 floor, a gate at gateCol with openness modifier = gateBG in
   the bg byte; recall `curr_room_modif = level.bg[room-1]`). Cold-start always lands on level 1.
2. Bootstrap a clean standing state: play >=33 null inputs (level-1 intro freeze = 33 frames) then
   quicksave (player `--runCommand s`, last frame saved). => chamber.state (Kid frame 15, standing).
3. Search config points Levels File Path -> chamber.dat, Initial State File -> chamber.state, offers
   all 18 control inputs every frame, magnet pulls +X, win = Kid Pos X >= 180 (genuinely past gate).
   Run `jaffar gate_search.jaffar`. Validate any win by reproducing best.sol (confirm x>=180, watch y).

## Gate-openness sweep result (gate col6)
| bg (openness) | passable? | technique |
|---|---|---|
| 0, 32 | NO (solid) | gate blocks; exhaustive search (~10M states) finds nothing |
| 64 | YES | CROUCH-crawl under (frames 108-115), y constant, genuine transit to x>187 |
| >=96 | YES | walk/run under (standing) |
Conclusion: no sub-threshold *clip* glitch in this build; gates behave per the char_height test
`(bg>>2)+6 < char_height`. Crouching (shorter char_height) passes at lower openness (~64) than
standing (~96). TAS lever: crouch-pass a rising gate earlier than walk-pass saves the open-wait.

## General harness (chamber.py + run.sh)
`chamber.py <scenario> [param]` forges chamber.dat + chamber.jaffar; `run.sh <scenario> [param]`
forges+bootstraps+searches+reports furthest progress. Method = MAGNET-ONLY (reward=Kid x): best.sol =
furthest-progress trajectory (avoids the driver win-save bug where End-On-First-Win breaks before
saving best.sol, driver.hpp:118). Validate by reproducing best.sol and reading MAX-X reached.

## Campaign results (full 18-input alphabet)
- wall (solid wall in corridor): blocked @x152 — no clip.
- gap N (jump distance, ~3-tile runway): clears N<=3; N>=4 the Kid falls in. Runway-dependent.
- gate openness sweep: solid bg<=32; CROUCH-under bg~64; walk/run-under bg>=96. No sub-threshold clip.
- guard: DEFERRED — guard won't spawn at cold-start (enter_guard fires on room-entry, not start room).
  Fix idea: start Kid in an adjacent room and walk into the guard room, or patch Guard.* in the state.

## Takeaway
quickerSDLPoP appears faithful: NO gate/wall clip glitches found via exhaustive chamber search.
Mechanics are honest. Redirect glitch-hunting toward: (1) input-gating gaps (crouch-crawl-under etc.),
(2) guard slip-past once spawn is fixed, (3) sequence-cancel input probes, (4) RNG/routing.

## Sequence-cancel probe (landing recovery) — DONE
Snapshot Kid into the medium-land recovery on clear floor (state-at-N via runCommand s), then full-
alphabet magnet search for earliest forward motion. RESULT: Kid stuck at frame 109 for ~25 frames, x
unchanged, despite every input + rightward magnet => medium-land recovery is NON-CANCELLABLE. Code
reason: landing leaves action=5/bumped (or freefall), and control() calls release_arrows() for those
actions (miniPoP.hpp control() dispatch) — input ignored until the recovery timer expires. TAS lever:
avoid hard landings (soft-land <=1.5 tiles or grab mid-fall), can't cancel them. Method reusable for
turn/run-stop/climb cancellation.

## Guard chamber — FULLY FUNCTIONAL (corrected)
2-room build (build_two_room_guard): Kid starts room1, runs right into room2 => enter_guard fires on
room-entry and the guard SPAWNS (HP 3/3, alive). BUT the spawned guard does not engage: across skill
0/5 and with/without ceiling, the Kid runs straight through at y=118, guard action stays 1, never draws/
strikes, HP stays 3. => synthetic guard AI doesn't activate (likely can_guard_see_kid / guard_notice /
sword-draw needs more setup than guards_tile+seq+color). Combat slip-past/kill NOT yet testable. Guard
params that DO spawn: guards_tile<30, dir, x, seq_lo=255, seq_hi=255, skill, color=2 (hi-nibble=HP),
roomlinks set so the Kid enters the guard room.

## PITFALL: best.sol staleness
The driver breaks on win before saving, AND fast searches may not overwrite best.sol within the 1s save
interval. ALWAYS `rm -f /tmp/jaffar.best.sol` before each search and verify its timestamp; a batch that
reuses best.sol across cells produces false results (bit us once on the guard matrix).

## Guard AI activation (RESOLVED 2026-06-17)
The guard DOES engage — my first "broken" read was wrong (stale best.sol + [Kid] print omits sword).
Verified via Print Properties ["Can Guard See Kid","Kid Sword","Guard Sword","Guard Action","Guard
Refrac"]: a Kid that runs straight in unarmed is seen (can_guard_see_kid=2), the guard draws sword
(frame 171) and strikes (frames 151/154), KILLING it (HP 3->0). Engagement gate (check_can_guard_see_kid)
needs same room + same curr_row (guard curr_row = guards_tile/10; match the Kid's floor row) + guard
dir != none + clear line-of-sight.

### FINDING: jump past the guard (combat-skip)
Full-alphabet search reliably passes the guard by a RUNNING JUMP (frames 34-40, y dips only 118->115)
while UNARMED (sword=0): the low fast jump clears the guard's x in ~2 frames during a non-strike window,
Kid survives unharmed. Robust across skill 0 & 5, with/without ceiling (jump too low to hit it), and 3
RNG seeds. TAS lever: jump past corridor guards instead of fighting. Requires the RUNNING input set
(has LU/RU) while unarmed — the sword-drawn "on guard" set omits jumps. The guard chamber is now a
combat lab (diagnostics wired) for testing slip-past / one-shot-sheathed / push-into-pit next.

## Strange-exploit hunt (2026-06-17) — engine appears FAITHFUL
- guard fastest-kill: NO real kill found; search REWARD-HACKED (Kid leaves guard's room → guardhp_curr
  reads 0 while guardhp_max stays 3 → max "damage" reward). Lesson: guard-HP reward is gameable; for a
  real-kill search reward Guard alive>=0 (death flag) or only reward HP while Kid Room==guard room.
- room-transition-into-wall (xclip): walled room2 entry → Kid crosses boundary but in_wall pushes it
  back to x~67, does NOT clip past. Solid.
- (earlier) closed/partial gate, solid wall, fall-into landing, jump gap: all faithful/known.
VERDICT: exhaustive full-alphabet chamber search finds NO exotic clips. Only known techniques work
(jump-past-guard, crouch-under-gate, mid-air grab — all already used in the WR TAS). quickerSDLPoP is a
faithful reimplementation; frame savings here won't come from new physics glitches.
UNTESTED exotic classes (if pushing further): mirror/shadow duplication (lvl4, needs state-patched
start), loose-tile port quirk (can grab shaking loose floor if custom->loose_floor_delay>11), push-guard
-into-hazard (pit/spike/chomper), one-shot still-sheathed guard (needs non-gameable reward).

## Exotic candidates closed (2026-06-17) — (a) done
- #2 loose-tile grab quirk: DEAD CODE (can_grab needs custom->loose_floor_delay>11; default const.h=11).
- #1 mirror/shadow dup: the intended level-4 mechanic (check_mirror/jump_through_mirror), already used.
- #3 push-into-hazard: mechanism present (check_guard_fallout/spiked/chomped -> guardhp_curr=0). Known.
- #4 sheathed one-shot: hurt_by_sword: sword!=drawn -> take_hp(100) instant kill. Known mechanic.
NONE are novel. Combat-kill chamber search didn't converge = reward-landscape limit (discrete hit, no
gradient, guard counters), NOT absence. FINAL: quickerSDLPoP is faithful; no hidden physics glitches.

## (b) Routing/RNG analysis (2026-06-17)
Profiled all full-level traces (/tmp/all_traces.txt): NO significant idle waiting — longest stand
(frame15) run anywhere = 4 frames (L3 s615, L12 s822). ~40% "frozen-x" is active vertical climb/hang/
fall/combat, not waste. Backtracks (L4=7 revisits, L7/L8=4) are geometry-forced (button->gate, mirror).
=> Solutions are TIGHT; no routing waste to trim. RNG already optimized by the author's initial-seed
search; in-run churn manipulation (torch/loose-tile to dial a favorable guard-block seed) is the only
untapped lever and is marginal/case-specific (detour frames must beat saved combat frames).
OVERALL META-CONCLUSION: in this faithful engine the WR TAS is near-optimal — no glitches, optimal
movement, tight routing. Realistic further gains are marginal (frame-level), via exhaustive per-level
re-search with seed variation, not a new exploit class.
