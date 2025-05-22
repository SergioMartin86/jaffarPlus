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

class Beetlejuice final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Beetlejuice"; }

  Beetlejuice(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties

    registerGameProperty("Player Frame",  &_lowMem[0x03CE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Pos X1", &_lowMem[0x03BF], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Pos X2", &_lowMem[0x03BE], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Pos X3", &_lowMem[0x03BD], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Vel X1", &_lowMem[0x03C4], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Vel X2", &_lowMem[0x03C3], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Pos Y1", &_lowMem[0x03C2], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Pos Y2", &_lowMem[0x03C1], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Player Pos Y3", &_lowMem[0x03C0], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos X",  &_lowMem[0x002C], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Screen Pos Y",  &_lowMem[0x002E], Property::datatype_t::dt_int8, Property::endianness_t::little); 
    registerGameProperty("Global Timer",  &_lowMem[0x03BB], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("RNG1",          &_lowMem[0x0024], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("RNG2",          &_lowMem[0x0025], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("RNG3",          &_lowMem[0x0026], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("RNG4",          &_lowMem[0x0027], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Beetle1 Type",  &_lowMem[0x042F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Beetle2 Type",  &_lowMem[0x0430], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Beetle3 Type",  &_lowMem[0x0431], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Money 1",       &_lowMem[0x03D2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Money 2",       &_lowMem[0x03D1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Money",         &_money, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Bug Worth",     &_bugWorth, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Game State",    &_lowMem[0x0313], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game Transition Timer", &_lowMem[0x0799], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Exit Switch State", &_lowMem[0x629], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level Transition 1", &_lowMem[0x01FA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level Transition 2", &_lowMem[0x044A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Level Transition 1", &_prevLevelTransition1, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Level Transition 2", &_prevLevelTransition2, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level Transition Timer", &_levelTransitionTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Level Transition State", &_levelTransitionState, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X",  &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little); 

    _playerFrame   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Frame"  )]->getPointer();
    _playerPosX1   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X1" )]->getPointer();
    _playerPosX2   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X2" )]->getPointer();
    _playerPosX3   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X3" )]->getPointer();
    _playerVelX1   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Vel X1" )]->getPointer();
    _playerVelX2   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Vel X2" )]->getPointer();
    _playerPosY1   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y1" )]->getPointer();
    _playerPosY2   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y2" )]->getPointer();
    _playerPosY3   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y3" )]->getPointer();
    _screenPosX    =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X" )]->getPointer();
    _screenPosY    =  (int8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos Y" )]->getPointer();
    _globalTimer   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer"  )]->getPointer();
    _RNG1          =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("RNG1"          )]->getPointer();
    _RNG2          =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("RNG2"          )]->getPointer();
    _RNG3          =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("RNG3"          )]->getPointer();
    _RNG4          =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("RNG4"          )]->getPointer();
    _beetle1Type   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Beetle1 Type" )]->getPointer();
    _beetle2Type   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Beetle2 Type" )]->getPointer();
    _beetle3Type   =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Beetle3 Type" )]->getPointer();
    _money1        =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Money 1"       )]->getPointer();
    _money2        =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Money 2"       )]->getPointer();
    _gameState     =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game State"    )]->getPointer();
    _gameTransitionTimer  =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Transition Timer"    )]->getPointer();

    _levelTransition1  =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level Transition 1"    )]->getPointer();
    _levelTransition2  =  (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level Transition 2"    )]->getPointer();

    _prevLevelTransition1 = *_levelTransition1;
    _prevLevelTransition2 = *_levelTransition2;

    _levelTransitionTimer = 0;
    _levelTransitionState = 0;

    stateUpdatePostHook();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _prevLevelTransition1 = *_levelTransition1;
    _prevLevelTransition2 = *_levelTransition2;

    _emulator->advanceState(input);

    // Start transition
    if (_prevLevelTransition1 == 4 && *_levelTransition1 == 5) _levelTransitionState = 1;

    // goes to red frames
    if (_levelTransitionState == 1 && *_levelTransition2 == 1) _levelTransitionState = 2;

    // Comes back from transition
    if (_levelTransitionState == 2 && *_levelTransition2 == 0)
    {
      _levelTransitionTimer = 0;
      _levelTransitionState = 0;
    } 

    if (_levelTransitionState > 0) _levelTransitionTimer++;
    
    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // hashEngine.Update(&_lowMem[0x0100], 0x03BA - 0x0100);
    hashEngine.Update(&_lowMem[0x03BD], 0x0400 - 0x03BD);

    // if (*_gameState == 0) hashEngine.Update(*_currentStep);
    // if (*_gameState == 137) hashEngine.Update(*_currentStep);
    // if (*_levelTransition1 == 5 || *_levelTransition2 == 0) hashEngine.Update(_currentStep);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _playerPosX = (float)*_playerPosX1 * 256.0 + (float)*_playerPosX2 * 1.0 + (float)*_playerPosX3 / 256.0;
    _playerPosY = (float)*_playerPosY1 * 256.0 + (float)*_playerPosY2 * 1.0 + (float)*_playerPosY3 / 256.0;

    _money = *_money1 * 256 + *_money2;
    _bugWorth = 0;
    if (*_beetle1Type == 1) _bugWorth += 25;
    if (*_beetle1Type == 2) _bugWorth += 50;
    if (*_beetle1Type == 3) _bugWorth += 75;
    if (*_beetle2Type == 1) _bugWorth += 25;
    if (*_beetle2Type == 2) _bugWorth += 50;
    if (*_beetle2Type == 3) _bugWorth += 75;
    if (*_beetle3Type == 1) _bugWorth += 25;
    if (*_beetle3Type == 2) _bugWorth += 50;
    if (*_beetle3Type == 3) _bugWorth += 75;
  }

  __INLINE__ void ruleUpdatePreHook() override 
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointXMagnet.intensity = 0.0;
    _pointXMagnet.pos       = 0.0;

    _pointYMagnet.intensity = 0.0;
    _pointYMagnet.pos       = 0.0;

    _screenYMagnet.intensity = 0.0;
    _screenYMagnet.pos       = 0.0;

    _moneyMagnet = 0;
    _bugWorthMagnet = 0;
    _gameTransitionTimerMagnet = 0;
    _playerFrameMagnet = 0.0;
    _levelTransitionTimerMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _playerDistanceToPointX = std::abs(_pointXMagnet.pos - _playerPosX);
    _playerDistanceToPointY = std::abs(_pointYMagnet.pos - _playerPosY);
    _screenDistanceToPointY = std::abs(_screenYMagnet.pos - (float)*_screenPosY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_playerPosX, sizeof(_playerPosX));
    serializer.pushContiguous(&_playerPosY, sizeof(_playerPosY));
    serializer.pushContiguous(&_prevLevelTransition1, sizeof(_prevLevelTransition1));
    serializer.pushContiguous(&_prevLevelTransition2, sizeof(_prevLevelTransition2));
    serializer.pushContiguous(&_levelTransitionTimer, sizeof(_levelTransitionTimer));
    serializer.pushContiguous(&_levelTransitionState, sizeof(_levelTransitionState));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_playerPosX, sizeof(_playerPosX));
    deserializer.popContiguous(&_playerPosY, sizeof(_playerPosY));
    deserializer.popContiguous(&_prevLevelTransition1, sizeof(_prevLevelTransition1));
    deserializer.popContiguous(&_prevLevelTransition2, sizeof(_prevLevelTransition2));
    deserializer.popContiguous(&_levelTransitionTimer, sizeof(_levelTransitionTimer));
    deserializer.popContiguous(&_levelTransitionState, sizeof(_levelTransitionState));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += -1.0 * _pointXMagnet.intensity * _playerDistanceToPointX;
    reward += -1.0 * _pointYMagnet.intensity * _playerDistanceToPointY;

    // Screen magnet
    reward += -1.0 * _screenYMagnet.intensity * _screenDistanceToPointY;

    // Money magnet
    reward += _moneyMagnet * (float)_money;

    // Bug worth magnet
    reward += _bugWorthMagnet * (float)_bugWorth;

    // Money magnet
    reward += _gameTransitionTimerMagnet * (float)*_gameTransitionTimer;

    // Level transition timer
    reward += _levelTransitionTimerMagnet * (float)_levelTransitionTimer;

    // Player frame magnet
    reward += _playerFrameMagnet * (float)*_playerFrame;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Money:                              %u\n", _money);
    jaffarCommon::logger::log("[J+]  + Player Pos X:                       %.3f\n", _playerPosX);
    jaffarCommon::logger::log("[J+]  + Player Pos Y:                       %.3f\n", _playerPosY);

    if (std::abs(_pointXMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point X Magnet                           Intensity: %.5f, Pos: %3.3f\n", _pointXMagnet.intensity, _pointXMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _playerDistanceToPointX);
    }

    if (std::abs(_pointYMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Y Magnet                           Intensity: %.5f, Pos: %3.3f\n", _pointYMagnet.intensity, _pointYMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _playerDistanceToPointY);
    }

    if (std::abs(_screenYMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Screen Y Magnet                          Intensity: %.5f, Pos: %3.3f\n", _screenYMagnet.intensity, _screenYMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _screenDistanceToPointY);
    }

    if (std::abs(_moneyMagnet) > 0.0f)
     jaffarCommon::logger::log("[J+]  + Money Magnet                                Intensity: %.5f\n", _moneyMagnet);

    if (std::abs(_bugWorthMagnet) > 0.0f)
     jaffarCommon::logger::log("[J+]  + Bug Worth Magnet                            Intensity: %.5f\n", _bugWorthMagnet);

    if (std::abs(_playerFrameMagnet) > 0.0f)
     jaffarCommon::logger::log("[J+]  + Player Frame Magnet                         Intensity: %.5f\n", _playerFrameMagnet);

    if (std::abs(_gameTransitionTimerMagnet) > 0.0f)
     jaffarCommon::logger::log("[J+]  + Game Transition Timer Magnet                Intensity: %.5f\n", _gameTransitionTimerMagnet);

    if (std::abs(_levelTransitionTimerMagnet) > 0.0f)
     jaffarCommon::logger::log("[J+]  + Level Transition Timer Magnet                Intensity: %.5f\n", _levelTransitionTimerMagnet);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_pointXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Point Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_pointYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Screen Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_screenYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Money Magnet")
    {
      auto intensity   = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_moneyMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Bug Worth Magnet")
    {
      auto intensity   = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bugWorthMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Game Transition Timer Magnet")
    {
      auto intensity   = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_gameTransitionTimerMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Level Transition Timer Magnet")
    {
      auto intensity   = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_levelTransitionTimerMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Frame Magnet")
    {
      auto intensity   = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_playerFrameMagnet = intensity; });
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
      float pos       = 0.0; // What is the point of attraction
    };

  pointMagnet_t _pointXMagnet; 
  pointMagnet_t _pointYMagnet; 
  pointMagnet_t _screenYMagnet; 
  float _playerFrameMagnet; 
  float _moneyMagnet;
  float _bugWorthMagnet;
  float _gameTransitionTimerMagnet;
  float _levelTransitionTimerMagnet;

  uint8_t* _playerFrame;
  uint8_t* _playerPosX1 ;
  uint8_t* _playerPosX2 ;
  uint8_t* _playerPosX3 ;
  uint8_t* _playerVelX1 ;
  uint8_t* _playerVelX2 ;
  uint8_t* _playerPosY1 ;
  uint8_t* _playerPosY2 ;
  uint8_t* _playerPosY3 ;
  uint8_t* _screenPosX ;
  int8_t* _screenPosY ;
  uint8_t* _globalTimer ;
  uint8_t* _RNG1        ;
  uint8_t* _RNG2        ;
  uint8_t* _RNG3        ;
  uint8_t* _RNG4        ;
  uint8_t* _beetle1Type;
  uint8_t* _beetle2Type;
  uint8_t* _beetle3Type;
  uint8_t* _money1      ;
  uint8_t* _money2      ;

  // Transition management
  uint8_t _levelTransitionState;
  uint8_t* _levelTransition1;
  uint8_t* _levelTransition2;
  uint8_t _prevLevelTransition1;
  uint8_t _prevLevelTransition2;
  uint16_t _levelTransitionTimer;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;
  float    _playerPosX;
  float    _playerPosY;
  uint16_t _money;
  uint16_t _bugWorth;

  // Game-Specific values
  float _playerDistanceToPointX;
  float _playerDistanceToPointY;
  float _screenDistanceToPointY;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;

  uint8_t* _lowMem;

  uint8_t* _gameState;
  uint8_t* _gameTransitionTimer;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus