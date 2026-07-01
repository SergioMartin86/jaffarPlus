#!/usr/bin/env bash
# Coverage + correctness for jaffar-player's --dumpReward: it writes the per-step game reward (one
# value per line) for a replayed solution, suitable as a driver "Reference Reward Floor" trace.
#
# GridWalker's reward is -(Manhattan distance to the goal), and reaching the (4,4) goal adds +100000
# (the win rule), so every step's reward is exactly predictable. We assert the full per-step trace for
# a winning route and a non-winning route. NOTE: step 0 is the root (post-initial-sequence) state whose
# stored reward is 0 (it is only computed once a move is applied); steps >= 1 carry the computed reward.
set -uo pipefail

PLAYER="${2:?usage: checkPlayerDumpReward.sh <jaffar binary> <player binary>}"
CONFIG="$PWD/gridWalker.player.jaffar"          # {U,D,L,R} allowed set, goal (4,4)
WIN_SOL="/tmp/jaffar.gw.dumpreward.win.sol"
NOWIN_SOL="/tmp/jaffar.gw.dumpreward.nowin.sol"
WIN_OUT="/tmp/jaffar.gw.dumpreward.win.txt"
NOWIN_OUT="/tmp/jaffar.gw.dumpreward.nowin.txt"

export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"
export OMP_NUM_THREADS=1
# These tools intentionally leave top-level objects allocated at exit; disable LeakSanitizer so a
# coverage/ASAN build does not abort (matches the other player tests).
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

PLAY_FLAGS=(--reproduce --disableRender --exitOnEnd --unattended)
fail() { echo "FAIL: $1"; exit 1; }

# Reads the dumped reward file into a single space-separated line for easy comparison.
trace() { tr '\n' ' ' < "$1" | sed 's/ *$//'; }

# --- Case 1: winning route RRRRDDDD reaches (4,4). Distances 8..1 give rewards -7..-1 for steps 1-8
#     up to the goal step, where the win rule adds +100000. Step 0 (root) is the stored 0. ---
printf '|R|\n|R|\n|R|\n|R|\n|D|\n|D|\n|D|\n|D|\n' > "$WIN_SOL"
"$PLAYER" "$CONFIG" "$WIN_SOL" "${PLAY_FLAGS[@]}" --dumpReward "$WIN_OUT" >/dev/null 2>&1 \
  || fail "player exited non-zero on --dumpReward (winning route)"
[[ -s "$WIN_OUT" ]] || fail "--dumpReward produced no file"
[[ "$(wc -l < "$WIN_OUT")" == "9" ]] || fail "winning trace line count != 9 (got $(wc -l < "$WIN_OUT"))"
EXP_WIN="0 -7 -6 -5 -4 -3 -2 -1 100000"
[[ "$(trace "$WIN_OUT")" == "$EXP_WIN" ]] || fail "winning reward trace mismatch: got [$(trace "$WIN_OUT")] want [$EXP_WIN]"

# --- Case 2: non-winning route RRRR stops at (4,0) (distance 4): rewards stay negative, no +100000. ---
printf '|R|\n|R|\n|R|\n|R|\n' > "$NOWIN_SOL"
"$PLAYER" "$CONFIG" "$NOWIN_SOL" "${PLAY_FLAGS[@]}" --dumpReward "$NOWIN_OUT" >/dev/null 2>&1 \
  || fail "player exited non-zero on --dumpReward (non-winning route)"
[[ "$(wc -l < "$NOWIN_OUT")" == "5" ]] || fail "non-winning trace line count != 5 (got $(wc -l < "$NOWIN_OUT"))"
EXP_NOWIN="0 -7 -6 -5 -4"
[[ "$(trace "$NOWIN_OUT")" == "$EXP_NOWIN" ]] || fail "non-winning reward trace mismatch: got [$(trace "$NOWIN_OUT")] want [$EXP_NOWIN]"

echo "PASS: --dumpReward trace win [$EXP_WIN] ; nowin [$EXP_NOWIN]"
exit 0
