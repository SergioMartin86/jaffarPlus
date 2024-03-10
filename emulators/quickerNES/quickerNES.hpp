#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <emulator.hpp>
#include <nesInstance.hpp>

#ifdef _USE_SDL2
  #include <SDL.h>
#endif

// Relevant defines for rendering
#define BLIT_SIZE 65536
#define DEFAULT_WIDTH 256
#define DEFAULT_HEIGHT 240

namespace jaffarPlus
{

namespace emulator
{

class QuickerNES final : public Emulator
{
  public:

  static std::string getName() { return "QuickerNES"; }

  // Constructor must only do configuration parsing
  QuickerNES(const nlohmann::json &config) : Emulator(config)
  {
    // Getting initial state file from the configuration
    _initialStateFilePath = jaffarCommon::json::getString(config, "Initial State File Path");

    // For testing purposes, the initial state file can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERNES_OVERRIDE_INITIAL_STATE_FILE_PATH"))
    {
      // Even if we override, we'd like to test whether the originally specified rom still exists to ensure consistency in Github
      std::string initialStateString;
      bool status = jaffarCommon::file::loadStringFromFile(initialStateString, _initialStateFilePath.c_str());
      if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _initialStateFilePath.c_str());
 
      // Now do the proper override
      _initialStateFilePath = std::string(value);
    } 

    // Parsing controller configuration
    _controller1Type = jaffarCommon::json::getString(config, "Controller 1 Type");
    _controller2Type = jaffarCommon::json::getString(config, "Controller 2 Type");

    // Parsing rom file path
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");

    // For testing purposes, the rom file path can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_PATH")) _romFilePath = std::string(value);

    // Parsing rom file SHA1
    _romFileSHA1 = jaffarCommon::json::getString(config, "Rom File SHA1");

    // For testing purposes, the rom file SHA1 can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_SHA1")) _romFileSHA1 = std::string(value);
  };

  void initializeImpl() override
  {
    // Setting game's internal video buffer
    ((emulator_t *)_quickerNES.getInternalEmulatorPointer())->set_pixels(_videoBuffer, DEFAULT_WIDTH + 8);

    // Setting controller types
    _quickerNES.setController1Type(_controller1Type);
    _quickerNES.setController2Type(_controller2Type);

    // Reading from ROM file
    std::string romFileData;
    bool status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = jaffarCommon::hash::getSHA1String(romFileData);
    if (_romFileSHA1 != actualRomSHA1) JAFFAR_THROW_LOGIC("ROM file: '%s' expected SHA1 ('%s') does not concide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());

    // Loading rom into emulator
    _quickerNES.loadROM((uint8_t *)romFileData.data(), romFileData.size());

    // If initial state file defined, load it
    if (_initialStateFilePath.empty() == false)
    {
      // Reading from initial state file
      std::string initialState;
      bool success = jaffarCommon::file::loadStringFromFile(initialState, _initialStateFilePath);
      if (success == false) JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial state file: %s\n", _initialStateFilePath.c_str());

      // Deserializing initial state into the emulator
      jaffarCommon::deserializer::Contiguous d(initialState.data(), initialState.size());
      deserializeState(d);
    }

    // Now disabling state properties, as requested
    disableStateProperties();
  }

  // State advancing function
  void advanceState(const std::string &move) override
  {
    _quickerNES.advanceState(move);
  }

  inline void serializeState(jaffarCommon::serializer::Base &serializer) const override { _quickerNES.serializeState(serializer); };
  inline void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _quickerNES.deserializeState(deserializer); };

  inline void disableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) disableStateProperty(property);
  }
  inline void enableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) enableStateProperty(property);
  }

  inline void printInfo() const override
  {
    printMemoryBlockHash("LRAM");
    printMemoryBlockHash("SRAM");
    printMemoryBlockHash("NTAB");
    printMemoryBlockHash("CHRR");
    printMemoryBlockHash("SPRT");
  }

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "LRAM") return property_t(_quickerNES.getLowMem(), _quickerNES.getLowMemSize());
    if (propertyName == "SRAM") return property_t(_quickerNES.getWorkMem(), _quickerNES.getWorkMemSize());
    if (propertyName == "NTAB") return property_t(_quickerNES.getNametableMem(), _quickerNES.getNametableMemSize());
    if (propertyName == "CHRR") return property_t(_quickerNES.getCHRMem(), _quickerNES.getCHRMemSize());
    if (propertyName == "SPRT") return property_t(_quickerNES.getSpriteMem(), _quickerNES.getSpriteMemSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  inline void enableStateProperty(const std::string &property) override
  {
    _quickerNES.enableStateBlock(property);
  }

  inline void disableStateProperty(const std::string &property) override
  {
    _quickerNES.disableStateBlock(property);
  }

  ////////// Rendering functions (some of these taken from https://github.com/Bindernews/HeadlessQuickNes / MIT License)

  // Function to initalize the video palette
  int32_t *_initF_VideoPalette()
  {
    static int32_t VideoPalette[512];
    const emulator_t::rgb_t *palette = emulator_t::nes_colors;
    for (int i = 0; i < 512; i++)
    {
      VideoPalette[i] = palette[i].red << 16 | palette[i].green << 8 | palette[i].blue | 0xff000000;
    }
    return VideoPalette;
  }

  // Initialize the video palette
  const int32_t *NES_VIDEO_PALETTE = _initF_VideoPalette();

  // Storage for nes renderer state
  int32_t _curBlit[BLIT_SIZE];
  uint8_t _videoBuffer[BLIT_SIZE];

  // Copied from bizinterface.cpp in BizHawk/quicknes
  inline void saveBlit(const void *ePtr, int32_t *dest, const int32_t *colors, int cropleft, int croptop, int cropright, int cropbottom)
  {
    const emulator_t *e = (emulator_t *)ePtr;
    const unsigned char *in_pixels = e->frame().pixels;
    if (in_pixels == NULL) return;
    int32_t *out_pixels = dest;

    for (unsigned h = 0; h < emulator_t::image_height; h++, in_pixels += e->frame().pitch, out_pixels += emulator_t::image_width)
      for (unsigned w = 0; w < emulator_t::image_width; w++)
      {
        unsigned col = e->frame().palette[in_pixels[w]];
        const emulator_t::rgb_t &rgb = e->nes_colors[col];
        unsigned r = rgb.red;
        unsigned g = rgb.green;
        unsigned b = rgb.blue;
        out_pixels[w] = (r << 16) | (g << 8) | (b << 0);
      }
  }

  inline void enableRendering() override { _quickerNES.enableRendering(); }
  inline void disableRendering() override { _quickerNES.disableRendering(); }
  inline void updateRendererState() override { saveBlit(_quickerNES.getInternalEmulatorPointer(), _curBlit, NES_VIDEO_PALETTE, 0, 0, 0, 0); }
  inline void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override { serializer.pushContiguous(_curBlit, sizeof(int32_t) * BLIT_SIZE); }
  inline void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override { deserializer.popContiguous(_curBlit, sizeof(int32_t) * BLIT_SIZE); }
  inline size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

#ifdef _USE_SDL2

  // Window pointer
  SDL_Window *m_window;

  // Renderer
  SDL_Renderer *m_renderer;

  // SDL Textures
  SDL_Texture *m_tex;

  // Destination rect for the texture
  SDL_Rect m_nesDest;

  const SDL_Rect NES_BLIT_RECT = {0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT};

  bool setScale(int scale)
  {
    int winW = DEFAULT_WIDTH * scale;
    int winH = DEFAULT_HEIGHT * scale;

    // Change the window size
    SDL_SetWindowSize(m_window, winW, winH);

    // update the overlay destination
    m_nesDest = {0, 0, winW, winH};

    return true;
  }

  inline void initializeVideoOutput(SDL_Window *window) override
  {
    m_window = window;
    if (!(m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED))) JAFFAR_THROW_RUNTIME("Coult not create SDL renderer in NES emulator");
    if (!(m_tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256))) JAFFAR_THROW_RUNTIME("Coult not create SDL texture in NES emulator");
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    if (!setScale(1)) JAFFAR_THROW_RUNTIME("Coult not set SDL scale in NES emulator");
  }

  inline void updateVideoOutput() override
  {
    void *nesPixels = nullptr;
    int pitch = 0;

    if (SDL_LockTexture(m_tex, nullptr, &nesPixels, &pitch) < 0) JAFFAR_THROW_RUNTIME("Coult not lock texture in NES emulator");

    memcpy(nesPixels, _curBlit, sizeof(int32_t) * BLIT_SIZE);
    SDL_UnlockTexture(m_tex);

    // render to screen
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_tex, &NES_BLIT_RECT, &m_nesDest);

    SDL_RenderPresent(m_renderer);
  }

  inline void finalizeVideoOutput() override
  {
    if (m_tex) SDL_DestroyTexture(m_tex);
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
  }
#else
  inline void initializeVideoOutput() override
  {
  }
  inline void updateVideoOutput() override {}
  inline void finalizeVideoOutput() override {}
#endif

  private:

  void printMemoryBlockHash(const std::string &blockName) const
  {
    auto p = getProperty(blockName);
    auto hash = jaffarCommon::hash::hashToString(jaffarCommon::hash::calculateMetroHash(p.pointer, p.size));
    jaffarCommon::logger::log("[J++] %s Hash:        %s\n", blockName.c_str(), hash.c_str());
  }

  NESInstance _quickerNES;

  std::string _controller1Type;
  std::string _controller2Type;
  std::string _romFilePath;
  std::string _romFileSHA1;
  std::string _initialStateFilePath;
};

} // namespace emulator

} // namespace jaffarPlus