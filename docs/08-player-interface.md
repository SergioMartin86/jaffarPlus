# 8. Jaffar Player Interface

`jaffar-player` replays and inspects a solution against the same game/emulator the search used. It is
how you watch a run, navigate frame-by-frame, pull a save-state out of the middle of a solution,
capture screenshots, or dump a state for scripting. This chapter covers its **interactive commands**
and **usage patterns**; the one-line flag reference also appears in [Tooling](06-tooling.md#jaffar-player).

- [Invocation](#invocation)
- [Modes](#modes)
- [Interactive commands](#interactive-commands)
- [Command-line options](#command-line-options)
- [Common workflows](#common-workflows)

## Invocation

```
jaffar-player <configFile> <solutionFile> [options]
```

- `configFile` — the same `.jaffar` script the search used (it defines the emulator, game, initial
  state, and initial sequence).
- `solutionFile` — a `.sol` file: one input per line, in the engine's input notation (e.g.
  `|.|.R...|`). The player applies the config's `Initial State File` and `Initial Sequence File Path`
  first, then plays the solution on top.

## Modes

The player runs in one of two modes:

- **Interactive** (default) — it shows a prompt and waits for single-key commands so you can scrub
  back and forth through the solution. Requires rendering (do not pass `--disableRender`).
- **Unattended / batch** (`--unattended`) — no prompt, no waiting; it plays through and (with
  `--reproduce --exitOnEnd`) runs to the end and exits. This is the form to use in scripts, with
  `--disableRender`, `--screenshotDir`, or `--printFinalState`.

`--reproduce` makes it start playing from step 0 immediately (rather than sitting at the first frame);
`--reload` loops back to the start when it reaches the end; `--exitOnEnd` quits at the last step.

## Interactive commands

In interactive mode the player prints:

```
Commands: n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 | k: -1000 | i: +1000 | s: quicksave | p: play | r: autoreload | q: quit
```

Each is a single keypress that operates on the **current step**:

| Key | Action |
|---|---|
| `n` / `m` | Step backward / forward **1** frame. |
| `h` / `j` | Step backward / forward **10** frames. |
| `y` / `u` | Step backward / forward **100** frames. |
| `k` / `i` | Step backward / forward **1000** frames. |
| `s` | **Quicksave** the current step's state to `quicksave.state` (in the working directory). |
| `p` | Toggle **play** (auto-advance / reproduce) on or off. |
| `r` | Toggle **autoreload** (loop back to the start at the end). |
| `q` | **Quit** the player. |

Because navigation jumps to any cached step instantly, the state shown is always the exact state at
that step — so `s` saves precisely the frame you are looking at.

## Command-line options

| Option | Default | Purpose |
|---|---|---|
| `--reproduce` | off | Start playing from step 0 instead of waiting at the first frame. |
| `--reload` | off | Loop back to the start after the last step. |
| `--exitOnEnd` | off | Exit the program when the last step is reached. |
| `--unattended` | off | No interactive prompt and no waiting for keys (for scripts/batch). |
| `--disableRender` | off | Do not open a game window (headless). |
| `--frameskip <n>` | `1` | Render/screenshot every `n`-th frame. |
| `--initialSequence <file>` | — | Override the config's initial sequence with this file (played before the solution). |
| `--runCommand <cmd>` | — | Run a single command at startup, then exit (non-interactive one-shot). |
| `--screenshotDir <dir>` | — | Write per-frame screenshots (BMP) here. Requires rendering enabled. |
| `--screenshotSteps <list>` | all | Comma-separated step numbers to capture (empty = every rendered frame). |
| `--printFinalState` | off | Print the full game state (the same per-state dump the engine shows) — an oracle for scripting and verification. |

## Common workflows

**Watch a solution interactively** (then scrub with `n`/`m`):

```
jaffar-player game.jaffar solution.sol --reproduce
```

**Run to the end and dump the final state** (scriptable oracle — read off any property):

```
jaffar-player game.jaffar solution.sol --reproduce --disableRender --exitOnEnd --unattended --printFinalState
```

**Pull a save-state out of the middle of a solution.** Navigate to the step you want and press `s`;
the state lands in `quicksave.state`, which you can then point a new config's `Initial State File`
at. In batch form, pipe the keys:

```
printf 's\nq\n' | jaffar-player game.jaffar firstN.sol --reproduce --disableRender
cp quicksave.state checkpoint.state
```

(Here `firstN.sol` is the first *N* inputs of a longer solution, so the player ends — and quicksaves
— at exactly step *N*. See [Solution files](06-tooling.md#solution-files) for truncating a `.sol`.)

**Capture frames for a video** (headless):

```
jaffar-player game.jaffar solution.sol --reproduce --unattended --exitOnEnd \
  --screenshotDir ./frames --screenshotSteps 840-920
```

then assemble them with the `make_video.py` helper — see
[Rendering a solution to video](06-tooling.md#rendering-a-solution-to-video).

**Replay onto a hand-made starting point** with `--initialSequence`, to test inputs from a position
without re-deriving how you got there:

```
jaffar-player game.jaffar tail.sol --reproduce --initialSequence prefix.seq
```

> A `.sol`/sequence file is **newline-separated** (one input per line). What the player reads as the
> initial sequence is read line-by-line, so convert any NUL-separated solution to newlines first.
