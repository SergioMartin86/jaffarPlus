#!/usr/bin/env bash
# Parametrized GridWalker puzzle runner for boundary-condition tests.
#
# Usage: runPuzzle.sh <jaffar binary> <config> <best-solution-path> <win|nowin> [expected length]
#   win   [N] : the engine must find a solution; if N is given, its length must equal N.
#   nowin     : the engine must NOT find a solution (it exhausts the space or hits Max Steps).
set -uo pipefail

JAFFAR="${1:?usage: runPuzzle.sh <jaffar> <config> <solpath> <win|nowin> [length]}"
CONFIG="${2:?missing config}"
SOL="${3:?missing solution path}"
EXPECT="${4:?missing win|nowin}"
EXPLEN="${5:-}"

export OMP_NUM_THREADS="${OMP_NUM_THREADS:-1}"
export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"

rm -f "$SOL"
out="$("$JAFFAR" "$CONFIG" 2>&1)"

# Substring matching via bash builtin, not `... | grep -q`: under `set -o pipefail` grep -q closing
# the pipe on first match makes the upstream writer fail with SIGPIPE, spuriously failing the test.
if [[ "$EXPECT" == "win" ]]; then
  [[ "$out" == *"Solution found"* ]] || { printf '%s\n' "$out" | tail -n 20; echo "FAIL: $CONFIG expected a solution"; exit 1; }
  if [[ -n "$EXPLEN" ]]; then
    len="$(tr '\0' '\n' < "$SOL" | grep -c .)"
    if [[ "$len" -ne "$EXPLEN" ]]; then
      echo "solution: $(tr '\0' ' ' < "$SOL")"
      echo "FAIL: $CONFIG solution length $len != expected $EXPLEN"
      exit 1
    fi
  fi
  echo "PASS: $CONFIG found a solution (length ${EXPLEN:-any})"
else
  if [[ "$out" == *"Solution found"* ]]; then
    printf '%s\n' "$out" | tail -n 20
    echo "FAIL: $CONFIG unexpectedly found a solution"
    exit 1
  fi
  echo "PASS: $CONFIG produced no solution, as expected"
fi
exit 0
