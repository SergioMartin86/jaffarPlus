#pragma once

#include <utils.hpp>
#include <string>
#include <ncurses.h>
#include <playbackInstanceBase.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <Constants.hpp>

class PlaybackInstance : public PlaybackInstanceBase
{
 private:

 SDL_Window* window;
 SDL_Renderer* renderer;
 SDL_Texture* texture;
 SDL_Texture* scanlineTexture;
 SMBEngine* smbEngine = nullptr;
 uint32_t renderBuffer[RENDER_WIDTH * RENDER_HEIGHT];
 uint8_t videoScale;

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
  videoScale = 1;
  smbEngine = _game->_emu->_nes;

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

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) EXIT_WITH_ERROR("[Error] Could not initialize SDL, Reason: %s\n", SDL_GetError());

  // Create the window
  window = SDL_CreateWindow(APP_TITLE,
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            RENDER_WIDTH * videoScale,
                            RENDER_HEIGHT * videoScale,
                            0);
  if (window == nullptr) EXIT_WITH_ERROR("[Error] Could not initialize window, Reason: %s\n", SDL_GetError());

  // Setup the renderer and texture buffer
  renderer = SDL_CreateRenderer(window, -1,  SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) EXIT_WITH_ERROR("[Error] Could not initialize rederer, Reason: %s\n", SDL_GetError());
  if (SDL_RenderSetLogicalSize(renderer, RENDER_WIDTH, RENDER_HEIGHT) < 0) EXIT_WITH_ERROR("[Error] Could not set render logical size, Reason: %s\n", SDL_GetError());
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, RENDER_WIDTH, RENDER_HEIGHT);
  if (texture == nullptr) EXIT_WITH_ERROR("[Error] Could not create texture, Reason: %s\n", SDL_GetError());
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

  // Reload game state
  smbEngine->update();
  smbEngine->render(renderBuffer);

  SDL_UpdateTexture(texture, NULL, renderBuffer, sizeof(uint32_t) * RENDER_WIDTH);

  SDL_RenderClear(renderer);

  // Render the screen
  SDL_RenderSetLogicalSize(renderer, RENDER_WIDTH, RENDER_HEIGHT);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
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
