# Jaffar 2

![](jaffar2.png)

High-performance fully scalable solver for Prince of Persia 2 tool-assisted speedrunning.  

This solver is based on [Jaffar](https://github.com/SergioMartin86/jaffar), a solver for the original Prince of Persia (DOS).

Installation
===============

1) Get code

 
```
  git clone --recursive https://github.com/SergioMartin86/jaffar2.git
```
  
2) Create build directory


```
  cd jaffar
  mkdir build
  cd build
```

3) Configure and compile

```
  meson ..
  ninja
```
  
Requisites
============


- MPI - For distributed execution. Jaffar has been tested exclusively on MPICH
- Meson - For compilation
- Ninja - For compilation
- SDL2: Jaffar has been tested using SDL2 2.0.14. 
- SDL2-image: Jaffar has been tested using SDL2_image 2.0.5.
- libncurses: For console output

Usage
=========

Commands:
-----------

+ To run the training process on 16 MPI ranks:

```
mpirun -n 16 jaffar-train example.config 
```


NOTE: `jaffar-train` requires an input file that describes the rules for exploration. Many examples of such a config file can be found in the `examples` folder. Detailed documentation is not currently availble as the contents of these files are still subject to changes.


+ To playback, analyze, produce replay or savestates for a given sequence of comands:

```
jaffar-play example.config example.seq 
```

+ To showcase the current/best state of training:

```
jaffar-show jaffar.current.sav
jaffar-show jaffar.best.sav  
```

Environment Variables:
------------------------

Indicate where the SDLpop root folder is located:

```
export SDLPOP_ROOT=$HOME/jaffar/extern/SDLPoP
```

Specify maximum memory size to be used for frame database per worker (in megabytes): 

```
export JAFFAR_MAX_WORKER_FRAME_DATABASE_SIZE_MB=1000
```

Specify maximum memory size to be used for hash databases per worker (in megabytes):

```
export JAFFAR_MAX_WORKER_HASH_DATABASE_SIZE_MB=100
```

Specify how frequently (in seconds) and where the `jaffar-train` command should save the best state of exploration:

```
export JAFFAR_SAVE_BEST_EVERY_SECONDS=5
export JAFFAR_SAVE_BEST_PATH=jaffar.best.sav
```

Specify how frequently (in seconds) and where the `jaffar-train` command should save the current state of exploration:

```
export JAFFAR_SAVE_CURRENT_EVERY_SECONDS=1
export JAFFAR_SAVE_CURRENT_PATH=jaffar.current.sav
```

Specify how frequently (in seconds) the `jaffar-show` command should check for updates in the save file:

```
JAFFAR_SHOW_UPDATE_EVERY_SECONDS
```

Authors
=============

- Sergio Martin (eien86)
  + Github: https://github.com/SergioMartin86
  + Twitch: https://www.twitch.tv/eien86
  + Youtube: https://www.youtube.com/channel/UCSXpK3d6vUq58fjgF5jFoKA

Acknowledgements
=================

The author would like to thank:

+ Thanks to Alexander Lyashuk (mooskagh, crem) for developing the original bot that gave birth to the Jaffar series. 

+ Near (Byuu) for their coroutine library used in this bot.

+ The developer(s) of BlastEm (https://www.retrodev.com/blastem/), used to drive emulation of the Genesis version of POP2. 

+ BlastEm's discord server users: 'Mask of Destiny', 'WinFutureX', and 'Sik' for their help.
