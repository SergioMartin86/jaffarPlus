# Rockman / Mega Man 1 (NES) — search notes

Notes for the ending-skip search. Practical hints — verify against current code.

## The ending glitch (arbitrary code execution)
- There is an ACE path via a delayed object: a specific object slot's handler executes
  from `$0600`, which reaches an indirect jump `JMP ($0018)` landing in a stage-clear
  routine (around `$C460`). This is the "ending skip."
- The completion signal used as the win is the game-state byte **`$0031 == 0x0B`**.
- This behavior is **NesHawk-only** in practice — the exact timing/state that arms it does
  not reproduce on every core. Use the ground-truth core when validating it.

## Level-start setup phase
- Triggering the glitch earlier requires arming a specific state during the level-start
  setup phase. A reward built for that phase should accumulate the *latched* progress that
  actually matters (e.g. the half-enemies-processed count `$06F0` moving 2→7, shots fired,
  and a capped rightward push), rather than an instantaneous "arming" term.
- **Lesson:** the latched backbone must dominate the reward; an instantaneous
  "arming-readiness" term is only a local refiner and will not carry the search on its own.
- Earlier-trigger variants remained search-limited / frame-tight; report them as such, not
  as optimal.
