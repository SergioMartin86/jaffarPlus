#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

#define KID_DIRECTION_LEFT 1
#define KID_DIRECTION_RIGHT 2

namespace jaffarPlus
{

namespace games
{

namespace gamegear
{

class PrinceOfPersia final : public jaffarPlus::Game
{
  public:

  enum transitionState_t { normal, exiting, entering };

  static __INLINE__ std::string getName() { return "Game Gear / Prince Of Persia"; }

  PrinceOfPersia(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Getting index for a non input 
    _nullInputIdx = _emulator->registerInput("|..|.......|");

    // Registering native game properties
    registerGameProperty("Global Timer",       &_workRAM[0x0299], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid HP",             &_workRAM[0x0292], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Pos X",          &_workRAM[0x0401], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Pos Y",          &_workRAM[0x0402], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Direction",      &_workRAM[0x0403], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Direction 2",    &_workRAM[0x0404], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Animation",      &_workRAM[0x040A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Frame",          &_workRAM[0x0407], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Room",           &_workRAM[0x0255], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Has Sword",      &_workRAM[0x0295], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Slowfall Potion",    &_workRAM[0x0291], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    
    registerGameProperty("Transition Timer 1",   &_workRAM[0x025E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Transition Timer 2",   &_workRAM[0x0249], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Transition Timer 3",   &_workRAM[0x0252], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Transition Timer 4",   &_workRAM[0x0254], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Guard HP",             &_workRAM[0x0294], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Pos X",          &_workRAM[0x04E1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Pos Y",          &_workRAM[0x04E2], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Animation Frame",      &_workRAM[0x1FDF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Exit Door Open",       &_workRAM[0x0F67], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Local Door State",     &_workRAM[0x04C5], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Level 4 Door State",   &_workRAM[0x1D90], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 9 Door State",   &_workRAM[0x1D31], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 10 Tile State",  &_workRAM[0x1CA6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    
    registerGameProperty("Current Step",   &_currentStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Last Input Step", &_lastInputStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _kidPosX    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Kid Pos X")]->getPointer();
    _kidPosY    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Kid Pos Y")]->getPointer();
    _kidHP      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Kid HP")]->getPointer();
    _kidDirection = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Kid Direction")]->getPointer();
    _kidFrame  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Kid Frame")]->getPointer();
    _kidRoom    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Kid Room")]->getPointer();
    _globalTimer  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _transitionTimer1  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Transition Timer 1")]->getPointer();
    _transitionTimer2  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Transition Timer 2")]->getPointer();
    _transitionTimer3  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Transition Timer 3")]->getPointer();
    _transitionTimer4  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Transition Timer 4")]->getPointer();
    _animationFrame  = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Animation Frame")]->getPointer();

    _guardPosX    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Guard Pos X")]->getPointer();
    _guardPosY    = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Guard Pos Y")]->getPointer();
    _guardHP      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Guard HP")]->getPointer();

    updateCharActualPos();
    _previousKidRoom = *_kidRoom;
    _transitionState = transitionState_t::normal;
    _transitionProgress = 0;

    // Initializing values
    _currentStep = 0;
    _lastInputStep = 0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override 
  {
    // hashEngine.Update(&_workRAM[0x0000], 0x0250 - 0x0000);

    // for (size_t i = 0x0250; i < 0x300; i++) 
    // if (i != 0x0299) 
    // if (i != 0x0254) 
    //  hashEngine.Update(_workRAM[i]);

    // hashEngine.Update(&_workRAM[0x0300], 0x2000 - 0x0300);

    hashEngine.Update(&_workRAM[0x0000], 0x024A - 0x0000);
    hashEngine.Update(&_workRAM[0x0255], 0x0298 - 0x0255);
    hashEngine.Update(&_workRAM[0x0301], 0x0500 - 0x0301);
    // hashEngine.Update(&_workRAM[0x1000], 0x2000 - 0x1000);

    // // Transition timer 3 is frame type
    hashEngine.Update(*_transitionTimer1);
    hashEngine.Update(*_transitionTimer2);
    hashEngine.Update(*_transitionTimer3);
    hashEngine.Update(*_animationFrame);

    ///// For screen transitions
    // Always compute transition timer 1
    //if (_transitionState != transitionState_t::normal) hashEngine.Update(*_globalTimer);
    hashEngine.Update(_transitionProgress);
  }

  __INLINE__ void updateCharActualPos()
  {
    _kidActualPosX = (float)*_kidPosX;
    _kidActualPosY = (float)*_kidPosY;
    if (_kidActualPosX > 230) _kidActualPosX -= 255.0;
    if (_kidActualPosY > 230) _kidActualPosY -= 255.0;

    _guardActualPosX = (float)*_guardPosX;
    if (_guardActualPosX > 230) _guardActualPosX -= 255.0;
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    // Managing transition states
    updateCharActualPos();
    if (*_kidRoom != _previousKidRoom) { _transitionState = transitionState_t::exiting; _transitionProgress = 0; }
    if (_transitionState == transitionState_t::exiting)  if (*_transitionTimer2 == 1) { _transitionState = transitionState_t::entering; } 
    if (_transitionState == transitionState_t::entering) if (*_transitionTimer3 == 1) { _transitionState = transitionState_t::normal; }
    if (_transitionState != transitionState_t::normal) _transitionProgress++;

    _previousKidRoom = *_kidRoom;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _kidPointXMagnet.room      = 0;
    _kidPointXMagnet.intensity = 0.0;
    _kidPointXMagnet.pos       = 0.0;

    _kidPointYMagnet.room      = 0;
    _kidPointYMagnet.intensity = 0.0;
    _kidPointYMagnet.pos       = 0.0;

    _guardPointXMagnet.room      = 0;
    _guardPointXMagnet.intensity = 0.0;
    _guardPointXMagnet.pos       = 0.0;

    _kidDirectionMagnet.room = 0;
    _kidDirectionMagnet.intensity = 0.0;
    _kidDirectionMagnet.direction = 0;

    _kidHPMagnet.room = 0;
    _kidHPMagnet.intensity = 0;

    _guardHPMagnet.room = 0;
    _guardHPMagnet.intensity = 0;

    _lastInputMagnet              = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _kidDistanceToPointX = std::abs((float)_kidPointXMagnet.pos - _kidActualPosX);
    _kidDistanceToPointY = std::abs((float)_kidPointYMagnet.pos - _kidActualPosY);

    _guardDistanceToPointX = std::abs((float)_guardPointXMagnet.pos - _guardActualPosX);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
   {
    serializer.push(&_transitionState, sizeof(_transitionState));
    serializer.push(&_previousKidRoom, sizeof(_previousKidRoom));
    serializer.push(&_transitionProgress, sizeof(_transitionProgress));
    serializer.push(&_kidActualPosX, sizeof(_kidActualPosX));
    serializer.push(&_kidActualPosY, sizeof(_kidActualPosY));
    serializer.push(&_guardActualPosX, sizeof(_guardActualPosX));
    serializer.push(&_currentStep, sizeof(_currentStep));
    serializer.push(&_lastInputStep, sizeof(_lastInputStep));
   }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
   {
    deserializer.pop(&_transitionState, sizeof(_transitionState));
    deserializer.pop(&_previousKidRoom, sizeof(_previousKidRoom));
    deserializer.pop(&_transitionProgress, sizeof(_transitionProgress));
    deserializer.pop(&_kidActualPosX, sizeof(_kidActualPosX));
    deserializer.pop(&_kidActualPosY, sizeof(_kidActualPosY));
    deserializer.pop(&_guardActualPosX, sizeof(_guardActualPosX));
    deserializer.pop(&_currentStep, sizeof(_currentStep));
    deserializer.pop(&_lastInputStep, sizeof(_lastInputStep));
   }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputMagnet * _lastInputStep;

    // Distance to point magnets
    if (_kidPointXMagnet.room == *_kidRoom) reward += -1.0 * _kidPointXMagnet.intensity * (_kidDistanceToPointX - 270.0);
    if (_kidPointYMagnet.room == *_kidRoom) reward += -1.0 * _kidPointYMagnet.intensity * (_kidDistanceToPointY - 270.0);

    if (_kidDirectionMagnet.room == *_kidRoom && *_kidDirection == _kidDirectionMagnet.direction) reward += _kidDirectionMagnet.intensity;

    if (*_guardHP > 0) if (_guardPointXMagnet.room == *_kidRoom) reward += -1.0 * _guardPointXMagnet.intensity * (_guardDistanceToPointX - 255.0);

    if(_kidHPMagnet.room == *_kidRoom)   reward += _kidHPMagnet.intensity * (3.0 - (float)*_kidHP);
    if(_guardHPMagnet.room == *_kidRoom) reward += _guardHPMagnet.intensity * (- 12.0 + (float)*_guardHP);
    
    // Rewarding room transitions
    if (_transitionState != transitionState_t::normal) reward += -255.0 + 1.0 * _transitionProgress;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Animation Frame                        %3u\n", *_animationFrame);
    jaffarCommon::logger::log("[J+]  + Transition State                           ");
    if (_transitionState == transitionState_t::normal) jaffarCommon::logger::log("Normal");
    if (_transitionState == transitionState_t::exiting) jaffarCommon::logger::log("Exiting");
    if (_transitionState == transitionState_t::entering) jaffarCommon::logger::log("Entering");
    jaffarCommon::logger::log("\n");
    jaffarCommon::logger::log("[J+]  + Transition Timers                          [ %3u, %3u, %3u, %3u ]\n", *_transitionTimer1, *_transitionTimer2, *_transitionTimer3, *_transitionTimer4);
    jaffarCommon::logger::log("[J+]  + Transition Progress                        %3u\n", _transitionProgress);
    jaffarCommon::logger::log("[J+]  + Guard HP                                   %3u\n", *_guardHP);
    jaffarCommon::logger::log("[J+]  + Guard Pos X                                %3u\n", *_guardPosX);

    if (*_kidRoom == _kidPointXMagnet.room)
    if (std::abs(_kidPointXMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Kid Point X Magnet                       Intensity: %.5f, Pos: %3.3f\n", _kidPointXMagnet.intensity, _kidPointXMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f (Actual Pos X: %.0f)\n", _kidDistanceToPointX, _kidActualPosX);
    }

    if (*_kidRoom == _kidPointYMagnet.room)
    if (std::abs(_kidPointYMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Kid Point Y Magnet                       Intensity: %.5f, Pos: %3.3f\n", _kidPointYMagnet.intensity, _kidPointYMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f (Actual Pos Y: %.0f)\n", _kidDistanceToPointY, _kidActualPosY);
    }
    
    if (*_kidRoom == _kidDirectionMagnet.room)
    if (std::abs(_kidDirectionMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Kid Direction Magnet                     Intensity: %.5f, Direction: %s\n", _kidDirectionMagnet.intensity, _kidDirectionMagnet.direction == KID_DIRECTION_LEFT ? "Left" : "Right");
    }

    if (*_kidRoom == _guardPointXMagnet.room)
    if (std::abs(_guardPointXMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Guard Point X Magnet                     Intensity: %.5f, Pos: %3.3f\n", _guardPointXMagnet.intensity, _guardPointXMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f (Actual Pos X: %.0f)\n", _guardDistanceToPointX, _guardActualPosX);
    }

    if(_kidHPMagnet.room == *_kidRoom)
    if (std::abs(_kidHPMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Kid HP Magnet                            Intensity: %.5f\n", _kidHPMagnet);

    if(_guardHPMagnet.room == *_kidRoom)
    if (std::abs(_guardHPMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Guard HP Magnet                          Intensity: %.5f\n", _guardHPMagnet);

    if (std::abs(_lastInputMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Last Input Magnet                      Intensity: %.5f\n", _lastInputMagnet);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Kid Point X Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_kidPointXMagnet = point1DMagnet_t{.room = room, .intensity = intensity, .pos = pos }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Kid Point Y Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_kidPointYMagnet = point1DMagnet_t{ .room = room, .intensity = intensity, .pos = pos }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Kid Direction Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto direction = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Direction");
      rule.addAction([=, this]() { this->_kidDirectionMagnet = directionMagnet_t{ .room = room, .intensity = intensity, .direction = direction }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Guard Point X Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      if (pos != KID_DIRECTION_LEFT && pos != KID_DIRECTION_RIGHT) JAFFAR_THROW_LOGIC("Wrong value for kid direction magnet: %u", pos);
      rule.addAction([=, this]() { this->_guardPointXMagnet = point1DMagnet_t{.room = room, .intensity = intensity, .pos = pos }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Kid HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      rule.addAction([=, this]() { this->_kidHPMagnet = { .room = room, .intensity = intensity }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Guard HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      rule.addAction([=, this]() { this->_guardHPMagnet = { .room = room, .intensity = intensity }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Last Input Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_lastInputMagnet = intensity; };
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

  // Datatype to describe a 1D point magnet
  struct point1DMagnet_t
  {
    uint8_t room    = 0; // The room this magnet affects
    float intensity = 0.0; // How strong the magnet is
    float pos       = 0.0; // What is the point of attraction
  };

  struct directionMagnet_t
  {
    uint8_t room    = 0; // The room this magnet affects
    float intensity = 0.0; // How strong the magnet is
    uint8_t direction = 0; // Which direction to face
  };

  struct hpMagnet_t
  {
    uint8_t room    = 0; // The room this magnet affects
    float intensity = 0.0; // How strong the magnet is
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  point1DMagnet_t _kidPointXMagnet;
  point1DMagnet_t _kidPointYMagnet;
  point1DMagnet_t _guardPointXMagnet;
  directionMagnet_t _kidDirectionMagnet;
  hpMagnet_t _kidHPMagnet;
  hpMagnet_t _guardHPMagnet;
  float         _lastInputMagnet         = 0.0;

  uint8_t *_kidFrame;
  uint8_t *_kidRoom;
  uint8_t *_kidPosX;
  uint8_t *_kidPosY;
  uint8_t *_kidHP;
  uint8_t *_kidDirection;

  uint8_t *_guardHP;
  uint8_t *_guardPosX;
  uint8_t *_guardPosY;

  uint8_t* _globalTimer;
  uint8_t* _transitionTimer1;
  uint8_t* _transitionTimer2;
  uint8_t* _transitionTimer3;
  uint8_t* _transitionTimer4;

  uint8_t* _animationFrame;

  // Game-Specific values
  float _kidDistanceToPointX;
  float _kidDistanceToPointY;
  float _kidActualPosX;
  float _kidActualPosY;

  float _guardDistanceToPointX;
  float _guardActualPosX;

  // Pointer to emulator's low memory storage
  uint8_t *_workRAM;

  // A variable to remembering room transition states
  transitionState_t _transitionState;
  uint8_t _transitionProgress;
  uint8_t _previousKidRoom;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace gamegear

} // namespace games

} // namespace jaffarPlus