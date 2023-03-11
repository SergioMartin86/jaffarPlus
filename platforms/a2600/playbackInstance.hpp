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
  _game->_emu->_a2600->advanceFrame();
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
