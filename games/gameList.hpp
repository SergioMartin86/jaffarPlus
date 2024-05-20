#pragma once

#include "../emulators/emulatorList.hpp"

#ifdef __JAFFAR_ENABLE_NES
  #include "nes/microMachines.hpp"
  #include "nes/sprilo.hpp"
#endif

#ifdef __JAFFAR_ENABLE_SDLPOP
  #include "sdlpop/princeOfPersia.hpp"
#endif

#ifdef __JAFFAR_ENABLE_SNES
  #include "snes/christmasCraze.hpp"
#endif

#ifdef __JAFFAR_ENABLE_GENESIS
  #include "genesis/dinoRunner.hpp"
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

#ifdef __JAFFAR_ENABLE_RAW
  #include "raw/anotherWorld.hpp"
#endif

#ifdef __JAFFAR_USE_ARKBOT
  #include "arkbot/arkanoid.hpp"
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

std::unique_ptr<Game> Game::getGame(const nlohmann::json &emulatorConfig, const nlohmann::json &gameConfig)
{
  // Base pointer for the game
  std::unique_ptr<Game> g;

  // Flag to indicate whether the name was recognized
  bool isRecognized = false;

  // Getting Emulator
  auto e = jaffarPlus::Emulator::getEmulator(emulatorConfig);

  // Getting game name
  const auto &gameName = jaffarCommon::json::getString(gameConfig, "Game Name");

// Trying to detect game by name
#ifdef __JAFFAR_ENABLE_NES
  DETECT_GAME(nes::MicroMachines);
  DETECT_GAME(nes::Sprilo);
#endif

#ifdef __JAFFAR_ENABLE_SDLPOP
  DETECT_GAME(sdlpop::PrinceOfPersia);
#endif

#ifdef __JAFFAR_ENABLE_SNES
  DETECT_GAME(snes::ChristmasCraze);
#endif

#ifdef __JAFFAR_ENABLE_GENESIS
  DETECT_GAME(genesis::DinoRunner);
#endif

#ifdef __JAFFAR_ENABLE_A2600
  DETECT_GAME(a2600::Hellway);
#endif

#if defined(__JAFFAR_ENABLE_SMBC) || defined(__JAFFAR_ENABLE_NES)
  DETECT_GAME(nes::SuperMarioBros);
#endif

#ifdef __JAFFAR_ENABLE_RAW
  DETECT_GAME(raw::AnotherWorld);
#endif

#ifdef __JAFFAR_USE_ARKBOT
  DETECT_GAME(arkbot::Arkanoid);
#endif

  // Check if game was recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Game '%s' not recognized\n", gameName.c_str());

  // Returning game pointer
  return g;
}

} // namespace jaffarPlus
