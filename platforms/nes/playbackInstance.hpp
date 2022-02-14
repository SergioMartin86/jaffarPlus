#pragma once

#include <SDL.h>
#include <hqn.h>
#include <hqn_gui_controller.h>
#include <utils.hpp>
#include <string>
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
 PlaybackInstance(const nlohmann::json& config) : PlaybackInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();

  // Loading Rom into HQN
  _hqnState.loadROM(romFilePath.c_str());

  // Opening rendering window
  SDL_SetMainReady();

  // We can only call SDL_InitSubSystem once
  if (!SDL_WasInit(SDL_INIT_VIDEO))
  {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        EXIT_WITH_ERROR("Failed to initialize video: %s", SDL_GetError());
    }
  }

  // Creating HQN GUI
  _hqnGUI = hqn::GUIController::create(_hqnState);
  _hqnGUI->setScale(2);
 }

 // Function to load state
 void loadState(const uint8_t* state) override
 {

 }

 // Function to render frame
 void renderFrame(const uint8_t move) override
 {

 }

};
