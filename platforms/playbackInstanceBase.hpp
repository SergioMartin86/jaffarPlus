#pragma once

#include <gameInstance.hpp>
#include <utils.hpp>
#include <string>

class PlaybackInstanceBase
{
 protected:

 GameInstance* _game;

 public:

  // Initializes the playback module instance
 PlaybackInstanceBase(GameInstance* game, const nlohmann::json& config) { _game = game; };
 virtual ~PlaybackInstanceBase() = default;

 // Function to render frame
 virtual void renderFrame(const uint16_t currentStep, const std::string& move) = 0;
};
