#!/usr/bin/env bash
# Correctness of jaffar-player's interactive frame advance / go-back controls -- the part most prone
# to off-by-one bugs. In non-ncurses mode the player reads commands with getchar(), so the real
# interactive loop can be driven by piping a keystroke sequence to stdin (each char is one command:
# m/n=+-1, j/h=+-10, u/y=+-100, i/k=+-1000, s=quicksave, q=quit). The player prints one frame per
# loop iteration BEFORE consuming the next command, so the sequence of "Current Step #" values is the
# step shown at each point. Correctness is checked three ways:
#   1. Index arithmetic & clamping: the navigated step sequence matches the expected one exactly.
#   2. Hash consistency: the state hash shown at each step equals the authoritative hash from an
#      automatic reproduction (so navigation lands on the same states the movie passes through).
#   3. Absolute state: the 's' quicksave (the loaded emulator state) decoded at a step equals the
#      known cursor position after that many inputs -- catching a load-the-wrong-state off-by-one
#      that the displayed (precomputed) hash could not reveal.
set -uo pipefail

JAFFAR="${1:?usage: checkPlayerNavigation.sh <jaffar binary> <player binary>}"  # unused, kept uniform
PLAYER="${2:?usage: checkPlayerNavigation.sh <jaffar binary> <player binary>}"

CONFIG="$PWD/gridWalker.player.jaffar"
LINEAR="/tmp/jaffar.gw.nav.linear.sol"   # 8 moves: (0,0) -> (4,4)
SNAKE="/tmp/jaffar.gw.nav.snake.sol"     # 24 moves snaking the 5x5 grid (length > jump sizes)

export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"
export OMP_NUM_THREADS=1
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"  # intentional at-exit allocations

printf '|R|\n|R|\n|R|\n|R|\n|D|\n|D|\n|D|\n|D|\n' > "$LINEAR"
printf '|R|\n|R|\n|R|\n|R|\n|D|\n|L|\n|L|\n|L|\n|L|\n|D|\n|R|\n|R|\n|R|\n|R|\n|D|\n|L|\n|L|\n|L|\n|L|\n|D|\n|R|\n|R|\n|R|\n|R|\n' > "$SNAKE"

REPRO=(--reproduce --disableRender --exitOnEnd --unattended)  # auto-plays; reference hashes
NAV=(--disableRender)                                         # interactive; reads stdin
fail() { echo "FAIL: $1"; exit 1; }

# Space-separated "Current Step #" indices / "State Hash" values, in frame order.
idxList()  { printf '%s\n' "$1" | sed -n 's/.*Current Step #: *\([0-9]*\).*/\1/p' | tr '\n' ' ' | sed 's/ *$//'; }
hashList() { printf '%s\n' "$1" | sed -n 's/.*State Hash: *//p'                   | tr '\n' ' ' | sed 's/ *$//'; }

# Reference step->hash map, from an automatic reproduction of the given solution.
declare -A REF
buildRef() {
  local out; out="$("$PLAYER" "$1" "$2" "${REPRO[@]}" 2>&1)"
  local -a ia ha; read -ra ia <<< "$(idxList "$out")"; read -ra ha <<< "$(hashList "$out")"
  REF=(); local i; for i in "${!ia[@]}"; do REF[${ia[$i]}]="${ha[$i]}"; done
  [[ ${#REF[@]} -gt 0 ]] || fail "could not build reference hashes for $2"
}

# Drive a command string; assert the index sequence equals $4 and every frame's hash matches REF.
checkNav() {
  local cfg="$1" sol="$2" cmds="$3" expect="$4" out idxs
  out="$(printf '%s' "$cmds" | "$PLAYER" "$cfg" "$sol" "${NAV[@]}" 2>&1)"
  idxs="$(idxList "$out")"
  [[ "$idxs" == "$expect" ]] || fail "cmds '$cmds': index sequence '$idxs' != '$expect'"
  local -a ia ha; read -ra ia <<< "$idxs"; read -ra ha <<< "$(hashList "$out")"
  local i; for i in "${!ia[@]}"; do
    [[ "${ha[$i]}" == "${REF[${ia[$i]}]}" ]] || fail "cmds '$cmds' frame $i (step ${ia[$i]}): hash '${ha[$i]}' != reference '${REF[${ia[$i]}]}'"
  done
}

# Navigate with $3 (a command prefix), quicksave, and print the decoded emulator bytes.
qsBytes() {
  local d; d="$(mktemp -d)"
  ( cd "$d" && printf '%ssq' "$3" | "$PLAYER" "$1" "$2" "${NAV[@]}" >/dev/null 2>&1 )
  od -An -tu1 "$d/quicksave.state" | tr -s ' ' | sed 's/^ *//; s/ *$//'
  rm -rf "$d"
}

# ===== Single-step controls + boundary clamping, on the 8-move linear solution =====
buildRef "$CONFIG" "$LINEAR"
# m (+1) through to the end, then extra m's clamp at the last step (8).
checkNav "$CONFIG" "$LINEAR" "mmmmmmmmmmq"  "0 1 2 3 4 5 6 7 8 8 8"
# m up 5, n (-1) back down past 0, clamping at the start; symmetric path revisits identical states.
checkNav "$CONFIG" "$LINEAR" "mmmmmnnnnnnq" "0 1 2 3 4 5 4 3 2 1 0 0"
# n at the very start stays at 0.
checkNav "$CONFIG" "$LINEAR" "nq"           "0 0"

# ===== Bulk-jump controls, on the 24-move solution (long enough that +10 does not instantly clamp) =====
buildRef "$CONFIG" "$SNAKE"
checkNav "$CONFIG" "$SNAKE" "jjjq"  "0 10 20 24"   # j (+10), last jump clamps at 24
checkNav "$CONFIG" "$SNAKE" "jjhq"  "0 10 20 10"   # h (-10)
checkNav "$CONFIG" "$SNAKE" "uyq"   "0 24 0"       # u (+100) / y (-100) both clamp
checkNav "$CONFIG" "$SNAKE" "ikq"   "0 24 0"       # i (+1000) / k (-1000) both clamp

# ===== Absolute loaded state via quicksave (catches a load-wrong-step off-by-one) =====
# After k single-steps on RRRRDDDD, the cursor must be at the known position.
[[ "$(qsBytes "$CONFIG" "$LINEAR" "")"          == "0 0" ]] || fail "quicksave at step 0 != (0,0)"
[[ "$(qsBytes "$CONFIG" "$LINEAR" "mmmm")"      == "4 0" ]] || fail "quicksave at step 4 != (4,0)"
[[ "$(qsBytes "$CONFIG" "$LINEAR" "mmmmm")"     == "4 1" ]] || fail "quicksave at step 5 != (4,1)"
[[ "$(qsBytes "$CONFIG" "$LINEAR" "mmmmmmmm")"  == "4 4" ]] || fail "quicksave at step 8 != (4,4)"

# Path independence: step 10 of the snake reached by one +10 jump vs ten +1 steps must be identical
# (and equal to the known cell (0,2)).
viaJump="$(qsBytes "$CONFIG" "$SNAKE" "j")"
viaSteps="$(qsBytes "$CONFIG" "$SNAKE" "mmmmmmmmmm")"
[[ "$viaJump" == "0 2" ]]      || fail "snake step 10 (via jump) != (0,2): got '$viaJump'"
[[ "$viaJump" == "$viaSteps" ]] || fail "snake step 10 differs by path: jump '$viaJump' vs steps '$viaSteps'"

echo "PASS: navigation -- single-step/jump arithmetic, start/end clamping, hash consistency, absolute state, path independence"
exit 0
