#!/usr/bin/env bash
# Tests the generational (fixed-capacity, single-lookup) hash database on the GridWalker maze
# (optimal solution = 8 moves):
#   1. Ample budget : no eviction occurs; validates that the generation-tagged set deduplicates
#                     correctly (custom hash/equality over the packed value) and finds the optimum.
#   2. Tiny budget  : the live-entry target is forced near zero, so nearly every generation is
#                     evicted and states are continually re-explored; the search must still find the
#                     optimal solution (eviction must not break correctness), and the run must report
#                     it used the generational scheme.
set -uo pipefail

JAFFAR="${1:?usage: checkGenerational.sh <jaffar binary>}"

CONFIG="gridWalker.generational.jaffar"
SOL="/tmp/jaffar.gw.generational.best.sol"

export OMP_NUM_THREADS="${OMP_NUM_THREADS:-1}"
export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-50}"

solLen() { tr '\0' '\n' < "$1" | grep -c .; }
fail()   { echo "FAIL: $1"; exit 1; }

# 1. Ample budget (no eviction): generational dedup must find the optimal 8-move solution.
unset JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB
rm -f "$SOL"
out="$("$JAFFAR" "$CONFIG" 2>&1)"
[[ "$out" == *"Solution found"* ]] || { printf '%s\n' "$out" | tail -n 20; fail "generational (ample): no solution"; }
[[ "$(solLen "$SOL")" -eq 8 ]] || fail "generational (ample): solution length $(solLen "$SOL") != optimal 8"
[[ "$out" == *"Generational"* ]] || fail "generational (ample): run did not report the generational scheme"

# 2. Tiny budget (heavy eviction): must still find the optimal solution.
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=0
rm -f "$SOL"
outEvict="$("$JAFFAR" "$CONFIG" 2>&1)"
[[ "$outEvict" == *"Solution found"* ]] || { printf '%s\n' "$outEvict" | tail -n 20; fail "generational (tiny budget): no solution under heavy eviction"; }
[[ "$(solLen "$SOL")" -eq 8 ]] || fail "generational (tiny budget): solution length $(solLen "$SOL") != optimal 8"

# 3. Larger maze (50x50, optimal = 98 moves) with a budget small enough that genuine reserved-shard
#    eviction occurs mid-search (target well below the ~2500 reachable cells). The optimal solution
#    must still be found, i.e. evicted cells are correctly re-explored.
BIG_SOL="/tmp/jaffar.gw.biggrid.best.sol"
unset JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB
rm -f "$BIG_SOL"
outBig="$("$JAFFAR" gridWalker.biggrid.jaffar 2>&1)"
[[ "$outBig" == *"Solution found"* ]] || { printf '%s\n' "$outBig" | tail -n 20; fail "generational (50x50): no solution"; }
[[ "$(solLen "$BIG_SOL")" -eq 98 ]] || fail "generational (50x50): solution length $(solLen "$BIG_SOL") != optimal 98"

echo "PASS: generational hash DB dedups correctly, stays optimal under heavy eviction, and solves the 50x50 maze optimally under reserved-shard eviction"
exit 0
