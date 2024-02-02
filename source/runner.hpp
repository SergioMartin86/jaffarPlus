#pragma once

#include <memory>
#include <common/hash.hpp>
#include <common/json.hpp>
#include "game.hpp"

namespace jaffarPlus
{

class Runner final
{
 public:

   // Base constructor
  Runner(std::unique_ptr<Game>& game, const nlohmann::json& config) : _game(std::move(game))
  {
    // Getting Game-specific storage size
    _hashStepTolerance = JSON_GET_NUMBER(uint32_t, config, "Hash Step Tolerance");
  }

  // Function to advance state. Returns a vector with the performed inputs (including skip frames)
  void advanceState(const std::string& input)
  {
    // Performing the requested input
    _game->advanceState(input);

    // Advancing step counter
    _currentStep++;
  }

 inline Game* getGame() const { return _game.get(); }

 private:

  // Pointer to the game instance
  std::unique_ptr<Game> _game;

  // Storage for the current step of the runner
  uint32_t _currentStep = 0;

    // Storage for the hash step tolerance
  uint32_t _hashStepTolerance;
};

} // namespace jaffarPlus