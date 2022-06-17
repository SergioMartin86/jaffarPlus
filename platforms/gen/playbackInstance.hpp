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

  system_frame_gen(0);
  sdl_video_update();
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {
  return true;
 }

};
