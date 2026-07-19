# Excitebike (NES) — search notes

Notes from studying the lap-1 time attack. Practical hints — verify against current code.

## Speed model (from bank-FF disassembly)
- Horizontal velocity is a 16.8 value: `velX16 = $94 * 256 + $90`.
- **B / turbo cap the velocity at 832**; the reference TAS rides that cap and coasts.
  The A button is not used for speed. Going downhill pushes velocity to ~1544.
- The reference varies **every control on nearly every frame** — this is what makes the
  time attack hard: any single hard-wired control drops the best line by sub-pixels.

## State / hashing
- The airborne Z position (`posZ2`, `$B8`) is both a TAS distinguisher *and* an airborne
  breadth driver — it splits every airborne position into many variants, so coarsening,
  dominance, quantizing, or A* on it all tend to fail. Handle it deliberately.
- On the native/QuickerNES core the hot state is dominated by LRAM (much of it constant)
  and by the input-history block; a trie input-history and/or disabling cosmetic blocks
  (APU/sprite) are the levers to shrink the DB.

## Native engine
- A byte-exact C++ reimplementation of the physics exists (very fast). It's gated against
  the emulator over thousands of frames for fidelity before use.

## Honest status
- **No lap-1 beat was found**, and the reference was *not* proven optimal — it is
  "no beat found / search-limited." Freeing all three controls (rather than hard-wiring
  any) is required just to reproduce the reference line; report exhaustive negatives as
  search limits, not optimality.
