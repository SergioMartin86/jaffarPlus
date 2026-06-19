#!/usr/bin/env bash
# Forward anti-drift check for the manual's example configurations.
#
# Every .jaffar file shipped under docs/examples/ is a complete, copy-pasteable configuration that
# the manual references. This dry-runs each one through the engine: it parses the JSON and builds
# the full driver/emulator/game/runner/rule graph (without running a search, loading trace files,
# or touching NUMA), so a key that is renamed or removed in the engine breaks the example here --
# turning silent doc rot into a CI failure. The docs examples use the ROM-free TestEmulator, so this
# needs no game data.
#
# Usage: checkDocsExamples.sh <jaffar binary>
set -uo pipefail

JAFFAR="${1:?usage: checkDocsExamples.sh <jaffar binary>}"

# Resolve docs/examples relative to this script (tests/ -> ../docs/examples).
scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
examplesDir="${scriptDir}/../docs/examples"

shopt -s nullglob
examples=( "${examplesDir}"/*.jaffar )
if [[ ${#examples[@]} -eq 0 ]]; then
  echo "FAIL: no example configurations found under docs/examples/"
  exit 1
fi

fail=0
for cfg in "${examples[@]}"; do
  if "${JAFFAR}" "${cfg}" --dryRun >/dev/null 2>&1; then
    echo "ok:   $(basename "${cfg}")"
  else
    echo "FAIL: $(basename "${cfg}") did not pass --dryRun"
    # Re-run showing output so the CI log explains why.
    "${JAFFAR}" "${cfg}" --dryRun 2>&1 | tail -n 5
    fail=1
  fi
done

if [[ ${fail} -ne 0 ]]; then
  echo "FAIL: one or more docs example configurations failed to validate"
  exit 1
fi

echo "PASS: all ${#examples[@]} docs example configuration(s) validated via --dryRun"
exit 0
