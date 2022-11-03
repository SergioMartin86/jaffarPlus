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
  Application::setName("bsnes");
  Application::setScreenSaver(settings.general.screenSaver);
  Application::setToolTips(settings.general.toolTips);

  Instances::presentation.construct();
  Instances::settingsWindow.construct();
  Instances::cheatDatabase.construct();
  Instances::cheatWindow.construct();
  Instances::stateWindow.construct();
  Instances::toolsWindow.construct();

  program.create();
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  uint8_t emuState[_STATE_DATA_SIZE_PLAY];
  _game->_emu->serializeState(emuState);

  _game->_emu->_emu->run();
  _game->_emu->_emu->refreshRender();
  program.updateStatus();
  video.poll();

  // Reload game state
  _game->_emu->deserializeState(emuState);
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
//  LOG("%u\n", emulator->read(0x2));
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {

  return true;
 }

};
