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
 PlaybackInstanceBase(GameInstance* game) { _game = game; };
 virtual ~PlaybackInstanceBase() = default;

 // Function to render frame
 virtual void renderFrame() = 0;
};
