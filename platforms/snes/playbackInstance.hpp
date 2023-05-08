#pragma once

#include <utils.hpp>
#include <string>
#include <ncurses.h>
#include <playbackInstanceBase.hpp>

class PlaybackInstance : public PlaybackInstanceBase
{
 private:


 public:

  // Initializes the playback module instance
 PlaybackInstance(GameInstance* game, const nlohmann::json& config) : PlaybackInstanceBase(game, config)
 {
  S9xInitInputDevices();
  S9xInitDisplay(0, NULL);
  S9xSetupDefaultKeymap();
  S9xTextMode();
  S9xSetTitle("");
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  // Sleeping for the inverse frame rate
  usleep(_INVERSE_FRAME_RATE);

  // Storing current game state
  uint8_t emuState[_STATE_DATA_SIZE_PLAY];
  _game->_emu->serializeState(emuState);

  _game->_emu->advanceState(0);
  doRendering = true;
  S9xMainLoop();
  doRendering = false;

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

  if (command == 'x')
  {

   LOG("Enter new pos x ");

   // Setting input as new rng
   char str[80]; getstr(str);

   int16_t* vmVariables = (int16_t*)  &_game->_emu->_baseMem[0x0C42];
   int16_t* lesterPosX  = (int16_t*) &vmVariables[0x01];

   *lesterPosX = atoi(str);

   // Replacing current sequence
   _game->popState(state);

   return true;
  }

  if (command == 'o')
  {
   // Obtaining RNG state
   LOG("Enter low memory file: ");

   // Setting input as new rng
   char str[80]; getstr(str);

   LOG("Loading low memory file '%s'...\n", str);
   std::string lowMemData;
   bool status = loadStringFromFile(lowMemData, str);
   if (status == false) { printw("Could not read from file %s.\n", str); return false; }
   memcpy(_game->_emu->_baseMem, (uint8_t*) lowMemData.data(), 131072);

   // Replacing current sequence
   _game->popState(state);

   return false;
  }

  #endif

  return true;
 }

};
