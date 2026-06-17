#!/usr/bin/env bash
# Core correctness test for the TestEmulator / GridWalker harness. Drives the real engine on a
# puzzle whose optimal solution is known to be exactly 8 moves ((0,0)->(4,4) on a 5x5 grid) and
# checks four properties of the main execution path:
#   1. Optimality:   the best solution found is exactly 8 inputs (BFS must return a shortest path).
#   2. Save Solution: the "Trigger Save Solution" rule action wrote the winning solution file.
#   3. Reproduction:  replaying the best solution reaches a Win state.
#   4. Determinism:   a multi-threaded run also finds an (8-move) optimal solution.
#   5. Equivalence:   differential-compression-on and -off produce the identical solution.
set -uo pipefail

JAFFAR="${1:?usage: checkOptimal.sh <jaffar binary> <player binary>}"
PLAYER="${2:?usage: checkOptimal.sh <jaffar binary> <player binary>}"

CONFIG_ON="gridWalker.jaffar"        # differential compression enabled
CONFIG_OFF="gridWalker.nodiff.jaffar" # identical puzzle, differential compression disabled
SOL_ON="/tmp/jaffar.gridWalker.best.sol"
WIN_SOL="/tmp/jaffar.gridWalker.win.sol"
SOL_OFF="/tmp/jaffar.gw.nodiff.best.sol"

export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"

solLen() { tr '\0' '\n' < "$1" | grep -c .; }
fail()   { echo "FAIL: $1"; exit 1; }

# 1-3. Optimality + save-solution + reproduction (differential compression on, single thread)
rm -f "$SOL_ON" "$WIN_SOL"
out="$(OMP_NUM_THREADS=1 "$JAFFAR" "$CONFIG_ON" 2>&1)"
echo "$out" | grep -q "Solution found" || { echo "$out" | tail -n 20; fail "engine did not report a solution (diff on)"; }
[[ -f "$SOL_ON" ]] || fail "best solution file not produced"
[[ "$(solLen "$SOL_ON")" -eq 8 ]] || fail "best solution length $(solLen "$SOL_ON") != optimal 8"
cp "$SOL_ON" /tmp/gw.sol.on1

# The "Trigger Save Solution" action (on the checkpoint rule) must have written a non-empty
# solution file for the checkpoint state it fired on.
[[ -f "$WIN_SOL" ]] || fail "Trigger Save Solution did not write $WIN_SOL"
[[ "$(solLen "$WIN_SOL")" -ge 1 ]] || fail "saved checkpoint solution file is empty"

repro="$(OMP_NUM_THREADS=1 "$PLAYER" "$CONFIG_ON" "$SOL_ON" --reproduce --disableRender --exitOnEnd --unattended 2>&1)"
echo "$repro" | grep -q "Game State Type: Win" || { echo "$repro" | tail -n 20; fail "replaying the solution did not reach a Win state"; }

# 4. Determinism: a multi-threaded search must also find an optimal (length-8) solution. Different
#    thread counts may surface a different shortest path, but BFS guarantees the same optimal depth.
#    JaffarPlus's NUMA worker model requires threads to cover all NUMA domains, so a multi-threaded
#    run is only valid on a single-NUMA host. CI runners are single-NUMA; on a multi-NUMA developer
#    machine this part is skipped (the rest of the test still runs single-threaded).
numaNodes=1
if command -v numactl >/dev/null 2>&1; then
  numaNodes="$(numactl --hardware 2>/dev/null | grep -cE '^node [0-9]+ cpus:')"
  [[ "$numaNodes" -ge 1 ]] || numaNodes=1
fi
if [[ "$numaNodes" -le 1 ]]; then
  rm -f "$SOL_ON"
  outN="$(OMP_NUM_THREADS="${DET_THREADS:-4}" "$JAFFAR" "$CONFIG_ON" 2>&1)"
  echo "$outN" | grep -q "Solution found" || { echo "$outN" | tail -n 20; fail "multi-threaded run found no solution"; }
  [[ "$(solLen "$SOL_ON")" -eq 8 ]] || fail "multi-threaded solution length $(solLen "$SOL_ON") != optimal 8"
  echo "  determinism: multi-threaded run also found an optimal 8-move solution"
else
  echo "  determinism: skipped multi-threaded check on multi-NUMA host ($numaNodes nodes); CI is single-NUMA"
fi

# 5. Equivalence: differential-off search (single thread, deterministic) must find the identical
#    solution to differential-on, proving compression does not alter the search result.
rm -f "$SOL_OFF"
outOff="$(OMP_NUM_THREADS=1 "$JAFFAR" "$CONFIG_OFF" 2>&1)"
echo "$outOff" | grep -q "Solution found" || { echo "$outOff" | tail -n 20; fail "engine did not report a solution (diff off)"; }
[[ "$(solLen "$SOL_OFF")" -eq 8 ]] || fail "diff-off solution length $(solLen "$SOL_OFF") != optimal 8"
if ! cmp -s /tmp/gw.sol.on1 "$SOL_OFF"; then
  echo "on : $(tr '\0' ' ' < /tmp/gw.sol.on1)"
  echo "off: $(tr '\0' ' ' < "$SOL_OFF")"
  fail "differential-on and differential-off produced different solutions"
fi

echo "PASS: optimal(8) + save-solution + reproduction + determinism + differential on==off"
exit 0
