#pragma once

#include <utils.hpp>
#include <string>

class PlaybackInstanceBase
{
  public:

  // Initializes the playback module instance
 PlaybackInstanceBase(const nlohmann::json& config) {};
 virtual ~PlaybackInstanceBase() = default;

 // Function to load state
 virtual void loadState(const uint8_t* state) = 0;

 // Function to render frame
 virtual void renderFrame(const uint8_t move) = 0;
};
