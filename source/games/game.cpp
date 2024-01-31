#include <emulators/emulator.hpp>
#include <games/game.hpp>
#include <games/nes/microMachines/game.hpp>

namespace jaffarPlus
{
 #define DETECT_GAME(G) if (gameName == games::G::Game::getName()) return std::make_unique<games::G::Game>(std::move(emulator), config);

 std::unique_ptr<Game> Game::getGame(const std::string& gameName, std::unique_ptr<Emulator>& emulator, const nlohmann::json& config)
 {
  DETECT_GAME(NES::microMachines);

  EXIT_WITH_ERROR("Game '%s' not recognized\n", gameName.c_str());
 }

}
