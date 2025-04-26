#pragma once

#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <memory>
#include <nesInstance.hpp>

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
#ifdef __JAFFAR_USE_QUICKERNES_ARKANOID
  static std::string getName() { return "QuickerNESArkanoid"; }
#else
  static std::string getName() { return "QuickerNES"; }
#endif

  // Constructor must only do configuration parsing
  QuickerNES(const nlohmann::json& config) : Emulator(config)
  {
    // Getting initial state file from the configuration
    _initialStateFilePath = jaffarCommon::json::getString(config, "Initial State File Path");

    // For testing purposes, the initial state file can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERNES_OVERRIDE_INITIAL_STATE_FILE_PATH")) _initialStateFilePath = std::string(value);

    // Parsing rom file path
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");

    // For testing purposes, the rom file path can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_PATH")) _romFilePath = std::string(value);

    // Getting initial sequence file path
    _initialSequenceFilePath = jaffarCommon::json::getString(config, "Initial Sequence File Path");

    // For testing purposes, the sequence file path can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERNES_OVERRIDE_INITIAL_SEQUENCE_FILE_PATH")) _initialSequenceFilePath = std::string(value);

    // Parsing rom file SHA1
    _romFileSHA1 = jaffarCommon::json::getString(config, "Rom File SHA1");

    // For testing purposes, the rom file SHA1 can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_SHA1")) _romFileSHA1 = std::string(value);

    // Parsing initial RAM Data file
    _initialRAMDataFilePath = jaffarCommon::json::getString(config, "Initial RAM Data File Path");

    // Getting Nametable size to use
    _NTABBlockSize = jaffarCommon::json::getNumber<size_t>(config, "Nametable Block Size");

    // Getting disabled state properties
    const auto disabledStateProperties = jaffarCommon::json::getArray<std::string>(config, "Disabled State Properties");
    for (const auto& property : disabledStateProperties) _disabledStateProperties.push_back(property);

    // Creating internal emulator instance
    _quickerNES = std::make_unique<NESInstance>(config);
  };

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override { return _quickerNES->getInputParser(); }

  void initializeImpl() override
  {
    // Setting game's internal video buffer
    ((emulator_t*)_quickerNES->getInternalEmulatorPointer())->set_pixels(_videoBuffer, DEFAULT_WIDTH + 8);

    // Reading from ROM file
    std::string romFileData;
    bool        status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = jaffarCommon::hash::getSHA1String(romFileData);
    if (_romFileSHA1 != actualRomSHA1)
      JAFFAR_THROW_LOGIC("ROM file: '%s' expected SHA1 ('%s') does not concide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());

    // Loading rom into emulator
    _quickerNES->loadROM((uint8_t*)romFileData.data(), romFileData.size());

    // If initial state file defined, load it
    if (_initialStateFilePath.empty() == false)
    {
      // Reading from initial state file
      std::string initialState;
      bool        success = jaffarCommon::file::loadStringFromFile(initialState, _initialStateFilePath);
      if (success == false) JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial state file: %s\n", _initialStateFilePath.c_str());

      // Deserializing initial state into the emulator
      enableStateProperties();
      jaffarCommon::deserializer::Contiguous d(initialState.data(), initialState.size());
      deserializeState(d);
    }

    // Now disabling state properties, as requested
    disableStateProperties();

    // Setting Nametable block size to serialize. Some games don't use the entire memory so it's ok to reduce it
    _quickerNES->setNTABBlockSize(_NTABBlockSize);

    // Getting input parser from the internal emulator
    const auto inputParser = _quickerNES->getInputParser();

    // Advancing the state using the initial sequence, if provided
    if (_initialSequenceFilePath != "")
    {
      // Load initial sequence
      std::string initialSequenceFileString;
      if (jaffarCommon::file::loadStringFromFile(initialSequenceFileString, _initialSequenceFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial sequence file: %s\n", _initialSequenceFilePath.c_str());

      // Getting input sequence
      const auto initialSequence = jaffarCommon::string::split(initialSequenceFileString, '\0');

      // Running inputs in the initial sequence
      for (const auto& inputString : initialSequence) advanceStateImpl(inputParser->parseInputString(inputString));
    }

    // Pushing initial RAM data
    if (_initialRAMDataFilePath != "")
    {
      // Load initial RAM Data
      std::string initialRAMDataString;
      if (jaffarCommon::file::loadStringFromFile(initialRAMDataString, _initialRAMDataFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from RAM Data file: %s\n", _initialRAMDataFilePath.c_str());

      // Pushing data into RAM
      memcpy(_quickerNES->getLowMem(), initialRAMDataString.data(), 0x800);
    }
  }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override { _quickerNES->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override { _quickerNES->serializeState(serializer); };
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override { _quickerNES->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override
  {
    printMemoryBlockHash("LRAM");
    printMemoryBlockHash("SRAM");
    printMemoryBlockHash("NTAB");
    printMemoryBlockHash("CHRR");
    printMemoryBlockHash("SPRT");
  }

  property_t getProperty(const std::string& propertyName) const override
  {
    if (propertyName == "LRAM") return property_t(_quickerNES->getLowMem(), _quickerNES->getLowMemSize());
    if (propertyName == "SRAM") return property_t(_quickerNES->getWorkMem(), _quickerNES->getWorkMemSize());
    if (propertyName == "NTAB") return property_t(_quickerNES->getNametableMem(), _quickerNES->getNametableMemSize());
    if (propertyName == "CHRR") return property_t(_quickerNES->getCHRMem(), _quickerNES->getCHRMemSize());
    if (propertyName == "SPRT") return property_t(_quickerNES->getSpriteMem(), _quickerNES->getSpriteMemSize());
    if (propertyName == "PALR") return property_t(_quickerNES->getPALMem(), _quickerNES->getPALMemSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void enableStateProperty(const std::string& property) { _quickerNES->enableStateBlock(property); }

  __INLINE__ void disableStateProperty(const std::string& property) { _quickerNES->disableStateBlock(property); }

  ////////// Rendering functions (some of these taken from https://github.com/Bindernews/HeadlessQuickNes / MIT License)

  // Function to initalize the video palette
  int32_t* _initF_VideoPalette()
  {
    static int32_t           VideoPalette[512];
    const emulator_t::rgb_t* palette = emulator_t::nes_colors;
    for (int i = 0; i < 512; i++) { VideoPalette[i] = palette[i].red << 16 | palette[i].green << 8 | palette[i].blue | 0xff000000; }
    return VideoPalette;
  }

  // Initialize the video palette
  const int32_t* NES_VIDEO_PALETTE = _initF_VideoPalette();

  // Storage for nes renderer state
  int32_t _curBlit[BLIT_SIZE];
  uint8_t _videoBuffer[BLIT_SIZE];

  // Copied from bizinterface.cpp in BizHawk/quicknes
  __INLINE__ void saveBlit(const void* ePtr, int32_t* dest, const int32_t* colors, int cropleft, int croptop, int cropright, int cropbottom)
  {
    const emulator_t*    e         = (emulator_t*)ePtr;
    const unsigned char* in_pixels = e->frame().pixels;
    if (in_pixels == NULL) return;
    int32_t* out_pixels = dest;

    for (unsigned h = 0; h < emulator_t::image_height; h++, in_pixels += e->frame().pitch, out_pixels += emulator_t::image_width)
      for (unsigned w = 0; w < emulator_t::image_width; w++)
      {
        unsigned                 col = e->frame().palette[in_pixels[w]];
        const emulator_t::rgb_t& rgb = e->nes_colors[col];
        unsigned                 r   = rgb.red;
        unsigned                 g   = rgb.green;
        unsigned                 b   = rgb.blue;
        out_pixels[w]                = (r << 16) | (g << 8) | (b << 0);
      }
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    // Opening rendering window
    SDL_SetMainReady();

    // We can only call SDL_InitSubSystem once
    if (!SDL_WasInit(SDL_INIT_VIDEO))
      if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) JAFFAR_THROW_LOGIC("Failed to initialize video: %s", SDL_GetError());

    m_window = SDL_CreateWindow("JaffarPlus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WIDTH, DEFAULT_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (m_window == nullptr) JAFFAR_THROW_LOGIC("Coult not open SDL window");

    // Creating SDL renderer
    if (!(m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED))) JAFFAR_THROW_RUNTIME("Coult not create SDL renderer in NES emulator");
    if (!(m_tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256)))
      JAFFAR_THROW_RUNTIME("Coult not create SDL texture in NES emulator");
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    if (!setScale(1)) JAFFAR_THROW_RUNTIME("Coult not set SDL scale in NES emulator");
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override
  {
    if (m_tex) SDL_DestroyTexture(m_tex);
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window) SDL_DestroyWindow(m_window);
  }

  __INLINE__ void enableRendering() override { _quickerNES->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerNES->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override
  {
    saveBlit(_quickerNES->getInternalEmulatorPointer(), _curBlit, NES_VIDEO_PALETTE, 0, 0, 0, 0);
  }
  __INLINE__ void   serializeRendererState(jaffarCommon::serializer::Base& serializer) const override { serializer.pushContiguous(_curBlit, sizeof(int32_t) * BLIT_SIZE); }
  __INLINE__ void   deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override { deserializer.popContiguous(_curBlit, sizeof(int32_t) * BLIT_SIZE); }
  __INLINE__ size_t getRendererStateSize() const override
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  // Window pointer
  SDL_Window* m_window;

  // Renderer
  SDL_Renderer* m_renderer;

  // SDL Textures
  SDL_Texture* m_tex;

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

  __INLINE__ void showRender() override
  {
    void* nesPixels = nullptr;
    int   pitch     = 0;

    if (SDL_LockTexture(m_tex, nullptr, &nesPixels, &pitch) < 0) JAFFAR_THROW_RUNTIME("Coult not lock texture in NES emulator");

    memcpy(nesPixels, _curBlit, sizeof(int32_t) * BLIT_SIZE);
    SDL_UnlockTexture(m_tex);

    // render to screen
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_tex, &NES_BLIT_RECT, &m_nesDest);

    SDL_RenderPresent(m_renderer);
  }

private:
  __INLINE__ void disableStateProperties()
  {
    for (const auto& property : _disabledStateProperties) disableStateProperty(property);
  }
  __INLINE__ void enableStateProperties()
  {
    for (const auto& property : _disabledStateProperties) enableStateProperty(property);
  }

  void printMemoryBlockHash(const std::string& blockName) const
  {
    auto p    = getProperty(blockName);
    auto hash = jaffarCommon::hash::hashToString(jaffarCommon::hash::calculateMetroHash(p.pointer, p.size));
    jaffarCommon::logger::log("[J+] %s Hash:        %s\n", blockName.c_str(), hash.c_str());
  }

  std::unique_ptr<NESInstance> _quickerNES;

  size_t      _NTABBlockSize;
  std::string _romFilePath;
  std::string _romFileSHA1;

  std::string _initialStateFilePath;
  std::string _initialSequenceFilePath;
  std::string _initialRAMDataFilePath;

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;
};

} // namespace emulator

} // namespace jaffarPlus