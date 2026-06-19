# 3. Rules, Conditions & Rewards

This is the chapter that turns JaffarPlus from "explores states" into "solves *your* problem". The
engine always expands the most promising state first; **you** define what "promising" means through
a *reward*, and what counts as winning, failing, or worth checkpointing through *rules*. Get this
right and a search that would never terminate becomes one that finds an optimal solution in seconds.

> Illustrations in this chapter (rendered Prince of Persia frames showing the effect of each lever)
> are added in a later pass; the mechanics below are complete without them.

- [The mental model](#the-mental-model)
- [Properties: naming game memory](#properties-naming-game-memory)
- [Conditions](#conditions)
- [Rules](#rules)
- [Core actions](#core-actions)
- [Reward magnets](#reward-magnets)
- [Checkpoints and tolerance](#checkpoints-and-tolerance)
- [Worked example: GridWalker](#worked-example-gridwalker)
- [Design recipes](#design-recipes)

## The mental model

Every state the engine reaches has a single **reward** value (a `float`). Best-first search keeps a
frontier of states and always expands the one with the highest reward next. So:

- **Reward is a compass, not a score.** Its absolute value is irrelevant; only the *ordering* it
  induces matters. A state with higher reward gets explored sooner.
- **Reward has two sources.** Discrete bumps from rule **actions** (`Add Reward`), and continuous
  gradients from **magnets** (game-specific actions that reward being close to a target). The two
  are summed.
- **Rules also gate the search.** A rule can mark a state as a **win** (a solution), a **fail**
  (pruned, never expanded), or a **checkpoint** (a milestone that resets deduplication tolerance).

A search succeeds when a state triggers a `Trigger Win`. It is your reward design that makes the
engine *walk toward* that win instead of wandering the entire state space.

## Properties: naming game memory

A **property** is a named, typed view into the game's memory — `Pos X`, `Kid HP`, `Guard Pos Y`,
and so on. Properties are registered by each game's C++ code (not in the config); the config refers
to them by name in three places:

- `Game Configuration` > `Hash Properties` — which properties define a state's *identity*.
- `Game Configuration` > `Print Properties` — which to show in status output.
- Inside **conditions** and **magnets** — to test or target a value.

To list a game's available properties, read its header under `games/` or see
[Adding a Game](05-adding-a-game.md). Property datatypes (`UINT8`…`FLOAT64`, `BOOL`) and endianness
are covered in the [Configuration Reference](02-config-reference.md#property-datatypes).

## Conditions

A condition is a single comparison: a property, an operator, and a value.

```json
{ "Property": "Pos X", "Op": "==", "Value": 4 }
```

- `Property` — the left operand (a registered property name).
- `Op` — one of `==`, `!=`, `>`, `>=`, `<`, `<=`, `%0`, `%N`, `BitTrue`, `BitFalse`.
  - `%0` / `%N`: property modulo `Value` is zero / non-zero.
  - `BitTrue` / `BitFalse`: the bit at index `Value` is set / clear.
- `Value` — the right operand. A **number** or **boolean** is an immediate value; a **string** is
  treated as *another property name*, so you can compare two properties (e.g. kid vs. guard X).

Conditions appear in two contexts, with identical grammar: inside a **rule** (when does this rule
fire?) and inside an **input set** (when is this input legal?). Within a single list, **all**
conditions must hold — they are AND-ed. There is no OR/NOT; express alternatives as separate rules
or input sets.

## Rules

A rule couples a set of conditions to a set of actions.

```json
{
  "Label": 200,
  "Conditions": [ { "Property": "Pos X", "Op": "==", "Value": 4 } ],
  "Satisfies": [],
  "Actions": [ { "Type": "Add Reward", "Value": 100.0 } ]
}
```

- `Label` — a unique number identifying the rule.
- `Conditions` — when all hold, the rule **fires**. An empty list means "always".
- `Actions` — what firing does (see below).
- `Satisfies` — labels of *other* rules to also mark as satisfied when this one fires. Use it to
  build hierarchies: satisfying a "level complete" rule can auto-satisfy the per-checkpoint rules
  leading up to it.

## Core actions

These five action `Type`s are available to every game *(source: `source/game.hpp`)*:

| `Type` | Extra keys | Effect |
|--------|-----------|--------|
| `Add Reward` | `Value` (number) | Add `Value` to the state's reward. Negative values penalize. |
| `Trigger Win` | — | This state is a solution; the search can stop (per `End On First Win State`). |
| `Trigger Fail` | — | Prune this state; it is never expanded. Use it to kill dead ends (death, out-of-bounds, wrong room). |
| `Trigger Checkpoint` | `Tolerance` (number) | Record a milestone (see [below](#checkpoints-and-tolerance)). |
| `Trigger Save Solution` | `Path` (string) | Write the current solution to `Path`. Handy for capturing the path to a checkpoint mid-run. |

## Reward magnets

A magnet is a **continuous** reward gradient toward a target — the workhorse for steering long
searches. Magnets are *game-specific* actions (each game registers its own in `parseRuleActionImpl`),
but they follow one of two shapes.

**Point magnets** pull a positional property toward a target value. In Prince of Persia, for
example *(source: `games/sdlpop/princeOfPersia.hpp`)*:

```json
{ "Type": "Set Kid Pos X Magnet", "Intensity": 1.0, "Position": 180, "Room": 5 }
```

- `Intensity` — strength (and sign). Positive pulls *toward* `Position`; negative pushes away.
- `Position` — the target value of the property.
- `Room` — the magnet is only active while the actor is in this room; elsewhere its intensity is
  treated as `0`. (This room-gating is specific to actors that move between rooms, like the Kid and
  the Guard.)

The reward contribution is `Intensity × −|current − Position|` — i.e. the closer the property is to
the target, the higher the reward. So a positive-intensity Kid-X magnet at `Position: 180` makes the
engine prefer states where the kid is nearer column 180 of that room.

**Scalar magnets** multiply a single property by an intensity, e.g. `Set Level Door Open Magnet` or
`Set Guard HP Magnet` — reward goes up as the door opens or as the guard loses HP. These take only
`Intensity`.

> **Discovering a game's magnets.** The full set is game-specific; read the game's
> `parseRuleActionImpl` (it is a chain of `if (actionType == "Set … Magnet")` blocks) to see every
> magnet it supports and which keys each takes. The naming is consistent: `Set <Thing> Magnet`.

The typical pattern is: keep a *standing* magnet pulling the player toward the level exit, and use
rules with `Add Reward` to give discrete bonuses at key milestones, and `Trigger Fail` to prune
death/dead-end states.

## Checkpoints and tolerance

`Trigger Checkpoint` records progress and carries a `Tolerance`. Tolerance interacts with
deduplication: normally two states with the same `Hash Properties` are merged, but right after a
checkpoint the engine tolerates a number of additional steps before re-merging, so it does not
immediately discard the slightly-different states that branch off a milestone. Larger tolerance =
more states kept around a checkpoint = broader local search at the cost of memory. The related
global knob is `Runner Configuration` > `Hash Step Tolerance`
(see [Search Concepts & Tuning](04-search-concepts.md#hash-step-tolerance)).

## Worked example: GridWalker

[`docs/examples/gridwalker.jaffar`](examples/gridwalker.jaffar) is a complete, ROM-free illustration. The
"game" is a cursor on a 5×5 grid starting at `(0,0)`; the goal is `(4,4)`. Its three rules:

```json
"Rules": [
  { "Label": 100,
    "Conditions": [ {"Property":"Pos X","Op":"==","Value":2}, {"Property":"Pos Y","Op":"==","Value":2} ],
    "Actions": [ {"Type":"Trigger Fail"} ] },

  { "Label": 200,
    "Conditions": [ {"Property":"Pos X","Op":"==","Value":4} ],
    "Actions": [ {"Type":"Add Reward","Value":100.0},
                 {"Type":"Trigger Checkpoint","Tolerance":8},
                 {"Type":"Trigger Save Solution","Path":"/tmp/jaffar.gridwalker.win.sol"} ] },

  { "Label": 300,
    "Conditions": [ {"Property":"Pos X","Op":"==","Value":4}, {"Property":"Pos Y","Op":"==","Value":4} ],
    "Satisfies": [ 200 ],
    "Actions": [ {"Type":"Add Reward","Value":100000.0}, {"Type":"Trigger Win"} ] }
]
```

Reading it as a search strategy:

- **Rule 100** prunes the center `(2,2)` — a hard "do not go here" via `Trigger Fail`.
- **Rule 200** rewards reaching the right-hand column (`Pos X == 4`) with `+100`, marks a
  checkpoint, and saves the partial solution. This is the *intermediate compass* pulling the search
  rightward.
- **Rule 300** is the win at `(4,4)`, worth a huge `+100000` so it dominates the ordering, and it
  `Satisfies: [200]` so reaching the corner also counts the column-4 milestone as met.

Because the hash properties are `Pos X` and `Pos Y`, the engine treats each grid cell as one state
and finds the shortest (8-move) path. Note there are no magnets here — on a 5×5 grid the discrete
rewards suffice. On a real game with thousands of positions, you would add a magnet toward the exit
instead of (or in addition to) discrete per-column rewards.

## Design recipes

- **Make the win dominate.** Give `Trigger Win` (or the rule leading to it) a reward far larger than
  any intermediate bonus, so a winning path always sorts to the front.
- **Prune aggressively.** Every `Trigger Fail` you add removes a whole subtree from the search.
  Death, wrong room, lost HP you can't recover — fail them early.
- **Steer with one standing magnet, reward milestones discretely.** A single exit-ward magnet gives
  a smooth gradient; `Add Reward` rules give sharp bumps at gates/keys/checkpoints.
- **Watch the state explosion.** Rewarding noisy properties (timers, RNG, cosmetic counters) makes
  near-identical states look different and floods the database. Keep `Hash Properties` minimal and,
  for Prince of Persia specifically, set `Disable Non-Gameplay RNG` (see
  [tuning](04-search-concepts.md) and the configuration reference).

The next chapter explains *why* these choices affect performance — how hashing, deduplication, and
the state database actually work.
