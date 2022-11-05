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

  doRendering = true;
  S9xMainLoop();
  doRendering = false;

  // Reload game state
  _game->_emu->deserializeState(emuState);
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
