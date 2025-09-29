#pragma once

#include <emulator.hpp>
#include <game.hpp>
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
        float x = std::atof(coordinates[0].c_str());
        float y = std::atof(coordinates[1].c_str());
        _trace.push_back(traceEntry_t{
          .x = x,
          .y = y,
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
    registerGameProperty("Screen Pos X1"             ,&_lowMem[0x071B], Property::datatype_t::dt_uint16, Property::endianness_t::little);
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
    registerGameProperty("Screen Pos X2"             ,&_lowMem[0x071A], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Screen Pos X3"             ,&_lowMem[0x071C], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
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

      
    _screenPosX1              =  (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X1"             )]->getPointer();
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

    registerGameProperty("Player Pos X"          , &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Y"          , &_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _playerPosX = 0;
    _playerPosY = 0;
    _screenPosX = 0;
    _playerScreenOffset = 0;
    _currentStep = 0;
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

    // hashEngine.Update(*_playerCollision);
    // hashEngine.Update(*_enemyCollision);
    hashEngine.Update(*_hitDetectionFlag);

    // To Reduce timer pressure on hash, have 0, 1, and >1 as possibilities only
    hashEngine.Update(*_animationTimer < 2 ? *_animationTimer : (uint8_t)2);
    hashEngine.Update(*_jumpSwimTimer < 2 ? *_jumpSwimTimer : (uint8_t)2);
    hashEngine.Update(*_runningTimer < 2 ? *_runningTimer : (uint8_t)2);
    hashEngine.Update(*_blockBounceTimer < 2 ? *_blockBounceTimer : (uint8_t)2);
    // hashEngine.Update(*_sideCollisionTimer);
    // hashEngine.Update(*_jumpspringTimer);
    // hashEngine.Update(*_gameControlTimer);
    // hashEngine.Update(*_climbSideTimer);
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
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _playerPosX = (float)*_playerPosX1 * 256.0f + (float)*_playerPosX2 + ((float)*_playerPosX3) / 256.0f;
    _playerPosY = (float)*_playerPosY2 + (float)*_playerPosY3 / 256.0f;
    _screenPosX = (float)*_screenPosX1 * 256.0f + (float)*_screenPosX2;
    _playerScreenOffset = _playerPosX - _screenPosX;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;

    _traceMagnet.intensityX = 0.0;
    _traceMagnet.intensityY = 0.0;
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
      _traceTargetX = _trace[_traceStep].x;
      _traceTargetY = _trace[_traceStep].y;

      // Updating distance to trace point
      _traceDistanceX = std::abs(_traceTargetX - _playerPosX);
      _traceDistanceY = std::abs(_traceTargetY - _playerPosY);

      if (_traceDistanceX > 100) _traceDistanceX = 10.0;
      if (_traceDistanceY > 100) _traceDistanceX = 10.0;
      _traceDistance  = sqrtf(_traceDistanceX * _traceDistanceX + _traceDistanceY * _traceDistanceY);
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_playerPosX, sizeof(_playerPosX));
     serializer.push(&_playerPosY, sizeof(_playerPosY));
     serializer.push(&_screenPosX, sizeof(_screenPosX));
     serializer.push(&_playerScreenOffset, sizeof(_playerScreenOffset));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
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
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityX * _traceDistanceX;
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityY * _traceDistanceY;

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
    jaffarCommon::logger::log("[J+]  + Player / Screen Offset:  %04d\n", _playerScreenOffset);
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
    jaffarCommon::logger::log("[J+]  + Player 1 Inputs:         %02u %02u %02u %02u\n", *_playerInput, *_playerButtons, *_playerGamePad1, *_playerGamePad2);
    jaffarCommon::logger::log("[J+]  + Powerup Active:          %1u\n", *_powerUpActive);
    jaffarCommon::logger::log("[J+]  + Enemy Active:            %1u%1u%1u%1u%1u\n", *_enemy1Active, *_enemy2Active, *_enemy3Active, *_enemy4Active, *_enemy5Active);
    jaffarCommon::logger::log("[J+]  + Enemy State:             %02u %02u %02u %02u %02u\n", *_enemy1State, *_enemy2State, *_enemy3State, *_enemy4State, *_enemy5State);
    jaffarCommon::logger::log("[J+]  + Enemy Type:              %02u %02u %02u %02u %02u\n", *_enemy1Type, *_enemy2Type, *_enemy3Type, *_enemy4Type, *_enemy5Type);
    jaffarCommon::logger::log("[J+]  + Hit Detection Flags:     %02u %02u %02u\n", *_playerCollision, *_enemyCollision, *_hitDetectionFlag);
    jaffarCommon::logger::log("[J+]  + LevelEntry / GameMode:   %02u / %02u\n", *_levelEntryFlag, *_gameMode);
    jaffarCommon::logger::log("[J+]  + Warp Area Offset:        %04u\n", *_warpAreaOffset);
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
      if (std::abs(_traceMagnet.intensityX) > 0.0f || std::abs(_traceMagnet.intensityY) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Trace Magnet                             Intensity: (X: %.5f, Y: %.5f), Step: %u (%+1u), X: %3.3f, Y: %3.3f\n", _traceMagnet.intensityX, _traceMagnet.intensityY, _traceStep, _traceMagnet.offset, _traceTargetX, _traceTargetY);
        jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _traceDistanceX);
        jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _traceDistanceY);
        jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _traceDistance);
      }
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
      auto intensityX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity X");
      auto intensityY = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Y");
      auto offset    = jaffarCommon::json::getNumber<int>(actionJs, "Offset");
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{.intensityX = intensityX, .intensityY = intensityY, .offset = offset }; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
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

  uint16_t* _screenPosX1          ;
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
    if (_isDumpingTrace == true) _traceDumpString += std::to_string(_playerPosX) + std::string(" ") + std::to_string(_playerPosY) + std::string("\n");

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
    float intensityX = 0.0; // How strong the magnet is on X
    float intensityY = 0.0; // How strong the magnet is on Y
    int offset      = 0; // Which entry (step) to look at wrt the current emulation step
  };

  // Trace contents
  struct traceEntry_t
  {
    float x;
    float y;
  };
  std::vector<traceEntry_t> _trace;

  // Location of the trace file
  traceMagnet_t _traceMagnet;
  std::string _traceFilePath;

  // Current trace target
  uint16_t _traceStep;
  float _traceTargetX;
  float _traceTargetY;
  float _traceDistanceX;
  float _traceDistanceY;
  float _traceDistance;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus