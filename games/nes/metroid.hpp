#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <numeric>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class Metroid final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Metroid"; }

  Metroid(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    _allowFire = jaffarCommon::json::getBoolean(config, "Allow Fire");
    _traceFilePath = jaffarCommon::json::getString(config, "Trace File Path");
    _allowPause = jaffarCommon::json::getBoolean(config, "Allow Pause");

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
    _srmMem = _emulator->getProperty("SRAM").pointer;

    // Registering native game properties

    // Setting relevant metroid pointers

    // https://datacrystal.romhacking.net/wiki/Metroid:RAM_map

    registerGameProperty("Pause Mode"               , &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Frame Counter"            , &_lowMem[0x002D], Property::datatype_t::dt_uint8, Property::endianness_t::little); 

    registerGameProperty("NMI Flag"                 , &_lowMem[0x001A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("NMI Data"                 , &_lowMem[0x001B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("NMI PAL Data"             , &_lowMem[0x001C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    
    registerGameProperty("Game Mode"                , &_lowMem[0x001E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("RNG"                      , &_lowMem[0x002E], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    
    registerGameProperty("Samus Pos X1"             , &_lowMem[0x030C], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Pos X2"             , &_lowMem[0x030E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Pos X3"             , &_lowMem[0x0311], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Screen Pos X1"            , &_lowMem[0x0050], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos X2"            , &_lowMem[0x0051], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Scroll X1"         , &_lowMem[0x000B], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Scroll X2"         , &_lowMem[0x00FD], Property::datatype_t::dt_uint8, Property::endianness_t::little); 

    registerGameProperty("Samus Pos Y1"             , &_lowMem[0x030B], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Pos Y2"             , &_lowMem[0x030D], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Pos Y3"             , &_lowMem[0x0310], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Screen Pos Y1"            , &_lowMem[0x004F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos Y2"            , &_lowMem[0x0052], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Scroll Y"          , &_lowMem[0x00FC], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    
    registerGameProperty("Samus Animation"          , &_lowMem[0x0306], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Direction"          , &_lowMem[0x004D], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Door Side"          , &_lowMem[0x004E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Door State"         , &_lowMem[0x0056], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Jump State"         , &_lowMem[0x0314], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Equipment Flags"          , &_srmMem[0x0878], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus Selected Weapon"    , &_lowMem[0x010E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Missile Count"            , &_srmMem[0x0879], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus HP 1"               , &_lowMem[0x0107], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Samus HP 2"               , &_lowMem[0x0106], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 1 State"              , &_lowMem[0x0380], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 2 State"              , &_lowMem[0x0390], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 3 State"              , &_lowMem[0x03A0], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 4 State"              , &_lowMem[0x03B0], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 1 Timer"              , &_lowMem[0x038F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 2 Timer"              , &_lowMem[0x039F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 3 Timer"              , &_lowMem[0x03AF], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Door 4 Timer"              , &_lowMem[0x03BF], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 1 State"           , &_lowMem[0x03D0], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 2 State"           , &_lowMem[0x03E0], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 3 State"           , &_lowMem[0x03F0], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 1 Pos X"           , &_lowMem[0x03DE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 2 Pos X"           , &_lowMem[0x03EE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 3 Pos X"           , &_lowMem[0x03FE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 1 Pos Y"           , &_lowMem[0x03DD], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 2 Pos Y"           , &_lowMem[0x03ED], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 3 Pos Y"           , &_lowMem[0x03FD], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Bullet 1 Vel X"           , &_lowMem[0x03D9], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Bullet 2 Vel X"           , &_lowMem[0x03E9], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Bullet 3 Vel X"           , &_lowMem[0x03F9], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Bullet 1 Vel Y"           , &_lowMem[0x03D8], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Bullet 2 Vel Y"           , &_lowMem[0x03E8], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Bullet 3 Vel Y"           , &_lowMem[0x03F8], Property::datatype_t::dt_int8,  Property::endianness_t::little);
    registerGameProperty("Bullet 1 Sprite"          , &_lowMem[0x03D6], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Upper Door State"         , &_lowMem[0x0380], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Enemy 1 State"           , &_lowMem[0x0406], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 State"           , &_lowMem[0x0416], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 3 State"           , &_lowMem[0x0426], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 4 State"           , &_lowMem[0x0436], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 5 State"           , &_lowMem[0x0446], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 6 State"           , &_lowMem[0x0456], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Enemy 1 Freeze Timer"     , &_lowMem[0x040D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Freeze Timer"     , &_lowMem[0x041D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 3 Freeze Timer"     , &_lowMem[0x042D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 4 Freeze Timer"     , &_lowMem[0x043D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 5 Freeze Timer"     , &_lowMem[0x044D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 6 Freeze Timer"     , &_lowMem[0x045D], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Enemy 1 HP"               , &_lowMem[0x040B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 HP"               , &_lowMem[0x041B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 3 HP"               , &_lowMem[0x042B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 4 HP"               , &_lowMem[0x043B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 5 HP"               , &_lowMem[0x044B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 6 HP"               , &_lowMem[0x045B], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Enemy 1 Pos Y"           , &_lowMem[0x0400], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Pos X"           , &_lowMem[0x0401], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Pos Y"           , &_lowMem[0x0410], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Pos X"           , &_lowMem[0x0411], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 3 Pos Y"           , &_lowMem[0x0420], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 3 Pos X"           , &_lowMem[0x0421], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 4 Pos Y"           , &_lowMem[0x0430], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 4 Pos X"           , &_lowMem[0x0431], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 5 Pos Y"           , &_lowMem[0x0440], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 5 Pos X"           , &_lowMem[0x0441], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 6 Pos Y"           , &_lowMem[0x0450], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 6 Pos X"           , &_lowMem[0x0451], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Boss Door 1 Hits"           , &_lowMem[0x075B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Door 2 Hits"           , &_lowMem[0x0763], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Door 3 Hits"           , &_lowMem[0x076B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Door 4 Hits"           , &_lowMem[0x0773], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Door 5 Hits"           , &_lowMem[0x077B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Glass Hit"             , &_lowMem[0x0413], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Hits"                  , &_lowMem[0x0099], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Pain State"            , &_lowMem[0x009F], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Missile Grab Status"     , &_lowMem[0x0095], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    
    registerGameProperty("Samus HP"                , &_samusHP, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Door Transition Timer"   , &_doorTransitionTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);


    _pauseMode                = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Pause Mode"               )]->getPointer();
    _frameCounter             = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Frame Counter"            )]->getPointer();
    
    _NMIFlag                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("NMI Flag"                 )]->getPointer();
    _NMIData                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("NMI Data"                 )]->getPointer();
    _NMIPALData               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("NMI PAL Data"              )]->getPointer();

    _gameMode                 = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Game Mode"                )]->getPointer();
    
    _samusPosX1               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Pos X1"              )]->getPointer();
    _samusPosX2               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Pos X2"              )]->getPointer();
    _samusPosX3               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Pos X3"              )]->getPointer();

    _screenPosX1              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Screen Pos X1"            )]->getPointer();
    _screenPosX2              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Screen Pos X2"             )]->getPointer();

    _screenScrollX1           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Screen Scroll X1"         )]->getPointer();
    _screenScrollX2           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Screen Scroll X2"         )]->getPointer();

    _samusPosY1               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Pos Y1"             )]->getPointer();
    _samusPosY2               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Pos Y2"             )]->getPointer();
    _samusPosY3               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Pos Y3"             )]->getPointer();

    _screenPosY1              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Screen Pos Y1"            )]->getPointer();
    _screenPosY2              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Screen Pos Y2"            )]->getPointer();
    _screenScrollY            = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y"          )]->getPointer();

    _samusAnimation           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Animation"          )]->getPointer();
    _samusDirection           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Direction"          )]->getPointer();
    _samusDoorSide            = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Door Side"          )]->getPointer();
    _samusDoorState           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Door State"         )]->getPointer();
    _samusJumpState           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Jump State"         )]->getPointer();
    _equipmentFlags           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Equipment Flags"          )]->getPointer();
    _samusSelectedWeapon      = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus Selected Weapon"    )]->getPointer();
    _missileCount             = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Missile Count"            )]->getPointer();
    _samusHP1                 = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus HP 1"               )]->getPointer();
    _samusHP2                 = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Samus HP 2"               )]->getPointer();
    _door1State               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 1 State"             )]->getPointer();
    _door2State               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 2 State"             )]->getPointer();
    _door3State               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 3 State"             )]->getPointer();
    _door4State               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 4 State"             )]->getPointer();
    _door1Timer               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 1 Timer"             )]->getPointer();
    _door2Timer               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 2 Timer"             )]->getPointer();
    _door3Timer               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 3 Timer"             )]->getPointer();
    _door4Timer               = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Door 4 Timer"             )]->getPointer();
    _bullet1State             = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 1 State"           )]->getPointer();
    _bullet2State             = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 2 State"           )]->getPointer();
    _bullet3State             = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 3 State"           )]->getPointer();
    _bullet1PosX              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 1 Pos X"           )]->getPointer();
    _bullet2PosX              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 2 Pos X"           )]->getPointer();
    _bullet3PosX              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 3 Pos X"           )]->getPointer();
    _bullet1PosY              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 1 Pos Y"           )]->getPointer();
    _bullet2PosY              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 2 Pos Y"           )]->getPointer();
    _bullet3PosY              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Bullet 3 Pos Y"           )]->getPointer();
    _bullet1VelX              = (int8_t*)  _propertyMap[jaffarCommon::hash::hashString("Bullet 1 Vel X"           )]->getPointer();
    _bullet2VelX              = (int8_t*)  _propertyMap[jaffarCommon::hash::hashString("Bullet 2 Vel X"           )]->getPointer();
    _bullet3VelX              = (int8_t*)  _propertyMap[jaffarCommon::hash::hashString("Bullet 3 Vel X"           )]->getPointer();
    _bullet1VelY              = (int8_t*)  _propertyMap[jaffarCommon::hash::hashString("Bullet 1 Vel Y"           )]->getPointer();
    _bullet2VelY              = (int8_t*)  _propertyMap[jaffarCommon::hash::hashString("Bullet 2 Vel Y"           )]->getPointer();
    _bullet3VelY              = (int8_t*)  _propertyMap[jaffarCommon::hash::hashString("Bullet 3 Vel Y"           )]->getPointer();
    _upperDoorState           = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Upper Door State"         )]->getPointer();

    _enemy1State              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 1 State"           )]->getPointer();
    _enemy2State              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 2 State"           )]->getPointer();
    _enemy3State              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 3 State"           )]->getPointer();
    _enemy4State              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 4 State"           )]->getPointer();
    _enemy5State              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 5 State"           )]->getPointer();
    _enemy6State              = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 6 State"           )]->getPointer();

    _enemy1HP                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 1 HP"           )]->getPointer();
    _enemy2HP                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 2 HP"           )]->getPointer();
    _enemy3HP                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 3 HP"           )]->getPointer();
    _enemy4HP                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 4 HP"           )]->getPointer();
    _enemy5HP                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 5 HP"           )]->getPointer();
    _enemy6HP                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 6 HP"           )]->getPointer();

    _enemy1FreezeTimer        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 1 Freeze Timer"           )]->getPointer();
    _enemy2FreezeTimer        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 2 Freeze Timer"           )]->getPointer();
    _enemy3FreezeTimer        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 3 Freeze Timer"           )]->getPointer();
    _enemy4FreezeTimer        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 4 Freeze Timer"           )]->getPointer();
    _enemy5FreezeTimer        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 5 Freeze Timer"           )]->getPointer();
    _enemy6FreezeTimer        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Enemy 6 Freeze Timer"           )]->getPointer();

    _bossDoor1Hits       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Door 1 Hits"         )]->getPointer();
    _bossDoor2Hits       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Door 2 Hits"         )]->getPointer();
    _bossDoor3Hits       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Door 3 Hits"         )]->getPointer();
    _bossDoor4Hits       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Door 4 Hits"         )]->getPointer();
    _bossDoor5Hits       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Door 5 Hits"         )]->getPointer();
    _bossGlassHit        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Glass Hit"           )]->getPointer();
    _bossHits            = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Hits"                )]->getPointer();
    _bossPainState       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Boss Pain State"          )]->getPointer();

    _missileGrabStatus         = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Missile Grab Status"      )]->getPointer();

    registerGameProperty("Samus Pos X"        , &_samusPosX,   Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Samus Pos Y"        , &_samusPosY,   Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Samus Rel Pos X"    , &_samusRelPosX,   Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Samus Rel Pos Y"    , &_samusRelPosY,   Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Lag Frame Counter"        , &_lagFrameCounter,   Property::datatype_t::dt_uint16, Property::endianness_t::little); 
    registerGameProperty("Pause Frame Counter"      , &_pauseFrameCounter, Property::datatype_t::dt_uint16, Property::endianness_t::little); 
    registerGameProperty("Prev Game Mode"      , &_prevGameMode, Property::datatype_t::dt_uint16, Property::endianness_t::little); 
    registerGameProperty("Boss Pending Hits"      , &_bossPendingHits, Property::datatype_t::dt_float32, Property::endianness_t::little); 
    

    for (size_t i = 0; i < 0x800; i++)
    {
      char name[512];
      sprintf(name, "RAM Value [0x%03lX]", i);
      registerGameProperty(std::string(name), &_lowMem[i], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    }

    for (size_t i = 0; i < 0x2000; i++)
    {
      char name[512];
      sprintf(name, "WRAM Value [0x%04lX]", i);
      registerGameProperty(std::string(name), &_srmMem[i], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    }

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _input_S      = _emulator->registerInput("|..|....S...|........|");
    _nullInputIdx = _emulator->registerInput("|..|........|........|");
    _input_U      = _emulator->registerInput("|..|U.......|........|");
    _input_D      = _emulator->registerInput("|..|.D......|........|");
    _input_L      = _emulator->registerInput("|..|..L.....|........|");
    _input_R      = _emulator->registerInput("|..|...R....|........|");
    _input_A      = _emulator->registerInput("|..|.......A|........|");
    _input_B      = _emulator->registerInput("|..|......B.|........|");
    _input_UD     = _emulator->registerInput("|..|UD......|........|");
    _input_UL     = _emulator->registerInput("|..|U.L.....|........|");
    _input_UR     = _emulator->registerInput("|..|U..R....|........|");
    _input_UA     = _emulator->registerInput("|..|U......A|........|");
    _input_UB     = _emulator->registerInput("|..|U.....B.|........|");
    _input_DL     = _emulator->registerInput("|..|.DL.....|........|");
    _input_DR     = _emulator->registerInput("|..|.D.R....|........|");
    _input_DA     = _emulator->registerInput("|..|.D.....A|........|");
    _input_DB     = _emulator->registerInput("|..|.D....B.|........|");
    _input_AL     = _emulator->registerInput("|..|..L....A|........|");
    _input_BL     = _emulator->registerInput("|..|..L...B.|........|");
    _input_AR     = _emulator->registerInput("|..|...R...A|........|");
    _input_BR     = _emulator->registerInput("|..|...R..B.|........|");
    _input_UDA    = _emulator->registerInput("|..|UD.....A|........|");
    _input_ULA    = _emulator->registerInput("|..|U.L....A|........|");
    _input_URA    = _emulator->registerInput("|..|U..R...A|........|");
    _input_UBA    = _emulator->registerInput("|..|U.....BA|........|");
    _input_DLA    = _emulator->registerInput("|..|.DL....A|........|");
    _input_DRA    = _emulator->registerInput("|..|.D.R...A|........|");
    _input_DBA    = _emulator->registerInput("|..|.D....BA|........|");
    _input_BLA    = _emulator->registerInput("|..|..L...BA|........|");
    _input_BRA    = _emulator->registerInput("|..|...R..BA|........|");
    _input_ULB    = _emulator->registerInput("|..|UD....B.|........|");
    _input_ULB    = _emulator->registerInput("|..|U.L...B.|........|");
    _input_URB    = _emulator->registerInput("|..|U..R..B.|........|");
    _input_DLB    = _emulator->registerInput("|..|.DL...B.|........|");
    _input_DRB    = _emulator->registerInput("|..|.D.R..B.|........|");
    _input_BA     = _emulator->registerInput("|..|......BA|........|");
    _input_UBA    = _emulator->registerInput("|..|U.....BA|........|");
    _input_UDR    = _emulator->registerInput("|..|UD.R....|........|");
    _input_UDL    = _emulator->registerInput("|..|UDL.....|........|");
    _input_LRA    = _emulator->registerInput("|..|..LR...A|........|");
    _input_LRB    = _emulator->registerInput("|..|..LR..B.|........|");
    _input_RBA    = _emulator->registerInput("|..|...R..BA|........|");
    _input_LBA    = _emulator->registerInput("|..|..L...BA|........|");
    _input_LR     = _emulator->registerInput("|..|..LR....|........|");
    _input_ARS    = _emulator->registerInput("|..|...RS..A|........|");
    _input_BS     = _emulator->registerInput("|..|....S.B.|........|");
    _input_AS     = _emulator->registerInput("|..|....S..A|........|");
    _input_ALS    = _emulator->registerInput("|..|..L.S..A|........|");
    _input_RS     = _emulator->registerInput("|..|...RS...|........|");
    _input_LS     = _emulator->registerInput("|..|..L.S...|........|");
    _input_URBA   = _emulator->registerInput("|..|U..R..BA|........|");
    _input_ULBA   = _emulator->registerInput("|..|U.L...BA|........|");
    _input_sB     = _emulator->registerInput("|..|.....sB.|........|");
    _input_sA     = _emulator->registerInput("|..|.....s.A|........|");
    _input_RsA    = _emulator->registerInput("|..|...R.s.A|........|");
    _input_LsA    = _emulator->registerInput("|..|..L..s.A|........|");
    _input_UsA    = _emulator->registerInput("|..|U....s.A|........|");
    _input_DsA    = _emulator->registerInput("|..|.D...s.A|........|");
    _input_URs    = _emulator->registerInput("|..|U..R.s..|........|");
    _input_URsA   = _emulator->registerInput("|..|U..R.s.A|........|");
    _input_Us     = _emulator->registerInput("|..|U....s..|........|");
    _input_Rs     = _emulator->registerInput("|..|...R.s..|........|");
    _input_Ls     = _emulator->registerInput("|..|..L..s..|........|");
    _input_s      = _emulator->registerInput("|..|.....s..|........|");

    _lagFrameCounter     = 0;
    _pauseFrameCounter   = 0;
    _doorTransitionTimer = 0;
    _prevGameMode = *_gameMode;
    _pauseModePrev = *_pauseMode;
    _bossPendingHits = 255.0;

    _samusRelPosX = 0.0;
    _samusRelPosY = 0.0;

    stateUpdatePostHook();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _pauseModePrev = *_pauseMode;
    _prevGameMode = *_gameMode;

    auto samusPrevPosX = (float)*_samusPosX2 + (float)*_samusPosX3 / 256.0;
    auto samusPrevPosY = (float)*_samusPosY2 + (float)*_samusPosY3 / 256.0;

    _emulator->advanceState(input);
    
    // count lag frames
    if (*_NMIFlag == 1) (_lagFrameCounter)++;
    if (*_pauseMode == 1) (_pauseFrameCounter)++;
    if (*_samusDoorState > 0) (_doorTransitionTimer)++;

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;

    auto samusCurrPosX = (float)*_samusPosX2 + (float)*_samusPosX3 / 256.0;
    auto samusCurrPosY = (float)*_samusPosY2 + (float)*_samusPosY3 / 256.0;

    if (samusPrevPosX > 200 && samusCurrPosX < 30) samusCurrPosX += 256.0;
    if (samusPrevPosY > 200 && samusCurrPosY < 30) samusCurrPosY += 256.0;
    if (samusCurrPosX > 200 && samusPrevPosX < 30) samusPrevPosX += 256.0;
    if (samusCurrPosY > 200 && samusPrevPosY < 30) samusPrevPosY += 256.0;

    _samusRelPosX += samusCurrPosX - samusPrevPosX;
    _samusRelPosY += samusCurrPosY - samusPrevPosY;
  }

  __INLINE__ void calculateHashValues(MetroHash128& hashEngine) const
  {
      hashEngine.Update(*_pauseMode);
      hashEngine.Update(*_gameMode);
      hashEngine.Update(*_NMIFlag);
      hashEngine.Update(*_NMIData);
      hashEngine.Update(*_NMIPALData);

      hashEngine.Update(*_samusPosX1);
      hashEngine.Update(*_samusPosX2);
      // hashEngine.Update(*_samusPosX3);

      hashEngine.Update(*_screenPosX1);
      hashEngine.Update(*_screenPosX2);

      hashEngine.Update(*_samusPosY1);
      hashEngine.Update(*_samusPosY2);
      // hashEngine.Update(*_samusPosY3);

      hashEngine.Update(*_screenPosY1);
      hashEngine.Update(*_screenPosY2);
      hashEngine.Update(*_screenScrollX1);
      hashEngine.Update(*_screenScrollX2);
      hashEngine.Update(*_screenScrollY);
      hashEngine.Update(*_missileCount);
      hashEngine.Update(*_samusSelectedWeapon);
      hashEngine.Update(*_samusAnimation);
      hashEngine.Update(*_samusDirection);
      hashEngine.Update(*_samusDoorSide);
      hashEngine.Update(*_samusJumpState);
      hashEngine.Update(*_equipmentFlags);
      hashEngine.Update(*_samusDoorState);

      hashEngine.Update(_lagFrameCounter);
      hashEngine.Update(_pauseFrameCounter);
      hashEngine.Update(_doorTransitionTimer);
      hashEngine.Update(_prevGameMode);

      hashEngine.Update(*_door1State);
      hashEngine.Update(*_door2State);
      hashEngine.Update(*_door3State);
      hashEngine.Update(*_door4State);
      // hashEngine.Update(*_upperDoorState);
      hashEngine.Update(*_missileGrabStatus);

    //  hashEngine.Update(*door1Timer);
    //  hashEngine.Update(*door2Timer);
    //  hashEngine.Update(*door3Timer);
    //  hashEngine.Update(*door4Timer);

    if (_hashBulletEnabled == true)
    {
      hashEngine.Update(*_bullet1State);
      // hashEngine.Update(*_bullet2State);
      // hashEngine.Update(*_bullet3State);
      hashEngine.Update(*_bullet1PosX);
      //hashEngine.Update(*_bullet2PosX);
      // hashEngine.Update(*_bullet3PosX);
      hashEngine.Update(*_bullet1PosY / 32);
      // hashEngine.Update(*_bullet2PosY);
      // hashEngine.Update(*_bullet3PosY);
    }

      // Samus-specific hashes
      hashEngine.Update(_lowMem[0x0053]); // Run Frame
      hashEngine.Update(_lowMem[0x0300]); // Buffered ACtion
      hashEngine.Update(_lowMem[0x0304]); // Buffered ACtion
      hashEngine.Update(_lowMem[0x0305]); // Buffered ACtion
      hashEngine.Update(_lowMem[0x0306]); // Buffered ACtion

      hashEngine.Update(_lowMem[0x0308]); // Vertical Speed
      hashEngine.Update(_lowMem[0x0309]); // Horizontal Speed

      hashEngine.Update(_lowMem[0x030F]); // Buffered ACtion
      hashEngine.Update(_lowMem[0x0314]); // Buffered ACtion
      hashEngine.Update(_lowMem[0x0316]); // Jump State2
      hashEngine.Update(_lowMem[0x0315]); // Buffered ACtion
      hashEngine.Update(_lowMem[0x030A]); // Hit By Enemy
      hashEngine.Update(_lowMem[0x0683]); // sound (transform into ball)

      // Boss room
      hashEngine.Update(*_bossDoor1Hits);
      hashEngine.Update(*_bossDoor2Hits);
      hashEngine.Update(*_bossDoor3Hits);
      hashEngine.Update(*_bossDoor4Hits);
      hashEngine.Update(*_bossDoor5Hits);
      hashEngine.Update(*_bossGlassHit);
      hashEngine.Update(*_bossHits);
      hashEngine.Update(*_bossPainState);

      // hashEngine.Update(&_lowMem[0x0300], 0x20);

      // Climb 1 Top Block states
      // hashEngine.Update(_lowMem[0x0500]); 
      // hashEngine.Update(_lowMem[0x0510]); 
      // hashEngine.Update(_lowMem[0x0520]); 
      // hashEngine.Update(_lowMem[0x0530]); 
      // hashEngine.Update(_lowMem[0x0540]); 
      // hashEngine.Update(_lowMem[0x0550]); 
      // hashEngine.Update(_lowMem[0x0560]); 
      // hashEngine.Update(_lowMem[0x0570]); 
      // hashEngine.Update(_lowMem[0x0580]); 
      // hashEngine.Update(_lowMem[0x0590]); 
      // hashEngine.Update(_lowMem[0x05A0]); 
      // hashEngine.Update(_lowMem[0x05B0]); 
      // hashEngine.Update(_lowMem[0x05C0]); 
      // hashEngine.Update(_lowMem[0x05D0]);
      // hashEngine.Update(_lowMem[0x05E0]);

      // hashEngine.Update(_lowMem[0x0503]); 
      // hashEngine.Update(_lowMem[0x0513]); 
      // hashEngine.Update(_lowMem[0x0523]); 
      // hashEngine.Update(_lowMem[0x0533]); 
      // hashEngine.Update(_lowMem[0x0543]); 
      // hashEngine.Update(_lowMem[0x0553]); 
      // hashEngine.Update(_lowMem[0x0563]); 
      // hashEngine.Update(_lowMem[0x0573]); 
      // hashEngine.Update(_lowMem[0x0583]); 
      // hashEngine.Update(_lowMem[0x0593]); 
      // hashEngine.Update(_lowMem[0x05A3]); 
      // hashEngine.Update(_lowMem[0x05B3]); 
      // hashEngine.Update(_lowMem[0x05C3]); 
      // hashEngine.Update(_lowMem[0x05D3]);
      // hashEngine.Update(_lowMem[0x05E3]);

      // Segment05 blocks

      // Enemies X and Y
      // hashEngine.Update(&_lowMem[0x0400], 2); 
      // hashEngine.Update(&_lowMem[0x0410], 2); 
      // hashEngine.Update(&_lowMem[0x0420], 2); 
      // hashEngine.Update(&_lowMem[0x0430], 2); 
      // hashEngine.Update(&_lowMem[0x0440], 2); 
      // hashEngine.Update(&_lowMem[0x0450], 2); 

      // Enemies Dead
      hashEngine.Update(_lowMem[0x0406] % 2); 
      hashEngine.Update(_lowMem[0x0416] % 2);
      hashEngine.Update(_lowMem[0x0426] % 2); 
      hashEngine.Update(_lowMem[0x0436] % 2); 
      hashEngine.Update(_lowMem[0x0446] % 2); 
      hashEngine.Update(_lowMem[0x0456] % 2); 

      // Enemies frozen
      hashEngine.Update(*_enemy1FreezeTimer > 1);
      hashEngine.Update(*_enemy2FreezeTimer > 1);
      hashEngine.Update(*_enemy3FreezeTimer > 1);
      hashEngine.Update(*_enemy4FreezeTimer > 1);
      hashEngine.Update(*_enemy5FreezeTimer > 1);
      hashEngine.Update(*_enemy6FreezeTimer > 1);

      // Metroid Capture
      // hashEngine.Update(_srmMem[0x17F0] % 2);
      // hashEngine.Update(_srmMem[0x17F1] % 2);
      // hashEngine.Update(_srmMem[0x17F2] % 2);
      // hashEngine.Update(_srmMem[0x17F3] % 2);
      // hashEngine.Update(_srmMem[0x17F4] % 2);
      // hashEngine.Update(_srmMem[0x17F5] % 2);
      // hashEngine.Update(_srmMem[0x17F6] % 2);
      // hashEngine.Update(_srmMem[0x17F7] % 2);
      // hashEngine.Update(_srmMem[0x17F8] % 2);
      // hashEngine.Update(_srmMem[0x17F9] % 2);
      // hashEngine.Update(_srmMem[0x17FA] % 2);
      // hashEngine.Update(_srmMem[0x17FB] % 2);
      // hashEngine.Update(_srmMem[0x17FC] % 2);
      // hashEngine.Update(_srmMem[0x17FD] % 2);
      // hashEngine.Update(_srmMem[0x17FE] % 2);

      // Controller 1 AutoRepeat timer
      // hashEngine.Update(_lowMem[0x0018] % 2); 

    //    hashEngine.Update(&_lowMem[0x0300], 0x0020);
    //    hashEngine.Update(&_lowMem[0x0314], 0x0010);

    //    hashEngine.Update(&_lowMem[0x0000], 0x0800);
    //    hashEngine.Update(&_emu->_highMem[0x0000], 0x2000);

    if (std::abs(_lastInputStepReward) > 0.0f) hashEngine.Update(_lastInputStep);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    calculateHashValues(hashEngine);

    // uint8_t _saveBuffer[1024*1024];
    // jaffarCommon::serializer::Contiguous s(_saveBuffer);
    // _emulator->serializeState(s);
    // _emulator->advanceState(_nullInputIdx);
    // _emulator->advanceState(_nullInputIdx);

    // calculateHashValues(hashEngine);

    // jaffarCommon::deserializer::Contiguous d(_saveBuffer);
    // _emulator->deserializeState(d);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    // float actualScrollX = *_screenScrollX2 == 0 ? 256.0 : (float)*_screenScrollX2;
    // float actualScrollY = *_screenScrollY == 0 ? 256.0 : (float)*_screenScrollY;

    // float _actualScreenPosX = 0.0;
    // if (*_screenScrollX1 == 1 && *_samusPosX1 == 1) _actualScreenPosX = 256.0;
    // if (*_screenScrollX1 == 0 && *_samusPosX1 == 1) _actualScreenPosX = 256.0;
    // if (*_screenScrollX1 == 1 && *_samusPosX1 == 0) _actualScreenPosX = 512.0;
    // if (*_screenPosX1 >= 5 && *_screenScrollX1 == 0 && *_screenScrollX2 == 0) _actualScreenPosX = 512.0;

    _samusPosX = 0;
    _samusPosX += (float)*_screenPosX1 * 256.0 + (float)*_screenPosX2;
    _samusPosX += (float)*_samusPosX1 * 256.0 + (float)*_samusPosX2;
    //_samusPosX += (float)*_screenScrollX1 * 256.0;
    // _samusPosX += (float)*_samusPosX2 + (float)*_samusPosX3 / 256.0;
    // if (*_samusPosX1 == 1) _samusPosX += 256.0;
    // if (*_screenScrollX2 == 0) _samusPosX += 256.0;

    _samusPosY = 0;
    _samusPosY += (float)*_screenPosY1 * 256 + (float)*_screenPosY2;
    _samusPosY += (float)*_screenScrollY;
    if (*_screenScrollY == 0) _samusPosY += 256.0;
    // _samusPosY += (float)*_samusPosY1 * 256.0 + (float)*_samusPosY2 + (float)*_samusPosY3 / 256.0;

    // Correction for segment06
    if (*_screenPosY1 < 9 && *_samusPosY1 < 0) _samusPosY += 256.0;

    _bulletCount = (uint8_t)(*_bullet1State > 0) + (uint8_t)(*_bullet2State > 0) + (uint8_t)(*_bullet3State > 0);
    _samusHP = 10.0 * (float)*_samusHP1 + (float)*_samusHP2 / 16.0;

    _bossPendingHits = 0.0;
    _bossPendingHits += 8.0 - (float)*_bossDoor1Hits;
    _bossPendingHits += 8.0 - (float)*_bossDoor2Hits;
    _bossPendingHits += 8.0 - (float)*_bossDoor3Hits;
    _bossPendingHits += 8.0 - (float)*_bossDoor4Hits;
    _bossPendingHits += 8.0 - (float)*_bossDoor5Hits;
    _bossPendingHits += *_bossGlassHit == 1 ? 1.0 : 0.0;
    _bossPendingHits += 32.0 - (float)*_bossHits;
    _bossPendingHits += (float)*_bossPainState / 18.0;
  }

  __INLINE__ void ruleUpdatePreHook() override 
  {
    _pointMagnet.intensityX = 0.0; 
    _pointMagnet.intensityY = 0.0; 
    _pointMagnet.x = 0.0; 
    _pointMagnet.y = 0.0; 

    _traceMagnet.intensityX = 0.0;
    _traceMagnet.intensityY = 0.0;
    _traceMagnet.offset = 0;

    _lagFrameCounterMagnet = 0.0;
    _missileCountMagnet = 0.0;
    _samusHPMagnet = 0.0;
    _lastInputStepReward = 0.0;
    _doorTransitionTimerMagnet = 0.0;
    _missileGrabStatusMagnet = 0.0;
    _door1TimerMagnet = 0.0;
    _door2TimerMagnet = 0.0;
    _door4TimerMagnet = 0.0;
    _pauseFrameCounterMagnet = 0.0;
    _enemy5HPMagnet = 0.0;
    _enemy5StateMagnet = 0.0;
    _bossPendingHitsMagnet = 0.0;
    _bullet1PosXMagnet = 0.0;

    _hashBulletEnabled = true;
    _allowChangeWeapon = true;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _samusDistanceToPointX = std::abs(_pointMagnet.x - _samusRelPosX);
    _samusDistanceToPointY = std::abs(_pointMagnet.y - _samusRelPosY);

    // Updating trace stuff
    if (_useTrace == true)
    {
      _traceStep = (uint16_t) std::max(std::min( (int)_currentStep + _traceMagnet.offset, (int) _trace.size() - 1), 0);
      _traceTargetX = _trace[_traceStep].x;
      _traceTargetY = _trace[_traceStep].y;

      // Updating distance to trace point
      _traceDistanceX = std::abs(_traceTargetX - _samusRelPosX);
      _traceDistanceY = std::abs(_traceTargetY - _samusRelPosY);

      if (_traceDistanceX > 100) _traceDistanceX = 10.0;
      if (_traceDistanceY > 100) _traceDistanceX = 10.0;
      _traceDistance  = sqrtf(_traceDistanceX * _traceDistanceX + _traceDistanceY * _traceDistanceY);
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_lagFrameCounter, sizeof(_lagFrameCounter));
    serializer.pushContiguous(&_pauseFrameCounter, sizeof(_pauseFrameCounter));
    serializer.pushContiguous(&_doorTransitionTimer, sizeof(_doorTransitionTimer));
    serializer.pushContiguous(&_prevGameMode, sizeof(_prevGameMode));
    serializer.pushContiguous(&_hashBulletEnabled, sizeof(_hashBulletEnabled));
    serializer.pushContiguous(&_samusRelPosX, sizeof(_samusRelPosX));
    serializer.pushContiguous(&_samusRelPosY, sizeof(_samusRelPosY));
    serializer.pushContiguous(&_bossPendingHits, sizeof(_bossPendingHits));
    serializer.pushContiguous(&_pauseModePrev, sizeof(_pauseModePrev));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_lagFrameCounter, sizeof(_lagFrameCounter));
    deserializer.popContiguous(&_pauseFrameCounter, sizeof(_pauseFrameCounter));
    deserializer.popContiguous(&_doorTransitionTimer, sizeof(_doorTransitionTimer));
    deserializer.popContiguous(&_prevGameMode, sizeof(_prevGameMode));
    deserializer.popContiguous(&_hashBulletEnabled, sizeof(_hashBulletEnabled));
    deserializer.popContiguous(&_samusRelPosX, sizeof(_samusRelPosX));
    deserializer.popContiguous(&_samusRelPosY, sizeof(_samusRelPosY));
    deserializer.popContiguous(&_bossPendingHits, sizeof(_bossPendingHits));
    deserializer.popContiguous(&_pauseModePrev, sizeof(_pauseModePrev));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // If trace is used, compute its magnet's effect
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityX * _traceDistanceX;
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityY * _traceDistanceY;

    // Evaluating lag frame counter reward
    reward += _lagFrameCounterMagnet * (float)_lagFrameCounter;

    // Evaluating missile count
    reward += _missileCountMagnet * (float)*_missileCount;

    // Evaluating grabbable missile count
    reward += _missileGrabStatusMagnet * (float)*_missileGrabStatus;

    // Evaluating Samus HP
    reward += _samusHPMagnet * _samusHP;

    // Evaluating point magnet
    reward += -1.0 * _pointMagnet.intensityX * _samusDistanceToPointX;
    reward += -1.0 * _pointMagnet.intensityY * _samusDistanceToPointY;

    // Rewarding door transition
    reward += _doorTransitionTimerMagnet * (float) _doorTransitionTimer;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputStepReward * _lastInputStep;

    // Rewarding hitting doors with missiles
    reward += _door1TimerMagnet * (float) *_door1Timer;
    reward += _door2TimerMagnet * (float) *_door2Timer;
    reward += _door4TimerMagnet * (float) *_door4Timer;

    // Rewarding enemy 5 hp
    reward += _enemy5HPMagnet * (float) (*_enemy5HP == 255 ? 5 : *_enemy5HP);
    reward += _enemy5StateMagnet * (float) *_enemy5State;

    reward += _pauseFrameCounterMagnet * (float) _pauseFrameCounter;

    // Rewarding boss room hits
    reward += _bossPendingHitsMagnet * (float) _bossPendingHits;

    // Reward Bullet 1 Pos X
    reward += _bullet1PosXMagnet * (float) *_bullet1PosX;

    // Returning reward
    return reward;
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    if (*_samusDoorState != 0)
    {
      allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx });
      if (_pauseModePrev == *_pauseMode) allowedInputSet.insert(allowedInputSet.end(), { _input_S });
      return;
    }

    if (*_pauseMode == 1)
    {
      if (_pauseModePrev == *_pauseMode) allowedInputSet.insert(allowedInputSet.end(), { _input_S });
      if (*_pauseMode == 1 && _pauseModePrev == 0) allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx });
      return;
    } 

    if (*_gameMode == 9)
    {
      allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx });
      return;
    }

    allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx });
    if (*_samusAnimation == 0x0001) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_AL, _input_AR, _input_UR, _input_UL });
    if (*_samusAnimation == 0x0002) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_AL, _input_AR, _input_Rs, _input_Ls, _input_UR, _input_UL });
    if (*_samusAnimation == 0x0003) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_AL, _input_AR, _input_Rs, _input_Ls, _input_UR, _input_UL });
    if (*_samusAnimation == 0x0005) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_UA, _input_UB, _input_UR, _input_UL, _input_URB, _input_ULB });
    if (*_samusAnimation == 0x0007) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_A, _input_D, _input_DB, _input_UD, _input_UL, _input_UR, _input_UB, _input_UA, _input_DA, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_LBA, _input_DBA, _input_RBA, _input_UBA, _input_URA, _input_URB, _input_ULA, _input_ULB, _input_UDB });
    if (*_samusAnimation == 0x0008) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_A, _input_D, _input_DB, _input_UD, _input_UL, _input_UR, _input_UB, _input_UA, _input_DA, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_LBA, _input_DBA, _input_RBA, _input_UBA, _input_URA, _input_URB, _input_ULA, _input_ULB, _input_UDB, _input_LsA });
    if (*_samusAnimation == 0x000A) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_U, _input_A, _input_L, _input_D, _input_UR, _input_UB, _input_UA, _input_UL, _input_DL, _input_DB, _input_LR, _input_UD, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_ULB, _input_ULA, _input_URB, _input_UDB, _input_URA, _input_UBA, _input_DLB, _input_DBA, _input_LRB, _input_LBA, _input_RBA, _input_sB });
    if (*_samusAnimation == 0x000B) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_U, _input_L, _input_D, _input_A, _input_LR, _input_UL, _input_UR, _input_UB, _input_DL, _input_DB, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_LBA, _input_DBA, _input_RBA, _input_URB, _input_ULB, _input_URA, _input_ULA, _input_sA, _input_RsA, _input_LsA });
    if (*_samusAnimation == 0x000C) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_U, _input_A, _input_L, _input_UR, _input_UB, _input_DL, _input_DB, _input_UL, _input_LR, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_ULA, _input_URB, _input_URA, _input_ULB, _input_UBA, _input_DLA, _input_LRA, _input_LBA, _input_RBA });
    if (*_samusAnimation == 0x000D) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_A, _input_DB, _input_UL, _input_UR, _input_UB, _input_UA, _input_DL, _input_LR, _input_BL, _input_BA, _input_AL, _input_BR, _input_AR, _input_UDA, _input_ULB, _input_ULA, _input_URB, _input_URA, _input_UBA, _input_RBA, _input_DLB, _input_DLA, _input_LBA, _input_LRB, _input_LRA, _input_URBA, _input_ULBA, _input_Us, _input_LsA, _input_Ls, _input_Rs, _input_s });
    if (*_samusAnimation == 0x000F) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_A, _input_D, _input_DA, _input_UL, _input_UR, _input_UB, _input_UA, _input_DB, _input_LR, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_LBA, _input_DLA, _input_RBA, _input_UBA, _input_URA, _input_URB, _input_ULA, _input_ULB, _input_UDA });
    if (*_samusAnimation == 0x0010) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_A, _input_D, _input_DB, _input_UL, _input_UR, _input_UB, _input_UA, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_LBA, _input_DLA, _input_RBA, _input_UBA, _input_URA, _input_URB, _input_ULA, _input_ULB });
    if (*_samusAnimation == 0x0011) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_A, _input_L, _input_U, _input_Ls, _input_AL, _input_UR, _input_UL, _input_UA, _input_UB, _input_DB, _input_BL, _input_BR, _input_AR, _input_BA, _input_ULA, _input_ULB, _input_URB, _input_URA, _input_UBA, _input_LBA, _input_RBA, _input_URBA, _input_ULBA });
    if (*_samusAnimation == 0x0012) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_A, _input_DB, _input_UL, _input_UR, _input_UB, _input_UA, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_LBA, _input_RBA, _input_UBA, _input_URA, _input_URB, _input_ULA, _input_ULB, _input_Rs, _input_Ls, _input_UsA });
    if (*_samusAnimation == 0x0013) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_D, _input_U, _input_UA, _input_UD, _input_UDR, _input_UR, _input_UL });
    if (*_samusAnimation == 0x0014) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_R, _input_L, _input_U });
    if (*_samusAnimation == 0x0017) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_R, _input_L, _input_D, _input_U, _input_BR, _input_DR, _input_DL, _input_UD, _input_DRA, _input_DLA, _input_UDR, _input_UDL, _input_UL, _input_UR, _input_AL, _input_AR });
    if (*_samusAnimation == 0x0018) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_R, _input_L, _input_D, _input_U, _input_UDR, _input_UL, _input_UR, _input_AL, _input_AR });
    if (*_samusAnimation == 0x0019) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_R, _input_L, _input_D, _input_U, _input_AR, _input_AL, _input_UL, _input_UR, _input_Ls, _input_Rs });
    if (*_samusAnimation == 0x001A) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_R, _input_L, _input_U, _input_AR, _input_AL, _input_UL, _input_UR, _input_Ls, _input_Rs });
    if (*_samusAnimation == 0x0020) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_R, _input_A, _input_BR, _input_UL, _input_UR, _input_UB, _input_UA, _input_DL, _input_DB, _input_BA, _input_LR, _input_BL, _input_AL, _input_AR, _input_LRA, _input_DLA, _input_LBA, _input_UBA, _input_RBA, _input_URA, _input_URB, _input_ULA, _input_ULB, _input_Ls, _input_Rs });
    if (*_samusAnimation == 0x0021) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_A, _input_D, _input_DA, _input_UL, _input_UR, _input_UB, _input_UA, _input_DL, _input_DB, _input_LR, _input_BA, _input_BL, _input_AL, _input_AR, _input_BR, _input_RBA, _input_ULB, _input_ULA, _input_URB, _input_URA, _input_UBA, _input_LRB, _input_DLB, _input_DLA, _input_DBA, _input_LBA, _input_LRA, _input_URsA, _input_Ls, _input_Rs });
    if (*_samusAnimation == 0x0023) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_LR, _input_LRB, _input_DLB });
    if (*_samusAnimation == 0x0024) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_AR, _input_AL, _input_LR });
    if (*_samusAnimation == 0x0025) allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_R, _input_L, _input_U, _input_UR, _input_UL });
    if (*_samusAnimation == 0x0027) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_A, _input_R, _input_DB, _input_UL, _input_UR, _input_UB, _input_UA, _input_UD, _input_BR, _input_DA, _input_BA, _input_BL, _input_AL, _input_AR, _input_UDB, _input_UDA, _input_ULB, _input_ULA, _input_URB, _input_URA, _input_UBA, _input_DBA, _input_LBA, _input_RBA });
    if (*_samusAnimation == 0x0028) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_A, _input_R, _input_DB, _input_UL, _input_UR, _input_UB, _input_UA, _input_UD, _input_BR, _input_DA, _input_BA, _input_BL, _input_AL, _input_AR, _input_UDB, _input_UDA, _input_ULB, _input_ULA, _input_URB, _input_URA, _input_UBA, _input_DBA, _input_LBA, _input_RBA });
    if (*_samusAnimation == 0x0034) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_R, _input_A, _input_BR, _input_AR, _input_AL, _input_BL, _input_LR, _input_BA, _input_DB, _input_UL, _input_DL, _input_UA, _input_UB, _input_UR, _input_ULB, _input_URB, _input_DBA, _input_RBA });
    if (*_samusAnimation == 0x0035) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_U, _input_A, _input_L, _input_D, _input_UR, _input_UB, _input_UA, _input_UL, _input_DL, _input_DB, _input_DA, _input_UD, _input_LR, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_ULB, _input_UDB, _input_ULA, _input_URB, _input_URA, _input_UDR, _input_UBA, _input_DLB, _input_DLA, _input_DBA, _input_LRB, _input_LRA, _input_LBA, _input_RBA });
    if (*_samusAnimation == 0x0036) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_A, _input_D, _input_DB, _input_UL, _input_UR, _input_UB, _input_UA, _input_DL, _input_LR, _input_BL, _input_AL, _input_BR, _input_AR, _input_BA, _input_LRA, _input_LBA, _input_DLA, _input_RBA, _input_UBA, _input_URA, _input_URB, _input_ULA, _input_ULB, _input_UDA, _input_UsA, _input_DsA, _input_URs });
    if (*_samusAnimation == 0x0038) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_UA, _input_UB, _input_UR, _input_UL, _input_UD, _input_URB, _input_ULB, _input_URA, _input_ULA, _input_s });
    if (*_samusAnimation == 0x0039) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_UA, _input_UB, _input_UR, _input_UL, _input_URB, _input_ULB });
    if (*_samusAnimation == 0x003A) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_UA, _input_UB, _input_UR, _input_UL, _input_URB, _input_ULB });
    if (*_samusAnimation == 0x003C) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_BL, _input_UA, _input_UB, _input_UR, _input_UL, _input_URB, _input_ULB, _input_UDL });
    if (*_samusAnimation == 0x003E) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_R, _input_L, _input_U, _input_BR, _input_BL, _input_UA, _input_UB, _input_UR, _input_UL, _input_URB, _input_ULB });
    if (*_samusAnimation == 0x0040) allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_BL, _input_UA, _input_UB, _input_UR, _input_UL, _input_URB, _input_ULB });

    if (_allowPause == true)
    {
     allowedInputSet.insert(allowedInputSet.end(), { _input_ALS, _input_LS, _input_S, _input_AS, _input_BS });
    }

    if (_allowFire == false)
    {
      std::vector<InputSet::inputIndex_t> newInputSet;
      for(const auto& input : allowedInputSet)
      {
        if (input != _input_B)
        if (input != _input_BR)
        if (input != _input_BL)
        if (input != _input_Ls)
        if (input != _input_Rs)
        if (input != _input_UB)
        if (input != _input_DB)
        if (input != _input_LBA)
        if (input != _input_RBA)
        if (input != _input_DBA)
        if (input != _input_URB)
        if (input != _input_UBA)
        if (input != _input_ULB)
        if (input != _input_DLB)
        if (input != _input_BA)
        if (input != _input_URs)
        if (input != _input_DsA)
        if (input != _input_LsA)
        if (input != _input_Us)
        if (input != _input_UsA)
        if (input != _input_URBA)
        if (input != _input_ULBA)
        if (input != _input_LRB)
        newInputSet.push_back(input);
      }
      allowedInputSet = newInputSet; 
    }

    if (_allowChangeWeapon == false)
    {
      std::vector<InputSet::inputIndex_t> newInputSet;
      for(const auto& input : allowedInputSet)
      {
        if (input != _input_sA  )
        if (input != _input_RsA  )
        if (input != _input_LsA  )
        if (input != _input_UsA  )
        if (input != _input_DsA  )
        if (input != _input_URs  )
        if (input != _input_URsA )
        if (input != _input_Us   )
        if (input != _input_Rs   )
        if (input != _input_Ls   )
        if (input != _input_s    )
        newInputSet.push_back(input);
      }
      allowedInputSet = newInputSet; 
    }

    
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Frame Counter:          %03u\n", *_frameCounter);
    jaffarCommon::logger::log("[J+]  + Game Mode:              %02u (Prev: %02u)\n", *_gameMode, _prevGameMode);
    jaffarCommon::logger::log("[J+]  + Pause Mode:             %02u (Prev: %02u)\n", *_pauseMode, _pauseModePrev);
    jaffarCommon::logger::log("[J+]  + NMI Flag:               %02u (%03u, %03u)\n", *_NMIFlag, *_NMIData, *_NMIPALData);
    jaffarCommon::logger::log("[J+]  + Lag Frame Counter:      %05u\n", _lagFrameCounter);
    jaffarCommon::logger::log("[J+]  + Pause Frame Counter:    %05u\n", _pauseFrameCounter);
    jaffarCommon::logger::log("[J+]  + Hash Bullet Enabled:    %s\n", _hashBulletEnabled ? "True" : "False");
    jaffarCommon::logger::log("[J+]  + Allow Change Weapon:    %s\n", _allowChangeWeapon ? "True" : "False");

    jaffarCommon::logger::log("[J+]  + Samus Rel Pos X:        %4.3f\n", _samusRelPosX);
    jaffarCommon::logger::log("[J+]  + Samus Rel Pos Y:        %4.3f\n", _samusRelPosY);

    jaffarCommon::logger::log("[J+]  + Samus Pos X:            %4.3f\n", _samusPosX);
    jaffarCommon::logger::log("[J+]    + Screen Pos X:         %03u %03u\n", *_screenPosX1, *_screenPosX2);
    jaffarCommon::logger::log("[J+]    + Scroll Pos X:         %03u %03u\n", *_screenScrollX1, *_screenScrollX2);
    jaffarCommon::logger::log("[J+]    + Samus Pos X:          %03u %03u %03u\n", *_samusPosX1, *_samusPosX2, *_samusPosX3);
    jaffarCommon::logger::log("[J+]  + Samus Pos Y:            %4.3f\n", _samusPosY);
    jaffarCommon::logger::log("[J+]    + Screen Pos Y:         %03u %03u %03u\n", *_screenPosY1, *_screenPosY2, *_screenScrollY);
    jaffarCommon::logger::log("[J+]    + Samus Pos Y:          %03u %03u %03u\n", *_samusPosY1, *_samusPosY2, *_samusPosY3);

    jaffarCommon::logger::log("[J+]  + Samus Animation:        0x%X\n", *_samusAnimation);
    jaffarCommon::logger::log("[J+]  + Samus Direction:        %03u\n", *_samusDirection);
    jaffarCommon::logger::log("[J+]  + Samus Door Side:        %03u\n", *_samusDoorSide);
    jaffarCommon::logger::log("[J+]  + Samus Door State:       %03u (Timer: %03u)\n", *_samusDoorState, _doorTransitionTimer);
    jaffarCommon::logger::log("[J+]  + Samus Jump State:       %03u\n", *_samusJumpState);
    jaffarCommon::logger::log("[J+]  + Enemy States:           [ %02u, %02u, %02u, %02u, %02u, %02u ]\n", *_enemy1State, *_enemy2State, *_enemy3State, *_enemy4State, *_enemy5State, *_enemy6State );
    jaffarCommon::logger::log("[J+]  + Enemy Freeze Timer:     [ %02u, %02u, %02u, %02u, %02u, %02u ]\n", *_enemy1FreezeTimer, *_enemy2FreezeTimer, *_enemy3FreezeTimer, *_enemy4FreezeTimer, *_enemy5FreezeTimer, *_enemy6FreezeTimer );
    jaffarCommon::logger::log("[J+]  + Enemy HP:               [ %02u, %02u, %02u, %02u, %02u, %02u ]\n", *_enemy1HP, *_enemy2HP, *_enemy3HP, *_enemy4HP, *_enemy5HP, *_enemy6HP );
    jaffarCommon::logger::log("[J+]  + Door States:            [ %02u, %02u, %02u, %02u ]\n", *_door1State, *_door2State, *_door3State, *_door4State);
    jaffarCommon::logger::log("[J+]  + Door Timers:            [ %02u, %02u, %02u, %02u ]\n", *_door1Timer, *_door2Timer, *_door3Timer, *_door4Timer);
    jaffarCommon::logger::log("[J+]  + Boss Door Hits:         [ %02u, %02u, %02u, %02u, %02u ]\n", *_bossDoor1Hits, *_bossDoor2Hits, *_bossDoor3Hits, *_bossDoor4Hits, *_bossDoor5Hits);
    jaffarCommon::logger::log("[J+]  + Boss Glass Hit:         %02u\n", *_bossGlassHit);
    jaffarCommon::logger::log("[J+]  + Boss Hits:              %02u\n", *_bossHits);
    jaffarCommon::logger::log("[J+]  + Boss Pain State:        %02u\n", *_bossPainState);
    jaffarCommon::logger::log("[J+]  + Boss Pending Hits:      %3.3f\n", _bossPendingHits);
    jaffarCommon::logger::log("[J+]  + Bullet Count:           %02u\n", _bulletCount);
    jaffarCommon::logger::log("[J+]  + Bullet States:          [ %02u, %02u, %02u ]\n", *_bullet1State, *_bullet2State, *_bullet3State);
    jaffarCommon::logger::log("[J+]  + Bullet Pos X:           [ %02u, %02u, %02u ]\n", *_bullet1PosX, *_bullet2PosX, *_bullet3PosX);
    jaffarCommon::logger::log("[J+]  + Bullet Pos Y:           [ %02u, %02u, %02u ]\n", *_bullet1PosY, *_bullet2PosY, *_bullet3PosY);
    jaffarCommon::logger::log("[J+]  + Bullet Vel X:           [ %02d, %02d, %02d ]\n", *_bullet1VelX, *_bullet2VelX, *_bullet3VelX);
    jaffarCommon::logger::log("[J+]  + Bullet Vel Y:           [ %02d, %02d, %02d ]\n", *_bullet1VelY, *_bullet2VelY, *_bullet3VelY);
    jaffarCommon::logger::log("[J+]  + Equipment Flags:        %03u\n", *_equipmentFlags);
    jaffarCommon::logger::log("[J+]  + Samus Selected Weapon:  %03u\n", *_samusSelectedWeapon);
    jaffarCommon::logger::log("[J+]  + Samus Missile Count:    %02u (Grab State: %02u)\n", *_missileCount, *_missileGrabStatus);
    jaffarCommon::logger::log("[J+]  + Samus HP:               %02f (%02u x 10 + %02u)\n", _samusHP, *_samusHP1, *_samusHP2);

    if (std::abs(_pointMagnet.intensityX) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet X                          \n");
      jaffarCommon::logger::log("[J+]    + Intensity                             %3.3f\n", _pointMagnet.intensityX);
      jaffarCommon::logger::log("[J+]    + Position                              %3.3f\n", _pointMagnet.x);
      jaffarCommon::logger::log("[J+]    + Distance                              %3.3f\n", _samusDistanceToPointX);
    }

    if (std::abs(_pointMagnet.intensityY) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet Y                          \n");
      jaffarCommon::logger::log("[J+]    + Intensity                             %3.3f\n", _pointMagnet.intensityY);
      jaffarCommon::logger::log("[J+]    + Position                              %3.3f\n", _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance                              %3.3f\n", _samusDistanceToPointY);
    }

    if (std::abs(_missileCountMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Missile Count Magnet                    Intensity: %.5f\n", _missileCountMagnet);

    if (std::abs(_missileGrabStatusMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Missile Grab Status Magnet              Intensity: %.5f\n", _missileGrabStatusMagnet);


    if (std::abs(_samusHPMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Samus HP Magnet                         Intensity: %.5f\n", _samusHPMagnet);

    if (std::abs(_lastInputStepReward) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Last Input Magnet                       Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _lastInputStepReward);
      jaffarCommon::logger::log("[J+]    + Last Input Step                       %04u\n", _lastInputStep);
    }

    if (std::abs(_doorTransitionTimerMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Door Transition Timer Magnet            Intensity: %.5f\n", _doorTransitionTimerMagnet);

    if (std::abs(_door2TimerMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Door 1 Timer Magnet                     Intensity: %.5f\n", _door1TimerMagnet);

    if (std::abs(_door2TimerMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Door 2 Timer Magnet                     Intensity: %.5f\n", _door2TimerMagnet);
    
    if (std::abs(_door2TimerMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Door 4 Timer Magnet                     Intensity: %.5f\n", _door2TimerMagnet);

    if (std::abs(_enemy5HPMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Enemy 5 HP Magnet                       Intensity: %.5f\n", _enemy5HPMagnet);
      
    if (std::abs(_enemy5StateMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Enemy 5 State Magnet                    Intensity: %.5f\n", _enemy5StateMagnet);
      
    if (std::abs(_pauseFrameCounterMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Pause Frame Counter Magnet              Intensity: %.5f\n", _pauseFrameCounterMagnet);
      
    if (std::abs(_lagFrameCounterMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Lag Frame Count Magnet                  Intensity: %.5f\n", _lagFrameCounterMagnet);

    if (std::abs(_bossPendingHitsMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Boss Pending Hits Manget                Intensity: %.5f\n", _bossPendingHitsMagnet);

    if (std::abs(_bullet1PosXMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Bullet 1 Pos X                          Intensity: %.5f, Effect: %3.3f\n", _bullet1PosXMagnet, _bullet1PosXMagnet * (float) *_bullet1PosX);
      

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

    if (actionType == "Set Trace Magnet")
    {
      if (_useTrace == false) JAFFAR_THROW_LOGIC("Specified Trace Magnet, but no trace file was provided.");
      auto intensityX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity X");
      auto intensityY = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Y");
      auto offset    = jaffarCommon::json::getNumber<int>(actionJs, "Offset");
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{.intensityX = intensityX, .intensityY = intensityY, .offset = offset }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Point Magnet")
    {
      auto intensityX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity X");
      auto intensityY = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Y");
      auto x       = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y       = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensityX = intensityX, .intensityY = intensityY, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Last Input Step Reward")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_lastInputStepReward = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Door Transition Timer Reward")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_doorTransitionTimerMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Lag Frame Counter Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_lagFrameCounterMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Missile Count Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_missileCountMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Missile Grab Status Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_missileGrabStatusMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Samus HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_samusHPMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Enemy 5 HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_enemy5HPMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Enemy 5 State Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_enemy5StateMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Bullet 1 Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bullet1PosXMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Door 1 Timer Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_door1TimerMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Door 2 Timer Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_door2TimerMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Door 4 Timer Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_door4TimerMagnet = intensity; });
      recognizedActionType = true;
    }


    if (actionType == "Set Pause Frame Counter Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_pauseFrameCounterMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Hash Bullet Enabled")
    {
      auto value = jaffarCommon::json::getBoolean(actionJs, "Value");
      rule.addAction([=, this]() { this->_hashBulletEnabled = value; });
      recognizedActionType = true;
    }

    if (actionType == "Set Allow Change Weapon")
    {
      auto value = jaffarCommon::json::getBoolean(actionJs, "Value");
      rule.addAction([=, this]() { this->_allowChangeWeapon = value; });
      recognizedActionType = true;
    }

    if (actionType == "Set Boss Pending Hits Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bossPendingHitsMagnet = intensity; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    // return jaffarCommon::hash::hash_t({0, *_samusAction});
    return jaffarCommon::hash::hash_t({0, 0});
  }

  // Datatype to describe a point magnet
  bool _isDumpingTrace = false;
  std::string _traceDumpString;

  __INLINE__ void playerPrintCommands() const override
  {
    jaffarCommon::logger::log("[J+] t: start/stop trace dumping (%s)\n", _isDumpingTrace ? "On" : "Off");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
    // If storing a trace, do it here
    if (_isDumpingTrace == true) _traceDumpString += std::to_string(_samusRelPosX) + std::string(" ") + std::to_string(_samusRelPosY) + std::string("\n");

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


  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensityX = 0.0; // How strong the magnet is on the X axis
    float intensityY = 0.0; // How strong the magnet is on the X axis
    float x         = 0.0; // What is the point of attraction X
    float y         = 0.0; // What is the point of attraction X
  };
  pointMagnet_t _pointMagnet;
  pointMagnet_t _screenMagnet;

  float _samusDistanceToPointX;
  float _samusDistanceToPointY;

  // Datatype to describe a magnet
  struct weaponMagnet_t {
    float intensity = 0.0; // How strong the magnet is
    uint8_t value = 0;  // Specifies the weapon number
  };

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;

  uint8_t* _lowMem;
  uint8_t* _srmMem;

  float _samusPosX;
  float _samusPosY;
  
  struct traceMagnet_t
  {
    float intensityX = 0.0; // How strong the magnet is on X
    float intensityY = 0.0; // How strong the magnet is on Y
    int offset      = 0; // Which entry (step) to look at wrt the current emulation step
  };

  // Reward for the last time an input was made (for early termination)
  float _lastInputStepReward;
  traceMagnet_t _traceMagnet;
  float _lagFrameCounterMagnet;
  float _missileCountMagnet;
  float _samusHPMagnet;
  float _doorTransitionTimerMagnet;
  float _missileGrabStatusMagnet;
  float _door1TimerMagnet;
  float _door2TimerMagnet;
  float _door4TimerMagnet;
  float _pauseFrameCounterMagnet;
  float _enemy5HPMagnet;
  float _enemy5StateMagnet;
  float _bossPendingHitsMagnet;
  float _bullet1PosXMagnet;

  // Whether we use a trace
  bool _useTrace = false;
  bool _allowFire;
  bool _allowPause;
  bool _allowChangeWeapon = true;
  bool _hashBulletEnabled = true;

  // Location of the trace file
  std::string _traceFilePath;

  // Trace contents
  struct traceEntry_t
  {
    float x;
    float y;
  };
  std::vector<traceEntry_t> _trace;

  // Current trace target
  uint16_t _traceStep;
  float _traceTargetX;
  float _traceTargetY;
  float _traceDistanceX;
  float _traceDistanceY;
  float _traceDistance;

  float _samusRelPosX;
  float _samusRelPosY;
  
  // Possible inputs
  InputSet::inputIndex_t _input_U  ;
  InputSet::inputIndex_t _input_D  ;
  InputSet::inputIndex_t _input_L  ;
  InputSet::inputIndex_t _input_R  ;
  InputSet::inputIndex_t _input_A  ;
  InputSet::inputIndex_t _input_B  ;
  InputSet::inputIndex_t _input_UD;
  InputSet::inputIndex_t _input_UL ;
  InputSet::inputIndex_t _input_UR ;
  InputSet::inputIndex_t _input_UA ;
  InputSet::inputIndex_t _input_UB ;
  InputSet::inputIndex_t _input_DL ;
  InputSet::inputIndex_t _input_DR ;
  InputSet::inputIndex_t _input_DA ;
  InputSet::inputIndex_t _input_DB ;
  InputSet::inputIndex_t _input_AL ;
  InputSet::inputIndex_t _input_BL ;
  InputSet::inputIndex_t _input_AR ;
  InputSet::inputIndex_t _input_BR ;
  InputSet::inputIndex_t _input_ULA;
  InputSet::inputIndex_t _input_URA;
  InputSet::inputIndex_t _input_UBA;
  InputSet::inputIndex_t _input_DLA;
  InputSet::inputIndex_t _input_DRA;
  InputSet::inputIndex_t _input_DBA;
  InputSet::inputIndex_t _input_BLA;
  InputSet::inputIndex_t _input_BRA;
  InputSet::inputIndex_t _input_UDB;
  InputSet::inputIndex_t _input_ULB;
  InputSet::inputIndex_t _input_URB;
  InputSet::inputIndex_t _input_DLB;
  InputSet::inputIndex_t _input_DRB;
  InputSet::inputIndex_t _input_LRA;
  InputSet::inputIndex_t _input_LRB;
  InputSet::inputIndex_t _input_BA;
  InputSet::inputIndex_t _input_RBA;
  InputSet::inputIndex_t _input_LBA;
  InputSet::inputIndex_t _input_LR;
  InputSet::inputIndex_t _input_UDA;
  InputSet::inputIndex_t _input_UDR;
  InputSet::inputIndex_t _input_UDL;
  InputSet::inputIndex_t _input_ARS ;
  InputSet::inputIndex_t _input_ALS ;
  InputSet::inputIndex_t _input_AS ;
  InputSet::inputIndex_t _input_RS  ;
  InputSet::inputIndex_t _input_LS  ;
  InputSet::inputIndex_t _input_s;
  InputSet::inputIndex_t _input_sA;
  InputSet::inputIndex_t _input_URBA;
  InputSet::inputIndex_t _input_ULBA;
  InputSet::inputIndex_t _input_RsA;
  InputSet::inputIndex_t _input_LsA;
  InputSet::inputIndex_t _input_UsA;
  InputSet::inputIndex_t _input_DsA;
  InputSet::inputIndex_t _input_URs  ;
  InputSet::inputIndex_t _input_URsA ;
  InputSet::inputIndex_t _input_Us   ;
  InputSet::inputIndex_t _input_Rs   ;
  InputSet::inputIndex_t _input_Ls   ;
  InputSet::inputIndex_t _input_S;
  InputSet::inputIndex_t _input_sB;
  InputSet::inputIndex_t _input_BS;

  uint8_t* _pauseMode           ;
  uint8_t* _frameCounter        ;
  uint8_t* _NMIFlag             ;
  uint8_t* _NMIData;
  uint8_t* _NMIPALData;
  uint8_t* _gameMode            ;

  uint8_t* _samusPosX1        ;
  uint8_t* _samusPosX2        ;
  uint8_t* _samusPosX3        ;

  uint8_t* _screenPosX1         ;
  uint8_t* _screenPosX2         ;
  uint8_t* _screenScrollX1;
  uint8_t* _screenScrollX2;

  uint8_t* _samusPosY1        ;
  uint8_t* _samusPosY2        ;
  uint8_t* _samusPosY3        ;

  uint8_t* _screenPosY1         ;
  uint8_t* _screenPosY2         ;
  uint8_t* _screenScrollY;
  
  uint8_t* _samusAnimation      ;
  uint8_t* _samusDirection      ;
  uint8_t* _samusDoorSide       ;
  uint8_t* _samusDoorState      ;
  uint8_t* _samusJumpState      ;
  uint8_t* _equipmentFlags      ;
  uint8_t* _samusSelectedWeapon ;
  uint8_t* _missileCount        ;
  uint8_t* _missileGrabStatus    ;
  uint8_t* _samusHP1            ;
  uint8_t* _samusHP2            ;
  uint8_t* _door1State          ;
  uint8_t* _door2State          ;
  uint8_t* _door3State          ;
  uint8_t* _door4State          ;
  uint8_t* _door1Timer          ;
  uint8_t* _door2Timer          ;
  uint8_t* _door3Timer          ;
  uint8_t* _door4Timer          ;
  uint8_t* _bullet1State        ;
  uint8_t* _bullet2State        ;
  uint8_t* _bullet3State        ;
  uint8_t* _bullet1PosX         ;
  uint8_t* _bullet2PosX         ;
  uint8_t* _bullet3PosX         ;
  uint8_t* _bullet1PosY         ;
  uint8_t* _bullet2PosY         ;
  uint8_t* _bullet3PosY         ;
  int8_t* _bullet1VelX         ;
  int8_t* _bullet2VelX         ;
  int8_t* _bullet3VelX         ;
  int8_t* _bullet1VelY         ;
  int8_t* _bullet2VelY         ;
  int8_t* _bullet3VelY         ;
  uint8_t* _upperDoorState;
  
  uint8_t* _enemy1State        ;
  uint8_t* _enemy2State        ;
  uint8_t* _enemy3State        ;
  uint8_t* _enemy4State        ;
  uint8_t* _enemy5State        ;
  uint8_t* _enemy6State        ;

  uint8_t* _enemy1HP        ;
  uint8_t* _enemy2HP        ;
  uint8_t* _enemy3HP        ;
  uint8_t* _enemy4HP        ;
  uint8_t* _enemy5HP        ;
  uint8_t* _enemy6HP        ;

  uint8_t* _bossDoor1Hits;
  uint8_t* _bossDoor2Hits;
  uint8_t* _bossDoor3Hits;
  uint8_t* _bossDoor4Hits;
  uint8_t* _bossDoor5Hits;
  uint8_t* _bossGlassHit;
  uint8_t* _bossHits;
  uint8_t* _bossPainState;


  uint8_t* _enemy1FreezeTimer;
  uint8_t* _enemy2FreezeTimer;
  uint8_t* _enemy3FreezeTimer;
  uint8_t* _enemy4FreezeTimer;
  uint8_t* _enemy5FreezeTimer;
  uint8_t* _enemy6FreezeTimer;

  uint16_t _lagFrameCounter     ;
  uint16_t _pauseFrameCounter   ;
  uint16_t _doorTransitionTimer;

  // Game values

  uint16_t _currentStep;
  uint16_t _lastInputStep;
  uint8_t _bulletCount;
  float _samusHP;
  uint8_t _prevGameMode;
  float _bossPendingHits;
  uint8_t _pauseModePrev;
};


} // namespace nes

} // namespace games

} // namespace jaffarPlus
