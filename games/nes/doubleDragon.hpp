#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class DoubleDragon final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Double Dragon"; }

  DoubleDragon(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {

    _lowMem = _emulator->getProperty("LRAM").pointer;

    registerGameProperty("Player Pos X1"       , &_lowMem[0x037E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Pos X1"      , &_lowMem[0x037F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Pos X1"      , &_lowMem[0x0380], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y1"       , &_lowMem[0x0072], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Pos Y1"      , &_lowMem[0x0073], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Pos Y1"      , &_lowMem[0x0074], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Pos X1"       , &_lowMem[0x0508], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Pos X2"       , &_lowMem[0x0507], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Pos Y1"       , &_lowMem[0x00DD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player HP"           , &_lowMem[0x03B4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 HP1"         , &_lowMem[0x03B5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 HP1"         , &_lowMem[0x03B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 HP2"         , &_lowMem[0x03C5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 HP2"         , &_lowMem[0x03C6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global Timer"        , &_lowMem[0x0502], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Type"          , &_lowMem[0x0002], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Input 1"      , &_lowMem[0x00F5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Input 2"      , &_lowMem[0x00F7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Action"       , &_lowMem[0x03B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 1"             , &_lowMem[0x0044], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 2"             , &_lowMem[0x0045], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 3"             , &_lowMem[0x0046], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game State"          , &_lowMem[0x003E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Air State"    , &_lowMem[0x038E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player In Action"    , &_lowMem[0x0416], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Action Timer" , &_lowMem[0x00B2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Stun State"  , &_lowMem[0x038F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Stun State"  , &_lowMem[0x0390], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Enemy 1 HP"        , &_enemy1HP, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 HP"        , &_enemy2HP, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Enemy 1 HP1"        , &_prevEnemy1HP1, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Enemy 2 HP1"        , &_prevEnemy2HP1, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score"          , &_score, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Current Step"   , &_currentStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Pos X"   , &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Y"   , &_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Screen Pos X"   , &_screenPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Screen Pos Y"   , &_screenPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Enemies Killed" , &_enemiesKilled, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemies Spawned" , &_enemiesSpawned, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Enemy 1 Stun State" , &_prevEnemy1StunState, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Enemy 2 Stun State" , &_prevEnemy2StunState, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting emulator's low memory pointer

    _playerPosX1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X1"   )]->getPointer();
    _enemy1PosX1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 Pos X1"  )]->getPointer();
    _enemy2PosX1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 Pos X1"  )]->getPointer();
    _playerPosY1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y1"   )]->getPointer();
    _enemy1PosY1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 Pos Y1"  )]->getPointer();
    _enemy2PosY1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 Pos Y1"  )]->getPointer();
    _screenPosX1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X1"  )]->getPointer();
    _screenPosX2  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X2"  )]->getPointer();
    _screenPosY1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos Y1"  )]->getPointer();
    _playerHP     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player HP"      )]->getPointer();
    _enemy1HP1    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 HP1"     )]->getPointer();
    _enemy2HP1    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 HP1"     )]->getPointer();
    _enemy1HP2    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 HP2"     )]->getPointer();
    _enemy2HP2    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 HP2"     )]->getPointer();
    _globalTimer  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer"   )]->getPointer();
    _frameType    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Type"     )]->getPointer();
    _playerInput1 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Input 1" )]->getPointer();
    _playerInput2 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Input 2" )]->getPointer();
    _playerAction = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Action"  )]->getPointer();
    _score1       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 1"        )]->getPointer();
    _score2       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 2"        )]->getPointer();
    _score3       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 3"        )]->getPointer();
    _gameState    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game State"     )]->getPointer();
    _enemy1StunState = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 1 Stun State"     )]->getPointer();
    _enemy2StunState = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy 2 Stun State"     )]->getPointer();

    _prevEnemy1StunState = *_enemy1StunState;
    _prevEnemy2StunState = *_enemy2StunState;
    _prevEnemy1HP1 = *_enemy1HP1;
    _prevEnemy2HP1 = *_enemy2HP1;
    _enemiesKilled = 0;
    _enemiesSpawned = 0;

    _currentStep = 0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _prevEnemy1HP1 = *_enemy1HP1;
    _prevEnemy2HP1 = *_enemy2HP1;

    // Running emulator
    _emulator->advanceState(input);

    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x000], 0x008 - 0x000); // Lag frame accounting
    // hashEngine.Update(_lowMem[0x03B]); // Transition Timer
    // hashEngine.Update(_lowMem[0x0B2]); // Transition Timer
    // hashEngine.Update(_lowMem[0x412]); // Transition Timer
    hashEngine.Update(&_lowMem[0x280], 0x400 - 0x280);

    if (*_frameType > 1) hashEngine.Update(_currentStep);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _playerPosX = (float)*_playerPosX1;
    _playerPosY = (float)*_playerPosY1;

    _enemy1PosX = (float)*_enemy1PosX1;
    _enemy2PosX = (float)*_enemy2PosX1;
    _enemy1PosY = (float)*_enemy1PosY1;
    _enemy2PosY = (float)*_enemy2PosY1;

    _screenPosX = (float)*_screenPosX1 * 256.0f + (float)*_screenPosX2;
    _screenPosY = (float)*_screenPosY1;

    _score = ((float)*_score1 * 10.0 + (float)*_score2 * 1000.0 + (float)*_score3 * 100000.0) * ( 10.0 / 16.0 );

    _enemy1HP = 0;
    if (*_enemy1HP1 > 0) _enemy1HP = *_enemy1HP1 + 64;
    if (*_enemy1HP1 == 0) _enemy1HP = 32 - *_enemy1HP2;

    _enemy2HP = 0;
    if (*_enemy2HP1 > 0) _enemy2HP = *_enemy2HP1 + 64;
    if (*_enemy2HP1 == 0) _enemy2HP = 32 - *_enemy2HP2;


    if (*_enemy1HP1 == 0 && *_enemy1StunState == 128 && _prevEnemy1StunState != 128) _enemiesKilled++;
    if (*_enemy2HP1 == 0 && *_enemy2StunState == 128 && _prevEnemy2StunState != 128) _enemiesKilled++;

    _prevEnemy1StunState = *_enemy1StunState;
    _prevEnemy2StunState = *_enemy2StunState;

    if (*_enemy1HP1 > _prevEnemy1HP1) _enemiesSpawned++;
    if (*_enemy2HP1 > _prevEnemy2HP1) _enemiesSpawned++;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    _playerEnemy1Magnet = 0.0;
    _playerEnemy2Magnet = 0.0;

    _playerPointMagnet.intensity = 0.0;
    _playerPointMagnet.x         = 0.0;
    _playerPointMagnet.y         = 0.0;

    _screenPointMagnet.intensity = 0.0;
    _screenPointMagnet.x         = 0.0;
    _screenPointMagnet.y         = 0.0;

    _enemy1HPMagnet = 0.0;
    _enemy2HPMagnet = 0.0;
    _enemiesKilledMagnet = 0.0;
    _enemiesSpawnedMagnet = 0.0;
    _scoreMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _playerDistanceToPointX = std::abs(_playerPointMagnet.x - _playerPosX);
    _playerDistanceToPointY = std::abs(_playerPointMagnet.y - _playerPosY);
    _playerDistanceToPoint  = sqrtf(_playerDistanceToPointX * _playerDistanceToPointX + _playerDistanceToPointY * _playerDistanceToPointY);

    _screenDistanceToPointX = std::abs(_screenPointMagnet.x - _screenPosX);
    _screenDistanceToPointY = std::abs(_screenPointMagnet.y - _screenPosY);
    _screenDistanceToPoint  = sqrtf(_screenDistanceToPointX * _screenDistanceToPointX + _screenDistanceToPointY * _screenDistanceToPointY);

    _playerDistanceToEnemy1X = std::abs(_enemy1PosX - _playerPosX);
    _playerDistanceToEnemy2X = std::abs(_enemy2PosX - _playerPosX);
    _playerDistanceToEnemy1Y = std::abs(_enemy1PosY - _playerPosY);
    _playerDistanceToEnemy2Y = std::abs(_enemy2PosY - _playerPosY);

    _playerDistanceToEnemy1 = sqrtf(_playerDistanceToEnemy1X * _playerDistanceToEnemy1X + _playerDistanceToEnemy1Y * _playerDistanceToEnemy1Y);
    _playerDistanceToEnemy2 = sqrtf(_playerDistanceToEnemy2X * _playerDistanceToEnemy2X + _playerDistanceToEnemy2Y * _playerDistanceToEnemy2Y);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override 
  {
    serializer.push(&_enemiesKilled, sizeof(_enemiesKilled));
    serializer.push(&_enemiesSpawned, sizeof(_enemiesSpawned));
    serializer.push(&_currentStep, sizeof(_currentStep));
    serializer.push(&_prevEnemy1StunState, sizeof(_prevEnemy1StunState));
    serializer.push(&_prevEnemy2StunState, sizeof(_prevEnemy2StunState));
    serializer.push(&_prevEnemy1HP1, sizeof(_prevEnemy1HP1));
    serializer.push(&_prevEnemy2HP1, sizeof(_prevEnemy2HP1));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializer.pop(&_enemiesKilled, sizeof(_enemiesKilled));
    deserializer.pop(&_enemiesSpawned, sizeof(_enemiesSpawned));
    deserializer.pop(&_currentStep, sizeof(_currentStep));
    deserializer.pop(&_prevEnemy1StunState, sizeof(_prevEnemy1StunState));
    deserializer.pop(&_prevEnemy2StunState, sizeof(_prevEnemy2StunState));
    deserializer.pop(&_prevEnemy1HP1, sizeof(_prevEnemy1HP1));
    deserializer.pop(&_prevEnemy2HP1, sizeof(_prevEnemy2HP1));
  }

  __INLINE__ float calculateGameSpecificReward() const override
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += -1.0 * _playerPointMagnet.intensity * _playerDistanceToPoint;
    reward += -1.0 * _screenPointMagnet.intensity * _screenDistanceToPoint;
    reward += _enemy1HPMagnet * (float)_enemy1HP;
    reward += _enemy2HPMagnet * (float)_enemy2HP;

    if (_enemy1HP > 0) reward += _playerEnemy1Magnet * _playerDistanceToEnemy1;
    if (_enemy2HP > 0) reward += _playerEnemy2Magnet * _playerDistanceToEnemy2;

    reward += (float)_enemiesKilled * _enemiesKilledMagnet;
    reward += (float)_enemiesSpawned * _enemiesSpawnedMagnet;
    reward += _score * _scoreMagnet;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_playerEnemy1Magnet) > 0.0f && _enemy1HP > 0)
    {
      jaffarCommon::logger::log("[J+]  + Player / Enemy 1 Magnet                  Intensity: %.5f\n", _playerEnemy1Magnet);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _playerDistanceToEnemy1X);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _playerDistanceToEnemy1Y);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _playerDistanceToEnemy1);
    }

    if (std::abs(_playerEnemy2Magnet) > 0.0f && _enemy2HP > 0)
    {
      jaffarCommon::logger::log("[J+]  + Player / Enemy 2 Magnet                  Intensity: %.5f\n", _playerEnemy2Magnet);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _playerDistanceToEnemy2X);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _playerDistanceToEnemy2Y);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _playerDistanceToEnemy2);
    }

    if (std::abs(_playerPointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Player Point Magnet                      Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _playerPointMagnet.intensity, _playerPointMagnet.x, _playerPointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _playerDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _playerDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _playerDistanceToPoint);
    }

    if (std::abs(_screenPointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Screen Point Magnet                      Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _screenPointMagnet.intensity, _screenPointMagnet.x, _screenPointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _screenDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _screenDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _screenDistanceToPoint);
    }
    
    if (std::abs(_enemy1HPMagnet) > 0.0f)       jaffarCommon::logger::log("[J+]  + Enemy 1 HP Magnet                      Intensity: %.5f\n", _enemy1HPMagnet);
    if (std::abs(_enemy2HPMagnet) > 0.0f)       jaffarCommon::logger::log("[J+]  + Enemy 2 HP Magnet                      Intensity: %.5f\n", _enemy2HPMagnet);
    if (std::abs(_enemiesKilledMagnet) > 0.0f)  jaffarCommon::logger::log("[J+]  + Enemies Killed Magnet                  Intensity: %.5f\n", _enemiesKilledMagnet);
    if (std::abs(_enemiesSpawnedMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Enemies Spawned Magnet                 Intensity: %.5f\n", _enemiesSpawnedMagnet);
    if (std::abs(_scoreMagnet) > 0.0f)          jaffarCommon::logger::log("[J+]  + Score Magnet                           Intensity: %.5f\n", _scoreMagnet);
    
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Player / Enemy 1 Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_playerEnemy1Magnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player / Enemy 2 Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_playerEnemy2Magnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_playerPointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Screen Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_screenPointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Score Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_scoreMagnet = intensity;  });
      recognizedActionType = true;
    }

    if (actionType == "Set Enemy 1 HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_enemy1HPMagnet = intensity;  });
      recognizedActionType = true;
    }

    if (actionType == "Set Enemy 2 HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_enemy2HPMagnet = intensity;  });
      recognizedActionType = true;
    }

    if (actionType == "Set Enemies Killed Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_enemiesKilledMagnet = intensity;  });
      recognizedActionType = true;
    }

    if (actionType == "Set Enemies Spawned Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_enemiesSpawnedMagnet = intensity;  });
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

  float _playerEnemy1Magnet;
  float _playerEnemy2Magnet;
  float _enemy1HPMagnet;
  float _enemy2HPMagnet;
  float _enemiesKilledMagnet;
  float _enemiesSpawnedMagnet;
  float _scoreMagnet;
  pointMagnet_t _playerPointMagnet;
  pointMagnet_t _screenPointMagnet;

  uint8_t* _playerPosX1  ;
  uint8_t* _enemy1PosX1  ;
  uint8_t* _enemy2PosX1  ;
  uint8_t* _playerPosY1  ;
  uint8_t* _enemy1PosY1  ;
  uint8_t* _enemy2PosY1  ;
  uint8_t* _screenPosX1 ;
  uint8_t* _screenPosX2 ;
  uint8_t* _screenPosY1 ;
  uint8_t* _playerHP    ;
  uint8_t* _enemy1HP1    ;
  uint8_t* _enemy2HP1    ;
  uint8_t* _enemy1HP2    ;
  uint8_t* _enemy2HP2    ;
  uint8_t* _globalTimer ;
  uint8_t* _frameType   ;
  uint8_t* _playerInput1;
  uint8_t* _playerInput2;
  uint8_t* _playerAction;
  uint8_t* _score1;
  uint8_t* _score2;
  uint8_t* _score3;
  uint8_t* _gameState;
  uint8_t* _enemy1StunState;
  uint8_t* _enemy2StunState;

  uint8_t _prevEnemy1StunState;
  uint8_t _prevEnemy2StunState;

  // Game-Specific values
  float _enemy1PosX;
  float _enemy1PosY;
  float _enemy2PosX;
  float _enemy2PosY;

  float _playerPosX;
  float _playerPosY;
  float _playerDistanceToPointX;
  float _playerDistanceToPointY;
  float _playerDistanceToPoint;

  float _playerDistanceToEnemy1X;
  float _playerDistanceToEnemy2X;
  float _playerDistanceToEnemy1Y;
  float _playerDistanceToEnemy2Y;
  float _playerDistanceToEnemy1;
  float _playerDistanceToEnemy2;

  float _screenPosX;
  float _screenPosY;
  float _screenDistanceToPointX;
  float _screenDistanceToPointY;
  float _screenDistanceToPoint;

  float _score;

  uint8_t _enemy1HP;
  uint8_t _enemy2HP;

  uint8_t _prevEnemy1HP1;
  uint8_t _prevEnemy2HP1;

  uint8_t _enemiesKilled;
  uint8_t _enemiesSpawned;
  uint16_t _currentStep;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus