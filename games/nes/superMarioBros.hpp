#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <atomic>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class SuperMarioBros final : public jaffarPlus::Game
{
public:

  static __INLINE__ std::string getName() { return "NES / Super Mario Bros"; }

  SuperMarioBros(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
{
    _traceFilePath = jaffarCommon::json::getString(config, "Trace File Path");

    // Loading trace
    if (_traceFilePath != "")
    {
      _useTrace = true;
      std::string traceData;
      bool        status = jaffarCommon::file::loadStringFromFile(traceData, _traceFilePath.c_str());
      if (status == false) JAFFAR_THROW_LOGIC("Could not find/read trace file: %s\n", _traceFilePath.c_str());

      std::istringstream f(traceData);
      std::string line;
      while (std::getline(f, line))
      {
        auto coordinates = jaffarCommon::string::split(line, ' ');
        float posx = std::atof(coordinates[0].c_str());
        float posy = std::atof(coordinates[1].c_str());
        float velx = std::atof(coordinates[2].c_str());
        float vely = std::atof(coordinates[3].c_str());
        float screenPosx = std::atof(coordinates[4].c_str());
        _trace.push_back(traceEntry_t{
          .posx = posx,
          .posy = posy,
          .velx = velx,
          .vely = vely,
          .screenPosx = screenPosx
        });
      }
    }
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Thanks to https://datacrystal.romhacking.net/wiki/Super_Player_Bros.:RAM_map and https://tasvideos.org/GameResources/NES/SuperMarioBros for helping me find some of these items
    registerGameProperty("Player Animation"          ,&_lowMem[0x0001], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player State"              ,&_lowMem[0x000E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos X1"             ,&_lowMem[0x006D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos X2"             ,&_lowMem[0x0086], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos X3"             ,&_lowMem[0x0400], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos Y2"             ,&_lowMem[0x00CE], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos Y3"             ,&_lowMem[0x0416], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Moving Direction"   ,&_lowMem[0x0045], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Facing Direction"   ,&_lowMem[0x0033], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Floating Mode"      ,&_lowMem[0x001D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Walking Mode"       ,&_lowMem[0x0702], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Walking Delay"      ,&_lowMem[0x070C], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Walking Frame"      ,&_lowMem[0x070D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Max Vel Left"       ,&_lowMem[0x0450], Property::datatype_t::dt_int8  , Property::endianness_t::little);
    registerGameProperty("Player Max Vel Right"      ,&_lowMem[0x0456], Property::datatype_t::dt_int8  , Property::endianness_t::little);
    registerGameProperty("Player Vel X1"             ,&_lowMem[0x0057], Property::datatype_t::dt_int8  , Property::endianness_t::little);
    registerGameProperty("Player Momentum X"         ,&_lowMem[0x0705], Property::datatype_t::dt_int8  , Property::endianness_t::little);
    registerGameProperty("Player Vel Y1"             ,&_lowMem[0x009F], Property::datatype_t::dt_int8  , Property::endianness_t::little);
    registerGameProperty("Player Vel Y2"             ,&_lowMem[0x0433], Property::datatype_t::dt_int8  , Property::endianness_t::little);
    registerGameProperty("Player Gravity"            ,&_lowMem[0x0709], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Friction"           ,&_lowMem[0x0701], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Time Left 100"             ,&_lowMem[0x07F8], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Time Left 10"              ,&_lowMem[0x07F9], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Time Left 1"               ,&_lowMem[0x07FA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Screen Pos X1"             ,&_lowMem[0x071A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Pos X2"             ,&_lowMem[0x071C], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Screen Pos X3"             ,&_lowMem[0x071B], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Current World"             ,&_lowMem[0x075F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Current Stage"             ,&_lowMem[0x075C], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Level Entry Flag"          ,&_lowMem[0x0752], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Game Mode"                 ,&_lowMem[0x0770], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 1 Active"            ,&_lowMem[0x000F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 2 Active"            ,&_lowMem[0x0010], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 3 Active"            ,&_lowMem[0x0011], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 4 Active"            ,&_lowMem[0x0012], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 5 Active"            ,&_lowMem[0x0013], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 1 State"             ,&_lowMem[0x001E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 2 State"             ,&_lowMem[0x001F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 3 State"             ,&_lowMem[0x0020], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 4 State"             ,&_lowMem[0x0021], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 5 State"             ,&_lowMem[0x0022], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 1 Type"              ,&_lowMem[0x0016], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 2 Type"              ,&_lowMem[0x0017], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 3 Type"              ,&_lowMem[0x0018], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 4 Type"              ,&_lowMem[0x0019], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy 5 Type"              ,&_lowMem[0x001A], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Collision"          ,&_lowMem[0x0490], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy Collision"           ,&_lowMem[0x0491], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Hit Detection Flag"        ,&_lowMem[0x0722], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Power Up Active"           ,&_lowMem[0x0014], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Animation Timer"           ,&_lowMem[0x0781], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Jump Swim Timer"           ,&_lowMem[0x0782], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Running Timer"             ,&_lowMem[0x0783], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Block Bounce Timer"        ,&_lowMem[0x0784], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Side Collision Timer"      ,&_lowMem[0x0785], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Jumpspring Timer"          ,&_lowMem[0x0786], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Game Control Timer"        ,&_lowMem[0x0787], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Climb Side Timer"          ,&_lowMem[0x0789], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy Frame Timer"         ,&_lowMem[0x078A], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Frenzy Enemy Timer"        ,&_lowMem[0x078F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bowser Fire Timer"         ,&_lowMem[0x0790], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Stomp Timer"               ,&_lowMem[0x0791], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Air Bubble Timer"          ,&_lowMem[0x0792], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Fall Pit Timer"            ,&_lowMem[0x0795], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Multi Coin Block Timer"    ,&_lowMem[0x079D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Invincible Timer"          ,&_lowMem[0x079E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Star Timer"                ,&_lowMem[0x079F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Input"              ,&_lowMem[0x06FC], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Buttons"            ,&_lowMem[0x074A], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player GamePad 1"          ,&_lowMem[0x000A], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player GamePad 2"          ,&_lowMem[0x000D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Warp Area Offset"          ,&_lowMem[0x0750], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Warp Zone Control"         ,&_lowMem[0x06D6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Next Screen"               ,&_lowMem[0x071B], Property::datatype_t::dt_uint8, Property::endianness_t::little);

      
    _screenPosX1              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X1"             )]->getPointer();
    _playerAnimation          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Animation"          )]->getPointer();
    _playerState              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player State"              )]->getPointer();
    _playerPosX1              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X1"             )]->getPointer();
    _playerPosX2              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X2"             )]->getPointer();
    _playerPosX3              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X3"             )]->getPointer();
    _playerPosY2              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y2"             )]->getPointer();
    _playerPosY3              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y3"             )]->getPointer();
    _playerMovingDirection    =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Moving Direction"   )]->getPointer();
    _playerFacingDirection    =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Facing Direction"   )]->getPointer();
    _playerFloatingMode       =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Floating Mode"      )]->getPointer();
    _playerWalkingMode        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Walking Mode"       )]->getPointer();
    _playerWalkingDelay       =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Walking Delay"      )]->getPointer();
    _playerWalkingFrame       =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Walking Frame"      )]->getPointer();
    _playerMaxVelLeft         =  (int8_t  *)_propertyMap[jaffarCommon::hash::hashString("Player Max Vel Left"       )]->getPointer();
    _playerMaxVelRight        =  (int8_t  *)_propertyMap[jaffarCommon::hash::hashString("Player Max Vel Right"      )]->getPointer();
    _playerVelX1              =  (int8_t  *)_propertyMap[jaffarCommon::hash::hashString("Player Vel X1"             )]->getPointer();
    _playerMomentumX          =  (int8_t  *)_propertyMap[jaffarCommon::hash::hashString("Player Momentum X"         )]->getPointer();
    _playerVelY1              =  (int8_t  *)_propertyMap[jaffarCommon::hash::hashString("Player Vel Y1"             )]->getPointer();
    _playerVelY2              =  (int8_t  *)_propertyMap[jaffarCommon::hash::hashString("Player Vel Y2"             )]->getPointer();
    _playerGravity            =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Gravity"            )]->getPointer();
    _playerFriction           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Friction"           )]->getPointer();
    _timeLeft100              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Time Left 100"             )]->getPointer();
    _timeLeft10               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Time Left 10"              )]->getPointer();
    _timeLeft1                =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Time Left 1"               )]->getPointer();
    _screenPosX2              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X2"             )]->getPointer();
    _screenPosX3              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X3"             )]->getPointer();
    _currentWorld             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Current World"             )]->getPointer();
    _currentStage             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Current Stage"             )]->getPointer();
    _levelEntryFlag           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Level Entry Flag"          )]->getPointer();
    _gameMode                 =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game Mode"                 )]->getPointer();
    _enemy1Active             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 Active"            )]->getPointer();
    _enemy2Active             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 Active"            )]->getPointer();
    _enemy3Active             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 3 Active"            )]->getPointer();
    _enemy4Active             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 4 Active"            )]->getPointer();
    _enemy5Active             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 5 Active"            )]->getPointer();
    _enemy1State              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 State"             )]->getPointer();
    _enemy2State              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 State"             )]->getPointer();
    _enemy3State              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 3 State"             )]->getPointer();
    _enemy4State              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 4 State"             )]->getPointer();
    _enemy5State              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 5 State"             )]->getPointer();
    _enemy1Type               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 Type"              )]->getPointer();
    _enemy2Type               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 Type"              )]->getPointer();
    _enemy3Type               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 3 Type"              )]->getPointer();
    _enemy4Type               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 4 Type"              )]->getPointer();
    _enemy5Type               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy 5 Type"              )]->getPointer();
    _playerCollision          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Collision"          )]->getPointer();
    _enemyCollision           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy Collision"           )]->getPointer();
    _hitDetectionFlag         =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Hit Detection Flag"        )]->getPointer();
    _powerUpActive            =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Power Up Active"           )]->getPointer();
    _animationTimer           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Animation Timer"           )]->getPointer();
    _jumpSwimTimer            =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Jump Swim Timer"           )]->getPointer();
    _runningTimer             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Running Timer"             )]->getPointer();
    _blockBounceTimer         =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Block Bounce Timer"        )]->getPointer();
    _sideCollisionTimer       =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Side Collision Timer"      )]->getPointer();
    _jumpspringTimer          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Jumpspring Timer"          )]->getPointer();
    _gameControlTimer         =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game Control Timer"        )]->getPointer();
    _climbSideTimer           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Climb Side Timer"          )]->getPointer();
    _enemyFrameTimer          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy Frame Timer"         )]->getPointer();
    _frenzyEnemyTimer         =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Frenzy Enemy Timer"        )]->getPointer();
    _bowserFireTimer          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bowser Fire Timer"         )]->getPointer();
    _stompTimer               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Stomp Timer"               )]->getPointer();
    _airBubbleTimer           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Air Bubble Timer"          )]->getPointer();
    _fallPitTimer             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Fall Pit Timer"            )]->getPointer();
    _multiCoinBlockTimer      =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Multi Coin Block Timer"    )]->getPointer();
    _invincibleTimer          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Invincible Timer"          )]->getPointer();
    _starTimer                =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Star Timer"                )]->getPointer();
    _playerInput              =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input"              )]->getPointer();
    _playerButtons            =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Buttons"            )]->getPointer();
    _playerGamePad1           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player GamePad 1"          )]->getPointer();
    _playerGamePad2           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player GamePad 2"          )]->getPointer();
    _warpAreaOffset           =  (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Warp Area Offset"          )]->getPointer();
    _warpZoneControl          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Warp Zone Control"         )]->getPointer();
    _nextScreen               =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Next Screen"               )]->getPointer();

    registerGameProperty("Player Pos X"          , &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Y"          , &_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _playerPosX = 0;
    _playerPosY = 0;
    _screenPosX = 0;
    _playerScreenOffset = 0;
    _currentStep = 0;

    // Getting emulator save state size
    _tempStorageSize = _emulator->getStateSize();
    _tempStorage     = (uint8_t*)malloc(_tempStorageSize);

    _nullInputIdx      = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // #define SMB_HASH_LOOKAHEAD

    #ifdef SMB_HASH_LOOKAHEAD
    // Storing current state
    jaffarCommon::serializer::Contiguous s(_tempStorage, _tempStorageSize);
    _emulator->serializeState(s);

    //  Advancing emulator state
    _emulator->advanceState(_nullInputIdx);
    #endif

    // If entering the pipe, hash with timer to wait for going down
    if (*_playerState == 3) hashEngine.Update(_currentStep);

    #define __HASH_METHOD_0
    // #define __HASH_METHOD_1

    #ifdef __HASH_METHOD_0
    {
      const size_t start = 0x0000;
      const size_t end = 0x0008;
      const size_t diff = end - start;
      hashEngine.Update(&_lowMem[start], diff);
    }
    // // Skipping input state
    {
      const size_t start = 0x000E;
      const size_t end = 0x06FB;
      const size_t diff = end - start;
      hashEngine.Update(&_lowMem[start], diff);
    }
    //  Skipping input state
    {
      const size_t start = 0x06FE;
      const size_t end = 0x0747;
      const size_t diff = end - start;
      hashEngine.Update(&_lowMem[start], diff);
    }
    // Skipping input state
    {
      const size_t start = 0x074E;
      const size_t end = 0x077E;
      const size_t diff = end - start;
      hashEngine.Update(&_lowMem[start], diff);
    }
    // Skipping timers state
    {
      const size_t start = 0x07C0;
      const size_t end = 0x07FF;
      const size_t diff = end - start;
      hashEngine.Update(&_lowMem[start], diff);
    }
    
    hashEngine.Update(*_animationTimer);
    hashEngine.Update(*_jumpSwimTimer);
    hashEngine.Update(*_runningTimer);
    hashEngine.Update(*_blockBounceTimer);
    hashEngine.Update(*_sideCollisionTimer);
     hashEngine.Update(*_jumpspringTimer);
    hashEngine.Update(*_gameControlTimer % 2);
     hashEngine.Update(*_climbSideTimer);
    hashEngine.Update(*_enemyFrameTimer % 2);
    hashEngine.Update(*_frenzyEnemyTimer % 2);
    hashEngine.Update(*_bowserFireTimer % 2);
    hashEngine.Update(*_stompTimer % 2);
    hashEngine.Update(*_airBubbleTimer % 2);
    hashEngine.Update(*_fallPitTimer % 2);
    hashEngine.Update(*_multiCoinBlockTimer % 2);
    hashEngine.Update(*_invincibleTimer % 2);
    hashEngine.Update(*_starTimer % 2);
    hashEngine.Update(*_powerUpActive % 2);

    #endif

    #ifdef __HASH_METHOD_1
    
      // Adding fixed hash elements
    hashEngine.Update(*_screenPosX1);
    hashEngine.Update(*_playerAnimation);
    hashEngine.Update(*_playerState);
    hashEngine.Update(*_playerPosX1);
    hashEngine.Update(*_playerPosX2);
    hashEngine.Update(*_playerPosX3);
    hashEngine.Update(*_playerPosY2);
    hashEngine.Update(*_playerPosY2);
    hashEngine.Update(*_playerMomentumX);
    hashEngine.Update(*_playerFacingDirection);
    hashEngine.Update(*_playerMovingDirection);
    hashEngine.Update(*_playerFloatingMode);
    hashEngine.Update(*_playerWalkingMode);
    hashEngine.Update(*_playerWalkingDelay);
    hashEngine.Update(*_playerWalkingFrame);
    hashEngine.Update(*_playerMaxVelLeft);
    hashEngine.Update(*_playerMaxVelRight);
    hashEngine.Update(*_playerVelX1);
    hashEngine.Update(*_playerVelY1);
    hashEngine.Update(*_playerVelY2);
    hashEngine.Update(*_playerGravity);
    hashEngine.Update(*_playerFriction);

    hashEngine.Update(*_screenPosX2);
    hashEngine.Update(*_screenPosX3);

    hashEngine.Update(*_currentWorld);
    hashEngine.Update(*_currentStage);
    hashEngine.Update(*_levelEntryFlag);
    hashEngine.Update(*_gameMode);

    // hashEngine.Update(*_enemy1Active);
    // hashEngine.Update(*_enemy2Active);
    // hashEngine.Update(*_enemy3Active);
    // hashEngine.Update(*_enemy4Active);
    // hashEngine.Update(*_enemy5Active);

    // hashEngine.Update(*_enemy1State);
    // hashEngine.Update(*_enemy2State);
    // hashEngine.Update(*_enemy3State);
    // hashEngine.Update(*_enemy4State);
    // hashEngine.Update(*_enemy5State);

    hashEngine.Update(*_enemy1Type);
    hashEngine.Update(*_enemy2Type);
    hashEngine.Update(*_enemy3Type);
    hashEngine.Update(*_enemy4Type);
    hashEngine.Update(*_enemy5Type);

    hashEngine.Update(*_playerCollision);
    // hashEngine.Update(*_enemyCollision);
    hashEngine.Update(*_hitDetectionFlag);

    hashEngine.Update(*_animationTimer);
    hashEngine.Update(*_jumpSwimTimer);
    hashEngine.Update(*_runningTimer);
    hashEngine.Update(*_blockBounceTimer);
    hashEngine.Update(*_sideCollisionTimer);
     hashEngine.Update(*_jumpspringTimer);
    // hashEngine.Update(*_gameControlTimer);
     hashEngine.Update(*_climbSideTimer);
    // hashEngine.Update(*_enemyFrameTimer);
    // hashEngine.Update(*_frenzyEnemyTimer);
    // hashEngine.Update(*_bowserFireTimer);
    // hashEngine.Update(*_stompTimer);
    // hashEngine.Update(*_airBubbleTimer);
    // hashEngine.Update(*_fallPitTimer);
    // hashEngine.Update(*_multiCoinBlockTimer);
    // hashEngine.Update(*_invincibleTimer);
    // hashEngine.Update(*_starTimer);
    // hashEngine.Update(*_powerUpActive);

    hashEngine.Update(*_warpAreaOffset);
    hashEngine.Update(*_warpZoneControl);
    hashEngine.Update(*_nextScreen);

    hashEngine.Update(_lowMem[0x0000]); // 	Temp/various uses. Is used in vertical physics for gravity acceleration (value copied from 0x0709).
    hashEngine.Update(_lowMem[0x0002]); // 	Temp/various. Something to do with player y (but it skips to 0 every x frames, even when you dont move)
    hashEngine.Update(_lowMem[0x0003]); // 	Player's direction (and others). 1 - Right 2 - Left
    hashEngine.Update(_lowMem[0x0005]); // 	Something to do with player x (same as 0x0002)
    hashEngine.Update(_lowMem[0x03AD]); // 	Player x pos within current screen offset
    hashEngine.Update(_lowMem[0x03B8]); // 	Player y pos within current screen (vertical screens always offset at 0?)
    hashEngine.Update(_lowMem[0x0400]); // 	Player Object X-MoveForce.
    hashEngine.Update(_lowMem[0x0416]); // 	Player Object YMF_Dummy.
    hashEngine.Update(_lowMem[0x0433]); // 	Player vertical fractional velocity. This is not accounted for when clamping to max fall velocity etc.
    hashEngine.Update(_lowMem[0x0450]); // 	Player max velocity to the left. Values taken from 0xE4: max walking vel 0xD8: max running vel (changes to this when pressing B+L)
    hashEngine.Update(_lowMem[0x0456]); // 	Player max velocity to the right 0x1C - max walking vel 0x30 - max running vel (changes to this when pressing B+R)
    hashEngine.Update(_lowMem[0x0490]); // 	Player Collision_Bits,if you collided with Any Block / Object / Brick , Then Value will change to 0xFE otherwise it will stay 0xFF.
    hashEngine.Update(_lowMem[0x04AC]); // 	Player hitbox (1x4 bytes, <<x1,y1> <x2,y2>>)
    hashEngine.Update(&_lowMem[0x0500], 0x019F); // -0x069F	Current tile (Does not effect graphics)
    hashEngine.Update(_lowMem[0x06D5]); // 	PlayerGfx_Offset , Player sprite Mario state (didn't check them myself)
    hashEngine.Update(_lowMem[0x0700]); // 	Player X-Speed Absolute ,Player speed in _either_ direction (0 - 0x28)
    hashEngine.Update(_lowMem[0x0701]); // 	Friction Adder High ,Is breaking when 1 (freeze this and you immediately stand still when you stop moving)
    hashEngine.Update(_lowMem[0x0702]); // 	Walk animation (didn't check values)
    hashEngine.Update(_lowMem[0x0704]); // 	Swimming Flag ,Set to 0 to swim
    hashEngine.Update(_lowMem[0x0705]); // 	Player X-MoveForce, runs when you press left or right
    hashEngine.Update(_lowMem[0x0709]); // 	Current gravity which will be applied to the player sprite (see 0x0000, 0x0433 and Notes page).
    hashEngine.Update(_lowMem[0x070A]); // 	Current fall gravity (not sure how this is decided). 
    hashEngine.Update(_lowMem[0x070B]); // 	When not 0, runs big-small animation (but does not affect anything internally)
    hashEngine.Update(_lowMem[0x070C]); // 	Player walk animation delay, in game frames (1/60 s). 0x05=slow walk, 0x03=full walk speed, 0x02=fastest/running speed
    hashEngine.Update(_lowMem[0x070D]); // 	Player walk animation current frame index (0,1,2,0,etc)
    hashEngine.Update(_lowMem[0x0714]); // 	0x04 when ducking as big mario, 0 otherwise (also when ducking as small). Keeps being 4 when you slide of an edge while ducking (so does not affect image, but does when set to 4 and being small...). When this register is frozen, you can move like normal, you're just ducking while doing so.
    hashEngine.Update(_lowMem[0x071C]); // 	ScreenEdge X-Position, loads next screen when player past it?
    hashEngine.Update(_lowMem[0x071D]); // 	Player x position, moves screen position forward when this moves
    hashEngine.Update(_lowMem[0x071E]); // 	Column Sets . Counts back, done when this is FF. you can see the level being built in memory up as this counter progresses, one frame at a time.
    hashEngine.Update(_lowMem[0x071F]); // 	AreaParser TaskNumber, runs from 4 to 0.
    hashEngine.Update(_lowMem[0x0722]); // 	Player HitDetect Flag. Determines whether allow Player to Pass through from Bricks or not. (Settting it to 0xFF will cause Player to Penetrate through Walls/Bricks)
    hashEngine.Update(_lowMem[0x0723]); // 	Scroll Lock, 1 = Prevent screen from scrolling right, i.e. Bowser, warp zone, etc. 0 = Allow scroll.
    hashEngine.Update(_lowMem[0x0754]); // 	Player's state. This also affects hitting stuff. Decreases when you eat a mushroom! Increases when you get hit.
    hashEngine.Update(_lowMem[0x0755]); // 	Player_Position For Scroll. It moves up to 0x70 (sometimes even 0x72) when player moves. When this register is frozen, NOTHING changes in the level, not even internally.
    
    #endif

    #ifdef SMB_HASH_LOOKAHEAD
    // Recovering emulator state
    jaffarCommon::deserializer::Contiguous d(_tempStorage, _tempStorageSize);
    _emulator->deserializeState(d);
    #endif
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _playerPosX = (float)*_playerPosX1 * 256.0f + (float)*_playerPosX2 + ((float)*_playerPosX3) / 256.0f;
    _playerPosY = (float)*_playerPosY2 + (float)*_playerPosY3 / 256.0f;
    if (_playerPosY > 210.0f) _playerPosY -= 256.0f;
    _screenPosX = (float)*_screenPosX1 * 256.0f + (float)*_screenPosX2;
    _playerScreenOffset = _playerPosX - _screenPosX;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;

    _traceMagnet.intensityPosX = 0.0;
    _traceMagnet.intensityPosY = 0.0;
    _traceMagnet.intensityVelX = 0.0;
    _traceMagnet.intensityVelY = 0.0;
    _traceMagnet.intensityScreenX = 0.0;
    _traceMagnet.offset = 0;

  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _playerDistanceToPointX = std::abs(_pointMagnet.x - _playerPosX);
    _playerDistanceToPointY = std::abs(_pointMagnet.y - _playerPosY);
    _playerDistanceToPoint  = sqrtf(_playerDistanceToPointX * _playerDistanceToPointX + _playerDistanceToPointY * _playerDistanceToPointY);

      // Updating trace stuff
    if (_useTrace == true)
    {
      _traceStep = (uint16_t) std::max(std::min( (int)_currentStep + _traceMagnet.offset, (int) _trace.size() - 1), 0);
      _tracePosTargetX = _trace[_traceStep].posx;
      _tracePosTargetY = _trace[_traceStep].posy;
      _traceVelTargetX = _trace[_traceStep].velx;
      _traceVelTargetY = _trace[_traceStep].vely;
      _traceScreenPosTargetX = _trace[_traceStep].screenPosx;

      // Updating distance to trace point
      _tracePosDistanceX = std::abs(_tracePosTargetX - _playerPosX);
      _tracePosDistanceY = std::abs(_tracePosTargetY - _playerPosY);
      _traceVelDistanceX = std::abs(_traceVelTargetX - (float)*_playerVelX1);
      _traceVelDistanceY = std::abs(_traceVelTargetY - (float)*_playerVelY1);
      _traceScreenPosDistanceX = std::abs(_traceScreenPosTargetX - _playerScreenOffset);

      _tracePosEffectX = _tracePosDistanceX * _traceMagnet.intensityPosX;
      _tracePosEffectY = _tracePosDistanceY * _traceMagnet.intensityPosY;
      _traceVelEffectX = _traceVelDistanceX * _traceMagnet.intensityVelX / 40.0; // Normalized by maximum
      _traceVelEffectY = _traceVelDistanceY * _traceMagnet.intensityVelY / 5.0; // Normalized by maximum
      _traceScreenPosEffectX = _traceScreenPosDistanceX * _traceMagnet.intensityScreenX;
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(_lowMem, 0x800);
     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_playerPosX, sizeof(_playerPosX));
     serializer.push(&_playerPosY, sizeof(_playerPosY));
     serializer.push(&_screenPosX, sizeof(_screenPosX));
     serializer.push(&_playerScreenOffset, sizeof(_playerScreenOffset));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(_lowMem, 0x800);
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_playerPosX, sizeof(_playerPosX));
     deserializer.pop(&_playerPosY, sizeof(_playerPosY));
     deserializer.pop(&_screenPosX, sizeof(_screenPosX));
     deserializer.pop(&_playerScreenOffset, sizeof(_playerScreenOffset));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // If trace is used, compute its magnet's effect
    if (_useTrace == true)  reward += -1.0 * _tracePosEffectX;
    if (_useTrace == true)  reward += -1.0 * _tracePosEffectY;
    if (_useTrace == true)  reward += -1.0 * _traceVelEffectX;
    if (_useTrace == true)  reward += -1.0 * _traceVelEffectY;
    if (_useTrace == true)  reward += -1.0 * _traceScreenPosEffectX;
    
    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _playerDistanceToPoint;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:            %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Current World-Stage:     %1u-%1u (Base Values: %2u - %2u)\n", *_currentWorld + 1, *_currentStage + 1, *_currentWorld, *_currentStage);
    jaffarCommon::logger::log("[J+]  + Time Left:               %1u%1u%1u\n", *_timeLeft100, *_timeLeft10, *_timeLeft1);
    jaffarCommon::logger::log("[J+]  + Player Animation:        %02u\n", *_playerAnimation);
    jaffarCommon::logger::log("[J+]  + Player State:            %02u\n", *_playerState);
    jaffarCommon::logger::log("[J+]  + Screen Pos X:            %5.3f (%03u %03u %03u)\n", _screenPosX, *_screenPosX1, *_screenPosX2, *_screenPosX3);
    jaffarCommon::logger::log("[J+]  + Player Pos X:            %5.3f (%03u %03u %03u)\n", _playerPosX, *_playerPosX1, *_playerPosX2, *_playerPosX3);
    jaffarCommon::logger::log("[J+]  + Player / Screen Offset:  %5.3f\n", _playerScreenOffset);
    jaffarCommon::logger::log("[J+]  + Player Pos Y:            %5.3f\n", _playerPosY);
    jaffarCommon::logger::log("[J+]  + Player SubPixel X/Y:     %02u / %02u\n", *_playerPosX3, *_playerPosY3);
    jaffarCommon::logger::log("[J+]  + Player Vel X:            %02d (Force: %02d, MaxL: %02d, MaxR: %02d)\n", *_playerVelX1, *_playerMomentumX, *_playerMaxVelLeft, *_playerMaxVelRight);
    jaffarCommon::logger::log("[J+]  + Player Vel Y:            %02d (%02d)\n", *_playerVelY1, *_playerVelY2);
    jaffarCommon::logger::log("[J+]  + Player Gravity:          %02u\n", *_playerGravity);
    jaffarCommon::logger::log("[J+]  + Player Friction:         %02u\n", *_playerFriction);
    jaffarCommon::logger::log("[J+]  + Player Moving Direction: %s\n", *_playerMovingDirection == 1 ? "Right" : "Left");
    jaffarCommon::logger::log("[J+]  + Player Facing Direction: %s\n", *_playerFacingDirection == 1 ? "Right" : "Left");
    jaffarCommon::logger::log("[J+]  + Player Floating Mode:    %02u\n", *_playerFloatingMode);
    jaffarCommon::logger::log("[J+]  + Player Walking:          %02u %02u %02u\n", *_playerWalkingMode, *_playerWalkingDelay, *_playerWalkingFrame);
    jaffarCommon::logger::log("[J+]  + Player Collision:        %02u\n", *_playerCollision);
    jaffarCommon::logger::log("[J+]  + Player 1 Inputs:         %02u %02u %02u %02u\n", *_playerInput, *_playerButtons, *_playerGamePad1, *_playerGamePad2);
    jaffarCommon::logger::log("[J+]  + Powerup Active:          %1u\n", *_powerUpActive);
    jaffarCommon::logger::log("[J+]  + Enemy Active:            %1u%1u%1u%1u%1u\n", *_enemy1Active, *_enemy2Active, *_enemy3Active, *_enemy4Active, *_enemy5Active);
    jaffarCommon::logger::log("[J+]  + Enemy State:             %02u %02u %02u %02u %02u\n", *_enemy1State, *_enemy2State, *_enemy3State, *_enemy4State, *_enemy5State);
    jaffarCommon::logger::log("[J+]  + Enemy Type:              %02u %02u %02u %02u %02u\n", *_enemy1Type, *_enemy2Type, *_enemy3Type, *_enemy4Type, *_enemy5Type);
    jaffarCommon::logger::log("[J+]  + Hit Detection Flags:     %02u %02u %02u\n", *_playerCollision, *_enemyCollision, *_hitDetectionFlag);
    jaffarCommon::logger::log("[J+]  + LevelEntry / GameMode:   %02u / %02u\n", *_levelEntryFlag, *_gameMode);
    jaffarCommon::logger::log("[J+]  + Warp:                    Offset: %04u, Zone %02u\n", *_warpAreaOffset, *_warpZoneControl);
    jaffarCommon::logger::log("[J+]  + Next Screen:             %02u\n", *_nextScreen);
    jaffarCommon::logger::log("[J+]  + Timers:                  %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n", *_animationTimer, *_jumpSwimTimer, *_runningTimer, *_blockBounceTimer, *_sideCollisionTimer, *_jumpspringTimer, *_gameControlTimer, *_climbSideTimer, *_enemyFrameTimer, *_frenzyEnemyTimer, *_bowserFireTimer, *_stompTimer, *_airBubbleTimer, *_multiCoinBlockTimer, *_invincibleTimer, *_starTimer);

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _playerDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _playerDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _playerDistanceToPoint);
    }

    if (_useTrace == true)
    {
      jaffarCommon::logger::log("[J+]  + Trace Magnet Step:                      %u (%+1u)\n", _traceStep, _traceMagnet.offset);
      if (std::abs(_traceMagnet.intensityPosX) > 0.0f)    jaffarCommon::logger::log("[J+]    + Pos X    / Intensity  %8.3f / Target %8.3f / Distance %7.3f / Effect %7.4f\n", _traceMagnet.intensityPosX, _tracePosTargetX, _tracePosDistanceX, _tracePosEffectX);
      if (std::abs(_traceMagnet.intensityPosY) > 0.0f)    jaffarCommon::logger::log("[J+]    + Pos Y    / Intensity  %8.3f / Target %8.3f / Distance %7.3f / Effect %7.4f\n", _traceMagnet.intensityPosY, _tracePosTargetY, _tracePosDistanceY, _tracePosEffectY);
      if (std::abs(_traceMagnet.intensityVelX) > 0.0f)    jaffarCommon::logger::log("[J+]    + Vel X    / Intensity  %8.3f / Target %8.3f / Distance %7.3f / Effect %7.4f\n", _traceMagnet.intensityVelX, _traceVelTargetX, _traceVelDistanceX, _traceVelEffectX);
      if (std::abs(_traceMagnet.intensityVelY) > 0.0f)    jaffarCommon::logger::log("[J+]    + Vel Y    / Intensity  %8.3f / Target %8.3f / Distance %7.3f / Effect %7.4f\n", _traceMagnet.intensityVelY, _traceVelTargetY, _traceVelDistanceY, _traceVelEffectY);
      if (std::abs(_traceMagnet.intensityScreenX) > 0.0f) jaffarCommon::logger::log("[J+]    + Screen X / Intensity  %8.3f / Target %8.3f / Distance %7.3f / Effect %7.4f\n", _traceMagnet.intensityScreenX, _traceScreenPosTargetX, _traceScreenPosDistanceX, _traceScreenPosEffectX);
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Trace Magnet")
    {
      if (_useTrace == false) JAFFAR_THROW_LOGIC("Specified Trace Magnet, but no trace file was provided.");
      auto intensityPosX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Pos X");
      auto intensityPosY = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Pos Y");
      auto intensityVelX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Vel X");
      auto intensityVelY = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Vel Y");
      auto intensityScreenX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Screen X");
      auto offset    = jaffarCommon::json::getNumber<int>(actionJs, "Offset");
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{
        .intensityPosX = intensityPosX,
        .intensityPosY = intensityPosY,
        .intensityVelX = intensityVelX,
        .intensityVelY = intensityVelY,
        .intensityScreenX = intensityScreenX,
        .offset = offset };
       });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    jaffarCommon::hash::hash_t inputHash;

    inputHash.first |= (uint64_t)*_playerPosX1 << 0;
    inputHash.first |= (uint64_t)*_playerPosX2 << 8;
    inputHash.first |= (uint64_t)*_playerPosX3 << 16;
    inputHash.first |= (uint64_t)*_playerPosY2 << 24;
    inputHash.first |= (uint64_t)*_playerPosX3 << 32;
    inputHash.first |= (uint64_t)*_playerVelX1 << 40;
    inputHash.first |= (uint64_t)*_playerVelY1 << 48;
    inputHash.first |= (uint64_t)*_playerVelY2 << 56;

    inputHash.second += _lowMem[0x0000] + 
                        _lowMem[0x0002] + 
                        _lowMem[0x0003] + 
                        _lowMem[0x0005] + 
                        _lowMem[0x03AD] + 
                        _lowMem[0x03B8] + 
                        _lowMem[0x0400] + 
                        _lowMem[0x0416] + 
                        _lowMem[0x0433] + 
                        _lowMem[0x0450] + 
                        _lowMem[0x0456] + 
                        _lowMem[0x0490] + 
                        _lowMem[0x04AC] + 
                        _lowMem[0x06D5] + 
                        _lowMem[0x0700] + 
                        _lowMem[0x0701] + 
                        _lowMem[0x0702] + 
                        _lowMem[0x0704] + 
                        _lowMem[0x0705] + 
                        _lowMem[0x0709] + 
                        _lowMem[0x070A] + 
                        _lowMem[0x070B] + 
                        _lowMem[0x070C] + 
                        _lowMem[0x070D] + 
                        _lowMem[0x0714] + 
                        _lowMem[0x071C] + 
                        _lowMem[0x071D] + 
                        _lowMem[0x071E] + 
                        _lowMem[0x071F] + 
                        _lowMem[0x0722] + 
                        _lowMem[0x0723] + 
                        _lowMem[0x0754] + 
                        _lowMem[0x0755];

    return inputHash;
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the x point of attraction
    float y         = 0.0; // What is the y point of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _pointMagnet;

  uint8_t* _screenPosX1          ;
  uint8_t * _playerAnimation      ;
  uint8_t * _playerState          ;
  uint8_t * _playerPosX1          ;
  uint8_t * _playerPosX2          ;
  uint8_t * _playerPosX3          ;
  uint8_t * _playerPosY2          ;
  uint8_t * _playerPosY3          ;
  uint8_t * _playerMovingDirection;
  uint8_t * _playerFacingDirection;
  uint8_t * _playerFloatingMode   ;
  uint8_t * _playerWalkingMode    ;
  uint8_t * _playerWalkingDelay   ;
  uint8_t * _playerWalkingFrame   ;
  int8_t  * _playerMaxVelLeft     ;
  int8_t  * _playerMaxVelRight    ;
  int8_t  * _playerVelX1          ;
  int8_t  * _playerMomentumX      ;
  int8_t  * _playerVelY1          ;
  int8_t  * _playerVelY2          ;
  uint8_t * _playerGravity        ;
  uint8_t * _playerFriction       ;
  uint8_t * _timeLeft100          ;
  uint8_t * _timeLeft10           ;
  uint8_t * _timeLeft1            ;
  uint8_t * _screenPosX2          ;
  uint8_t * _screenPosX3          ;
  uint8_t * _currentWorld         ;
  uint8_t * _currentStage         ;
  uint8_t * _levelEntryFlag       ;
  uint8_t * _gameMode             ;
  uint8_t * _enemy1Active         ;
  uint8_t * _enemy2Active         ;
  uint8_t * _enemy3Active         ;
  uint8_t * _enemy4Active         ;
  uint8_t * _enemy5Active         ;
  uint8_t * _enemy1State          ;
  uint8_t * _enemy2State          ;
  uint8_t * _enemy3State          ;
  uint8_t * _enemy4State          ;
  uint8_t * _enemy5State          ;
  uint8_t * _enemy1Type           ;
  uint8_t * _enemy2Type           ;
  uint8_t * _enemy3Type           ;
  uint8_t * _enemy4Type           ;
  uint8_t * _enemy5Type           ;
  uint8_t * _playerCollision      ;
  uint8_t * _enemyCollision       ;
  uint8_t * _hitDetectionFlag     ;
  uint8_t * _powerUpActive        ;
  uint8_t * _animationTimer       ;
  uint8_t * _jumpSwimTimer        ;
  uint8_t * _runningTimer         ;
  uint8_t * _blockBounceTimer     ;
  uint8_t * _sideCollisionTimer   ;
  uint8_t * _jumpspringTimer      ;
  uint8_t * _gameControlTimer     ;
  uint8_t * _climbSideTimer       ;
  uint8_t * _enemyFrameTimer      ;
  uint8_t * _frenzyEnemyTimer     ;
  uint8_t * _bowserFireTimer      ;
  uint8_t * _stompTimer           ;
  uint8_t * _airBubbleTimer       ;
  uint8_t * _fallPitTimer         ;
  uint8_t * _multiCoinBlockTimer  ;
  uint8_t * _invincibleTimer      ;
  uint8_t * _starTimer            ;
  uint8_t * _playerInput          ;
  uint8_t * _playerButtons        ;
  uint8_t * _playerGamePad1       ;
  uint8_t * _playerGamePad2       ;
  uint16_t* _warpAreaOffset       ;
  uint8_t * _warpZoneControl;
  uint8_t * _nextScreen;

  // Game-Specific values
  float _playerDistanceToPointX;
  float _playerDistanceToPointY;
  float _playerDistanceToPoint;

  // Game-Specific values
  float _playerPosX;
  float _playerPosY;
  float _screenPosX;
  float _playerScreenOffset;

  uint16_t _currentStep;

  ///////////// Tracing stuff

  bool _isDumpingTrace = false;
  std::string _traceDumpString;

  __INLINE__ void playerPrintCommands() const override
  {
    jaffarCommon::logger::log("[J+] t: start/stop trace dumping (%s)\n", _isDumpingTrace ? "On" : "Off");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
    // If storing a trace, do it here
    if (_isDumpingTrace == true) _traceDumpString += std::to_string(_playerPosX) + std::string(" ") +
                                                     std::to_string(_playerPosY) + std::string(" ") +
                                                     std::to_string(*_playerVelX1) + std::string(" ") +
                                                     std::to_string(*_playerVelY1) + std::string(" ") +
                                                     std::to_string(_playerScreenOffset) + std::string("\n");

     if (command == 't')
     {
      if (_isDumpingTrace == false)
      {
        _isDumpingTrace = true;
        _traceDumpString = "";
        return false;
      }
      else
      {
        const std::string dumpOutputFile = "jaffar.trace";
        jaffarCommon::logger::log("[J+] Dumping trace to file: '%s'", dumpOutputFile.c_str());
        jaffarCommon::file::saveStringToFile(_traceDumpString, dumpOutputFile);
        _isDumpingTrace = false;
        return true;
      }
     }

     return false;
  };

  // Whether we use a trace
  bool _useTrace = false;

  struct traceMagnet_t
  {
    float intensityPosX = 0.0;
    float intensityPosY = 0.0;
    float intensityVelX = 0.0;
    float intensityVelY = 0.0;
    float intensityScreenX = 0.0;
    int offset      = 0; // Which entry (step) to look at wrt the current emulation step
  };

  // Trace contents
  struct traceEntry_t
  {
    float posx;
    float posy;
    float velx;
    float vely;
    float screenPosx;
  };
  std::vector<traceEntry_t> _trace;

  // Location of the trace file
  traceMagnet_t _traceMagnet;
  std::string _traceFilePath;

  // Current trace target
  uint16_t _traceStep;
  
  float _tracePosTargetX;
  float _tracePosTargetY;
  float _traceVelTargetX;
  float _traceVelTargetY;
  float _traceScreenPosTargetX;
 
  float _tracePosDistanceX;
  float _tracePosDistanceY;
  float _traceVelDistanceX;
  float _traceVelDistanceY;
  float _traceScreenPosDistanceX;

  float _tracePosEffectX;
  float _tracePosEffectY;
  float _traceVelEffectX;
  float _traceVelEffectY;
  float _traceScreenPosEffectX;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;

  // Temporary storage for the emulator state for calculating hash
  uint8_t* _tempStorage;
  size_t   _tempStorageSize;

  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
