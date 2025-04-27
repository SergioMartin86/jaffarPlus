#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{

namespace games
{

namespace genesis
{

class MicroMachines final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "Genesis / Micro Machines"; }

  MicroMachines(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Current Race", &_workRAM[0x8080], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos X", &_workRAM[0x80AE], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Camera Pos Y", &_workRAM[0x80B0], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X", &_workRAM[0x80A6], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos Y", &_workRAM[0x80A8], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Vel X", &_workRAM[0xA65A], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Vel Y", &_workRAM[0xA680], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Angle", &_workRAM[0xA656], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Current Laps Remaining", &_workRAM[0xA69E], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Checkpoint 1", &_workRAM[0xA6DE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Checkpoint 2", &_workRAM[0xA6E1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Offroad Checkpoint", &_workRAM[0xA6E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Recovery Mode", &_workRAM[0xA672], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Recovery Timer", &_workRAM[0xA670], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pre-Race Timer", &_workRAM[0x8000], Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Registering derivative game properties
    registerGameProperty("Player 1 Previous Laps Remaining", &_player1LapsRemainingPrev, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Previous Checkpoint", &_player1CheckpointPrev, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    registerGameProperty("Current Step", &_currentStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Last Input Step", &_lastInputStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _currentRace = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Race")]->getPointer();
    _cameraPosX  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Camera Pos X")]->getPointer();
    _cameraPosY  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Camera Pos Y")]->getPointer();

    _player1PosX = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos X")]->getPointer();
    _player1PosY = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos Y")]->getPointer();

    _player1VelX  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Vel X")]->getPointer();
    _player1VelY  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Vel Y")]->getPointer();
    _player1Angle = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Angle")]->getPointer();

    _player1LapsRemaining     = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Current Laps Remaining")]->getPointer();
    _player1Checkpoint1       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Checkpoint 1")]->getPointer();
    _player1Checkpoint2       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Checkpoint 2")]->getPointer();
    _player1OffroadCheckpoint = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Offroad Checkpoint")]->getPointer();

    _player1RecoveryMode  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Recovery Mode")]->getPointer();
    _player1RecoveryTimer = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Recovery Timer")]->getPointer();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _player1LapsRemainingPrev = *_player1LapsRemaining;
    _player1CheckpointPrev    = *_player1Checkpoint1;

    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_workRAM[0xA65A]);
    hashEngine.Update(_workRAM[0xA6DA]);
    //  hashEngine.Update(_workRAM[0xC77A]);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    // Re-calculating distance to camera
    _player1DistanceToCameraX = std::abs((float)*_cameraPosX - (float)*_player1PosX);
    _player1DistanceToCameraY = std::abs((float)*_cameraPosY - (float)*_player1PosY);
    _player1DistanceToCamera  = sqrtf(_player1DistanceToCameraX * _player1DistanceToCameraX + _player1DistanceToCameraY * _player1DistanceToCameraY);
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _playerCurrentLapMagnet       = 0.0;
    _playerLapProgressMagnet      = 0.0;
    _cameraDistanceMagnet         = 0.0;
    _recoveryTimerMagnet          = 0.0;
    _lastInputMagnet              = 0.0;
    _playerVelMagnet              = 0.0;
    _player1AngleMagnet.intensity = 0.0;
    _player1AngleMagnet.angle     = 0.0;

    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs((float)_pointMagnet.x - (float)*_player1PosX);
    _player1DistanceToPointY = std::abs((float)_pointMagnet.y - (float)*_player1PosY);
    _player1DistanceToPoint  = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);

    _player1DistanceToMagnetAngle = std::abs(_player1AngleMagnet.angle - (float)*_player1Angle);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    // Storing previous lap count
    serializer.pushContiguous(&_player1LapsRemainingPrev, sizeof(_player1LapsRemainingPrev));
    serializer.pushContiguous(&_player1CheckpointPrev, sizeof(_player1CheckpointPrev));
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    // Restoring previous lap count
    deserializer.popContiguous(&_player1LapsRemainingPrev, sizeof(_player1LapsRemainingPrev));
    deserializer.popContiguous(&_player1CheckpointPrev, sizeof(_player1CheckpointPrev));
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputMagnet * _lastInputStep;

    // Evaluating player current lap  magnet
    reward += _playerCurrentLapMagnet * (float)(*_player1LapsRemaining);

    // Evaluating player lap progress  magnet
    reward += _playerLapProgressMagnet * (float)(*_player1Checkpoint1);

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Distance to camera
    reward += -1.0 * _cameraDistanceMagnet * _player1DistanceToCamera;

    // Velocity Magnet
    reward += _playerVelMagnet * (*_player1VelX + *_player1VelY);

    // Evaluating player health  magnet
    reward += _recoveryTimerMagnet * (float)(*_player1RecoveryTimer);

    // Calculating angle magnet
    reward += (255.0 - _player1DistanceToMagnetAngle) * _player1AngleMagnet.intensity;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }

    if (std::abs(_cameraDistanceMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Camera Distance Magnet                   Intensity: %.5f, Dist: %3.3f\n", _cameraDistanceMagnet, _player1DistanceToCamera);
    if (std::abs(_recoveryTimerMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Recovery Timer Magnet                    Intensity: %.5f\n", _recoveryTimerMagnet);
    if (std::abs(_playerCurrentLapMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Player Current Lap Magnet                Intensity: %.5f\n", _playerCurrentLapMagnet);
    if (std::abs(_playerLapProgressMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Player Lap Progress Magnet               Intensity: %.5f\n", _playerLapProgressMagnet);
    if (std::abs(_playerVelMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Player Vel Magnet                      Intensity: %.5f\n", _playerVelMagnet);
    if (std::abs(_lastInputMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Last Input Magnet                      Intensity: %.5f\n", _lastInputMagnet);
    if (std::abs(_player1AngleMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Player Angle Magnet                      Intensity: %.5f, Angle: %3.0f, Dist: %3.0f\n", _player1AngleMagnet.intensity,
                                _player1AngleMagnet.angle, _player1DistanceToMagnetAngle);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      auto action    = [=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Current Lap Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_playerCurrentLapMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Lap Progress Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_playerLapProgressMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Vel Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_playerVelMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Camera Distance Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_cameraDistanceMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Recovery Timer Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_recoveryTimerMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Last Input Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_lastInputMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player 1 Angle Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto angle     = jaffarCommon::json::getNumber<float>(actionJs, "Angle");
      auto action    = [=, this]() { this->_player1AngleMagnet = angleMagnet_t{.intensity = intensity, .angle = angle}; };
      rule.addAction(action);
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

  // Datatype to describe an angle magnet
  struct angleMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float angle     = 0.0; // What is the angle we look for
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  float         _playerCurrentLapMagnet  = 0.0;
  float         _playerLapProgressMagnet = 0.0;
  float         _cameraDistanceMagnet    = 0.0;
  float         _recoveryTimerMagnet     = 0.0;
  float         _lastInputMagnet         = 0.0;
  float         _playerVelMagnet         = 0.0;
  angleMagnet_t _player1AngleMagnet;
  pointMagnet_t _pointMagnet;

  // Property pointers for quick access
  uint8_t* _workRAM;
  uint8_t* _currentRace;

  uint16_t* _preRaceTimer;

  uint16_t* _player1LapsRemaining;
  uint16_t  _player1LapsRemainingPrev;

  uint8_t* _player1Checkpoint1;
  uint8_t* _player1Checkpoint2;
  uint8_t* _player1OffroadCheckpoint;
  uint8_t  _player1CheckpointPrev = 0;

  uint8_t* _player1RecoveryMode;
  uint8_t* _player1RecoveryTimer;

  uint16_t* _player1PosX;
  uint16_t* _player1PosY;

  uint16_t* _player1VelX;
  uint16_t* _player1VelY;

  uint16_t* _player1Angle;
  uint16_t* _cameraPosX;
  uint16_t* _cameraPosY;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;
  float _player1DistanceToCameraX;
  float _player1DistanceToCameraY;
  float _player1DistanceToCamera;
  float _player1DistanceToMagnetAngle;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace genesis

} // namespace games

} // namespace jaffarPlus