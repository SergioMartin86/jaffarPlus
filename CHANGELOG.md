# Changelog

All notable changes to JaffarPlus are documented here. The format is based on
[Keep a Changelog](https://keepachangelog.com/), and the project aims to follow
[Semantic Versioning](https://semver.org/).

## [2.0.0] - 2026-06-19

A large release consolidating a rewritten engine core, a complete documentation set, broad
emulator support, and a permissive license. Highlights below.

> **Note on licensing.** JaffarPlus's own source code is now distributed under the **MIT License**.
> The emulator cores it builds against are included as separate git submodules and are licensed under
> the **GPL** — a binary you build links one of those cores and is therefore a GPL derivative work.
> See the README for details.

> **Note on submodules.** A source archive from a GitHub release does **not** include submodules.
> Clone with `git submodule update --init <core>` (and `extern/jaffarCommon`) before building.

### Licensing
- Relicensed JaffarPlus's own source under the **MIT License**, and documented that built binaries
  are GPL because of the linked emulator cores (previously the `LICENSE`, README, and build metadata
  disagreed).

### Documentation
- Added a complete **user & developer manual** under `docs/` (getting started, full `.jaffar`
  configuration reference, rules/conditions/rewards, search concepts & tuning, adding a game or
  emulator, and a tooling reference), illustrated with schematic diagrams.
- Published the manual online via **mkdocs-material on GitHub Pages**, with a generated **Doxygen
  C++ API reference** served alongside it under `/api`.
- Added **anti-drift CI checks** that keep the configuration reference in sync with the code (every
  documented core key is verified against the source, and shipped example configs must pass
  `--dryRun`).

### Build & developer experience
- **Automatic game and emulator registration** — adding a game or emulator is now just dropping a
  header into the right directory; the include/detection lists are generated at configure time (no
  manual list editing).
- Added the **`-Dgame=`** option to compile a single game instead of all games of a core
  (`-Demulator=` selects the core).
- Clearer configuration and command-line **error messages** (invalid keys/operators/actions now list
  the valid options).
- `--dryRun` validates a full configuration without running the engine, host-independently.

### Engine & correctness
- **NUMA-aware two-tier hash deduplication** (per-domain L1 in front of a shared global L2) is now the
  default, with budget controls.
- Made multi-threaded execution robust on unpinned / oversubscribed machines: per-thread and
  per-NUMA-domain identity is taken from the dense OpenMP thread id rather than `sched_getcpu()`,
  fixing crashes and spurious "ran out of states" failures.
- **Removed differential compression** and fixed the state-corruption bugs surrounding it.
- Throughput work: batched base-state processing, per-thread (atomic-free) statistics counters, and a
  free-slot cache.
- Fine-grained hot-loop profiling is now a **compile-time option** (`-DdetailedProfiling`, off by
  default) with negligible cost otherwise.

### Tooling
- **Headless frame screenshots** in the player (`--screenshotDir` / `--screenshotSteps`) and a
  `make_video.py` helper to turn a solution into a video.
- A `--printFinalState` oracle and richer playback diagnostics (not-allowed-input and repeated-state
  reporting; first win/fail step).

### Emulator & game support
- Broad core support: NES (QuickerNES / QuickNES), SNES (QuickerSnes9x), Sega Genesis / Sega CD /
  Master System / Game Gear / SG-1000 (QuickerGPGX), Atari 2600 (QuickerStella, Atari2600Hawk),
  Game Boy / Color (QuickerGambatte), Game Boy Advance (QuickerMGBA), Doom (QuickerDSDA),
  Prince of Persia (QuickerSDLPoP), Another World (QuickerRAWGL / QuickerNEORAW),
  Super Mario Bros (QuickerSMBC), Arkanoid (QuickerArkBot), plus PipeBot and the in-repo,
  ROM-free TestEmulator.
- Prince of Persia: added a "Disable Non-Gameplay RNG" option and gameplay-tile hashing controls, and
  reconstructed the published world-record TAS scripts/solutions for the current engine.

### Testing & CI
- Added a ROM-free **TestEmulator + GridWalker** harness that exercises the engine's full execution
  path with a provably-optimal solution, plus unit tests for properties/conditions, rules/input sets,
  and the hash database.
- Added **AddressSanitizer/UndefinedBehaviorSanitizer**, detailed-profiling, and TestEmulator
  coverage CI jobs, and pinned the clang-format style check.

[2.0.0]: https://github.com/SergioMartin86/jaffarPlus/releases/tag/v2.0.0
