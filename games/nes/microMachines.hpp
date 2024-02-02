#pragma once

#include <common/json.hpp>
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

  Game(std::unique_ptr<Emulator>& emulator, const nlohmann::json& config) : jaffarPlus::Game(emulator, config)
  {
    // Perform parsing of the configuration here, if anything special settings are required.
  }

  private:

  void initializeImpl() override
  {
    // Getting some properties' pointers now for quick access later
    _player1TankFireTimer              = (uint8_t*)  _propertyMap[hashString("Player 1 Tank Fire Timer")]->getPointer();
    _cameraPosX1                       = (uint8_t*)  _propertyMap[hashString("Camera Pos X1")]->getPointer();
    _cameraPosX2                       = (uint8_t*)  _propertyMap[hashString("Camera Pos X2")]->getPointer();
    _cameraPosY1                       = (uint8_t*)  _propertyMap[hashString("Camera Pos Y1")]->getPointer();
    _cameraPosY2                       = (uint8_t*)  _propertyMap[hashString("Camera Pos Y2")]->getPointer();
    _player1PosX1                      = (uint8_t*)  _propertyMap[hashString("Player 1 Pos X1")]->getPointer();
    _player1PosX2                      = (uint8_t*)  _propertyMap[hashString("Player 1 Pos X2")]->getPointer();
    _player1PosY1                      = (uint8_t*)  _propertyMap[hashString("Player 1 Pos Y1")]->getPointer();
    _player1PosY2                      = (uint8_t*)  _propertyMap[hashString("Player 1 Pos Y2")]->getPointer();
    _player1Accel                      = (int8_t*)   _propertyMap[hashString("Player 1 Accel")]->getPointer();
    _player1AccelTimer2                = (uint8_t*)  _propertyMap[hashString("Player 1 Accel Timer 2")]->getPointer();
    _player1Angle1                     = (uint8_t*)  _propertyMap[hashString("Player 1 Angle 1")]->getPointer();
    _player1LapsRemaining              = (uint8_t*)  _propertyMap[hashString("Player 1 Current Laps Remaining")]->getPointer();
    _player1LapsRemainingPrev          = (uint8_t*)  _propertyMap[hashString("Player 1 Previous Laps Remaining")]->getPointer();
    _player1Checkpoint                 = (uint8_t*)  _propertyMap[hashString("Player 1 Checkpoint")]->getPointer();
    _player1RecoveryTimer              = (uint8_t*)  _propertyMap[hashString("Player 1 Recovery Timer")]->getPointer();

    // Setting initial value for previous laps remaining
    *_player1LapsRemainingPrev = *_player1LapsRemaining;
  }

  void advanceStateImpl(const std::string& input) override
  {
    *_player1LapsRemainingPrev = *_player1LapsRemaining;

    _emulator->advanceState(input);
  }

  void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(*_player1TankFireTimer > 0);
  }

  void updateGameSpecificValues() override
  {
    _player1PosX = (uint16_t)*_player1PosX1 * 256 + (uint16_t)*_player1PosX2;
    _player1PosY = (uint16_t)*_player1PosY1 * 256 + (uint16_t)*_player1PosY2;

    _cameraPosX = (uint16_t)*_cameraPosX1 * 256 + (uint16_t)*_cameraPosX2;
    _cameraPosY = (uint16_t)*_cameraPosY1 * 256 + (uint16_t)*_cameraPosY2;

    // Re-calclating stats
    _player1DistanceToPointY  = std::abs(pointMagnet.x - _player1PosX);
    _player1DistanceToPointY  = std::abs(pointMagnet.y - _player1PosY);
    _player1DistanceToPoint   = sqrtf(_player1DistanceToPointX*_player1DistanceToPointX + _player1DistanceToPointY*_player1DistanceToPointY);

    _player1DistanceToCameraX = std::abs(_cameraPosX - _player1PosX);
    _player1DistanceToCameraY = std::abs(_cameraPosY - _player1PosY);
    _player1DistanceToCamera  = sqrtf(_player1DistanceToCameraX*_player1DistanceToCameraX + _player1DistanceToCameraY*_player1DistanceToCameraY);

    // Resetting magnets ahead of rule re-evaluation
    playerCurrentLapMagnet = 0.0;
    playerLapProgressMagnet = 0.0;
    playerAccelMagnet = 0.0;
    cameraDistanceMagnet = 0.0;
    recoveryTimerMagnet = 0.0;
    car1AngleMagnet.intensity = 0.0;
    pointMagnet.intensity = 0.0;
  }

  float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Evaluating player health  magnet
    reward += playerCurrentLapMagnet * (float)(*_player1LapsRemaining);

    // Evaluating player health  magnet
    reward += playerLapProgressMagnet * (float)(*_player1Checkpoint);

    // Evaluating player health  magnet
    reward += playerAccelMagnet * ( std::abs((float)*_player1Accel) - 0.1*(float)*_player1AccelTimer2);

    // Distance to point magnet
    reward += pointMagnet.intensity * -_player1DistanceToPoint;

    // Distance to camera
    reward += cameraDistanceMagnet * -_player1DistanceToCamera;

    // Evaluating player health  magnet
    reward += recoveryTimerMagnet * (float)(*_player1RecoveryTimer);

    // Calculating angle magnet
    reward += (255.0 - _player1DistanceToMagnetAngle) * car1AngleMagnet.intensity;

    // Returning reward
    return reward;
  }

  void printStateInfoImpl() const override
  {
    if (std::abs(pointMagnet.intensity) > 0.0f)      LOG("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f, Dist: %3.3f\n", pointMagnet.intensity, pointMagnet.x, pointMagnet.y, _player1DistanceToPoint);
    if (std::abs(cameraDistanceMagnet) > 0.0f)       LOG("[J+]  + Camera Distance Magnet                   Intensity: %.5f, Dist: %3.3f\n", cameraDistanceMagnet, _player1DistanceToCamera);
    if (std::abs(recoveryTimerMagnet) > 0.0f)        LOG("[J+]  + Recovery Timer Magnet                    Intensity: %.5f\n", recoveryTimerMagnet);
    if (std::abs(playerCurrentLapMagnet) > 0.0f)     LOG("[J+]  + Player Current Lap Magnet                Intensity: %.5f\n", playerCurrentLapMagnet);
    if (std::abs(playerLapProgressMagnet) > 0.0f)    LOG("[J+]  + Player Lap Progress Magnet               Intensity: %.5f\n", playerLapProgressMagnet);
    if (std::abs(playerAccelMagnet) > 0.0f)          LOG("[J+]  + Player Accel Magnet                      Intensity: %.5f\n", playerAccelMagnet);
    if (std::abs(car1AngleMagnet.intensity) > 0.0f)  LOG("[J+]  + Angle Magnet                             Intensity: %.5f, Angle: %3.0f, Dist: %3.0f\n", car1AngleMagnet.intensity, car1AngleMagnet.angle, _player1DistanceToMagnetAngle);
  } 

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = JSON_GET_NUMBER(float, actionJs, "Intensity");
      auto x = JSON_GET_NUMBER(float, actionJs, "X");
      auto y = JSON_GET_NUMBER(float, actionJs, "Y");
      rule.addAction([=, this](){ this->pointMagnet = pointMagnet_t { .intensity = intensity, .x = x, .y = y }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Current Lap Magnet")
    {
      auto intensity = JSON_GET_NUMBER(float, actionJs, "Intensity");
      rule.addAction([=, this](){ this->playerCurrentLapMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Lap Progress Magnet")
    {
      auto intensity = JSON_GET_NUMBER(float, actionJs, "Intensity");
      rule.addAction([=, this](){ this->playerLapProgressMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Accel Magnet")
    {
      auto intensity = JSON_GET_NUMBER(float, actionJs, "Intensity");
      rule.addAction([=, this](){ this->playerAccelMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Camera Distance Magnet")
    {
      auto intensity = JSON_GET_NUMBER(float, actionJs, "Intensity");
      rule.addAction([=, this](){ this->cameraDistanceMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Recovery Timer Magnet")
    {
      auto intensity = JSON_GET_NUMBER(float, actionJs, "Intensity");
      rule.addAction([=, this](){ this->recoveryTimerMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Car 1 Angle Magnet")
    {
      auto intensity = JSON_GET_NUMBER(float, actionJs, "Intensity");
      auto angle = JSON_GET_NUMBER(float, actionJs, "Angle");
      rule.addAction([=, this]()
       { 
        this->car1AngleMagnet = angleMagnet_t { .intensity = intensity, .angle = angle};
        this->_player1DistanceToMagnetAngle = std::abs(*_player1Angle1 - car1AngleMagnet.angle);
       });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x = 0.0;  // What is the x point of attraction
    float y = 0.0;  // What is the y point of attraction
  };

  // Datatype to describe an angle magnet
  struct angleMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float angle = 0.0;  // What is the angle we look for
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  float playerCurrentLapMagnet = 0.0;
  float playerLapProgressMagnet = 0.0;
  float playerAccelMagnet = 0.0;
  float cameraDistanceMagnet = 0.0;
  float recoveryTimerMagnet = 0.0;
  angleMagnet_t car1AngleMagnet;
  pointMagnet_t pointMagnet;

  // Property pointers for quick access
  uint8_t*  _cameraPosX1;
  uint8_t*  _cameraPosX2;
  uint8_t*  _cameraPosY1;
  uint8_t*  _cameraPosY2;

  uint8_t*  _player1PosX1;
  uint8_t*  _player1PosX2;
  uint8_t*  _player1PosY1;
  uint8_t*  _player1PosY2;

  uint8_t*  _player1AccelTimer2;
  int8_t*   _player1Accel;
  uint8_t*  _player1Angle1;
  uint8_t*  _player1LapsRemaining;
  uint8_t*  _player1LapsRemainingPrev;
  uint8_t*  _player1Checkpoint;

  uint8_t*  _player1RecoveryTimer;
  uint8_t*  _player1TankFireTimer;

  // Calculated values
  uint16_t _player1PosX;
  uint16_t _player1PosY;
  uint16_t _cameraPosX;
  uint16_t _cameraPosY;
  uint8_t  _timerTolerance;

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