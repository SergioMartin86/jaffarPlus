# Super Mario Bros (NES) — search notes

Notes for running JaffarPlus searches on SMB, gathered while studying the warped
any% route. These are practical hints, not a spec — verify against current code.

## Engine: QuickerNES vs SMBC
- The **SMBC** core (a from-scratch SMB engine) is **gameplay-faithful** to the NES
  ROM on QuickerNES: 1-1 TAS solutions transfer between them, and hashes match except
  in the **sound RAM** region. Useful for cross-validation and for a much faster core.
- **Gotcha when injecting NES RAM into SMBC:** SMBC relocates several ROM tables, so a
  blind RAM copy corrupts pointer state. Inject the NES work RAM but **preserve SMBC's
  native `$E7`–`$EA` pointers**.
- Cross-validate two cores with `player --dumpHashes` / `--dumpRam`: diffing the two
  per-step dumps pinpoints the first frame (and RAM address) at which they diverge.

## Hashing / pruning
- **Keep sub-pixel position in the hash.** Coarsening it merges physically distinct
  states and quietly loses frames.
- Many status/animation/timer bytes are safe to drop from the hash (a couple dozen were
  identified). A boolean "jump/swim timer" and a "swimming flag" property discriminate
  the states that actually matter without hashing the raw counters.
- The **allowed-input set can be made context-adaptive** (e.g. restrict inputs while
  swimming vs. on ground) to shrink the branching factor.

## Route knowledge (world-8 warped any%)
- The warped any% WR (HappyLee, ~4:57) was studied closely. The binding structure is
  the **21-frame rule** (level-completion timing quantum) plus per-level slack: several
  levels (e.g. 8-2, 8-4) are frame-rule-tight, and the warp forces a world-start.
- Conclusion of that study: beating the mature WR would require slack that the rule
  structure doesn't provide (or an ACE approach). Recorded here so others don't
  re-derive it from scratch — but treat "optimal" as "no beat found under these
  assumptions," not proven.
- A full-game TAS makes a good **segment-search seed**: use its input prefix as the
  initial sequence and its trajectory as trace-magnet waypoints, then re-search a
  segment with a per-segment reward.
