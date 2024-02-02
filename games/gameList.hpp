#include <emulator.hpp>
#include <game.hpp>
#include "nes/microMachines.hpp"

namespace jaffarPlus
{
 #define DETECT_GAME(GAME) if (gameName == games::GAME::Game::getName()) return std::make_unique<games::GAME::Game>(emulator, config);

 std::unique_ptr<Game> Game::getGame(const std::string& gameName, std::unique_ptr<Emulator>& emulator, const nlohmann::json& config)
 {
  DETECT_GAME(NES::microMachines);

  EXIT_WITH_ERROR("Game '%s' not recognized\n", gameName.c_str());
 }

}