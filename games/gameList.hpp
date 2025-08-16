#pragma once

#include <emulatorList.hpp>

#ifdef __JAFFAR_ENABLE_NES
#include "nes/arkanoid2.hpp"
#include "nes/iceClimber.hpp"
#include "nes/beetlejuice.hpp"
#include "nes/indyHeat.hpp"
#include "nes/lunarBall.hpp"
#include "nes/microMachines.hpp"
#include "nes/saintSeiyaOugonDensetsu.hpp"
#include "nes/sidePocket.hpp"
#include "nes/sprilo.hpp"
#include "nes/marbleMadness.hpp"
#include "nes/bestOfTheBest.hpp"
#include "nes/doubleDragon.hpp"
#include "nes/gimmick.hpp"
#include "nes/ninjaGaiden3.hpp"
#include "nes/batman.hpp"
#include "nes/darkwingDuck.hpp"
#include "nes/metroid.hpp"
#include "nes/pacmanChampionship.hpp"
#endif

#ifdef __JAFFAR_ENABLE_SDLPOP
#include "sdlpop/princeOfPersia.hpp"
#endif

#ifdef __JAFFAR_ENABLE_SNES
#include "snes/arkanoid.hpp"
#include "snes/christmasCraze.hpp"
#include "snes/superOffRoad.hpp"
#include "snes/bestOfTheBest.hpp"
#endif

#ifdef __JAFFAR_ENABLE_GENESIS
#include "gamegear/pop.hpp"
#include "genesis/avuado.hpp"
#include "genesis/dinoRunner.hpp"
#include "genesis/microMachines.hpp"
#include "genesis/pop.hpp"
#include "genesis/popUSA.hpp"
#include "genesis/segapede.hpp"
#include "genesis/shoveit.hpp"
#include "genesis/sonic.hpp"
#include "genesis/bestOfTheBest.hpp"
#include "sms/snailMaze.hpp"
#endif

#ifdef __JAFFAR_ENABLE_A2600
#include "a2600/hellway.hpp"
#endif

#ifdef __JAFFAR_ENABLE_A2600
#include "a2600/hellway.hpp"
#endif

#if defined(__JAFFAR_ENABLE_SMBC) || defined(__JAFFAR_ENABLE_NES)
#include "nes/superMarioBros.hpp"
#endif

#if defined(__JAFFAR_ENABLE_NEORAW) || defined(__JAFFAR_ENABLE_RAWGL)
#include "raw/anotherWorld.hpp"
#endif

#ifdef __JAFFAR_USE_ARKBOT
#include "arkbot/arkanoid.hpp"
#endif

#ifdef __JAFFAR_ENABLE_GBC
#include "gbc/aSlimeTravel.hpp"
#include "gbc/pop.hpp"
#endif

#ifdef __JAFFAR_ENABLE_DOOM
#include "doom/doom.hpp"
#endif

#ifdef __JAFFAR_ENABLE_GBA
#include "gba/tollrunner.hpp"
#endif

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{
#define DETECT_GAME(GAME)                                                                                                                                                          \
  if (gameName == games::GAME::getName())                                                                                                                                          \
  {                                                                                                                                                                                \
    g            = std::make_unique<games::GAME>(std::move(e), gameConfig);                                                                                                        \
    isRecognized = true;                                                                                                                                                           \
  }

std::unique_ptr<Game> Game::getGame(const nlohmann::json& emulatorConfig, const nlohmann::json& gameConfig)
{
  // Base pointer for the game
  std::unique_ptr<Game> g;

  // Flag to indicate whether the name was recognized
  bool isRecognized = false;

  // Getting Emulator
  auto e = jaffarPlus::Emulator::getEmulator(emulatorConfig);

  // Getting game name
  const auto& gameName = jaffarCommon::json::getString(gameConfig, "Game Name");

// Trying to detect game by name
#ifdef __JAFFAR_ENABLE_NES
  DETECT_GAME(nes::MicroMachines);
  DETECT_GAME(nes::MarbleMadness);
  DETECT_GAME(nes::SaintSeiyaOugonDensetsu);
  DETECT_GAME(nes::Sprilo);
  DETECT_GAME(nes::IndyHeat);
  DETECT_GAME(nes::LunarBall);
  DETECT_GAME(nes::SidePocket);
  DETECT_GAME(nes::Beetlejuice);
  DETECT_GAME(nes::IceClimber);
  DETECT_GAME(nes::BestOfTheBest);
  DETECT_GAME(nes::DoubleDragon);
  DETECT_GAME(nes::Gimmick);
  DETECT_GAME(nes::NinjaGaiden3);
  DETECT_GAME(nes::Batman);
  DETECT_GAME(nes::DarkwingDuck);
  DETECT_GAME(nes::Metroid);
  DETECT_GAME(nes::PacManChampionship);
#ifdef __JAFFAR_USE_QUICKERNES_ARKANOID
  DETECT_GAME(nes::Arkanoid2);
#endif
#endif

#ifdef __JAFFAR_ENABLE_SDLPOP
  DETECT_GAME(sdlpop::PrinceOfPersia);
#endif

#ifdef __JAFFAR_ENABLE_SNES
  DETECT_GAME(snes::ChristmasCraze);
  DETECT_GAME(snes::Arkanoid);
  DETECT_GAME(snes::SuperOffRoad);
  DETECT_GAME(snes::BestOfTheBest);
#endif

#ifdef __JAFFAR_ENABLE_GENESIS
  DETECT_GAME(genesis::Avuado);
  DETECT_GAME(genesis::DinoRunner);
  DETECT_GAME(genesis::MicroMachines);
  DETECT_GAME(genesis::Segapede);
  DETECT_GAME(genesis::Sonic);
  DETECT_GAME(genesis::PrinceOfPersia);
  DETECT_GAME(genesis::PrinceOfPersiaUSA);
  DETECT_GAME(genesis::ShoveIt);
  DETECT_GAME(genesis::BestOfTheBest);
  DETECT_GAME(gamegear::PrinceOfPersia);
  DETECT_GAME(sms::SnailMaze);
#endif

#ifdef __JAFFAR_ENABLE_A2600
  DETECT_GAME(a2600::Hellway);
#endif

#if defined(__JAFFAR_ENABLE_SMBC) || defined(__JAFFAR_ENABLE_NES)
  DETECT_GAME(nes::SuperMarioBros);
#endif

#if defined(__JAFFAR_ENABLE_NEORAW) || defined(__JAFFAR_ENABLE_RAWGL)
  DETECT_GAME(raw::AnotherWorld);
#endif

#ifdef __JAFFAR_USE_ARKBOT
  DETECT_GAME(arkbot::Arkanoid);
#endif

#ifdef __JAFFAR_ENABLE_GBA
  DETECT_GAME(gba::TollRunner);
#endif

#ifdef __JAFFAR_ENABLE_GBC
  DETECT_GAME(gbc::PrinceOfPersia);
  DETECT_GAME(gbc::ASlimeTravel);
#endif

#ifdef __JAFFAR_ENABLE_DOOM
  DETECT_GAME(doom::Doom);
#endif

  // Check if game was recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Game '%s' not recognized\n", gameName.c_str());

  // Returning game pointer
  return g;
}

} // namespace jaffarPlus
