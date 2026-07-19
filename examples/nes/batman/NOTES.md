# Batman (NES) — search notes

Notes for the boss-rush segments (preboss / boss / boss2). Practical hints — verify
against current code.

## Dedup hash via a causal-state allowlist
- Rather than hashing the whole RAM (or a hand-picked subset by trial and error), hash an
  **allowlist of bytes known to be causal** — the addresses that actually influence future
  gameplay. This dramatically shrinks the deduplicated state space.
- The allowlist was validated by checking **per-depth best-reward parity** against a
  full-hash baseline at multiple checkpoints: if the best reward per depth is unchanged,
  the dropped bytes were non-causal. Do this validation before trusting an allowlist for a
  real search.
- A next refinement is to carve the object table by object *type* so only the relevant
  fields are hashed. Deep-validate before committing to a search.

## Boss segment
- The boss reference's last meaningful input is around frame 408; the win is the boss HP
  reaching 255 (its death/underflow value).
- `getAdditionalAllowedInputs` discriminates on the player-state byte (`$a7 & 0xFC`) to
  keep the allowed-input set context-appropriate.

## General
- Size the state DB from `Max Store Size × Max Store Count`; state + hash must fit RAM.
- A companion 6502 disassembly of the relevant banks makes the causal-byte analysis far
  easier than guessing.
