#pragma once

#include <SDL.h>
#include <hqn.h>
#include <hqn_gui_controller.h>
#include <utils.hpp>
#include <string>
#include <ncurses.h>
#include <playbackInstanceBase.hpp>

class PlaybackInstance : public PlaybackInstanceBase
{
 private:

 // Storage for the HQN state
 hqn::HQNState _hqnState;

 // Storage for the HQN GUI controller
 hqn::GUIController* _hqnGUI;

 public:

  // Initializes the playback module instance
 PlaybackInstance(GameInstance* game, const nlohmann::json& config) : PlaybackInstanceBase(game, config)
 {
  // Loading Emulator instance HQN
  _hqnState.m_emu = _game->_emu->_nes;
  _hqnState.m_emu->_doRendering = true;

  // Opening rendering window
  SDL_SetMainReady();

  // We can only call SDL_InitSubSystem once
  if (!SDL_WasInit(SDL_INIT_VIDEO))
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    EXIT_WITH_ERROR("Failed to initialize video: %s", SDL_GetError());

  // Creating HQN GUI
  _hqnGUI = hqn::GUIController::create(_hqnState);
  _hqnGUI->setScale(2);
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  // Sleeping for 1/60th of a second
  usleep(16667);

  // If using a new game and it doesn't seem to load, then get its savestate size here
//    size_t size;
//    _hqnState.saveStateSize(&size);
//    printw("%lu\n", size);
//    refresh();
//    while(1);

  // Storing current game state
  uint8_t emuState[_STATE_DATA_SIZE];
  _game->_emu->serializeState(emuState);

  // Since renderer is off by one frame, we need to emulate an additional frame
  _game->advanceState(".");
  int32_t curImage[BLIT_SIZE];
  _hqnState.blit(curImage, hqn::HQNState::NES_VIDEO_PALETTE, 0, 0, 0, 0);
  _hqnGUI->update_blit(curImage);

  // Reload game state
  _game->_emu->deserializeState(emuState);
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
  printw("[Jaffar] Commands: o: load low mem file\n");
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {
  if (command == 'o')
  {
   // Obtaining RNG state
   printw("Enter low memory file: ");

   // Setting input as new rng
   char str[80]; getstr(str);

   printw("Loading low memory file '%s'...\n", str);
   std::string lowMemData;
   bool status = loadStringFromFile(lowMemData, str);
   if (status == false) { printw("Could not read from file.\n", str); return false; }
   memcpy(_game->_emu->_baseMem, (uint8_t*) lowMemData.data(), 2048);

   // Replacing current sequence
   _game->popState(state);

   return false;
  }

  return true;
 }

};
