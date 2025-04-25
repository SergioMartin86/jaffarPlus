#pragma once

#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

#define _ARKANOID2_TILES_PER_ROW 11
#define _ARKANOID2_TILES_PER_COL 17

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class Arkanoid2 final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "NES / Arkanoid2"; }

  Arkanoid2(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Registering input index LUT
    paddlePositionIndexLUT[0] = 255;
    for (size_t i = 1; i <= 142; i++) paddlePositionIndexLUT[i] = _emulator->registerInput(paddlePositionStringLUT[i]);

    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Game State",                  &_lowMem[0x0006], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 State",                &_lowMem[0x0410], Property::datatype_t::dt_uint8, Property::endianness_t::little);
  
    registerGameProperty("Ball 1 Pos X",                &_lowMem[0x022B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Pos Y",                &_lowMem[0x0228], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Pos X",                &_lowMem[0x022F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Pos Y",                &_lowMem[0x022C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Pos X",                &_lowMem[0x0233], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 3 Pos Y",                &_lowMem[0x0230], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Pos X",                &_lowMem[0x0237], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 4 Pos Y",                &_lowMem[0x0234], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Pos X",                &_lowMem[0x023B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 5 Pos Y",                &_lowMem[0x0238], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Pos X",                &_lowMem[0x023F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 6 Pos Y",                &_lowMem[0x023C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Pos X",                &_lowMem[0x0243], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 7 Pos Y",                &_lowMem[0x0240], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 8 Pos X",                &_lowMem[0x0247], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 8 Pos Y",                &_lowMem[0x0244], Property::datatype_t::dt_uint8, Property::endianness_t::little);
  
  
    registerGameProperty("Ball 1 Angle",                &_lowMem[0x04B8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Input1",                      &_lowMem[0x003D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Input2",                      &_lowMem[0x003E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Input3",                      &_lowMem[0x003F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Input (Pot)",                 &_lowMem[0x003B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle Pos X",                &_lowMem[0x0043], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle Pos X1",               &_lowMem[0x0203], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle Pos X2",               &_lowMem[0x0207], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle Pos X3",               &_lowMem[0x020B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle Pos X4",               &_lowMem[0x020F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss State",                  &_lowMem[0x052A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Damage",                 &_lowMem[0x052B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Brain Boss Damage",           &_lowMem[0x050B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Second Boss Death Animation", &_lowMem[0x052C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 1 Exists",        &_lowMem[0x0580], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 2 Exists",        &_lowMem[0x0590], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 3 Exists",        &_lowMem[0x05A0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 4 Exists",        &_lowMem[0x05B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 5 Exists",        &_lowMem[0x05C0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 6 Exists",        &_lowMem[0x05D0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 1 Pos X",         &_lowMem[0x0584], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 2 Pos X",         &_lowMem[0x0594], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 3 Pos X",         &_lowMem[0x05A4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 4 Pos X",         &_lowMem[0x05B4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 5 Pos X",         &_lowMem[0x05C4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 6 Pos X",         &_lowMem[0x05D4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 1 Pos Y",         &_lowMem[0x0583], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 2 Pos Y",         &_lowMem[0x0593], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 3 Pos Y",         &_lowMem[0x05A3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 4 Pos Y",         &_lowMem[0x05B3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 5 Pos Y",         &_lowMem[0x05C3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Attack 6 Pos Y",         &_lowMem[0x05D3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Stage Timer",                 &_lowMem[0x00D7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
     
    registerGameProperty("Remaining Hits",              &_remainingHits,   Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Remaining Blocks",            &_remainingBlocks, Property::datatype_t::dt_uint8, Property::endianness_t::little);
     
    registerGameProperty("Score",                       &_score, Property::datatype_t::dt_uint32, Property::endianness_t::little);
    registerGameProperty("Score x1",                    &_lowMem[0x00FA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x10",                   &_lowMem[0x00F9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x100",                  &_lowMem[0x00F8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x1000",                 &_lowMem[0x00F7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x10000",                &_lowMem[0x00F6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x100000",               &_lowMem[0x00F5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
     
    registerGameProperty("PowerUp Pos Y",               &_lowMem[0x0543], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PowerUp Pos X",               &_lowMem[0x0544], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PowerUp Type",                &_lowMem[0x0541], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PowerUp Falling State",       &_lowMem[0x0540], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PowerUp M Active",            &_lowMem[0x0229], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball Count",                  &_lowMem[0x0402], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball Has Launched",           &_lowMem[0x07F0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle Sprite",               &_lowMem[0x0201], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Laser Timer",                 &_lowMem[0x004E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Laser Position Decision",     &_laserPositionDecision, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Portal State",                &_lowMem[0x0535], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    _gameState                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game State"            )]->getPointer();
    _ball1State                = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 State"          )]->getPointer();
 
    _ball1PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Pos X"          )]->getPointer();
    _ball1PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Pos Y"          )]->getPointer();
    _ball2PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Pos X"          )]->getPointer();
    _ball2PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 2 Pos Y"          )]->getPointer();
    _ball3PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Pos X"          )]->getPointer();
    _ball3PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 3 Pos Y"          )]->getPointer();
    _ball4PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Pos X"          )]->getPointer();
    _ball4PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 4 Pos Y"          )]->getPointer();
    _ball5PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Pos X"          )]->getPointer();
    _ball5PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 5 Pos Y"          )]->getPointer();
    _ball6PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Pos X"          )]->getPointer();
    _ball6PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 6 Pos Y"          )]->getPointer();
    _ball7PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Pos X"          )]->getPointer();
    _ball7PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 7 Pos Y"          )]->getPointer();
    _ball8PosX                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 8 Pos X"          )]->getPointer();
    _ball8PosY                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 8 Pos Y"          )]->getPointer();
 
    _ball1Angle                = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball 1 Angle"          )]->getPointer();
    _paddlePosX                = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Paddle Pos X"          )]->getPointer();
    _bossDamage                = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Damage"           )]->getPointer();
    _brainBossDamage           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Brain Boss Damage"     )]->getPointer();
    _bossAttack1Exists         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 1 Exists"  )]->getPointer();
    _bossAttack2Exists         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 2 Exists"  )]->getPointer();
    _bossAttack3Exists         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 3 Exists"  )]->getPointer();
    _bossAttack4Exists         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 4 Exists"  )]->getPointer();
    _bossAttack5Exists         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 5 Exists"  )]->getPointer();
    _bossAttack6Exists         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 6 Exists"  )]->getPointer();
    _bossAttack1PosX           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 1 Pos X"   )]->getPointer();
    _bossAttack2PosX           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 2 Pos X"   )]->getPointer();
    _bossAttack3PosX           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 3 Pos X"   )]->getPointer();
    _bossAttack4PosX           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 4 Pos X"   )]->getPointer();
    _bossAttack5PosX           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 5 Pos X"   )]->getPointer(); 
    _bossAttack6PosX           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 6 Pos X"   )]->getPointer();
    _bossAttack1PosY           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 1 Pos Y"   )]->getPointer();
    _bossAttack2PosY           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 2 Pos Y"   )]->getPointer();
    _bossAttack3PosY           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 3 Pos Y"   )]->getPointer();
    _bossAttack4PosY           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 4 Pos Y"   )]->getPointer();
    _bossAttack5PosY           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 5 Pos Y"   )]->getPointer();
    _bossAttack6PosY           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Attack 6 Pos Y"   )]->getPointer();
    _score1                    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Score x1"              )]->getPointer();
    _score10                   = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Score x10"             )]->getPointer();
    _score100                  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Score x100"            )]->getPointer();
    _score1000                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Score x1000"           )]->getPointer();
    _score10000                = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Score x10000"          )]->getPointer();
    _score100000               = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Score x100000"         )]->getPointer();
    _powerUpPosY               = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("PowerUp Pos Y"         )]->getPointer();
    _powerUpPosX               = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("PowerUp Pos X"         )]->getPointer();
    _powerUpType               = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("PowerUp Type"          )]->getPointer();
    _powerUpMActive            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("PowerUp M Active"      )]->getPointer();
    _powerUpPosFallingState    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("PowerUp Falling State" )]->getPointer();
    _ballCount                 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball Count"            )]->getPointer();
    _ballHasLaunched           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Ball Has Launched"     )]->getPointer();
    _laserTimer                = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Laser Timer"           )]->getPointer();

    _blocks = &_lowMem[0x0700];
    for (size_t i = 0; i < _ARKANOID2_TILES_PER_COL; i++)
     for (size_t j = 0; j < _ARKANOID2_TILES_PER_ROW; j++)
      {
        auto propertyName = std::string("Block Value [") + std::to_string(i) + std::string("][") + std::to_string(j) + std::string("]");
        registerGameProperty(propertyName, &_blocks[i * _ARKANOID2_TILES_PER_ROW + j], Property::datatype_t::dt_uint8, Property::endianness_t::little);
      }

    _laserPositionDecision = 0;
    _remainingBlocks = 255;
    _remainingHits = 255;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Irreversively modify ball launched state
    if (_ball1State != 0) *_ballHasLaunched = 1;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x0210], 0x0080);
    hashEngine.Update(&_lowMem[0x0400], 0x0200);
    hashEngine.Update(&_lowMem[0x00F0], 0x0010);
    hashEngine.Update(&_lowMem[0x0700], 0x00C0); // Tiles
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
   {
    _score =
    (uint32_t) *_score1      * 1 +
    (uint32_t) *_score10     * 10 +
    (uint32_t) *_score100    * 100 +
    (uint32_t) *_score1000   * 1000 +
    (uint32_t) *_score10000  * 10000 +
    (uint32_t) *_score100000 * 100000;

    _remainingBlocks = 0;
    _remainingHits = 0;
    for (size_t i = 0; i < _ARKANOID2_TILES_PER_ROW * _ARKANOID2_TILES_PER_COL; i++)
    {
      switch(_blocks[i])
      {
        case 0xC8: { _remainingHits += 4; _remainingBlocks++; break; } // Silver Block (4 hits)
        case 0xC9: { _remainingHits += 3; _remainingBlocks++; break; } // Silver Block (3 hits)
        case 0xCA: { _remainingHits += 2; _remainingBlocks++; break; } // Silver Block (2 hits)
        case 0xC0: { _remainingHits += 2; _remainingBlocks++; break; } // Silver Block (2 hits)
        case 0xB0: break; // Bonus block
        case 0xD0: break; // Returning Brick
        case 0xD1: break; // Returning Brick
        case 0xD2: break; // Returning Brick
        case 0xD3: break; // Returning Brick
        case 0xD8: break; // Returning Brick
        case 0xD9: break; // Returning Brick
        case 0xDA: break; // Returning Brick
        case 0xDB: break; // Returning Brick
        case 0xE0: break; // Anihilated Returning Brick
        case 0xE8: break; // Anihilated Returning Brick
        case 0xF0: break; // Golden Brick
        case 0xF8: break; // Golden Brick
        case 0x00: break;
        default: { _remainingHits++; _remainingBlocks++; }
      }
    }

    // Establishing when it is possible to shoot laser in invincible mode
    _laserPositionDecision = 0;
    if (*_laserTimer % 4 == 3) _laserPositionDecision = 1;
   }

  __INLINE__ void ruleUpdatePreHook() override
  {
    _bossDamageMagnet         = 0.0;
    _brainBossDamageMagnet         = 0.0;
    _scoreMagnet              = 0.0;
    _ball1PosYMagnet          = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.push(&_laserPositionDecision, sizeof(_laserPositionDecision));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.pop(&_laserPositionDecision, sizeof(_laserPositionDecision));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += _bossDamageMagnet * (float)*_bossDamage;

    // Remaining hits
    reward -= 1.0f * _remainingHits;

    // Ball 1 Pos Y Magnet
    reward += _ball1PosYMagnet * (float)*_ball1PosY;

    // Rewarding powerup falling state, no matter what
    if (*_powerUpPosFallingState == 128) reward += 0.01;

    // If we havent achieved 8-ball, reward the presence of powerups
    if (*_ballCount == 0)
    {
      if (*_powerUpType != 0) reward += 0.1;
      if (*_powerUpType == 7) reward += 0.3; // 3 ball
      if (*_powerUpType == 4) reward += 0.5; // 8 ball
      if (*_powerUpType == 7 && *_powerUpPosFallingState == 128) reward += 0.5;
      if (*_powerUpType == 4 && *_powerUpPosFallingState == 128) reward += 1.0;
    }

    // If we havent achieved 3-ball, reward the presence of 8 ball powerup
    if (*_ballCount > 1 && *_ballCount < 8)
    {
      if (*_powerUpType != 0) reward += 0.1;
      if (*_powerUpType == 4) reward += 0.5;
      if (*_powerUpType == 4 && *_powerUpPosFallingState == 128) reward += 1.0;
    }

    // In any case, reward getting the M
    if (*_powerUpType == 1) reward += 0.1;
    if (*_powerUpType == 1 && *_powerUpPosFallingState == 128) reward += 0.2;

    // Score reward
    reward += _scoreMagnet * _score;

    // Returning reward
    return reward;
  }

__INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t> &allowedInputSet) override
  {
    addBallInputs(*_ball1PosX, *_ball1PosY, allowedInputSet);
    addBallInputs(*_ball2PosX, *_ball2PosY, allowedInputSet);
    addBallInputs(*_ball3PosX, *_ball3PosY, allowedInputSet);
    addBallInputs(*_ball4PosX, *_ball4PosY, allowedInputSet);
    addBallInputs(*_ball5PosX, *_ball5PosY, allowedInputSet);
    addBallInputs(*_ball6PosX, *_ball6PosY, allowedInputSet);
    addBallInputs(*_ball7PosX, *_ball7PosY, allowedInputSet);
    addBallInputs(*_ball8PosX, *_ball8PosY, allowedInputSet);

    // Evading boss attacks
    if (*_bossAttack1Exists > 0) addBossAttackInputs(*_bossAttack1PosX, *_bossAttack1PosY, allowedInputSet);
    if (*_bossAttack2Exists > 0) addBossAttackInputs(*_bossAttack2PosX, *_bossAttack2PosY, allowedInputSet);
    if (*_bossAttack3Exists > 0) addBossAttackInputs(*_bossAttack3PosX, *_bossAttack3PosY, allowedInputSet);
    if (*_bossAttack4Exists > 0) addBossAttackInputs(*_bossAttack4PosX, *_bossAttack4PosY, allowedInputSet);
    if (*_bossAttack5Exists > 0) addBossAttackInputs(*_bossAttack5PosX, *_bossAttack5PosY, allowedInputSet);
    if (*_bossAttack6Exists > 0) addBossAttackInputs(*_bossAttack6PosX, *_bossAttack6PosY, allowedInputSet);

    if (*_powerUpPosFallingState == 128 && *_powerUpPosY >= 208 && *_powerUpPosY <= 216)
      {
        auto input = (uint8_t)std::max(std::min(142, (int)*_powerUpPosX - 12), 1);
        allowedInputSet.push_back(paddlePositionIndexLUT[input]);

        // Add the option to skip taking the bonus in this frame
        if (*_powerUpPosX < 80) allowedInputSet.push_back(paddlePositionIndexLUT[142]);
        if (*_powerUpPosX > 80) allowedInputSet.push_back(paddlePositionIndexLUT[1]);
      }

    // If nothing else, add a default position
    if (allowedInputSet.empty() == true) allowedInputSet.push_back(paddlePositionIndexLUT[1]);
  };

  __INLINE__ void addBossAttackInputs(const uint8_t attackPosX, const uint8_t attackPosY, std::vector<InputSet::inputIndex_t>& allowedInputSet) const
    {
      if (attackPosY >= 208 && attackPosY <= 220)
      {
        if (attackPosX < 80) allowedInputSet.push_back(paddlePositionIndexLUT[142]);
        if (attackPosX > 80) allowedInputSet.push_back(paddlePositionIndexLUT[1]);
      }
    };

  __INLINE__ void addBallInputs(const uint8_t ballPosX, const uint8_t ballPosY, std::vector<InputSet::inputIndex_t>& allowedInputSet) const
    {
      if (ballPosY >= 208 && ballPosY <= 213)
      {
        auto input0 = (uint8_t)std::max(std::min(142, (int)ballPosX - 42), 1);
        auto input1 = (uint8_t)std::max(std::min(142, (int)ballPosX - 39), 1);
        auto input2 = (uint8_t)std::max(std::min(142, (int)ballPosX - 34), 1);
        auto input3 = (uint8_t)std::max(std::min(142, (int)ballPosX - 26), 1);
        auto input4 = (uint8_t)std::max(std::min(142, (int)ballPosX - 18), 1);
        auto input5 = (uint8_t)std::max(std::min(142, (int)ballPosX - 13), 1);

        allowedInputSet.push_back(paddlePositionIndexLUT[input0]);
        allowedInputSet.push_back(paddlePositionIndexLUT[input1]);
        allowedInputSet.push_back(paddlePositionIndexLUT[input2]);
        allowedInputSet.push_back(paddlePositionIndexLUT[input3]);
        allowedInputSet.push_back(paddlePositionIndexLUT[input4]);
        allowedInputSet.push_back(paddlePositionIndexLUT[input5]);

        // Add the option to skip hitting the ball in this frame
        if (ballPosX < 80) allowedInputSet.push_back(paddlePositionIndexLUT[142]);
        if (ballPosX > 80) allowedInputSet.push_back(paddlePositionIndexLUT[1]);
      }
    };

  void printInfoImpl() const override
  {
    //  jaffarCommon::logger::log("[J+] Round: %u\n", _arkState.level);
    jaffarCommon::logger::log("[J+]   + Game State: %u\n", *_gameState);
    //  jaffarCommon::logger::log("[J+] PowerUp Type: %u\n", _arkState.spawnedPowerup);
     jaffarCommon::logger::log("[J+]  + Paddle X: %u\n", *_paddlePosX);
     jaffarCommon::logger::log("[J+]  + Ball 1 Pos: (%u, %u)\n", *_ball1PosX, *_ball1PosY);
     jaffarCommon::logger::log("[J+]  + Ball 2 Pos: (%u, %u)\n", *_ball2PosX, *_ball2PosY);
     jaffarCommon::logger::log("[J+]  + Ball 3 Pos: (%u, %u)\n", *_ball3PosX, *_ball3PosY);
     jaffarCommon::logger::log("[J+]  + Ball 4 Pos: (%u, %u)\n", *_ball4PosX, *_ball4PosY);
     jaffarCommon::logger::log("[J+]  + Ball 5 Pos: (%u, %u)\n", *_ball5PosX, *_ball5PosY);
     jaffarCommon::logger::log("[J+]  + Ball 6 Pos: (%u, %u)\n", *_ball6PosX, *_ball6PosY);
     jaffarCommon::logger::log("[J+]  + Ball 7 Pos: (%u, %u)\n", *_ball7PosX, *_ball7PosY);
     jaffarCommon::logger::log("[J+]  + Ball 8 Pos: (%u, %u)\n", *_ball8PosX, *_ball8PosY);

    if (*_bossAttack1Exists > 0) jaffarCommon::logger::log("[J+]  + Boss Attack 1 Pos: (%u, %u)\n", *_bossAttack1PosX, *_bossAttack1PosY);
    if (*_bossAttack2Exists > 0) jaffarCommon::logger::log("[J+]  + Boss Attack 2 Pos: (%u, %u)\n", *_bossAttack2PosX, *_bossAttack2PosY);
    if (*_bossAttack3Exists > 0) jaffarCommon::logger::log("[J+]  + Boss Attack 3 Pos: (%u, %u)\n", *_bossAttack3PosX, *_bossAttack3PosY);
    if (*_bossAttack4Exists > 0) jaffarCommon::logger::log("[J+]  + Boss Attack 4 Pos: (%u, %u)\n", *_bossAttack4PosX, *_bossAttack4PosY);
    if (*_bossAttack5Exists > 0) jaffarCommon::logger::log("[J+]  + Boss Attack 5 Pos: (%u, %u)\n", *_bossAttack5PosX, *_bossAttack5PosY);
    if (*_bossAttack6Exists > 0) jaffarCommon::logger::log("[J+]  + Boss Attack 6 Pos: (%u, %u)\n", *_bossAttack6PosX, *_bossAttack6PosY);


    jaffarCommon::logger::log("[J+] PowerUp Pos X / Y:   %u / %u\n", *_powerUpPosX, *_powerUpPosY);
    jaffarCommon::logger::log("[J+] PowerUp Pos Type:   %u\n", *_powerUpType);
    jaffarCommon::logger::log("[J+] PowerUp Falling State:   %u\n", *_powerUpPosFallingState);
    jaffarCommon::logger::log("[J+] PowerUp M Active:   %u\n", *_powerUpMActive);
    
    jaffarCommon::logger::log("[J+] Ball Count  %u\n", *_ballCount);

    jaffarCommon::logger::log("[J+] Blocks Remaining:   %u\n", _remainingBlocks);
    jaffarCommon::logger::log("[J+] Hits Remaining:     %u\n", _remainingHits);
    jaffarCommon::logger::log("[J+] Score:   %u\n", _score);
    
     jaffarCommon::logger::log("[J+] Block State:\n");

     for (uint8_t i = 0; i < _ARKANOID2_TILES_PER_COL; i++)
     {
      jaffarCommon::logger::log("[J+] ");
      for (uint8_t j = 0; j < _ARKANOID2_TILES_PER_ROW; j++)
       if (_blocks[i*_ARKANOID2_TILES_PER_ROW + j] == 0) 
         jaffarCommon::logger::log("---- ");
       else
         jaffarCommon::logger::log("0x%02X ", _blocks[i*_ARKANOID2_TILES_PER_ROW + j]);
      jaffarCommon::logger::log("\n");
     }

    if (std::abs(_bossDamageMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Boss Damage Magnet                      %.5f\n", _bossDamageMagnet);
    if (std::abs(_brainBossDamageMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Brain Boss Damage Magnet                      %.5f\n", _brainBossDamageMagnet);
    if (std::abs(_ball1PosYMagnet)  > 0.0f) jaffarCommon::logger::log("[J+]  + Ball 1 Pos Y Magnet                     %.5f\n", _ball1PosYMagnet);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Boss Damage Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bossDamageMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Brain Boss Damage Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_brainBossDamageMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Score Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_scoreMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Ball 1 Pos Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_ball1PosYMagnet = intensity; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  float _bossDamageMagnet;
  float _brainBossDamageMagnet;
  float _scoreMagnet;
  float _ball1PosYMagnet;

  // Tile value vector
  uint8_t* _blocks;

  // I/O game value
  uint8_t *_ballHasLaunched;

  // Game Values
  uint8_t* _gameState          ;
  uint8_t* _ball1State         ;

  uint8_t* _ball1PosY          ;
  uint8_t* _ball2PosY          ;
  uint8_t* _ball3PosY          ;
  uint8_t* _ball4PosY          ;
  uint8_t* _ball5PosY          ;
  uint8_t* _ball6PosY          ;
  uint8_t* _ball7PosY          ;
  uint8_t* _ball8PosY          ;

  uint8_t* _ball1Angle         ;
  uint8_t* _bossDamage         ;
  uint8_t* _bossAttack1Exists  ;
  uint8_t* _bossAttack2Exists  ;
  uint8_t* _bossAttack3Exists  ;
  uint8_t* _bossAttack4Exists  ;
  uint8_t* _bossAttack5Exists  ;
  uint8_t* _bossAttack6Exists  ;
  uint8_t* _bossAttack1PosX    ;
  uint8_t* _bossAttack2PosX    ;
  uint8_t* _bossAttack3PosX    ;
  uint8_t* _bossAttack4PosX    ;
  uint8_t* _bossAttack5PosX    ;
  uint8_t* _bossAttack6PosX    ;
  uint8_t* _bossAttack1PosY    ;
  uint8_t* _bossAttack2PosY    ;
  uint8_t* _bossAttack3PosY    ;
  uint8_t* _bossAttack4PosY    ;
  uint8_t* _bossAttack5PosY    ;
  uint8_t* _bossAttack6PosY    ;

  uint8_t* _score1;
  uint8_t* _score10;
  uint8_t* _score100;
  uint8_t* _score1000;
  uint8_t* _score10000;
  uint8_t* _score100000;

  uint8_t* _powerUpPosY;
  uint8_t* _powerUpPosX;
  uint8_t* _powerUpType;
  uint8_t* _powerUpPosFallingState;

  // Pointer to emulator's low memory storage
  uint8_t *_lowMem;

  // Checks for laser shooting
  uint8_t* _laserTimer;
  uint8_t _laserPositionDecision;

  public:

  uint8_t* _ball1PosX          ;
  uint8_t* _ball2PosX          ;
  uint8_t* _ball3PosX          ;
  uint8_t* _ball4PosX          ;
  uint8_t* _ball5PosX          ;
  uint8_t* _ball6PosX          ;
  uint8_t* _ball7PosX          ;
  uint8_t* _ball8PosX          ;
  uint8_t* _brainBossDamage    ;
  
  // Derivative values
  uint8_t* _paddlePosX         ;
  uint8_t _remainingBlocks;
  uint8_t _remainingHits;
  uint32_t _score;
  uint8_t* _powerUpMActive;
  uint8_t* _ballCount;
  

    // Lookup table from paddle position to input index
  InputSet::inputIndex_t  paddlePositionIndexLUT[143];

  // Lookup table for target position and its corresponding input
  const char* paddlePositionStringLUT[143] =
  {
    "|..|........|.......|    0,F|", 
    "|..|........|.......|    1,F|", 
    "|..|........|.......|    2,F|", 
    "|..|........|.......|    3,F|", 
    "|..|........|.......|    4,F|", 
    "|..|........|.......|    5,F|", 
    "|..|........|.......|    6,F|", 
    "|..|........|.......|    7,F|", 
    "|..|........|.......|    8,F|", 
    "|..|........|.......|    9,F|", 
    "|..|........|.......|   10,F|", 
    "|..|........|.......|   11,F|", 
    "|..|........|.......|   12,F|", 
    "|..|........|.......|   13,F|", 
    "|..|........|.......|   14,F|", 
    "|..|........|.......|   15,F|", 
    "|..|........|.......|   16,F|", 
    "|..|........|.......|   17,F|", 
    "|..|........|.......|   18,F|", 
    "|..|........|.......|   19,F|", 
    "|..|........|.......|   20,F|", 
    "|..|........|.......|   21,F|", 
    "|..|........|.......|   22,F|", 
    "|..|........|.......|   23,F|", 
    "|..|........|.......|   24,F|", 
    "|..|........|.......|   25,F|", 
    "|..|........|.......|   26,F|", 
    "|..|........|.......|   27,F|", 
    "|..|........|.......|   28,F|", 
    "|..|........|.......|   29,F|", 
    "|..|........|.......|   30,F|", 
    "|..|........|.......|   31,F|", 
    "|..|........|.......|   32,F|", 
    "|..|........|.......|   33,F|", 
    "|..|........|.......|   34,F|", 
    "|..|........|.......|   35,F|", 
    "|..|........|.......|   36,F|", 
    "|..|........|.......|   37,F|", 
    "|..|........|.......|   38,F|", 
    "|..|........|.......|   39,F|", 
    "|..|........|.......|   40,F|", 
    "|..|........|.......|   41,F|", 
    "|..|........|.......|   42,F|", 
    "|..|........|.......|   43,F|", 
    "|..|........|.......|   44,F|", 
    "|..|........|.......|   45,F|", 
    "|..|........|.......|   46,F|", 
    "|..|........|.......|   47,F|", 
    "|..|........|.......|   48,F|", 
    "|..|........|.......|   49,F|", 
    "|..|........|.......|   50,F|", 
    "|..|........|.......|   51,F|", 
    "|..|........|.......|   52,F|", 
    "|..|........|.......|   53,F|", 
    "|..|........|.......|   54,F|", 
    "|..|........|.......|   55,F|", 
    "|..|........|.......|   56,F|", 
    "|..|........|.......|   57,F|", 
    "|..|........|.......|   58,F|", 
    "|..|........|.......|   59,F|", 
    "|..|........|.......|   60,F|", 
    "|..|........|.......|   61,F|", 
    "|..|........|.......|   62,F|", 
    "|..|........|.......|   63,F|", 
    "|..|........|.......|   64,F|", 
    "|..|........|.......|   65,F|", 
    "|..|........|.......|   66,F|", 
    "|..|........|.......|   67,F|", 
    "|..|........|.......|   68,F|", 
    "|..|........|.......|   69,F|", 
    "|..|........|.......|   70,F|", 
    "|..|........|.......|   71,F|", 
    "|..|........|.......|   72,F|", 
    "|..|........|.......|   73,F|", 
    "|..|........|.......|   74,F|", 
    "|..|........|.......|   75,F|", 
    "|..|........|.......|   76,F|", 
    "|..|........|.......|   77,F|", 
    "|..|........|.......|   78,F|", 
    "|..|........|.......|   79,F|", 
    "|..|........|.......|   80,F|", 
    "|..|........|.......|   81,F|", 
    "|..|........|.......|   82,F|", 
    "|..|........|.......|   83,F|", 
    "|..|........|.......|   84,F|", 
    "|..|........|.......|   85,F|", 
    "|..|........|.......|   86,F|", 
    "|..|........|.......|   87,F|", 
    "|..|........|.......|   88,F|", 
    "|..|........|.......|   89,F|", 
    "|..|........|.......|   90,F|", 
    "|..|........|.......|   91,F|", 
    "|..|........|.......|   92,F|", 
    "|..|........|.......|   93,F|", 
    "|..|........|.......|   94,F|", 
    "|..|........|.......|   95,F|", 
    "|..|........|.......|   96,F|", 
    "|..|........|.......|   97,F|", 
    "|..|........|.......|   98,F|", 
    "|..|........|.......|   99,F|", 
    "|..|........|.......|  100,F|", 
    "|..|........|.......|  101,F|", 
    "|..|........|.......|  102,F|", 
    "|..|........|.......|  103,F|", 
    "|..|........|.......|  104,F|", 
    "|..|........|.......|  105,F|", 
    "|..|........|.......|  106,F|", 
    "|..|........|.......|  107,F|", 
    "|..|........|.......|  108,F|", 
    "|..|........|.......|  109,F|", 
    "|..|........|.......|  110,F|", 
    "|..|........|.......|  111,F|", 
    "|..|........|.......|  112,F|", 
    "|..|........|.......|  113,F|", 
    "|..|........|.......|  114,F|", 
    "|..|........|.......|  115,F|", 
    "|..|........|.......|  116,F|", 
    "|..|........|.......|  117,F|", 
    "|..|........|.......|  118,F|", 
    "|..|........|.......|  119,F|", 
    "|..|........|.......|  120,F|", 
    "|..|........|.......|  121,F|", 
    "|..|........|.......|  122,F|", 
    "|..|........|.......|  123,F|", 
    "|..|........|.......|  124,F|", 
    "|..|........|.......|  125,F|", 
    "|..|........|.......|  126,F|", 
    "|..|........|.......|  127,F|", 
    "|..|........|.......|  128,F|", 
    "|..|........|.......|  129,F|", 
    "|..|........|.......|  130,F|", 
    "|..|........|.......|  131,F|", 
    "|..|........|.......|  132,F|", 
    "|..|........|.......|  133,F|", 
    "|..|........|.......|  134,F|", 
    "|..|........|.......|  135,F|", 
    "|..|........|.......|  136,F|", 
    "|..|........|.......|  137,F|", 
    "|..|........|.......|  138,F|", 
    "|..|........|.......|  139,F|", 
    "|..|........|.......|  140,F|", 
    "|..|........|.......|  141,F|", 
    "|..|........|.......|  142,F|"
  };
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
