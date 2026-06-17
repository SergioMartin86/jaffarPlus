#!/usr/bin/env bash
# Generic "search-then-reproduce" check for a GridWalker config: the engine must find a solution of
# an exact expected length, and replaying that solution with the player must reach a Win state.
#
# Usage: checkReproduce.sh <jaffar> <player> <config> <best-solution-path> <expected length> [description]
set -uo pipefail

JAFFAR="${1:?usage: checkReproduce.sh <jaffar> <player> <config> <solpath> <length> [desc]}"
PLAYER="${2:?missing player binary}"
CONFIG="${3:?missing config}"
SOL="${4:?missing best-solution path}"
EXPLEN="${5:?missing expected length}"
DESC="${6:-$CONFIG}"

export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"

solLen() { tr '\0' '\n' < "$1" | grep -c .; }
fail()   { echo "FAIL ($DESC): $1"; exit 1; }

# 1. Search (single thread, deterministic): must find a solution of the expected length.
# String matching uses bash builtins, not `... | grep -q`: under `set -o pipefail`, grep -q closing
# the pipe on first match makes the upstream writer fail with SIGPIPE, spuriously failing the test.
rm -f "$SOL"
out="$(OMP_NUM_THREADS=1 "$JAFFAR" "$CONFIG" 2>&1)"
[[ "$out" == *"Solution found"* ]] || { printf '%s\n' "$out" | tail -n 20; fail "engine did not report a solution"; }
[[ -f "$SOL" ]] || fail "best solution file not produced"
len="$(solLen "$SOL")"
if [[ "$len" -ne "$EXPLEN" ]]; then
  echo "solution: $(tr '\0' ' ' < "$SOL")"
  fail "solution length $len != expected $EXPLEN"
fi

# 2. Reproduction: replaying the found solution must reach a Win state. (The player neutralizes
#    frameskip during reproduction, so frameskip-expanded histories replay verbatim.)
repro="$(OMP_NUM_THREADS=1 "$PLAYER" "$CONFIG" "$SOL" --reproduce --disableRender --exitOnEnd --unattended 2>&1)"
[[ "$repro" == *"Game State Type: Win"* ]] || { printf '%s\n' "$repro" | tail -n 20; fail "replaying the solution did not reach a Win state"; }

echo "PASS: $DESC -- solution length $EXPLEN found and reproduced to Win"
exit 0
