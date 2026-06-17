#!/usr/bin/env bash
# Correctness of jaffar-player's diagnostic reports, checked against crafted solutions with a known,
# exactly-predictable answer:
#   - "Not Allowed Input Steps": inputs in the solution the engine would NOT consider at that frame
#     (not in the allowed input set for that state).
#   - "Repeated Hash Steps":     states whose hash already appeared earlier, i.e. the duplicates the
#     engine would have pruned on encountering them.
#   - "First Win/Fail Step":     the number of inputs at which the solution first reaches a win/fail
#     state (or "none"), surfaced in the --printFinalState summary.
# Both the per-step lists (frame info) and the aggregate counts/steps (--printFinalState) are verified,
# and a clean optimal solution is used as a negative control (both reports empty).
set -uo pipefail

JAFFAR="${1:?usage: checkPlayerReports.sh <jaffar binary> <player binary>}"
PLAYER="${2:?usage: checkPlayerReports.sh <jaffar binary> <player binary>}"

# UDLR config for the repeated-state case; right-only config for the not-allowed case (so the
# not-allowed input is a real move, not a no-op -- a no-op would itself create a repeated state).
# All solutions are crafted here (private /tmp paths) so this test drives only the player and never
# races the other tests that run `jaffar gridWalker.player.jaffar` concurrently.
CONFIG="$PWD/gridWalker.player.jaffar"
CONFIG_RIGHT="$PWD/gridWalker.rightonly.jaffar"
CONFIG_DETOUR="$PWD/gridWalker.detour.jaffar"   # has a fail rule (only the config is read here)
REP_SOL="/tmp/jaffar.gw.reports.repeated.sol"
NA_SOL="/tmp/jaffar.gw.reports.notallowed.sol"
CLEAN_SOL="/tmp/jaffar.gw.reports.clean.sol"
FAIL_SOL="/tmp/jaffar.gw.reports.fail.sol"

export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB:-10}"
export JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB="${JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB:-100}"
export OMP_NUM_THREADS=1
# These tools intentionally leave top-level objects allocated at exit; disable LeakSanitizer so a
# coverage/ASAN build does not abort (matches the CI sanitizers job policy). Harmless for release.
export ASAN_OPTIONS="detect_leaks=0${ASAN_OPTIONS:+:$ASAN_OPTIONS}"

PLAY_FLAGS=(--reproduce --disableRender --exitOnEnd --unattended --printFinalState)
fail() { echo "FAIL: $1"; exit 1; }

# Extracts an aggregate count line ("<label>: N") from a captured run.
countOf() { printf '%s\n' "$2" | sed -n "s/.*$1: *//p" | tr -d '[:space:]'; }
# Extracts the step list inside the first "<label>: ... [ ... ]" line, as space-separated indices.
stepsOf() { printf '%s\n' "$2" | grep -m1 "$1:" | sed -n 's/.*\[\(.*\)\].*/\1/p' | tr -s ' ' | sed 's/^ *//; s/ *$//'; }
# Extracts the final state type from the --printFinalState summary.
finalTypeOf() { printf '%s\n' "$1" | sed -n 's/.*Final State Type: *//p' | tr -d '[:space:]'; }

# --- Case 1: repeated states. R,L,R,L,U returns to the origin twice and to (1,0) once, so steps
#     2, 3 and 4 each duplicate an earlier state; nothing is out of the allowed set. ---
printf '|R|\n|L|\n|R|\n|L|\n|U|\n' > "$REP_SOL"
outR="$("$PLAYER" "$CONFIG" "$REP_SOL" "${PLAY_FLAGS[@]}" 2>&1)"
[[ "$(countOf 'Repeated State Count' "$outR")" == "3" ]] || fail "repeated-state count != 3 (got '$(countOf 'Repeated State Count' "$outR")')"
[[ "$(stepsOf 'Repeated Hash Steps' "$outR")" == "2 3 4" ]] || fail "repeated-hash steps != '2 3 4' (got '$(stepsOf 'Repeated Hash Steps' "$outR")')"
[[ "$(countOf 'Not Allowed Input Count' "$outR")" == "0" ]] || fail "repeated-state case should have 0 not-allowed inputs"
# This wandering solution never wins or fails.
[[ "$(countOf 'First Win Step' "$outR")" == "none" ]] || fail "non-winning solution reported a win step (got '$(countOf 'First Win Step' "$outR")')"

# --- Case 2: not-allowed inputs. With a right-only allowed set, "|D|" is parseable and moves the
#     cursor (so no spurious repeat) but is not allowed. The solution R,D,R,D moves strictly into new
#     cells, with not-allowed inputs at steps 1 and 3 and no repeated states. ---
printf '|R|\n|D|\n|R|\n|D|\n' > "$NA_SOL"
outN="$("$PLAYER" "$CONFIG_RIGHT" "$NA_SOL" "${PLAY_FLAGS[@]}" 2>&1)"
[[ "$(countOf 'Not Allowed Input Count' "$outN")" == "2" ]] || fail "not-allowed count != 2 (got '$(countOf 'Not Allowed Input Count' "$outN")')"
[[ "$(stepsOf 'Not Allowed Input Steps' "$outN")" == "1 3" ]] || fail "not-allowed steps != '1 3' (got '$(stepsOf 'Not Allowed Input Steps' "$outN")')"
[[ "$(countOf 'Repeated State Count' "$outN")" == "0" ]] || fail "not-allowed case should have 0 repeated states"

# --- Negative control: a clean monotone winning path (RRRRDDDD reaches the (4,4) goal) is fully
#     legal under {U,D,L,R} and visits only new cells, so both reports must be empty. ---
printf '|R|\n|R|\n|R|\n|R|\n|D|\n|D|\n|D|\n|D|\n' > "$CLEAN_SOL"
outW="$("$PLAYER" "$CONFIG" "$CLEAN_SOL" "${PLAY_FLAGS[@]}" 2>&1)"
[[ "$(finalTypeOf "$outW")" == "Win" ]] || fail "clean solution did not reach Win (got '$(finalTypeOf "$outW")')"
[[ "$(countOf 'Not Allowed Input Count' "$outW")" == "0" ]] || fail "clean solution reported not-allowed inputs"
[[ "$(countOf 'Repeated State Count' "$outW")" == "0" ]] || fail "clean solution reported repeated states"
# The win happens exactly at the last (8th) input, and the solution never fails.
[[ "$(countOf 'First Win Step' "$outW")" == "8" ]] || fail "first-win step != 8 (got '$(countOf 'First Win Step' "$outW")')"
[[ "$(countOf 'First Fail Step' "$outW")" == "none" ]] || fail "clean winning solution reported a fail step"

# --- First-fail step: on the detour config (which has a fail rule at column 0, rows 1-3), a single
#     "|D|" from the start moves to (0,1) -- a fail -- after exactly 1 input. ---
printf '|D|\n' > "$FAIL_SOL"
outF="$("$PLAYER" "$CONFIG_DETOUR" "$FAIL_SOL" "${PLAY_FLAGS[@]}" 2>&1)"
[[ "$(finalTypeOf "$outF")" == "Fail" ]] || fail "detour |D| did not reach Fail (got '$(finalTypeOf "$outF")')"
[[ "$(countOf 'First Fail Step' "$outF")" == "1" ]] || fail "first-fail step != 1 (got '$(countOf 'First Fail Step' "$outF")')"
[[ "$(countOf 'First Win Step' "$outF")" == "none" ]] || fail "failing solution reported a win step"

echo "PASS: report correctness -- repeated [2 3 4]; not-allowed [1 3]; win step 8 / fail step 1 / none cases"
exit 0
