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

#define ENEMY_COUNT 8
#define ORB_COUNT 8

class NinjaGaiden3 final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Ninja Gaiden 3"; }

  NinjaGaiden3(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Enable B button (attack)
    _enableAttack = jaffarCommon::json::getBoolean(config, "Enable Attack");

    // Getting emulator name (for runtime use)
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

    // Registering native game properties
    
    registerGameProperty("Game Mode"                   , &_lowMem[0x001F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Current Stage"               , &_lowMem[0x005F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Global Timer"                , &_lowMem[0x005E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Game Transition"             , &_lowMem[0x002C], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Animation 1"           , &_lowMem[0x00FA], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Animation 2"           , &_lowMem[0x00FF], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Action"                , &_lowMem[0x037E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Frame"                 , &_lowMem[0x0057], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Weapon"                , &_lowMem[0x009F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Power"                 , &_lowMem[0x00CD], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Power Max"             , &_lowMem[0x00CE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja HP"                    , &_lowMem[0x00A7], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Pos X1"                , &_lowMem[0x00FD], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Pos X2"                , &_lowMem[0x04FE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Speed X1"              , &_lowMem[0x055E], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Ninja Speed X2"              , &_lowMem[0x0546], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Pos Y1"                , &_lowMem[0x058E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Pos Y2"                , &_lowMem[0x0576], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Speed Y1"              , &_lowMem[0x05D6], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Ninja Speed Y2"              , &_lowMem[0x05BE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Momentum"              , &_lowMem[0x0089], Property::datatype_t::dt_int8,  Property::endianness_t::little); 
    registerGameProperty("Ninja Direction"             , &_lowMem[0x00FC], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos X1"               , &_lowMem[0x00DC], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos X2"               , &_lowMem[0x00DB], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos X3"               , &_lowMem[0x00DA], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos Y1"               , &_lowMem[0x0085], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos Y2"               , &_lowMem[0x0084], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos Y3"               , &_lowMem[0x0059], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Boss HP"                     , &_lowMem[0x00A8], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Boss Pos Y"                  , &_lowMem[0x05A5], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Boss Pos X"                  , &_lowMem[0x052D], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Invincibility Timer"   , &_lowMem[0x00AD], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Invincibility State"   , &_lowMem[0x007B], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Sword Type"            , &_lowMem[0x00A9], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Vertical Collision"    , &_lowMem[0x0074], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Ninja Horizontal Collision"  , &_lowMem[0x0076], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Orb State Vector"            , &_lowMem[0x0381], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Enemy State Vector"          , &_lowMem[0x038E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Buffered Movement"           , &_lowMem[0x0089], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Level Exit Flag 1"           , &_lowMem[0x002C], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Level Exit Flag 2"           , &_lowMem[0x01FE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Weapon 1 Active"             , &_lowMem[0x04EE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Weapon 2 Active"             , &_lowMem[0x04EF], Property::datatype_t::dt_uint8, Property::endianness_t::little); 

    _gameMode                          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Mode"                    )]->getPointer();
    _currentStage                      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Transition"              )]->getPointer();
    _globalTimer                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer"                 )]->getPointer();
    _gameTransition                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer"                 )]->getPointer();
    _ninjaAnimation1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Animation 1"            )]->getPointer();
    _ninjaAnimation2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Animation 2"            )]->getPointer();
    _ninjaAction                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Action"                 )]->getPointer();
    _ninjaFrame                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Frame"                  )]->getPointer();
    _ninjaWeapon                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Weapon"                 )]->getPointer();
    _ninjaPower                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Power"                  )]->getPointer();
    _ninjaPowerMax                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Power Max"              )]->getPointer();
    _ninjaHP                           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja HP"                     )]->getPointer();
    _ninjaPosX1                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Pos X1"                 )]->getPointer();
    _ninjaPosX2                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Pos X2"                 )]->getPointer();
    _ninjaSpeedX1                      = (int8_t* )_propertyMap[jaffarCommon::hash::hashString("Ninja Speed X1"               )]->getPointer();
    _ninjaSpeedX2                      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Speed X2"               )]->getPointer();
    _ninjaPosY1                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Pos Y1"                 )]->getPointer();
    _ninjaPosY2                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Pos Y2"                 )]->getPointer();
    _ninjaSpeedY1                      = (int8_t* )_propertyMap[jaffarCommon::hash::hashString("Ninja Speed Y1"               )]->getPointer();
    _ninjaSpeedY2                      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Speed Y2"               )]->getPointer();
    _ninjaDirection                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Direction"              )]->getPointer();
    _ninjaMomentum                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Momentum"               )]->getPointer();
    _screenPosX1                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X1"                )]->getPointer();
    _screenPosX2                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X2"                )]->getPointer();
    _screenPosX3                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X3"                )]->getPointer();
    _screenPosY1                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos Y1"                )]->getPointer();
    _screenPosY2                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos Y2"                )]->getPointer();
    _screenPosY3                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos Y3"                )]->getPointer();
    _bossHP                            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Boss HP"                      )]->getPointer();
    _bossPosY                          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Boss Pos Y"                   )]->getPointer();
    _bossPosX                          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Boss Pos X"                   )]->getPointer();
    _ninjaInvincibilityTimer           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Invincibility Timer"    )]->getPointer();
    _ninjaInvincibilityState           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Invincibility State"    )]->getPointer();
    _ninjaSwordType                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Sword Type"             )]->getPointer();
    _ninjaVerticalCollision            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Vertical Collision"     )]->getPointer();
    _ninjaHorizontalCollision          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ninja Horizontal Collision"   )]->getPointer();
    _orbStateVector                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Orb State Vector"             )]->getPointer();
    _enemyStateVector                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy State Vector"           )]->getPointer();
    _bufferedMovement                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Buffered Movement"            )]->getPointer();
    _levelExitFlag1                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level Exit Flag 1"            )]->getPointer();
    _levelExitFlag2                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level Exit Flag 2"            )]->getPointer();

    _weapon1Active                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon 1 Active"              )]->getPointer();
    _weapon2Active                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon 2 Active"              )]->getPointer();

    registerGameProperty("Prev Ninja Power"          , &_prevNinjaPower, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ninja Pos X"               , &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little); 
    registerGameProperty("Ninja Pos Y"               , &_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little); 

    stateUpdatePostHook();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");

    _inputU  = _emulator->registerInput("|..|U.......|");
    _inputD  = _emulator->registerInput("|..|.D......|");
    _inputL  = _emulator->registerInput("|..|..L.....|");
    _inputR  = _emulator->registerInput("|..|...R....|");
    _inputA  = _emulator->registerInput("|..|.......A|");
    _inputB  = _emulator->registerInput("|..|......B.|");
    _inputUL = _emulator->registerInput("|..|U.L.....|");
    _inputUR = _emulator->registerInput("|..|U..R....|");
    _inputUA = _emulator->registerInput("|..|U......A|");
    _inputUB = _emulator->registerInput("|..|U.....B.|");
    _inputDL = _emulator->registerInput("|..|.DL.....|");
    _inputDR = _emulator->registerInput("|..|.D.R....|");
    _inputDA = _emulator->registerInput("|..|.D.....A|");
    _inputDB = _emulator->registerInput("|..|.D....B.|");
    _inputAL = _emulator->registerInput("|..|..L....A|");
    _inputBL = _emulator->registerInput("|..|..L...B.|");
    _inputAR = _emulator->registerInput("|..|...R...A|");
    _inputBR = _emulator->registerInput("|..|...R..B.|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _prevNinjaPower = *_ninjaPower;

    _emulator->advanceState(input);
    
    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // uint8_t _saveBuffer[1024*1024];
    // jaffarCommon::serializer::Contiguous s(_saveBuffer);
    // _emulator->serializeState(s);
    // _emulator->advanceState(_nullInputIdx);

    hashEngine.Update(*_gameMode                 );
    hashEngine.Update(*_currentStage             );
    hashEngine.Update(*_gameTransition           );
    hashEngine.Update(*_ninjaAnimation1          );
    hashEngine.Update(*_ninjaAnimation2          );
    hashEngine.Update(*_ninjaAction              );
    hashEngine.Update(*_ninjaFrame               );
    hashEngine.Update(*_ninjaWeapon              );
    hashEngine.Update(*_ninjaPower               );
    hashEngine.Update(*_ninjaPowerMax            );
    hashEngine.Update(*_ninjaHP                  );
    hashEngine.Update(*_ninjaPosX1               );
    hashEngine.Update(*_ninjaPosX2               );
    hashEngine.Update(*_ninjaSpeedX1             );
    hashEngine.Update(*_ninjaSpeedX2             );
    hashEngine.Update(*_ninjaPosY1               );
    hashEngine.Update(*_ninjaPosY2               );
    hashEngine.Update(*_ninjaSpeedY1             );
    hashEngine.Update(*_ninjaSpeedY2             );
    hashEngine.Update(*_ninjaDirection           );
    hashEngine.Update(*_ninjaMomentum            );
    hashEngine.Update(*_screenPosX1              );
    hashEngine.Update(*_screenPosX2              );
    hashEngine.Update(*_screenPosX3              );
    hashEngine.Update(*_screenPosY1              );
    hashEngine.Update(*_screenPosY2              );
    hashEngine.Update(*_screenPosY3              );
    hashEngine.Update(*_bossHP                   );
    hashEngine.Update(*_bossPosY                 );
    hashEngine.Update(*_bossPosX                 );
  //  hashEngine.Update(*_ninjaInvincibilityTimer  );
    hashEngine.Update(*_ninjaInvincibilityState  );
    hashEngine.Update(*_ninjaSwordType           );
    hashEngine.Update(*_ninjaVerticalCollision   );
    hashEngine.Update(*_ninjaHorizontalCollision );
    hashEngine.Update(*_levelExitFlag1);
    hashEngine.Update(*_levelExitFlag2);
    hashEngine.Update(*_bufferedMovement);
    hashEngine.Update(_orbStateVector, ORB_COUNT);
    hashEngine.Update(_enemyStateVector, ENEMY_COUNT);

    hashEngine.Update(_prevNinjaPower               );
    hashEngine.Update(_weapon1Active);
    hashEngine.Update(_weapon2Active);

    // Animation Array
    hashEngine.Update(&_lowMem[0x0060], 0x20);
    hashEngine.Update(&_lowMem[0x0080], 0x08);

    // Ninja State
     hashEngine.Update(&_lowMem[0x00F9], 0x00FF - 0x00F9);
//    hashEngine.Update(&_lowMem[0x0400], 0x200);

    // jaffarCommon::deserializer::Contiguous d(_saveBuffer);
    // _emulator->deserializeState(d);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    // If game mode is 40, then it's a vertical level
    if (*_gameMode == 40)
    {
      _screenPosX = 0;
      _screenPosY = (float)*_screenPosY1 * 256.0 + (float)*_screenPosY2 + (float)*_screenPosY3 / 256.0;

      // Only update if not visible (ninja frame == 15) because it affects position
      if (*_ninjaFrame != 15)  _playerPosX = _screenPosX + (float)*_ninjaPosX1 + (float)*_ninjaPosX2 / 256.0;
      _playerPosY = _screenPosY + (float)*_ninjaPosY1 + (float)*_ninjaPosY2 / 256.0;
    }

    // If game mode is 42, then it's a horizontal level
    if (*_gameMode == 42)
    {
      _screenPosX = (float)*_screenPosX1 * 256.0 + (float)*_screenPosX2 + (float)*_screenPosX3 / 256.0;
      _screenPosY = 0;

      // Only update if not visible (ninja frame == 15) because it affects position
      if (*_ninjaFrame != 15) _playerPosX = _screenPosX + (float)*_ninjaPosX1 + (float)*_ninjaPosX2 / 256.0;
      _playerPosY = _screenPosY + (float)*_ninjaPosY1 + (float)*_ninjaPosY2 / 256.0;
    }

    _ninjaBossDistance = std::abs((float)*_ninjaPosX1 - (float)*_bossPosX) + std::abs((float)*_ninjaPosY1 - (float)*_bossPosY);
  }

  __INLINE__ void ruleUpdatePreHook() override 
  {
    _pointMagnet.intensity = 0.0; 
    _pointMagnet.x = 0.0; 
    _pointMagnet.y = 0.0; 

    _ninjaPowerMagnet.intensity = 0.0;
    _ninjaPowerMagnet.min = 0;
    _ninjaPowerMagnet.max = 0;
    _ninjaPowerMagnet.center = 0;

    _ninjaHPMagnet.intensity = 0.0;
    _ninjaHPMagnet.min = 0;
    _ninjaHPMagnet.max = 0;
    _ninjaHPMagnet.center = 0;

    _bossHPMagnet.intensity = 0.0;
    _bossHPMagnet.min = 0;
    _bossHPMagnet.max = 0;
    _bossHPMagnet.center = 0;

    _ninjaWeaponMagnet.intensity = 0.0;
    _ninjaWeaponMagnet.value = 0;

    _ninjaBossDistanceMagnet = 0.0;

    _traceMagnet.intensityX = 0.0;
    _traceMagnet.intensityY = 0.0;
    _traceMagnet.offset = 0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _playerDistanceToPointX = std::abs(_pointMagnet.x - _playerPosX);
    _playerDistanceToPointY = std::abs(_pointMagnet.y - _playerPosY);
    _playerDistanceToPoint  =  sqrtf(_playerDistanceToPointX * _playerDistanceToPointX + _playerDistanceToPointY * _playerDistanceToPointY);

     // Updating distance to user-defined point
    _screenDistanceToPointX = std::abs(_screenMagnet.x - _screenPosX);
    _screenDistanceToPointY = std::abs(_screenMagnet.y - _screenPosY);
    _screenDistanceToPoint  =  sqrtf(_screenDistanceToPointX * _screenDistanceToPointX + _screenDistanceToPointY * _screenDistanceToPointY);

    // Updating trace stuff
    if (_useTrace == true)
    {
      _traceStep = (uint16_t) std::max(std::min( (int)_currentStep + _traceMagnet.offset, (int) _trace.size() - 1), 0);
      _traceTargetX = _trace[_traceStep].x;
      _traceTargetY = _trace[_traceStep].y;

      // Updating distance to trace point
      _traceDistanceX = std::abs(_traceTargetX - _playerPosX);
      _traceDistanceY = std::abs(_traceTargetY - _playerPosY);
      _traceDistance  = sqrtf(_traceDistanceX * _traceDistanceX + _traceDistanceY * _traceDistanceY);
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_prevNinjaPower, sizeof(_prevNinjaPower));
    serializer.pushContiguous(&_playerPosX, sizeof(_playerPosX));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_prevNinjaPower, sizeof(_prevNinjaPower));
    deserializer.popContiguous(&_playerPosX, sizeof(_playerPosX));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // If trace is used, compute its magnet's effect
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityX * _traceDistanceX;
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityY * _traceDistanceY;

    // Evaluating point magnet
    reward += -1.0 * _pointMagnet.intensity * _playerDistanceToPoint;

    // Evaluating ninja power magnet
    {
      float boundedValue = (float)*_ninjaPower;
      boundedValue = std::min(boundedValue, _ninjaPowerMagnet.max);
      boundedValue = std::max(boundedValue, _ninjaPowerMagnet.min);
      float diff = std::abs(_ninjaPowerMagnet.center - boundedValue);
      reward += _ninjaPowerMagnet.intensity * -diff;
    }

    // Evaluating ninja HP magnet
    {
      float boundedValue = (float)*_ninjaHP;
      boundedValue = std::min(boundedValue, _ninjaHPMagnet.max);
      boundedValue = std::max(boundedValue, _ninjaHPMagnet.min);
      float diff = std::abs(_ninjaHPMagnet.center - boundedValue);
      reward += _ninjaHPMagnet.intensity * -diff;
    }

    // Evaluating boss HP magnet
    {
      float boundedValue = (float)*_bossHP;
      boundedValue = std::min(boundedValue, _bossHPMagnet.max);
      boundedValue = std::max(boundedValue, _bossHPMagnet.min);
      float diff = std::abs(_bossHPMagnet.center - boundedValue);
      reward += _bossHPMagnet.intensity * -diff;
    }

    // Evaluating ninja's weapon magnet
    if (_ninjaWeaponMagnet.value == *_ninjaWeapon) reward += _ninjaWeaponMagnet.intensity;

    // Evaluating ninja/boss distance magnet
    reward += _ninjaBossDistanceMagnet * _ninjaBossDistance;

    // Returning reward
    return reward;
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    if (_enableAttack)
    {
      if (*_ninjaAction == 0x0000) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputBR, _inputAL, _inputBL, _inputDA, _inputDB, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0001) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputAR, _inputBR, _inputAL, _inputBL, _inputDA, _inputDB, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0002) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputAR, _inputAL, _inputDA, _inputDB, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0003) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputBR, _inputAL, _inputBL, _inputDA, _inputDB, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0004) allowedInputSet.insert(allowedInputSet.end(), { _inputB, _inputD, _inputU, _inputAR, _inputBR, _inputAL, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0005) allowedInputSet.insert(allowedInputSet.end(), { _inputR, _inputL, _inputB, _inputD, _inputU, _inputAR, _inputBR, _inputAL });
      if (*_ninjaAction == 0x0006) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputBR, _inputAL, _inputBL, _inputDA, _inputDB, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0007) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputBR, _inputAL, _inputBL, _inputDA, _inputDB, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0008) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputAR, _inputBR, _inputAL, _inputBL, _inputDA, _inputUA, _inputUB });
      if (*_ninjaAction == 0x0009) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputAR, _inputBR, _inputAL, _inputBL, _inputDA, _inputUA, _inputUB });
      if (*_ninjaAction == 0x000A) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputAR, _inputAL, _inputDB, _inputUA, _inputUB });
      if (*_ninjaAction == 0x000B) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x000C) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x000D) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL });
      if (*_ninjaAction == 0x000E) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL });
      if (*_ninjaAction == 0x000F) allowedInputSet.insert(allowedInputSet.end(), { _inputR, _inputL, _inputAR, _inputAL, _inputUA });
      if (*_ninjaAction == 0x0010) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0012) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputBR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0014) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0004) allowedInputSet.insert(allowedInputSet.end(), { _inputBL, _inputBR });
      if (*_ninjaAction == 0x0005) allowedInputSet.insert(allowedInputSet.end(), { _inputBL, _inputBR });
      if (*_ninjaAction == 0x000A) allowedInputSet.insert(allowedInputSet.end(), { _inputDA });
      if (*_ninjaAction == 0x000C) allowedInputSet.insert(allowedInputSet.end(), { _inputB });
      if (*_ninjaAction == 0x0011) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0012) allowedInputSet.insert(allowedInputSet.end(), { _inputBL, _inputBR });
      if (*_ninjaAction == 0x0013) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputB, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputBL, _inputBR, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0015) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
    }
    else
    {
      if (*_ninjaAction == 0x0000) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0001) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputAR, _inputAL, _inputDA, _inputUA  });
      if (*_ninjaAction == 0x0002) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0003) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0004) allowedInputSet.insert(allowedInputSet.end(), { _inputD, _inputU, _inputAR, _inputAL, _inputUA });
      if (*_ninjaAction == 0x0005) allowedInputSet.insert(allowedInputSet.end(), { _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL });
      if (*_ninjaAction == 0x0006) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA  });
      if (*_ninjaAction == 0x0007) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA  });
      if (*_ninjaAction == 0x0008) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0009) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x000A) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputAR, _inputAL, _inputUA });
      if (*_ninjaAction == 0x000B) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x000C) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x000D) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL });
      if (*_ninjaAction == 0x000E) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL });
      if (*_ninjaAction == 0x000F) allowedInputSet.insert(allowedInputSet.end(), { _inputR, _inputL, _inputAR, _inputAL, _inputUA });
      if (*_ninjaAction == 0x0010) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0012) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0014) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x000A) allowedInputSet.insert(allowedInputSet.end(), { _inputDA });
      if (*_ninjaAction == 0x0011) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0013) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputDA, _inputUA });
      if (*_ninjaAction == 0x0015) allowedInputSet.insert(allowedInputSet.end(), { _inputA, _inputAR, _inputAL, _inputDA, _inputUA });
    }
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]   + Global Timer:                      %02u\n", *_globalTimer);
    jaffarCommon::logger::log("[J+]   + Current Stage:                     %02u\n", *_currentStage);
    jaffarCommon::logger::log("[J+]   + Game Mode:                         %02u\n", *_gameMode);
    jaffarCommon::logger::log("[J+]   + Game Transition:                   %02u\n", *_gameTransition);
    jaffarCommon::logger::log("[J+]   + Ninja Animation                    0x%02X / 0x%02X\n", *_ninjaAnimation1, *_ninjaAnimation2);
    jaffarCommon::logger::log("[J+]   + Ninja Action                       0x%02X\n", *_ninjaAction);
    jaffarCommon::logger::log("[J+]   + Ninja Frame:                       %02u\n", *_ninjaFrame);
    jaffarCommon::logger::log("[J+]   + Ninja Weapon:                      %02u\n", *_ninjaWeapon);
    jaffarCommon::logger::log("[J+]   + Ninja Sword Type:                  %02u\n", *_ninjaSwordType);
    jaffarCommon::logger::log("[J+]   + Ninja Power:                       %02u (Prev: %02u, Max: %02u)\n", *_ninjaPower, _prevNinjaPower, *_ninjaPowerMax);
    jaffarCommon::logger::log("[J+]   + Ninja HP:                          %02u\n", *_ninjaHP);
    jaffarCommon::logger::log("[J+]   + Ninja Position X:                  %.3f: %02u + %02u\n", _playerPosX, *_ninjaPosX1, *_ninjaPosX2);
    jaffarCommon::logger::log("[J+]   + Ninja Position Y:                  %.3f: %02u + %02u\n", _playerPosY, *_ninjaPosY1, *_ninjaPosY2);
    jaffarCommon::logger::log("[J+]   + Ninja Speed X:                     %02d + %02u\n", *_ninjaSpeedX1, *_ninjaSpeedX2);
    jaffarCommon::logger::log("[J+]   + Ninja Speed Y:                     %02d + %02u\n", *_ninjaSpeedY1, *_ninjaSpeedY2);
    jaffarCommon::logger::log("[J+]   + Weapon Active:                     %02u / %02u\n", *_weapon1Active, *_weapon2Active);
    jaffarCommon::logger::log("[J+]   + Screen Pos X:                      %.3f: %02u + %02u + %02u\n", _screenPosX, *_screenPosX1, *_screenPosX2, *_screenPosX3);
    jaffarCommon::logger::log("[J+]   + Screen Pos Y:                      %.3f: %02u + %02u + %02u\n", _screenPosY, *_screenPosY1, *_screenPosY2, *_screenPosY3);
    jaffarCommon::logger::log("[J+]   + Ninja Invincibility:               %02u %02u\n", *_ninjaInvincibilityState, *_ninjaInvincibilityTimer);
    jaffarCommon::logger::log("[J+]   + Boss HP:                           %02u\n", *_bossHP);
    jaffarCommon::logger::log("[J+]   + Boss Pos X:                        %02u\n", *_bossPosX);
    jaffarCommon::logger::log("[J+]   + Boss Pos Y:                        %02u\n", *_bossPosY);
    jaffarCommon::logger::log("[J+]   + Ninja/Boss Distance:               %.3f\n", _ninjaBossDistance);

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _playerDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _playerDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _playerDistanceToPoint);
    }

      if (std::abs(_screenMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Screen Magnet                            Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _screenMagnet.intensity, _screenMagnet.x, _screenMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _screenDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _screenDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _screenDistanceToPoint);
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

      if (std::abs(_ninjaPowerMagnet.intensity) > 0.0f)          jaffarCommon::logger::log("[J+]  + Ninja Power Magnet             - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", _ninjaPowerMagnet.intensity, _ninjaPowerMagnet.center, _ninjaPowerMagnet.min, _ninjaPowerMagnet.max);
      if (std::abs(_ninjaHPMagnet.intensity) > 0.0f)             jaffarCommon::logger::log("[J+]  + Ninja HP Magnet                - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", _ninjaHPMagnet.intensity, _ninjaHPMagnet.center, _ninjaHPMagnet.min, _ninjaHPMagnet.max);
      if (std::abs(_bossHPMagnet.intensity) > 0.0f)              jaffarCommon::logger::log("[J+]  + Boss HP Magnet                 - Intensity: %.5f, Center: %3.3f, Min: %3.3f, Max: %3.3f\n", _bossHPMagnet.intensity, _bossHPMagnet.center, _bossHPMagnet.min, _bossHPMagnet.max);
      if (std::abs(_ninjaBossDistanceMagnet) > 0.0f)             jaffarCommon::logger::log("[J+]  + Ninja/Boss Distance Magnet     - Intensity: %.5f\n", _ninjaBossDistanceMagnet);
      if (std::abs(_ninjaWeaponMagnet.intensity) > 0.0f)         jaffarCommon::logger::log("[J+]  + Ninja Weapon Magnet            - Intensity: %.5f, Weapon: %u\n", _ninjaWeaponMagnet.intensity, _ninjaWeaponMagnet.value);
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
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x       = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y       = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Screen Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x       = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y       = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_screenMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Ninja Power Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto min       = jaffarCommon::json::getNumber<float>(actionJs, "Min");
      auto max       = jaffarCommon::json::getNumber<float>(actionJs, "Max");
      auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
      rule.addAction([=, this]() { this->_ninjaPowerMagnet = boundMagnet_t { .intensity = intensity, .min = min, .max = max, .center = center }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Ninja HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto min       = jaffarCommon::json::getNumber<float>(actionJs, "Min");
      auto max       = jaffarCommon::json::getNumber<float>(actionJs, "Max");
      auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
      rule.addAction([=, this]() { this->_ninjaHPMagnet = boundMagnet_t { .intensity = intensity, .min = min, .max = max, .center = center }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Ninja Weapon Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto value     = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Value");
      rule.addAction([=, this]() { this->_ninjaWeaponMagnet = weaponMagnet_t { .intensity = intensity, .value = value }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Boss HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto min       = jaffarCommon::json::getNumber<float>(actionJs, "Min");
      auto max       = jaffarCommon::json::getNumber<float>(actionJs, "Max");
      auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
      rule.addAction([=, this]() { this->_bossHPMagnet = boundMagnet_t { .intensity = intensity, .min = min, .max = max, .center = center }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Ninja/Boss Distance Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_ninjaBossDistanceMagnet = intensity; });
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


  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the point of attraction X
    float y         = 0.0; // What is the point of attraction X
  };
  pointMagnet_t _pointMagnet;
  pointMagnet_t _screenMagnet;

  float _playerDistanceToPointX;
  float _playerDistanceToPointY;
  float _playerDistanceToPoint;

  float _screenDistanceToPointX;
  float _screenDistanceToPointY;
  float _screenDistanceToPoint;

  struct boundMagnet_t {
  float intensity = 0.0; // How strong the magnet is
  float min = 0.0;  // What is the minimum input value to the calculation.
  float max = 0.0;  // What is the maximum input value to the calculation.
    float center = 0.0;  // What is the central point of attraction
  };

  // Datatype to describe a magnet
  struct weaponMagnet_t {
    float intensity = 0.0; // How strong the magnet is
    uint8_t value = 0;  // Specifies the weapon number
  };
  boundMagnet_t  _ninjaPowerMagnet;
  boundMagnet_t  _ninjaHPMagnet;
  boundMagnet_t  _bossHPMagnet;
  weaponMagnet_t _ninjaWeaponMagnet;
  float _ninjaBossDistanceMagnet;

  bool _enableAttack;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;

  uint8_t* _lowMem;

  float _playerPosX;
  float _playerPosY;
  float _screenPosX;
  float _screenPosY;
  float _ninjaBossDistance;

  struct traceMagnet_t
  {
    float intensityX = 0.0; // How strong the magnet is on X
    float intensityY = 0.0; // How strong the magnet is on Y
    int offset      = 0; // Which entry (step) to look at wrt the current emulation step
  };

  traceMagnet_t _traceMagnet;

  // Whether we use a trace
  bool _useTrace = false;

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
  
  // Possible inputs
  InputSet::inputIndex_t _inputU ;
  InputSet::inputIndex_t _inputD ;
  InputSet::inputIndex_t _inputL ;
  InputSet::inputIndex_t _inputR ;
  InputSet::inputIndex_t _inputA ;
  InputSet::inputIndex_t _inputB ;
  InputSet::inputIndex_t _inputUL;
  InputSet::inputIndex_t _inputUR;
  InputSet::inputIndex_t _inputUA;
  InputSet::inputIndex_t _inputUB;
  InputSet::inputIndex_t _inputDL;
  InputSet::inputIndex_t _inputDR;
  InputSet::inputIndex_t _inputDA;
  InputSet::inputIndex_t _inputDB;
  InputSet::inputIndex_t _inputAL;
  InputSet::inputIndex_t _inputBL;
  InputSet::inputIndex_t _inputAR;
  InputSet::inputIndex_t _inputBR;

  // Game values

  uint8_t* _gameMode              ;
  uint8_t* _currentStage            ;
  uint8_t* _globalTimer             ;
  uint8_t* _gameTransition;
  uint8_t* _ninjaAnimation1         ;
  uint8_t* _ninjaAnimation2         ;
  uint8_t* _ninjaAction             ;
  uint8_t* _ninjaFrame              ;
  uint8_t* _ninjaWeapon             ;
  uint8_t* _ninjaPower              ;
  uint8_t* _ninjaPowerMax           ;
  uint8_t* _ninjaHP                 ;
  uint8_t* _ninjaPosX1              ;
  uint8_t* _ninjaPosX2              ;
  int8_t*  _ninjaSpeedX1            ;
  uint8_t* _ninjaSpeedX2            ;
  uint8_t* _ninjaPosY1              ;
  uint8_t* _ninjaPosY2              ;
  int8_t*  _ninjaSpeedY1            ;
  uint8_t* _ninjaSpeedY2            ;
  uint8_t* _ninjaDirection          ;
  uint8_t* _ninjaMomentum;
  uint8_t* _screenPosX1             ;
  uint8_t* _screenPosX2             ;
  uint8_t* _screenPosX3             ;
  uint8_t* _screenPosY1             ;
  uint8_t* _screenPosY2             ;
  uint8_t* _screenPosY3             ;
  uint8_t* _bossHP                  ;
  uint8_t* _bossPosY                ;
  uint8_t* _bossPosX                ;
  uint8_t* _ninjaInvincibilityTimer ;
  uint8_t* _ninjaInvincibilityState ;
  uint8_t* _ninjaSwordType          ;
  uint8_t* _ninjaVerticalCollision  ;
  uint8_t* _ninjaHorizontalCollision;
  uint8_t* _orbStateVector          ;
  uint8_t* _enemyStateVector        ;
  uint8_t* _bufferedMovement        ;
  uint8_t* _levelExitFlag1          ;
  uint8_t* _levelExitFlag2          ;
  uint8_t* _weapon1Active;
  uint8_t* _weapon2Active;
  
  uint8_t _prevNinjaPower              ;

  uint16_t _currentStep;
  uint16_t _lastInputStep;
};


} // namespace nes

} // namespace games

} // namespace jaffarPlus
