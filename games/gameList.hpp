#pragma once

#include "../emulators/emulatorList.hpp"

#ifdef __JAFFAR_USE_QUICKERNES
  #include "nes/microMachines.hpp"
  #include "nes/sprilo.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERSDLPOP
  #include "sdlpop/princeOfPersia.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERSNES9X
  #include "snes/christmasCraze.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERGPGX
  #include "genesis/dinoRunner.hpp"
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
#ifdef __JAFFAR_USE_QUICKERNES
  DETECT_GAME(nes::MicroMachines);
  DETECT_GAME(nes::Sprilo);
#endif

#ifdef __JAFFAR_USE_QUICKERSDLPOP
  DETECT_GAME(sdlpop::PrinceOfPersia);
#endif

#ifdef __JAFFAR_USE_QUICKERSNES9X
  DETECT_GAME(snes::ChristmasCraze);
#endif

#ifdef __JAFFAR_USE_QUICKERGPGX
  DETECT_GAME(genesis::DinoRunner);
#endif

  // Check if game was recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Game '%s' not recognized\n", gameName.c_str());

  // Returning game pointer
  return g;
}

} // namespace jaffarPlus
