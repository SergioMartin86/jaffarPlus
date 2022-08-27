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

 // Overlay info
 std::string _overlayPath;
 SDL_Surface* _overlayBaseSurface;
 SDL_Surface* _overlayButtonASurface;
 SDL_Surface* _overlayButtonBSurface;
 SDL_Surface* _overlayButtonSelectSurface;
 SDL_Surface* _overlayButtonStartSurface;
 SDL_Surface* _overlayButtonLeftSurface;
 SDL_Surface* _overlayButtonRightSurface;
 SDL_Surface* _overlayButtonUpSurface;
 SDL_Surface* _overlayButtonDownSurface;

 public:

  // Initializes the playback module instance
 PlaybackInstance(GameInstance* game, const nlohmann::json& config) : PlaybackInstanceBase(game, config)
 {
  if (isDefined(config, "Overlay Path") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Overlay Path' key.\n");
  _overlayPath = config["Overlay Path"].get<std::string>();

  // Loading overlay images

  std::string imagePath;

  imagePath = _overlayPath + std::string("/base.png");
  _overlayBaseSurface = IMG_Load(imagePath.c_str());
  if (_overlayBaseSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_a.png");
  _overlayButtonASurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonASurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_b.png");
  _overlayButtonBSurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonBSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_select.png");
  _overlayButtonSelectSurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonSelectSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_start.png");
  _overlayButtonStartSurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonStartSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_left.png");
  _overlayButtonLeftSurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonLeftSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_right.png");
  _overlayButtonRightSurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonRightSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_up.png");
  _overlayButtonUpSurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonUpSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  imagePath = _overlayPath + std::string("/button_down.png");
  _overlayButtonDownSurface = IMG_Load(imagePath.c_str());
  if (_overlayButtonDownSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

  // Loading Emulator instance HQN
  // TBD

  // Opening rendering window
  SDL_SetMainReady();

  // We can only call SDL_InitSubSystem once
  if (!SDL_WasInit(SDL_INIT_VIDEO))
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    EXIT_WITH_ERROR("Failed to initialize video: %s", SDL_GetError());

  // Creating HQN GUI
  // TBD
 }

 // Function to render frame
 void renderFrame(const uint16_t currentStep, const std::string& move) override
 {
  // Sleeping for the inverse frame rate
  usleep(_INVERSE_FRAME_RATE);

  // Storing current game state
  // TBD

  SDL_Surface* overlayButtonASurface = NULL;
  SDL_Surface* overlayButtonBSurface = NULL;
  SDL_Surface* overlayButtonSelectSurface = NULL;
  SDL_Surface* overlayButtonStartSurface = NULL;
  SDL_Surface* overlayButtonLeftSurface = NULL;
  SDL_Surface* overlayButtonRightSurface = NULL;
  SDL_Surface* overlayButtonUpSurface = NULL;
  SDL_Surface* overlayButtonDownSurface = NULL;

  if (move.find("A") != std::string::npos) overlayButtonASurface = _overlayButtonASurface;
  if (move.find("B") != std::string::npos) overlayButtonBSurface = _overlayButtonBSurface;
  if (move.find("S") != std::string::npos) overlayButtonSelectSurface = _overlayButtonSelectSurface;
  if (move.find("T") != std::string::npos) overlayButtonStartSurface = _overlayButtonStartSurface;
  if (move.find("L") != std::string::npos) overlayButtonLeftSurface = _overlayButtonLeftSurface;
  if (move.find("R") != std::string::npos) overlayButtonRightSurface = _overlayButtonRightSurface;
  if (move.find("U") != std::string::npos) overlayButtonUpSurface = _overlayButtonUpSurface;
  if (move.find("D") != std::string::npos) overlayButtonDownSurface = _overlayButtonDownSurface;

  // Since renderer is off by one frame, we need to emulate an additional frame
  // TBD

  // Reload game state
  //_game->_emu->deserializeState(emuState);
 }

 // Function to render frame
 void printPlaybackCommands() const override
 {
  LOG("[Jaffar] Commands: o: load low mem file\n");
 }

 // Function to render frame
 bool parseCommand(const char command, uint8_t* state) override
 {
  return true;
 }

};
