
[![Build and Run Tests](https://github.com/SergioMartin86/jaffarPlusPlus/actions/workflows/make.yml/badge.svg)](https://github.com/SergioMartin86/jaffarPlusPlus/actions/workflows/make.yml) [![codecov](https://codecov.io/github/SergioMartin86/jaffarPlusPlus/graph/badge.svg?token=NF9W0XZ16h)](https://codecov.io/github/SergioMartin86/jaffarPlusPlus) 

# JaffarPlus

![](jaffar.png)

Jaffar is a high-performance general-purpose breath-first search optimizer. This project is specially tailored for the production tool-assisted speedruns (TAS). Its features are:

* High-Performance: Tailored for multi-core CPUs, especially for high core counts.
* Multi-Platform: Supports games from multiple consoles/game emulators
* Extensible: Any new emulators or games that support load/save and advance step functions can be added to JaffarPlus via a common API

This work is based on [Jaffar](https://github.com/SergioMartin86/jaffar), a solver for the original Prince of Persia (DOS).

# Supported Emulators

## Consoles

| Console            | Core(s)                                                                      | Target        |  Notes        |
| --------           | -------                                                                      | ------        | ------        |
| Atari 2600         | [QuickerStella](https://github.com/SergioMartin86/quickerStella)             | Bizhawk (dev) | Faster than Atari2600Hawk  |
| Atari 2600         | [Atari2600Hawk](https://github.com/CasualPokePlayer/libAtari2600Hawk)        | Bizhawk 2.9.2 |               |
| NES                | [QuickerNES](https://github.com/SergioMartin86/quickerNES)                   | Bizhawk 2.9.2 |               |
| SNES               | [QuickerSnes9x](https://github.com/SergioMartin86/quickerSnes9x)             | Bizhawk 2.9.2 |               |
| Sega Genesis       | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 | Bizhawk 2.9.2 |               |
| Sega CD            | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 | Bizhawk 2.9.2 |               |
| Sega SG-1000       | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 | Bizhawk 2.9.2 |               |
| Sega Master System | [QuickerGPGX](https://github.com/SergioMartin86/quickerGPGX)                 | Bizhawk 2.9.2 |               |

## Game-Specific

| Game                                | Core(s)                                                           |   Target      |  Notes   |
| --------                            | -------                                                           | ------        | ------   |
| Prince of Persia (DOS / Apple ][)   | [QuickerSDLPoP](https://github.com/SergioMartin86/quickerSDLPoP)  | LibTAS+PCem   |          |
| Another World                       | [QuickerRaw](https://github.com/SergioMartin86/QuickerRAW)        | *many*        |  All ports of AW use the same engine |
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
  + Alexander Lyashuk (mooskagh, crem) for kickstarting the idea of creating a TASing bot.
  + The authors of the third party libraries used.

- JaffarPlus is distributed freely under the GPL3 license for any purpose and use, as long as:
  + The license and proper credits to its author are preserved
  + If you publish a TAS or any public work using this software, I'd appreciate you mention it and linking this repository in your description
