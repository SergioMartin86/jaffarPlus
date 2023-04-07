#pragma once

#include <utils.hpp>
#include <string>
#include <ncurses.h>
#include <playbackInstanceBase.hpp>
#include <SDL.h>
#include <SDL_image.h>

class PlaybackInstance : public PlaybackInstanceBase
{
 private:


 public:

  // Initializes the playback module instance
 PlaybackInstance(GameInstance* game, const nlohmann::json& config) : PlaybackInstanceBase(game, config)
 {
  _game = game;
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  usleep(_INVERSE_FRAME_RATE);

  uint8_t emuState[_STATE_DATA_SIZE_PLAY];
  _game->_emu->serializeState(emuState);

  _enableRender = true;
  _game->advanceGameState(0);
  _enableRender = false;

  // Reload game state
  _game->_emu->deserializeState(emuState);
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
  LOG("[Jaffar] Commands: o: load low mem file\n");
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {
   #ifdef NCURSES

   if (command == 'c')
   {
    _game->vmVariablesClear();
    return true;
   }

   if (command == 'w')
   {
    _game->vmVariablesKeepDifferent();
    return true;
   }

   if (command == 'e')
   {
    _game->vmVariablesKeepEqual();
    return true;
   }

   #endif

   return true;
 }

};
