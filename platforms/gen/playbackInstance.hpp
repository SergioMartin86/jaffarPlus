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

  uint8_t state[_STATE_DATA_SIZE];
  _game->_emu->serializeState(state);

  system_frame_gen(0);
  sdl_video_update();

  _game->_emu->deserializeState(state);
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
  LOG("[Jaffar] Commands: o: load 68K RAM mem file\n");
  LOG("[Jaffar] Commands: w: load Z80 RAM mem file\n");
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {

  if (command == 'o')
  {
   // Obtaining RNG state
   LOG("Enter low memory file: ");

   // Setting input as new rng
   char str[80]; getstr(str);

   LOG("Loading 68K RAM file '%s'...\n", str);
   std::string _68KMemData;
   bool status = loadStringFromFile(_68KMemData, str);
   if (status == false) { printw("Could not read from file.\n", str); return false; }
   memcpy(_game->_emu->_68KRam, (uint8_t*) _68KMemData.data(), 0x10000);

   // Replacing current sequence
   _game->popState(state);

   return false;
  }

  if (command == 'w')
  {
   // Obtaining RNG state
   LOG("Enter low memory file: ");

   // Setting input as new rng
   char str[80]; getstr(str);

   LOG("Loading Z80 RAM file '%s'...\n", str);
   std::string _Z80MemData;
   bool status = loadStringFromFile(_Z80MemData, str);
   if (status == false) { printw("Could not read from file.\n", str); return false; }
   memcpy(_game->_emu->_Z80Ram, (uint8_t*) _Z80MemData.data(), 0x2000);

   // Replacing current sequence
   _game->popState(state);

   return false;
  }

  return true;
 }

};
