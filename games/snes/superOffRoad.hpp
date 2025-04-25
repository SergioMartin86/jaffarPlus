#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>
#include <map>

namespace jaffarPlus
{

namespace games
{

namespace snes
{

class SuperOffRoad final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "SNES / Super Off Road"; }

  SuperOffRoad(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
    // Getting checkpoint magnets
    for (auto checkpointMagnetJs : config["Checkpoint Magnets"])
    {
       const auto checkpointId = checkpointMagnetJs[0].get<uint8_t>();
       const auto checkpointPosX = checkpointMagnetJs[1].get<uint8_t>();
       const auto checkpointPosY = checkpointMagnetJs[2].get<uint8_t>();
       _checkpointPointMagnets[checkpointId] = std::make_pair(checkpointPosX, checkpointPosY);
    }
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("RAM").pointer;

    registerGameProperty("Money100",             &_lowMem[0x000561], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Money10",              &_lowMem[0x000562], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Money1",               &_lowMem[0x000563], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Input",         &_lowMem[0x000512], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Speed",         &_lowMem[0x000501], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X1",        &_lowMem[0x0004EA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X2",        &_lowMem[0x0004E9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y1",        &_lowMem[0x0004F2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y2",        &_lowMem[0x0004F1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Angle",         &_lowMem[0x00050B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Turbos",               &_lowMem[0x00061B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Lap",           &_lowMem[0x0004E5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Checkpoint",    &_lowMem[0x0004D1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Money Grabbed",        &_lowMem[0x000633], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _money100         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Money100"      )]->getPointer();
    _money10          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Money10"       )]->getPointer();
    _money1           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Money1"        )]->getPointer();
    _playerInput      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input"  )]->getPointer();
    _playerSpeed      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Speed"  )]->getPointer();
    _playerPosX1      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X1" )]->getPointer();
    _playerPosX2      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X2" )]->getPointer();
    _playerPosY1      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y1" )]->getPointer();
    _playerPosY2      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y2" )]->getPointer();
    _playerAngle      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Angle"  )]->getPointer();
    _turbos           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Turbos"        )]->getPointer();
    _playerCheckpoint = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Checkpoint")]->getPointer();
    _playerLap        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Lap")]->getPointer();
    _moneyGrabbed     = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Money Grabbed")]->getPointer();

    _prevCheckpoint = *_playerCheckpoint;
    _actualCheckpoint = *_playerCheckpoint;
    _prevTurbos = *_turbos;
    _lastInputStep = 0;
    _currentStep = 0;

    // Getting index for a non input 
    _nullInputIdx = _emulator->registerInput("|..|............|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // If this is is a non-null input, update the last input step
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    // Running emulator
    _emulator->advanceState(input);

    // Advance current step
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override {}

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _money = 100 * *_money100 + 10 * *_money10 + 1 * *_money1;

    if (*_turbos > _prevTurbos) _turboGrabs += *_turbos - _prevTurbos;
    _prevTurbos = *_turbos;

    if (*_playerCheckpoint != 0) _actualCheckpoint = *_playerCheckpoint;
    if (*_playerCheckpoint == 0 && _prevCheckpoint != 0) _actualCheckpoint = _prevCheckpoint + 1;
    _prevCheckpoint = *_playerCheckpoint;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;

    if (_checkpointPointMagnets.contains(*_playerCheckpoint) == false) JAFFAR_THROW_LOGIC("Manget information for checkpoint %u not found", *_playerCheckpoint);
    _pointMagnet.x         = _checkpointPointMagnets[*_playerCheckpoint].first;
    _pointMagnet.y         = _checkpointPointMagnets[*_playerCheckpoint].second;

    // Resetting magnets ahead of rule re-evaluation
    _overridePointMagnet.intensity = 0.0;
    _overridePointMagnet.x         = 0.0;
    _overridePointMagnet.y         = 0.0;

    _speedMagnet = 0.0;
    _moneyMagnet = 0.0;
    _turboMagnet = 0.0;
    _checkpointMagnet = 0.0;
    _lapMagnet = 0.0;
    _lastInputMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    if (_overridePointMagnet.intensity > 0.0)
    {
      _player1DistanceToPointX = std::abs((float)_overridePointMagnet.x - (float)*_playerPosX1);
      _player1DistanceToPointY = std::abs((float)_overridePointMagnet.y - (float)*_playerPosY1);
      _player1DistanceToPoint  = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);
    }
    else
    {
      _player1DistanceToPointX = std::abs((float)_pointMagnet.x - (float)*_playerPosX1);
      _player1DistanceToPointY = std::abs((float)_pointMagnet.y - (float)*_playerPosY1);
      _player1DistanceToPoint  = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.push(&_prevTurbos, sizeof(_prevTurbos));
    serializer.push(&_turboGrabs, sizeof(_turboGrabs));
    serializer.push(&_prevCheckpoint, sizeof(_prevCheckpoint));
    serializer.push(&_actualCheckpoint, sizeof(_actualCheckpoint));
    serializer.push(&_lastInputStep, sizeof(_lastInputStep));
    serializer.push(&_currentStep, sizeof(_currentStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) override
  {
    deserializer.pop(&_prevTurbos, sizeof(_prevTurbos));
    deserializer.pop(&_turboGrabs, sizeof(_turboGrabs));
    deserializer.pop(&_prevCheckpoint, sizeof(_prevCheckpoint));
    deserializer.pop(&_actualCheckpoint, sizeof(_actualCheckpoint));
    deserializer.pop(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.pop(&_currentStep, sizeof(_currentStep));
  }

  __INLINE__ float calculateGameSpecificReward() const override
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    if (_overridePointMagnet.intensity > 0.0) reward += -1.0 * _overridePointMagnet.intensity * _player1DistanceToPoint;
    else reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Reward Speed
    reward += _speedMagnet * (float)*_playerSpeed;

    // Reward Money
    reward += _moneyMagnet * (float)*_moneyGrabbed;

    // Reward new turbo grabs
    reward += _turboMagnet * (float)_turboGrabs;

    // Reward passed laps
    reward += _lapMagnet * (float)*_playerLap;

    // Reward passed checkpoints
    reward += _checkpointMagnet * (float)_actualCheckpoint;

    // Reward for last input
    reward += _lastInputMagnet * (float)_lastInputStep;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Last Not Null Input Step                   %u\n", _lastInputStep);
    jaffarCommon::logger::log("[J+]  + Grabbed Money                              %u\n", *_moneyGrabbed);
    jaffarCommon::logger::log("[J+]  + Grabbed Turbos                             %u\n", _turboGrabs);

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }

    if (std::abs(_overridePointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Override Point Magnet                    Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _overridePointMagnet.intensity, _overridePointMagnet.x, _overridePointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }

    if (std::abs(_speedMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Lap Magnet                               Intensity: %.5f\n", _lapMagnet);
    if (std::abs(_speedMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Checkpoint Magnet                        Intensity: %.5f (Actual Checkpoint: %u)\n", _checkpointMagnet, _actualCheckpoint);
    if (std::abs(_speedMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Speed Magnet                             Intensity: %.5f\n", _speedMagnet);
    if (std::abs(_moneyMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Money Magnet                             Intensity: %.5f\n", _moneyMagnet);
    if (std::abs(_turboMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Turbo Magnet                             Intensity: %.5f (Grabbed: %u)\n", _turboMagnet, _turboGrabs);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      // auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      // auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      // rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      rule.addAction([=, this]() { this->_pointMagnet.intensity = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Override Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_overridePointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }


    if (actionType == "Set Speed Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_speedMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Money Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_moneyMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Turbo Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_turboMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Lap Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_lapMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Checkpoint Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_checkpointMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Last Input Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_lastInputMagnet = intensity; });
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
  pointMagnet_t _overridePointMagnet;
  float _speedMagnet;
  float _moneyMagnet;
  float _turboMagnet;
  float _checkpointMagnet;
  float _lapMagnet;
  float _lastInputMagnet;

  uint8_t* _money100       ;
  uint8_t* _money10        ;
  uint8_t* _money1         ;
  uint8_t* _playerInput    ;
  uint8_t* _playerSpeed    ;
  uint8_t* _playerPosX1    ;
  uint8_t* _playerPosX2    ;
  uint8_t* _playerPosY1    ;
  uint8_t* _playerPosY2    ;
  uint8_t* _playerAngle    ;
  uint8_t* _playerCheckpoint   ;
  uint8_t* _playerLap   ;
  uint8_t* _turbos;
  uint8_t* _moneyGrabbed;

  uint8_t _prevCheckpoint;
  uint8_t _actualCheckpoint;

  uint16_t _money;
  uint8_t _prevTurbos;
  uint16_t _turboGrabs;

  InputSet::inputIndex_t _nullInputIdx;
  uint16_t _lastInputStep;
  uint16_t _currentStep;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;

  // Pointer to emulator's low memory storage
  uint8_t *_lowMem;

  // Checkpoint point magnets
  std::map<uint8_t, std::pair<uint8_t, uint8_t>> _checkpointPointMagnets;
};

} // namespace snes

} // namespace games

} // namespace jaffarPlus