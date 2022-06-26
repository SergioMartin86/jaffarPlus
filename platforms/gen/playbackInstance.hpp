#pragma once

#include <SDL.h>
#include <utils.hpp>
#include <string>
#include <ncurses.h>
#include <playbackInstanceBase.hpp>
#include <SDL_image.h>
#include <main.h>

class PlaybackInstance : public PlaybackInstanceBase
{
 private:


 public:

  // Initializes the playback module instance
 PlaybackInstance(GameInstance* game, const nlohmann::json& config) : PlaybackInstanceBase(game, config)
 {
  initSDLWindow();
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  // Sleeping for the inverse frame rate
  usleep(_INVERSE_FRAME_RATE);

  uint8_t state[_STATE_DATA_SIZE_PLAY];
  _game->_emu->serializeState(state);

  system_frame_gen(0);
  sdl_video_update();

  _game->_emu->deserializeState(state);
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
  LOG("[Jaffar] Commands: w: save light state\n");
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {

  if (command == 'w')
  {
   std::string stateBuffer;
   stateBuffer.resize(_STATE_DATA_SIZE_PLAY);
   size_t stateSize = state_save_light((uint8_t*)stateBuffer.c_str());
   stateBuffer.resize(stateSize);
   saveStringToFile(stateBuffer, "jaffar.state");
   printw("[Jaffar] Saved state to %s\n", "jaffar.state");
   return false;
  }

  return true;
 }

};
