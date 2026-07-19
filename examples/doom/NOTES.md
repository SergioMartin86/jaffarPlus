# Doom (quickerDSDA) — search notes

Notes for running JaffarPlus on the deglobalized DSDA-Doom core. Practical hints —
verify against current code.

## Input legitimacy (important for shareable demos)
- A `.lmp` demo is 4 bytes per tic: forward, side/strafe, turn (angleturn), buttons.
  In the JaffarPlus input, `turn` is the angleturn byte.
- **`X = -127` in the strafe/side byte is TURBO.** A solution that contains it is not a
  legitimate demo. Keep it out of the allowed-input set unless you specifically want a
  turbo demo, and check any imported `.sol` for it.
- Movement caps by category: **Normal** play allows `side > 40` / SR50 only when *not*
  turning; **TAS** allows `side <= 50` always; `forward <= 50` in both. An "opt40" set is
  a TAS-mode variant. Round-trip through the `.lmp` converter to confirm a solution stays
  within your intended category.

## Search setup
- Run **nomonsters** for movement-only routes. The win condition is reaching the exit
  line/switch (E1M1: the exit switch line at the far room); on-path doors must be opened
  with USE.
- Build the search grid **turbo-free** and place waypoint magnets along the intended
  path. Momentum is the binding constraint — the player accelerates over several tics, so
  reward crossing distance, not just proximity.
- Reduced/cut-down WADs make good fast test targets; a reduced E1M1 validated well below
  the reference tic count via **dedup-hash quantization** (quantize the hashed position/
  momentum) plus a free point-magnet toward the exit.

## Core caveat
- The deglobalized core reproduces the base emulator's *behavior*, but equivalence is not
  the same as bit-fidelity in every region — run the serial equivalence tests when
  validating a rebuild, and prefer tolerance-aware comparisons.
