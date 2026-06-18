#!/usr/bin/env bash
# Core correctness test for the TestEmulator / GridWalker harness. Drives the real engine on a
# puzzle whose optimal solution is known to be exactly 8 moves ((0,0)->(4,4) on a 5x5 grid) and
# checks four properties of the main execution path:
#   1. Optimality:   the best solution found is exactly 8 inputs (BFS must return a shortest path).
#   2. Save Solution: the "Trigger Save Solution" rule action wrote the winning solution file.
#   3. Reproduction:  replaying the best solution reaches a Win state.
#   4. Determinism:   a multi-threaded run also finds an (8-move) optimal solution.
set -uo pipefail

JAFFAR="${1:?usage: checkOptimal.sh <jaffar binary> <player binary>}"
PLAYER="${2:?usage: checkOptimal.sh <jaffar binary> <player binary>}"

CONFIG="gridWalker.jaffar"
SOL="/tmp/jaffar.gridWalker.best.sol"
WIN_SOL="/tmp/jaffar.gridWalker.win.sol"

export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"

solLen() { tr '\0' '\n' < "$1" | grep -c .; }
fail()   { echo "FAIL: $1"; exit 1; }

# 1-3. Optimality + save-solution + reproduction (single thread)
# String checks use bash builtins, not `... | grep -q`: under `set -o pipefail`, grep -q closing
# the pipe on first match makes the upstream writer fail with SIGPIPE, which pipefail then reports
# as a spurious failure on large output.
rm -f "$SOL" "$WIN_SOL"
out="$(OMP_NUM_THREADS=1 "$JAFFAR" "$CONFIG" 2>&1)"
[[ "$out" == *"Solution found"* ]] || { printf '%s\n' "$out" | tail -n 20; fail "engine did not report a solution"; }
[[ -f "$SOL" ]] || fail "best solution file not produced"
[[ "$(solLen "$SOL")" -eq 8 ]] || fail "best solution length $(solLen "$SOL") != optimal 8"

# The "Trigger Save Solution" action (on the checkpoint rule) must have written a non-empty
# solution file for the checkpoint state it fired on.
[[ -f "$WIN_SOL" ]] || fail "Trigger Save Solution did not write $WIN_SOL"
[[ "$(solLen "$WIN_SOL")" -ge 1 ]] || fail "saved checkpoint solution file is empty"

repro="$(OMP_NUM_THREADS=1 "$PLAYER" "$CONFIG" "$SOL" --reproduce --disableRender --exitOnEnd --unattended 2>&1)"
[[ "$repro" == *"Game State Type: Win"* ]] || { printf '%s\n' "$repro" | tail -n 20; fail "replaying the solution did not reach a Win state"; }

# 4. Determinism: a multi-threaded search must also find an optimal (length-8) solution. Different
#    thread counts may surface a different shortest path, but BFS guarantees the same optimal depth.
#    The engine handles threads covering any subset of NUMA domains, so this runs on any host.
rm -f "$SOL"
outN="$(OMP_NUM_THREADS="${DET_THREADS:-4}" "$JAFFAR" "$CONFIG" 2>&1)"
[[ "$outN" == *"Solution found"* ]] || { printf '%s\n' "$outN" | tail -n 20; fail "multi-threaded run found no solution"; }
[[ "$(solLen "$SOL")" -eq 8 ]] || fail "multi-threaded solution length $(solLen "$SOL") != optimal 8"

echo "PASS: optimal(8) + save-solution + reproduction + determinism"
exit 0
