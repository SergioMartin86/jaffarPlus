#pragma once 

#include <emulator.hpp>
#include <game.hpp>
#include "nes/microMachines.hpp"
#include "nes/sprilo.hpp"
#include "../emulators/emulatorList.hpp"
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{
 #define DETECT_GAME(GAME) if (gameName == games::GAME::getName()) { g = std::make_unique<games::GAME>(std::move(e), gameConfig); isRecognized = true; }

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
  DETECT_GAME(NES::MicroMachines);
  DETECT_GAME(NES::Sprilo);

  // Check if game was recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Game '%s' not recognized\n", gameName.c_str());

  // Initializing game
  g->initialize();

  // Returning game pointer
  return g;
 }

}
