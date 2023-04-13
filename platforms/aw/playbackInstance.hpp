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

   if (command == ',')
   {
    _game->vmVariablesKeepGreater();
    return true;
   }

   if (command == '.')
   {
    _game->vmVariablesKeepSmaller();
    return true;
   }

   if (command == 'o')
   {
    // Obtaining RNG state
    LOG("Enter memory file: ");

    // Setting input as new rng
    char str[80]; getstr(str);

    LOG("Loading low memory file '%s'...\n", str);
    std::string lowMemData;
    bool status = loadStringFromFile(lowMemData, str);
    if (status == false) { printw("Could not read from file %s.\n", str); return false; }
    for (size_t i = 0; i < 0x100; i++) if (i < 0xF2 || i > 0xF6)
    {
      ((uint8_t*)_game->_emu->_engine->vm.vmVariables)[i*2 + 0] = lowMemData.data()[i*2 + 1];
      ((uint8_t*)_game->_emu->_engine->vm.vmVariables)[i*2 + 1] = lowMemData.data()[i*2 + 0];
    }

    // Replacing current sequence
    _game->popState(state);

    return true;
   }

   if (command == 'a')
   {

    LOG("Enter new RNG value: ");

    // Setting input as new rng
    char str[80]; getstr(str);

    _game->_emu->_engine->vm.vmVariables[0x37] = atoi(str);

    // Replacing current sequence
    _game->popState(state);

    return true;
   }

   #endif

   return true;
 }

};
