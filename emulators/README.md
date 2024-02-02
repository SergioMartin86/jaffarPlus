For emulators to be included in JaffarPlus, the following conditions must be met:

- They core needs to be developed in their own repository and included here as a submodule
- They need to implement Meson as build system so that they can easily be loaded as a subproject dependency
- They need to pass a CI pipeline with build and run tests.
- The run tests must include the reproduction of movies (preferably published at tasvideos.org) with the following conditions
  + The end state hash must be equal for a simple reproduction (only advance state) and a full re-record reproduction (save / advance / load / advance). This ensures to some extent that the solution that JaffarPlus produces through repeated re-recording can be reproduced later in a simple playback in the emulator.
  + If the emulator is an improved version of an original one, the original must be preserved in the repository and the test above must produce the exact same results for both of them.
- Must be fully thread-safe
- Must have no memory leaks in load/save/advance state routines
