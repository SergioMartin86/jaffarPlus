#!/usr/bin/env bash
# Correctness test for the TestEmulator / GridWalker harness.
#
# Drives the real JaffarPlus engine on a designed puzzle whose optimal solution is
# known to be exactly 8 moves (from (0,0) to (4,4) on a 5x5 grid). Asserts that:
#   1. The engine reports a solution was found.
#   2. The best solution found is exactly 8 inputs long (i.e. provably optimal: BFS
#      must return a shortest path, never a longer one).
#   3. Replaying that solution through the player reaches a Win state.
set -euo pipefail

JAFFAR="${1:?usage: checkOptimal.sh <jaffar binary> <player binary>}"
PLAYER="${2:?usage: checkOptimal.sh <jaffar binary> <player binary>}"

CONFIG="gridWalker.jaffar"
SOL="/tmp/jaffar.gridWalker.best.sol"

# Single-threaded keeps the run fully deterministic and avoids multi-NUMA setup;
# optimality (solution length) is independent of thread count regardless.
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-1}"
export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"

rm -f "$SOL"

# 1. Run the search
runOutput="$("$JAFFAR" "$CONFIG" 2>&1)"
if ! echo "$runOutput" | grep -q "Solution found"; then
  echo "FAIL: engine did not report a solution"
  echo "$runOutput" | tail -n 20
  exit 1
fi

# 2. Assert optimality: the best solution must be exactly 8 inputs
if [[ ! -f "$SOL" ]]; then
  echo "FAIL: best solution file '$SOL' was not produced"
  exit 1
fi
solutionLength="$(tr '\0' '\n' < "$SOL" | grep -c .)"
if [[ "$solutionLength" -ne 8 ]]; then
  echo "FAIL: best solution length is $solutionLength, expected the optimal 8"
  tr '\0' '\n' < "$SOL"
  exit 1
fi

# 3. Replay the solution and assert it reaches a Win
reproOutput="$("$PLAYER" "$CONFIG" "$SOL" --reproduce --disableRender --exitOnEnd --unattended 2>&1)"
if ! echo "$reproOutput" | grep -q "Game State Type: Win"; then
  echo "FAIL: replaying the solution did not reach a Win state"
  echo "$reproOutput" | tail -n 20
  exit 1
fi

echo "PASS: engine found and reproduced the optimal 8-move solution"
exit 0
