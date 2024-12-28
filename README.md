
[![Build and Run Tests](https://github.com/SergioMartin86/jaffarPlus/actions/workflows/make.yml/badge.svg)](https://github.com/SergioMartin86/jaffarPlus/actions/workflows/make.yml) [![codecov](https://codecov.io/gh/SergioMartin86/jaffarPlus/graph/badge.svg?token=B9KMR864ZP)](https://codecov.io/gh/SergioMartin86/jaffarPlus)

# JaffarPlus

![](jaffar.png)

JaffarPlus is a high-performance general-purpose breadth-first search optimizer. This project is specially tailored for the production of tool-assisted speedruns (TAS). Its features are:

* High-Performance: Tailored for multi-core CPUs, especially for high core counts.
* Multi-Platform: Supports games from multiple consoles/game emulators
* Extensible: Any new emulators or games that support load/save and advance step functions can be added to JaffarPlus via a common API

This work is based on [Jaffar](https://github.com/SergioMartin86/jaffar), a solver for the original Prince of Persia (DOS).

# Built-in Emulator Support

## Consoles

| Console                  | Core(s)                                                                      |
| --------                 | -------                                                                      |
| Atari 2600               | [QuickerStella](https://github.com/SergioMartin86/quickerStella)             |
| Atari 2600               | [Atari2600Hawk](https://github.com/CasualPokePlayer/libAtari2600Hawk)        |
| NES                      | [QuickerNES](https://github.com/SergioMartin86/quickerNES)                   |
| SNES                     | [QuickerSnes9x](https://github.com/SergioMartin86/quickerSnes9x)             |
| Sega Genesis             | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 |
| Sega CD                  | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 |
| Sega SG-1000             | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 |
| Sega Master System       | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 |
| Gameboy Advance          | [QuickerMGBA](https://github.com/SergioMartin86/quickerMGBA)                 |
| Gameboy / Gameboy Color  | [QuickerGambatte](https://github.com/SergioMartin86/quickerGambatte)         |

## Game-Specific

| Game                                | Core(s)                                                           |   Target      |  Notes   |
| --------                            | -------                                                           | ------        | ------   |
| Prince of Persia                    | [QuickerSDLPoP](https://github.com/SergioMartin86/quickerSDLPoP)  | LibTAS+PCem   |  Many PoP ports use this same (AppleII / DOS) game logic |
| Another World                       | [QuickerNEORAW](https://github.com/SergioMartin86/QuickerNEORAW)  | DOS           |  This AW interpreter only works with DOS files |
| Another World                       | [QuickerRAWGL](https://github.com/SergioMartin86/QuickerRAWGL)    | Multiple      |  This AW interpreter works with most AW ports |
| Super Mario Bros (NES)              | [QuickerSMBC](https://github.com/SergioMartin86/quickerSMBC)      | Bizhawk 2.9.2 |  Inaccurate in transitions, but good for solving levels |
| Arkanoid (NES)                      | [QuickerArkbot](https://github.com/SergioMartin86/quickerArkBot)  | Bizhawk 2.9.2 (NesHawk Core) |          |

Author
=============

- Sergio Martin (eien86)
  + Github: https://github.com/SergioMartin86
  + Twitch: https://www.twitch.tv/eien86
  + Youtube: https://www.youtube.com/channel/UCSXpK3d6vUq58fjgF5jFoKA
  + TASVideos: https://tasvideos.org/Users/Profile/eien86

- A list of TAS movies produced by eien86 using JaffarPlus can be found [here](https://tasvideos.org/Subs-List?user=eien86&statusfilter=6)

- Contributions via pull requests are highly appreciated.

- Thanks to:
  + TASVideos' staff (judges, encoders, admins, etc)
  + The Bizhawk development team (YoshiRulz, feos, Morilli, CasualPokePlayer, NattTheBear, Alyosha, feos, zeromus, and many others)
  + Dávid Nagy and all SDLPoP developers
  + Gregory Montoir and Fabien Sanglard (authors of Fabother World)
  + Eke-Eke and all Genesis Plus GX developers
  + Shay Green, Christopher Snowhill and all QuickNES developers
  + sbroger (a.k.a Chef Stef), developer of Arkbot
  + Mitchell Sternke, developer of SMB-C
  + sinamas, et al. for Gambatte-Speedrun
  + Vicki Pfau, et al. for MGBA
  + Alexander Lyashuk (mooskagh, crem) for kickstarting the idea of creating a TASing bot.
  + The authors of the third party libraries used.

- JaffarPlus is distributed freely under the GPL3 license for any purpose and use, as long as:
  + The license and proper credits to its author are preserved
  + If you publish a TAS or any public work using this software, I'd appreciate you mention it and linking this repository in your description
