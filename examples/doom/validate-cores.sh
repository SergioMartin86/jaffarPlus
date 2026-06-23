#!/usr/bin/env bash
# Per-step base-vs-new(quicker) DSDA core validation for a Jaffar Doom replay.
#
# Replays the same solution through both the base (pristine, source/base) and the
# new/quicker (deglobalized + trimmed, source/new) cores, dumping each core's
# per-step game-state hash, then diffs them. Identical at every step == the
# quicker core is gameplay-faithful to base for this run.
#
# Usage: ./validate-cores.sh <config.jaffar> <solution.sol>
#   Use the ordinary search config -- the base player auto-sizes its state buffer
#   on rendering builds, so no special large-State-Size config is needed.
#
# Requires: build-dsda (new core) and build-dsda-base (-DdoomCore=base) built.
set -euo pipefail

cfg="${1:?usage: validate-cores.sh <config.jaffar> <solution.sol>}"
sol="${2:?usage: validate-cores.sh <config.jaffar> <solution.sol>}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
new_player="$root/build-dsda/jaffar-player"
base_player="$root/build-dsda-base/jaffar-player"

for p in "$new_player" "$base_player"; do
  [ -x "$p" ] || { echo "missing player: $p (build it first)"; exit 2; }
done

common=(--reproduce --unattended --exitOnEnd --disableRender)
"$new_player"  "$cfg" "$sol" "${common[@]}" --dumpHashes /tmp/cores.new.hashes  >/dev/null 2>&1
"$base_player" "$cfg" "$sol" "${common[@]}" --dumpHashes /tmp/cores.base.hashes >/dev/null 2>&1

steps=$(wc -l < /tmp/cores.new.hashes)
if diff -q /tmp/cores.new.hashes /tmp/cores.base.hashes >/dev/null; then
  echo "OK: new == base at every step ($steps steps)"
else
  echo "DIVERGENCE (new vs base) -- first differing steps (step  new  base):"
  diff <(cat /tmp/cores.new.hashes) <(cat /tmp/cores.base.hashes) | head -20
  exit 1
fi
