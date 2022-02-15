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
 PlaybackInstance(GameInstance* game) : PlaybackInstanceBase(game)
 {
  // Loading Emulator instance HQN
  _hqnState.m_emu = _game->_emu->_nes;
  _hqnState.m_emu->_doRendering = true;

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

 // Function to render frame
 void renderFrame() override
 {
  // Sleeping for 1/60th of a second
  usleep(16667);

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
};
