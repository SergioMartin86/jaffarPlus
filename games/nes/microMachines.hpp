#pragma once

#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace NES
{

class MicroMachines final : public jaffarPlus::Game
{
  public:
  static inline std::string getName() { return "NES / Micro Machines"; }

  MicroMachines(std::unique_ptr<Emulator> emulator, const nlohmann::json &config) : jaffarPlus::Game(std::move(emulator), config)
  {

  }

  private:

  inline void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    auto lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Current Race", &lowMem[0x0308], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Frame Type", &lowMem[0x007C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Lag Frame", &lowMem[0x01F8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos X1", &lowMem[0x00D5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos X2", &lowMem[0x00D4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos Y1", &lowMem[0x00D7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos Y2", &lowMem[0x00D6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X1", &lowMem[0x03DE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X2", &lowMem[0x03DA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos Y1", &lowMem[0x03EA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos Y2", &lowMem[0x03E6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos Y3", &lowMem[0x03EE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Accel", &lowMem[0x0386], Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Player 1 Accel Timer 1", &lowMem[0x0102], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Accel Timer 2", &lowMem[0x0103], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Accel Timer 3", &lowMem[0x010E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Inertia 1", &lowMem[0x00B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Inertia 2", &lowMem[0x00B2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Inertia 3", &lowMem[0x00B4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Inertia 4", &lowMem[0x00B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Angle 1", &lowMem[0x04B2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Angle 2", &lowMem[0x040A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Angle 3", &lowMem[0x04CA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Current Laps Remaining", &lowMem[0x04B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Checkpoint", &lowMem[0x04CE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Input 1", &lowMem[0x009B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Input 2", &lowMem[0x00CF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Input 3", &lowMem[0x0352], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Recovery Mode", &lowMem[0x0416], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Recovery Timer", &lowMem[0x041A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Resume Timer", &lowMem[0x00DA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Can Control Car", &lowMem[0x01BF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Tank Fire Timer", &lowMem[0x041E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pre-Race Timer", &lowMem[0x00DD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Active Frame 1", &lowMem[0x00B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Active Frame 2", &lowMem[0x00B2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Active Frame 3", &lowMem[0x00B4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Active Frame 4", &lowMem[0x00B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Registering derivative game properties
    registerGameProperty("Player 1 Previous Laps Remaining", &_player1LapsRemainingPrev, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos Y", &_player1PosY, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X", &_player1PosX, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _player1TankFireTimer = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Tank Fire Timer")]->getPointer();
    _currentRace = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Current Race")]->getPointer();
    _cameraPosX1 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Camera Pos X1")]->getPointer();
    _cameraPosX2 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Camera Pos X2")]->getPointer();
    _cameraPosY1 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Camera Pos Y1")]->getPointer();
    _cameraPosY2 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Camera Pos Y2")]->getPointer();
    _player1PosX1 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos X1")]->getPointer();
    _player1PosX2 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos X2")]->getPointer();
    _player1PosY1 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos Y1")]->getPointer();
    _player1PosY2 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos Y2")]->getPointer();
    _player1Accel = (int8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Accel")]->getPointer();
    _player1AccelTimer2 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Accel Timer 2")]->getPointer();
    _player1Angle1 = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Angle 1")]->getPointer();
    _player1LapsRemaining = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Current Laps Remaining")]->getPointer();
    _player1Checkpoint = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Checkpoint")]->getPointer();
    _player1RecoveryTimer = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player 1 Recovery Timer")]->getPointer();
  }

  inline void advanceStateImpl(const std::string &input) override
  {
    _player1LapsRemainingPrev = *_player1LapsRemaining;

    _emulator->advanceState(input);
  }

  inline void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(*_player1TankFireTimer > 0);
  }

  // Updating derivative values after updating the internal state
  inline void stateUpdatePostHook() override
  {
    _player1PosX = (uint16_t)*_player1PosX1 * 256 + (uint16_t)*_player1PosX2;
    _player1PosY = (uint16_t)*_player1PosY1 * 256 + (uint16_t)*_player1PosY2;

    _cameraPosX = (uint16_t)*_cameraPosX1 * 256 + (uint16_t)*_cameraPosX2;
    _cameraPosY = (uint16_t)*_cameraPosY1 * 256 + (uint16_t)*_cameraPosY2;

    // Re-calculating distance to camera
    _player1DistanceToCameraX = std::abs((float)_cameraPosX - (float)_player1PosX);
    _player1DistanceToCameraY = std::abs((float)_cameraPosY - (float)_player1PosY);
    _player1DistanceToCamera = sqrtf(_player1DistanceToCameraX * _player1DistanceToCameraX + _player1DistanceToCameraY * _player1DistanceToCameraY);
  }

  inline void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _playerCurrentLapMagnet = 0.0;
    _playerLapProgressMagnet = 0.0;
    _playerAccelMagnet = 0.0;
    _cameraDistanceMagnet = 0.0;
    _recoveryTimerMagnet = 0.0;
    _player1AngleMagnet.intensity = 0.0;
    _player1AngleMagnet.angle = 0.0;

    _pointMagnet.intensity = 0.0;
    _pointMagnet.x = 0.0;
    _pointMagnet.y = 0.0;
  }

  inline void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs((float)_pointMagnet.x - (float)_player1PosX);
    _player1DistanceToPointY = std::abs((float)_pointMagnet.y - (float)_player1PosY);
    _player1DistanceToPoint = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);

    _player1DistanceToMagnetAngle = std::abs(_player1AngleMagnet.angle - (float)*_player1Angle1);
  }

  inline void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    // Storing previous lap count
    serializer.pushContiguous(&_player1LapsRemainingPrev, 1);
  }

  inline void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    // Restoring previous lap count
    deserializer.popContiguous(&_player1LapsRemainingPrev, 1);
  }

  inline float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Evaluating player current lap  magnet
    reward += _playerCurrentLapMagnet * (float)(*_player1LapsRemaining);

    // Evaluating player lap progress  magnet
    reward += _playerLapProgressMagnet * (float)(*_player1Checkpoint);

    // Evaluating player accel  magnet
    reward += _playerAccelMagnet * (std::abs((float)*_player1Accel) - 0.1 * (float)*_player1AccelTimer2);

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // // Distance to camera
    reward += -1.0 * _cameraDistanceMagnet * _player1DistanceToCamera;

    // // Evaluating player health  magnet
    reward += _recoveryTimerMagnet * (float)(*_player1RecoveryTimer);

    // // Calculating angle magnet
    reward += (255.0 - _player1DistanceToMagnetAngle) * _player1AngleMagnet.intensity;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J++]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J++]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
      jaffarCommon::logger::log("[J++]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
      jaffarCommon::logger::log("[J++]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }

    if (std::abs(_cameraDistanceMagnet) > 0.0f) jaffarCommon::logger::log("[J++]  + Camera Distance Magnet                   Intensity: %.5f, Dist: %3.3f\n", _cameraDistanceMagnet, _player1DistanceToCamera);
    if (std::abs(_recoveryTimerMagnet) > 0.0f) jaffarCommon::logger::log("[J++]  + Recovery Timer Magnet                    Intensity: %.5f\n", _recoveryTimerMagnet);
    if (std::abs(_playerCurrentLapMagnet) > 0.0f) jaffarCommon::logger::log("[J++]  + Player Current Lap Magnet                Intensity: %.5f\n", _playerCurrentLapMagnet);
    if (std::abs(_playerLapProgressMagnet) > 0.0f) jaffarCommon::logger::log("[J++]  + Player Lap Progress Magnet               Intensity: %.5f\n", _playerLapProgressMagnet);
    if (std::abs(_playerAccelMagnet) > 0.0f) jaffarCommon::logger::log("[J++]  + Player Accel Magnet                      Intensity: %.5f\n", _playerAccelMagnet);
    if (std::abs(_player1AngleMagnet.intensity) > 0.0f) jaffarCommon::logger::log("[J++]  + Player Angle Magnet                      Intensity: %.5f, Angle: %3.0f, Dist: %3.0f\n", _player1AngleMagnet.intensity, _player1AngleMagnet.angle, _player1DistanceToMagnetAngle);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      auto action = [=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Current Lap Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action = [=, this]() { this->_playerCurrentLapMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Lap Progress Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action = [=, this]() { this->_playerLapProgressMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Accel Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action = [=, this]() { this->_playerAccelMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Camera Distance Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action = [=, this]() { this->_cameraDistanceMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Recovery Timer Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action = [=, this]() { this->_recoveryTimerMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player 1 Angle Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto angle = jaffarCommon::json::getNumber<float>(actionJs, "Angle");
      auto action = [=, this]() { this->_player1AngleMagnet = angleMagnet_t{.intensity = intensity, .angle = angle}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x = 0.0;         // What is the x point of attraction
    float y = 0.0;         // What is the y point of attraction
  };

  // Datatype to describe an angle magnet
  struct angleMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float angle = 0.0;     // What is the angle we look for
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  float _playerCurrentLapMagnet = 0.0;
  float _playerLapProgressMagnet = 0.0;
  float _playerAccelMagnet = 0.0;
  float _cameraDistanceMagnet = 0.0;
  float _recoveryTimerMagnet = 0.0;
  angleMagnet_t _player1AngleMagnet;
  pointMagnet_t _pointMagnet;

  // Property pointers for quick access
  uint8_t *_currentRace;

  uint8_t *_cameraPosX1;
  uint8_t *_cameraPosX2;
  uint8_t *_cameraPosY1;
  uint8_t *_cameraPosY2;

  uint8_t *_player1PosX1;
  uint8_t *_player1PosX2;
  uint8_t *_player1PosY1;
  uint8_t *_player1PosY2;

  uint8_t *_player1AccelTimer2;
  int8_t *_player1Accel;
  uint8_t *_player1Angle1;
  uint8_t *_player1LapsRemaining;
  uint8_t _player1LapsRemainingPrev = 255;
  uint8_t *_player1Checkpoint;

  uint8_t *_player1RecoveryTimer;
  uint8_t *_player1TankFireTimer;

  // Calculated values
  uint16_t _player1PosX;
  uint16_t _player1PosY;
  uint16_t _cameraPosX;
  uint16_t _cameraPosY;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;
  float _player1DistanceToCameraX;
  float _player1DistanceToCameraY;
  float _player1DistanceToCamera;
  float _player1DistanceToMagnetAngle;
};

} // namespace NES

} // namespace games

} // namespace jaffarPlus