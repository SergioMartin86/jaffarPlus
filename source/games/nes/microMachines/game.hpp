#pragma once

#include <common/json.hpp>
#include "../../game.hpp"

namespace jaffarPlus
{

namespace games
{

namespace NES
{

namespace microMachines
{

class Game final : public jaffarPlus::Game
{
  public:

  static inline std::string getName() { return "NES / Micro Machines"; } 

  Game(std::unique_ptr<Emulator>& emulator, const nlohmann::json& config) : jaffarPlus::Game(emulator, config)
  {
    // Timer tolerance
    timerTolerance = JSON_GET_NUMBER(uint8_t, config, "Timer Tolerance");
  }

  private:

  void initializeImpl() override
  {
    // Getting NES low RAM pointer
    auto ram = _emulator->getProperty("LRAM").pointer;

    // Game-specific values
    frameType                         = (uint8_t*)   &ram[0x007C];
    lagFrame                          = (uint8_t*)   &ram[0x01F8];
    cameraPosX1                       = (uint8_t*)   &ram[0x00D5];
    cameraPosX2                       = (uint8_t*)   &ram[0x00D4];
    cameraPosY1                       = (uint8_t*)   &ram[0x00D7];
    cameraPosY2                       = (uint8_t*)   &ram[0x00D6];
    currentRace                       = (uint8_t*)   &ram[0x0308];

    player1PosX1                      = (uint8_t*)   &ram[0x03DE];
    player1PosX2                      = (uint8_t*)   &ram[0x03DA];
    player1PosY1                      = (uint8_t*)   &ram[0x03EA];
    player1PosY2                      = (uint8_t*)   &ram[0x03E6];
    player1PosY3                      = (uint8_t*)   &ram[0x03EE];
    player1Accel                      = (int8_t*)    &ram[0x0386];
    player1AccelTimer1                = (uint8_t*)   &ram[0x0102];
    player1AccelTimer2                = (uint8_t*)   &ram[0x0103];
    player1AccelTimer3                = (uint8_t*)   &ram[0x010E];
    player1Inertia1                   = (uint8_t*)   &ram[0x00B0];
    player1Inertia2                   = (uint8_t*)   &ram[0x00B2];
    player1Inertia3                   = (uint8_t*)   &ram[0x00B4];
    player1Inertia4                   = (uint8_t*)   &ram[0x00B6];
    player1Angle1                     = (uint8_t*)   &ram[0x04B2];
    player1Angle2                     = (uint8_t*)   &ram[0x040A];
    player1Angle3                     = (uint8_t*)   &ram[0x04CA];
    player1LapsRemaining              = (uint8_t*)   &ram[0x04B6];
    player1LapsRemainingPrev          = (uint8_t*)   &ram[0x07FF];
    player1Checkpoint                 = (uint8_t*)   &ram[0x04CE];
    player1Input1                     = (uint8_t*)   &ram[0x009B];
    player1Input2                     = (uint8_t*)   &ram[0x00CF];
    player1Input3                     = (uint8_t*)   &ram[0x0352];

    player1RecoveryMode               = (uint8_t*)   &ram[0x0416];
    player1RecoveryTimer              = (uint8_t*)   &ram[0x041A];
    player1ResumeTimer                = (uint8_t*)   &ram[0x00DA];
    player1CanControlCar              = (uint8_t*)   &ram[0x01BF];
    player1TankFireTimer              = (uint8_t*)   &ram[0x041E];

    preRaceTimer                      = (uint8_t*)   &ram[0x00DD];
    activeFrame1                      = (uint8_t*)   &ram[0x00B0];
    activeFrame2                      = (uint8_t*)   &ram[0x00B2];
    activeFrame3                      = (uint8_t*)   &ram[0x00B4];
    activeFrame4                      = (uint8_t*)   &ram[0x00B6];

    playerLastInputKey                = (uint8_t*)   &ram[0x009B];
    playerLastInputFrame              = (uint16_t*)  &ram[0x01A0];


    // Getting properties for custom hashing
    player1TankFireTimer              = (uint8_t*) _propertyMap[hashString("Player 1 Tank Fire Timer")]->getPointer();

    // Game-specific values
    currentStep                       = (uint32_t*)  &_gameSpecificStorage[0x0000];
    *currentStep = 0;
  }

  size_t getGameSpecificStorageSize() const override { return sizeof(uint32_t); }

  std::vector<std::string> advanceStateImpl(const std::string& input) override
  {
    *player1LapsRemainingPrev = *player1LapsRemaining;
    *currentStep = *currentStep + 1;

    _emulator->advanceState(input);

    if (input != "|..|........|") *playerLastInputFrame = *currentStep;

    return {input};
  }

  void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    if (timerTolerance > 0) hashEngine.Update(*currentStep % (timerTolerance+1));

    hashEngine.Update(*player1TankFireTimer > 0);
  }

  void updateGameSpecificValues() override
  {
    player1PosX = (uint16_t)*player1PosX1 * 256 + (uint16_t)*player1PosX2;
    player1PosY = (uint16_t)*player1PosY1 * 256 + (uint16_t)*player1PosY2;

    cameraPosX = (uint16_t)*cameraPosX1 * 256 + (uint16_t)*cameraPosX2;
    cameraPosY = (uint16_t)*cameraPosY1 * 256 + (uint16_t)*cameraPosY2;

    // Re-calclating stats
    _player1DistanceToPointY  = std::abs(pointMagnet.x - player1PosX);
    _player1DistanceToPointY  = std::abs(pointMagnet.y - player1PosY);
    _player1DistanceToPoint   = sqrtf(_player1DistanceToPointX*_player1DistanceToPointX + _player1DistanceToPointY*_player1DistanceToPointY);

    _player1DistanceToCameraX = std::abs(cameraPosX - player1PosX);
    _player1DistanceToCameraY = std::abs(cameraPosY - player1PosY);
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

    // We calculate a different reward if this is a winning frame
    if (_stateType == stateType_t::win) return -1.0f * (float)*playerLastInputFrame;

    // Evaluating player health  magnet
    reward += playerCurrentLapMagnet * (float)(*player1LapsRemaining);

    // Evaluating player health  magnet
    reward += playerLapProgressMagnet * (float)(*player1Checkpoint);

    // Evaluating player health  magnet
    reward += playerAccelMagnet * ( std::abs((float)*player1Accel) - 0.1*(float)*player1AccelTimer2);

    // Distance to point magnet
    reward += pointMagnet.intensity * -_player1DistanceToPoint;

    // Distance to camera
    reward += cameraDistanceMagnet * -_player1DistanceToCamera;

    // Evaluating player health  magnet
    reward += recoveryTimerMagnet * (float)(*player1RecoveryTimer);

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
        this->_player1DistanceToMagnetAngle = std::abs(*player1Angle1 - car1AngleMagnet.angle);
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

  // Container for game-specific values
  uint8_t*  currentRace;
  uint8_t*  preRaceTimer;
  uint8_t*  frameType;
  uint8_t*  lagFrame;

  uint8_t*  cameraPosX1;
  uint8_t*  cameraPosX2;
  uint8_t*  cameraPosY1;
  uint8_t*  cameraPosY2;

  uint8_t*  player1PosX1;
  uint8_t*  player1PosX2;
  uint8_t*  player1PosY1;
  uint8_t*  player1PosY2;
  uint8_t*  player1PosY3;
  uint8_t*  player1Inertia1;
  uint8_t*  player1Inertia2;
  uint8_t*  player1Inertia3;
  uint8_t*  player1Inertia4;
  uint8_t*  player1AccelTimer1;
  uint8_t*  player1AccelTimer2;
  uint8_t*  player1AccelTimer3;
  int8_t*   player1Accel;
  uint8_t*  player1Angle1;
  uint8_t*  player1Angle2;
  uint8_t*  player1Angle3;
  uint8_t*  player1LapsRemaining;
  uint8_t*  player1LapsRemainingPrev;
  uint8_t*  player1Checkpoint;
  uint8_t*  player1Input1;
  uint8_t*  player1Input2;
  uint8_t*  player1Input3;

  uint8_t*  player1RecoveryMode;
  uint8_t*  player1RecoveryTimer;
  uint8_t*  player1CanControlCar;
  uint8_t*  player1ResumeTimer;
  uint8_t*  player1TankFireTimer;
  uint8_t*  playerLastInputKey;
  uint16_t*  playerLastInputFrame;

  uint8_t* activeFrame1;
  uint8_t* activeFrame2;
  uint8_t* activeFrame3;
  uint8_t* activeFrame4;

  uint16_t player1PosX;
  uint16_t player1PosY;
  uint16_t cameraPosX;
  uint16_t cameraPosY;
  uint8_t timerTolerance;
  uint32_t* currentStep;

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  float playerCurrentLapMagnet = 0.0;
  float playerLapProgressMagnet = 0.0;
  float playerAccelMagnet = 0.0;
  float cameraDistanceMagnet = 0.0;
  float recoveryTimerMagnet = 0.0;
  angleMagnet_t car1AngleMagnet;
  pointMagnet_t pointMagnet;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;
  float _player1DistanceToCameraX;
  float _player1DistanceToCameraY;
  float _player1DistanceToCamera;
  float _player1DistanceToMagnetAngle;
};

} // namespace microMachines

} // namespace NES

} // namespace games

} // namespace jaffarPlus