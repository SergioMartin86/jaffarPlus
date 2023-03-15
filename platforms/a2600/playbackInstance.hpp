#pragma once

#include <utils.hpp>
#include <string>
#include <playbackInstanceBase.hpp>

class PlaybackInstance : public PlaybackInstanceBase
{
 private:


 public:

  // Initializes the playback module instance
 PlaybackInstance(GameInstance* game, const nlohmann::json& config) : PlaybackInstanceBase(game, config)
 {
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  _game->_emu->_a2600->renderFrame();
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {
 #ifdef NCURSES

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
  memcpy(_game->_emu->_ram, (uint8_t*) lowMemData.data(), 0x80);

  // Replacing current sequence
  _game->popState(state);

  return false;
 }

 #endif

 return true;

 }

};
