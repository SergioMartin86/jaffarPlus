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

class StreetFighter2010 final : public jaffarPlus::Game
{
public:

  static __INLINE__ std::string getName() { return "NES / Street Fighter 2010"; }

  StreetFighter2010(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    registerGameProperty("Global Timer"           ,&_lowMem[0x00A5], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("RNG Value"              ,&_lowMem[0x00B0], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Lag Frame"              ,&_lowMem[0x00A4], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Lag Type"               ,&_lowMem[0x00A1], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Action"          ,&_lowMem[0x0400], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Direction"       ,&_lowMem[0x0440], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player HP"              ,&_lowMem[0x00B1], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos X1"          ,&_lowMem[0x04E0], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos X2"          ,&_lowMem[0x04C0], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos X3"          ,&_lowMem[0x04C1], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos Y1"          ,&_lowMem[0x0540], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos Y2"          ,&_lowMem[0x0520], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos Y3"          ,&_lowMem[0x0521], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss01 X"               ,&_lowMem[0x04DD], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss01 Y"               ,&_lowMem[0x053D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss02 X1"              ,&_lowMem[0x04FF], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss02 Y1"              ,&_lowMem[0x055F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss02 X2"              ,&_lowMem[0x04DF], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss02 Y2"              ,&_lowMem[0x053F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss HP 1"              ,&_lowMem[0x05FF], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss HP 2"              ,&_lowMem[0x05F4], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss Timer"             ,&_lowMem[0x065F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Open Level"             ,&_lowMem[0x007D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Exit Level"             ,&_lowMem[0x0085], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Portal Timer"           ,&_lowMem[0x05E9], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 03 State"        ,&_lowMem[0x0661], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 02 State"        ,&_lowMem[0x0662], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 01 State"        ,&_lowMem[0x0663], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 00 State"        ,&_lowMem[0x0664], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 03 Direction 01" ,&_lowMem[0x0424], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 02 Direction 01" ,&_lowMem[0x0425], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 01 Direction 01" ,&_lowMem[0x0426], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 00 Direction 01" ,&_lowMem[0x0427], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 03 Direction 02" ,&_lowMem[0x0441], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 02 Direction 02" ,&_lowMem[0x0442], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 01 Direction 02" ,&_lowMem[0x0443], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Bullet 00 Direction 02" ,&_lowMem[0x0444], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    _globalTimer        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Global Timer"    )]->getPointer();
    _RNGValue           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("RNG Value"       )]->getPointer();
    _lagFrame           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Lag Frame"       )]->getPointer();
    _lagType            =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Lag Type"        )]->getPointer();
    _playerAction       =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Action"   )]->getPointer();
    _playerDirection    =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Direction")]->getPointer();
    _playerHP           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player HP"       )]->getPointer();
    _playerPosX1        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X1"   )]->getPointer();
    _playerPosX2        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X2"   )]->getPointer();
    _playerPosX3        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X3"   )]->getPointer();
    _playerPosY1        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y1"   )]->getPointer();
    _playerPosY2        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y2"   )]->getPointer();
    _playerPosY3        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y3"   )]->getPointer();
    _boss01X            =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss01 X"        )]->getPointer();
    _boss01Y            =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss01 Y"        )]->getPointer();
    _boss02X1           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss02 X1"       )]->getPointer();
    _boss02Y1           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss02 Y1"       )]->getPointer();
    _boss02X2           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss02 X2"       )]->getPointer();
    _boss02Y2           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss02 Y2"       )]->getPointer();
    _bossHP1            =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss HP 1"       )]->getPointer();
    _bossHP2            =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss HP 2"       )]->getPointer();
    _bossTimer          =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Timer"      )]->getPointer();
    _openLevel          =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Open Level"      )]->getPointer();
    _portalTimer        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Portal Timer"    )]->getPointer();

    _bullet03State      =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 03 State" )]->getPointer();
    _bullet02State      =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 02 State" )]->getPointer();
    _bullet01State      =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 01 State" )]->getPointer();
    _bullet00State      =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 00 State" )]->getPointer();
    _bullet03Direction01  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 03 Direction 01" )]->getPointer();
    _bullet02Direction01  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 02 Direction 01" )]->getPointer();
    _bullet01Direction01  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 01 Direction 01" )]->getPointer();
    _bullet00Direction01  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 00 Direction 01" )]->getPointer();
    _bullet03Direction02  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 03 Direction 02" )]->getPointer();
    _bullet02Direction02  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 02 Direction 02" )]->getPointer();
    _bullet01Direction02  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 01 Direction 02" )]->getPointer();
    _bullet00Direction02  =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bullet 00 Direction 02" )]->getPointer();

    registerGameProperty("Boss 02 Pos X"      ,&_boss02PosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Boss 02 Pos Y"      ,&_boss02PosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos X"       ,&_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Y"       ,&_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Boss HP"            ,&_bossHP, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    _currentStep = 0;
    _nullInputIdx      = _emulator->registerInput("|..|........|");

    _input_U    = _emulator->registerInput("|..|U.......|");
    _input_D    = _emulator->registerInput("|..|.D......|");
    _input_L    = _emulator->registerInput("|..|..L.....|");
    _input_R    = _emulator->registerInput("|..|...R....|");
    _input_A    = _emulator->registerInput("|..|.......A|");
    _input_B    = _emulator->registerInput("|..|......B.|");
    _input_UL   = _emulator->registerInput("|..|U.L.....|");
    _input_UR   = _emulator->registerInput("|..|U..R....|");
    _input_UA   = _emulator->registerInput("|..|U......A|");
    _input_UB   = _emulator->registerInput("|..|U.....B.|");
    _input_UD   = _emulator->registerInput("|..|UD......|");
    _input_DL   = _emulator->registerInput("|..|.DL.....|");
    _input_DR   = _emulator->registerInput("|..|.D.R....|");
    _input_DA   = _emulator->registerInput("|..|.D.....A|");
    _input_DB   = _emulator->registerInput("|..|.D....B.|");
    _input_AL   = _emulator->registerInput("|..|..L....A|");
    _input_BL   = _emulator->registerInput("|..|..L...B.|");
    _input_AR   = _emulator->registerInput("|..|...R...A|");
    _input_BR   = _emulator->registerInput("|..|...R..B.|");
    _input_BA   = _emulator->registerInput("|..|......BA|");
    _input_ULA  = _emulator->registerInput("|..|U.L....A|");
    _input_URA  = _emulator->registerInput("|..|U..R...A|");
    _input_UBA  = _emulator->registerInput("|..|U.....BA|");
    _input_DLA  = _emulator->registerInput("|..|.DL....A|");
    _input_DRA  = _emulator->registerInput("|..|.D.R...A|");
    _input_DBA  = _emulator->registerInput("|..|.D....BA|");
    _input_BLA  = _emulator->registerInput("|..|..L...BA|");
    _input_BRA  = _emulator->registerInput("|..|...R..BA|");
    _input_ULB  = _emulator->registerInput("|..|U.L...B.|");
    _input_URB  = _emulator->registerInput("|..|U..R..B.|");
    _input_UDB  = _emulator->registerInput("|..|UD....B.|");
    _input_DLB  = _emulator->registerInput("|..|.DL...B.|");
    _input_DRB  = _emulator->registerInput("|..|.D.R..B.|");
    _input_UBA  = _emulator->registerInput("|..|U.....BA|");
    _input_LRA  = _emulator->registerInput("|..|..LR...A|");
    _input_RBA  = _emulator->registerInput("|..|...R..BA|");
    _input_LBA  = _emulator->registerInput("|..|..L...BA|");
    _input_LR   = _emulator->registerInput("|..|..LR....|");
    _input_DRBA = _emulator->registerInput("|..|.D.R..BA|");
    _input_DLBA = _emulator->registerInput("|..|.DL...BA|");

    _lastInput = _nullInputIdx;
    _allowFire = true;
    _bossDeathTimer = 0;

    stateUpdatePostHook();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;
    if (*_lagFrame == 0 && *_bossHP2 == 255) _bossDeathTimer++;

    _lastInput = input;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    //  hashEngine.Update(*_RNGValue    );
      hashEngine.Update(*_lagFrame    );
      hashEngine.Update(*_lagType     );
      hashEngine.Update(*_playerHP    );
      hashEngine.Update(*_playerPosX1 );
      hashEngine.Update(*_playerPosX2 );
      hashEngine.Update(*_playerPosX3 );
      hashEngine.Update(*_playerPosY1 );
      hashEngine.Update(*_playerPosY2 );
      hashEngine.Update(*_playerPosY3 );
      hashEngine.Update(*_boss01X       );
      hashEngine.Update(*_boss01Y       );
      hashEngine.Update(*_boss02X1       );
      hashEngine.Update(*_boss02Y1       );
      hashEngine.Update(*_boss02X2       );
      hashEngine.Update(*_boss02Y2       );
      hashEngine.Update(*_bossHP1      );
      hashEngine.Update(*_bossHP2      );
      // hashEngine.Update(*_bossTimer);
      hashEngine.Update(*_playerAction);

      hashEngine.Update(_bossDeathTimer);

      if (_allowFire == true)
      {
        hashEngine.Update(*_bullet03State);
        hashEngine.Update(*_bullet02State);
        hashEngine.Update(*_bullet01State);
        hashEngine.Update(*_bullet00State);
        
        hashEngine.Update(*_bullet03Direction01);
        hashEngine.Update(*_bullet02Direction01);
        hashEngine.Update(*_bullet01Direction01);
        hashEngine.Update(*_bullet00Direction01);

        hashEngine.Update(*_bullet03Direction02);
        hashEngine.Update(*_bullet02Direction02);
        hashEngine.Update(*_bullet01Direction02);
        hashEngine.Update(*_bullet00Direction02);

        hashEngine.Update(&_lowMem[0x0525], 0x004); // Bullets Pos Y
      }

      if (*_lagFrame == 1) hashEngine.Update(&_lowMem[0x00A0], 0x040);

      // hashEngine.Update(&_lowMem[0x0100], 0x0080);
    //  hashEngine.Update(&_lowMem[0x0260], 0x0090);
    //  hashEngine.Update(&_lowMem[0x0300], 0x0100);
      // hashEngine.Update(&_lowMem[0x0400], 0x0100); // Bullets
      // hashEngine.Update(&_lowMem[0x0600], 0x0080);

  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _playerPosX = (float)*_playerPosX1 * 256.0f + (float)*_playerPosX2 + (float)*_playerPosX3 / 256.0;
    _playerPosY = (float)*_playerPosY1 * 256.0f + (float)*_playerPosY2 + (float)*_playerPosY3 / 256.0;

    _boss02PosX = (float)*_boss02X1 * 256.0f + (float)*_boss02X2;
    _boss02PosY = (float)*_boss02Y1 * 256.0f + (float)*_boss02Y2;

    _bossHP = *_bossHP1 + *_bossHP2;

    _playerDistanceToBossX = std::abs(_playerPosX - _boss02PosX);
    _playerDistanceToBossY = std::abs(_playerPosY - _boss02PosY);
    _playerDistanceToBoss = sqrt(_playerDistanceToBossX * _playerDistanceToBossX + _playerDistanceToBossY * _playerDistanceToBossY);
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
      _playerPosXMagnet.intensity = 0.0f;
      _playerPosYMagnet.intensity = 0.0f;
      _playerPosXMagnet.pos = 0.0f;
      _playerPosYMagnet.pos = 0.0f;
      _bossHPMagnet = 0.0f;
      _playerDistanceToBossMagnet = 0.0f;
      _allowFire = true;
      _bossPosXMagnet.intensity = 0.0f;
      _bossPosXMagnet.pos = 0.0f;
      _bossDeathTimerMagnet = 0.0f;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    _bossDistanceToPointX = std::abs((float)_bossPosXMagnet.pos - _boss02PosX);
    _playerDistanceToPointX = std::abs((float)_playerPosXMagnet.pos - _playerPosX);
    _playerDistanceToPointY = std::abs((float)_playerPosYMagnet.pos - _playerPosY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_lastInput, sizeof(_lastInput));
     serializer.push(&_allowFire, sizeof(_allowFire));
     serializer.push(&_bossDeathTimer, sizeof(_bossDeathTimer));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_lastInput, sizeof(_lastInput));
     deserializer.pop(&_allowFire, sizeof(_allowFire));
     deserializer.pop(&_bossDeathTimer, sizeof(_bossDeathTimer));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Evaluating point magnet
    reward += -1.0 * _bossPosXMagnet.intensity * _bossDistanceToPointX;
    reward += -1.0 * _playerPosXMagnet.intensity * _playerDistanceToPointX;
    reward += -1.0 * _playerPosYMagnet.intensity * _playerDistanceToPointY;

    reward += _bossHPMagnet * (float)_bossHP;
    reward += -1.0 * _playerDistanceToBossMagnet * _playerDistanceToBoss;

    reward += _bossDeathTimerMagnet * (float)_bossDeathTimer;

    // Returning reward
    return reward;
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    switch(*_playerAction)
    {
      case 0x0000: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_A, _input_B, _input_R, _input_L, _input_BA, _input_AR, _input_AL, _input_DA, _input_DB, _input_UB, _input_U }); break;
      case 0x0001: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_A, _input_B, _input_R, _input_L, _input_AR, _input_BR, _input_AL, _input_BL, _input_DB, _input_UB }); break;
      case 0x0002: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_A, _input_B, _input_R, _input_L, _input_AR, _input_BR, _input_AL, _input_BL, _input_DA, _input_DB, _input_UB }); break;
      case 0x0003: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_R, _input_A, _input_L, _input_D, _input_BA, _input_AR, _input_BR, _input_AL, _input_BL, _input_UB, _input_DA }); break;
      case 0x0004: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_U, _input_D, _input_L, _input_R, _input_A, _input_BR, _input_AR, _input_AL, _input_BL, _input_DA, _input_DB, _input_UB, _input_UD }); break;
      case 0x0005: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_U, _input_D, _input_L, _input_R, _input_A, _input_BR, _input_AR, _input_AL, _input_BL, _input_DA, _input_DB, _input_UB, _input_UD }); break;
      case 0x0006: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_U, _input_D, _input_L, _input_A, _input_R, _input_BR, _input_AR, _input_AL, _input_BL, _input_UD, _input_BA, _input_DA, _input_DB, _input_UB }); break;
      case 0x0007: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_U, _input_D, _input_L, _input_A, _input_R, _input_BR, _input_AR, _input_AL, _input_BL, _input_UD, _input_BA, _input_DA, _input_DB, _input_UB }); break;
      case 0x0009: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_R, _input_A, _input_L, _input_U, _input_BA, _input_AR, _input_BR, _input_AL, _input_BL, _input_UA, _input_UB, _input_DA}); break;
      case 0x000A: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_U, _input_D, _input_L, _input_A, _input_R, _input_BR, _input_AR, _input_AL, _input_BL, _input_UD, _input_BA, _input_DA, _input_DB, _input_UB }); break;
      case 0x0010: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_R, _input_L, _input_BR, _input_BL, _input_DB, _input_AR, _input_AL, _input_UL, _input_UR }); break;
      case 0x0011: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_R, _input_L, _input_AR, _input_AL, _input_UL, _input_UR }); break;
      case 0x0012: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_R, _input_L, _input_AR, _input_AL, _input_UL, _input_UR }); break;
      case 0x0013: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_B, _input_R, _input_L, _input_BR, _input_BL }); break;
      case 0x0014: allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx, _input_R, _input_L, _input_AR, _input_AL }); break;
      default: 
            allowedInputSet.insert(allowedInputSet.end(), {
            _nullInputIdx,
            _input_U ,
            _input_D ,
            _input_L ,
            _input_R ,
            _input_A ,
            _input_B ,
            _input_UL,
            _input_UR,
            _input_UA,
            _input_UB,
            _input_UD,
            _input_DL,
            _input_DR,
            _input_DA,
            _input_DB,
            _input_AL,
            _input_BL,
            _input_AR,
            _input_BR
            }); break;
    } 

    if (_allowFire == false && *_playerAction != 0x0A) for (ssize_t i = 0; i < (ssize_t)allowedInputSet.size(); i++) 
    {
      const auto input = allowedInputSet[i];
      if (input == _input_B ||
          input == _input_UB ||
          input == _input_DB ||
          input == _input_BA ||
          input == _input_BL ||
          input == _input_BR ) { allowedInputSet.erase(allowedInputSet.begin() + i); i--; }
    }
  }

    // Function to report what all the possible input that the game might require
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    return {
      "|..|........|",
      "|..|U.......|",
      "|..|.D......|",
      "|..|..L.....|",
      "|..|...R....|",
      "|..|.......A|",
      "|..|......B.|",
      "|..|U.L.....|",
      "|..|U..R....|",
      "|..|U......A|",
      "|..|U.....B.|",
      "|..|UD......|",
      "|..|.DL.....|",
      "|..|.D.R....|",
      "|..|.D.....A|",
      "|..|.D....B.|",
      "|..|..L....A|",
      "|..|..L...B.|",
      "|..|...R...A|",
      "|..|...R..B.|" 
    };
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:                     %04u\n", _currentStep);

    jaffarCommon::logger::log("[J+]  + Global Timer:               %02u\n", *_globalTimer);
    jaffarCommon::logger::log("[J+]  + Lag Frame:                  %02u (Type: %02u)\n", *_lagFrame, *_lagType);
    jaffarCommon::logger::log("[J+]  + Player HP:                  %02u\n", *_playerHP);
    jaffarCommon::logger::log("[J+]  + Player Action:              0x%02X\n", *_playerAction);
    jaffarCommon::logger::log("[J+]  + Player Direction:           %02u\n", *_playerDirection);
    jaffarCommon::logger::log("[J+]  + Player Pos X:               %.2f (%02u %02u %02u)\n", _playerPosX, *_playerPosX1, *_playerPosX2, *_playerPosX3);
    jaffarCommon::logger::log("[J+]  + Player Pos Y:               %.2f (%02u %02u %02u)\n", _playerPosY, *_playerPosY1, *_playerPosY2, *_playerPosY3);
    jaffarCommon::logger::log("[J+]  + Boss02 Pos:                 [ %.3f, %.3f ]\n", _boss02PosX, _boss02PosY);
    jaffarCommon::logger::log("[J+]  + Boss HP:                    %02u (%02u + %02u)\n", _bossHP, *_bossHP1, *_bossHP2);
    jaffarCommon::logger::log("[J+]  + Boss Timer:                 %03u\n", *_bossTimer);
    jaffarCommon::logger::log("[J+]  + Boss Death Timer:           %03u\n", _bossDeathTimer);
    jaffarCommon::logger::log("[J+]  + Open Level:                 %03u\n", *_openLevel);
    jaffarCommon::logger::log("[J+]  + Allow Fire:                 %s\n", _allowFire ? "True" : "False");
    jaffarCommon::logger::log("[J+]  + Portal Timer:               %03u\n", *_portalTimer);
    jaffarCommon::logger::log("[J+]  + Bullet States:              [ %03u, %03u, %03u, %03u ]\n", *_bullet00State, *_bullet01State, *_bullet02State, *_bullet03State);
    jaffarCommon::logger::log("[J+]  + Bullet Directions 01:       [ %03u, %03u, %03u, %03u ]\n", *_bullet00Direction01, *_bullet01Direction01, *_bullet02Direction01, *_bullet03Direction01);
    jaffarCommon::logger::log("[J+]  + Bullet Directions 02:       [ %03u, %03u, %03u, %03u ]\n", *_bullet00Direction02, *_bullet01Direction02, *_bullet02Direction02, *_bullet03Direction02);
    jaffarCommon::logger::log("[J+]  + Player Distance To Boss:    %.3f (X: %.3f, Y: %0.3f)\n", _playerDistanceToBoss, _playerDistanceToBossX, _playerDistanceToBossY);

    if (std::abs(_bossPosXMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Boss Pos X Magnet                   Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _bossPosXMagnet.intensity, _bossPosXMagnet.pos, _bossDistanceToPointX);
    if (std::abs(_playerPosXMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Player Pos X Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _playerPosXMagnet.intensity, _playerPosXMagnet.pos, _playerDistanceToPointX);
    if (std::abs(_playerPosYMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Player Pos Y Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _playerPosYMagnet.intensity, _playerPosYMagnet.pos, _playerDistanceToPointY);
    if (std::abs(_bossHPMagnet) > 0.0f)
       jaffarCommon::logger::log("[J+]  + Boss HP Magnet                      Intensity: %.5f\n", _bossHPMagnet);
    if (std::abs(_bossDeathTimerMagnet) > 0.0f)
       jaffarCommon::logger::log("[J+]  + Boss Death Timer Magnet             Intensity: %.5f\n", _bossDeathTimerMagnet);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Boss Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_bossPosXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }
    
    if (actionType == "Set Player Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_playerPosXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Pos Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_playerPosYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Boss HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bossHPMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Distance To Boss Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_playerDistanceToBossMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Boss Death Timer Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bossDeathTimerMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Allow Fire")
    {
      auto value = jaffarCommon::json::getBoolean(actionJs, "Value");
      rule.addAction([=, this]() { this->_allowFire = value; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t({0, *_playerAction});
  }


  __INLINE__ void playerPrintCommands() const override
  {
    // jaffarCommon::logger::log("[J+] t: Print Initial Info\n");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
     return false;
  };

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float pos       = 0.0; // What is the point of attraction X
  };

  pointMagnet_t _bossPosXMagnet;
  pointMagnet_t _playerPosXMagnet;
  pointMagnet_t _playerPosYMagnet;
  
  float _bossDeathTimerMagnet;
  float _bossHPMagnet;
  float _bossDistanceToPointX;
  float _playerDistanceToPointX;
  float _playerDistanceToPointY;

  uint8_t* _lowMem;
  uint16_t _currentStep;
  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _lastInput;

  // Possible inputs
  InputSet::inputIndex_t _input_U  ;
  InputSet::inputIndex_t _input_D  ;
  InputSet::inputIndex_t _input_L  ;
  InputSet::inputIndex_t _input_R  ;
  InputSet::inputIndex_t _input_A  ;
  InputSet::inputIndex_t _input_B  ;
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
  InputSet::inputIndex_t _input_ULB;
  InputSet::inputIndex_t _input_URB;
  InputSet::inputIndex_t _input_DLB;
  InputSet::inputIndex_t _input_DRB;
  InputSet::inputIndex_t _input_LRA;
  InputSet::inputIndex_t _input_BA;
  InputSet::inputIndex_t _input_RBA;
  InputSet::inputIndex_t _input_LBA;
  InputSet::inputIndex_t _input_LR;
  InputSet::inputIndex_t _input_UDB;
  InputSet::inputIndex_t _input_UD;
  InputSet::inputIndex_t _input_DRBA;
  InputSet::inputIndex_t _input_DLBA;

  uint8_t* _globalTimer      ;
  uint8_t* _RNGValue         ;
  uint8_t* _lagFrame         ;
  uint8_t* _lagType          ;
  uint8_t* _playerAction     ;
  uint8_t* _playerDirection  ;
  uint8_t* _playerHP         ;
  uint8_t* _playerPosX1      ;
  uint8_t* _playerPosX2      ;
  uint8_t* _playerPosX3      ;
  uint8_t* _playerPosY1      ;
  uint8_t* _playerPosY2      ;
  uint8_t* _playerPosY3      ;
  uint8_t* _boss01X            ;
  uint8_t* _boss01Y            ;
  uint8_t* _boss02X1            ;
  uint8_t* _boss02Y1            ;
  uint8_t* _boss02X2            ;
  uint8_t* _boss02Y2            ;
  uint8_t* _bossHP1           ;
  uint8_t* _bossHP2           ;
  uint8_t* _bossTimer        ;
  uint8_t* _openLevel        ;
  uint8_t* _portalTimer;

  uint8_t _bossDeathTimer;

  float _playerPosX;
  float _playerPosY;

  float _boss02PosX;
  float _boss02PosY;

  float _playerDistanceToBossMagnet;
  float _playerDistanceToBossX;
  float _playerDistanceToBossY;
  float _playerDistanceToBoss;

  uint8_t* _bullet03State;
  uint8_t* _bullet02State;
  uint8_t* _bullet01State;
  uint8_t* _bullet00State;

  uint8_t* _bullet03Direction01;
  uint8_t* _bullet02Direction01;
  uint8_t* _bullet01Direction01;
  uint8_t* _bullet00Direction01;

  uint8_t* _bullet03Direction02;
  uint8_t* _bullet02Direction02;
  uint8_t* _bullet01Direction02;
  uint8_t* _bullet00Direction02;

  uint8_t _bossHP;

  bool _allowFire;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
