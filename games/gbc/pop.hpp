#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>

#define KID_DIRECTION_LEFT 1
#define KID_DIRECTION_RIGHT 0

namespace jaffarPlus
{

namespace games
{

namespace gbc
{

class PrinceOfPersia final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "GBC / Prince Of Persia"; }

  PrinceOfPersia(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM            = _emulator->getProperty("WRAM").pointer;
    _objectAttributeRAM = _emulator->getProperty("OAM").pointer;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|.........|");

    // Registering native game properties
    registerGameProperty("Frame Type", &_workRAM[0x010A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Frame", &_workRAM[0x0139], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Pos X", &_workRAM[0x0009], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Pos Y", &_workRAM[0x0121], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Room", &_workRAM[0x014C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Column", &_workRAM[0x0113], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Actual Kid Room", &_actualKidRoom, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Previous Kid Room", &_previousKidRoom, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid HP", &_workRAM[0x015D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Direction", &_workRAM[0x010F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Action", &_workRAM[0x013A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Column", &_workRAM[0x011E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard HP", &_workRAM[0x0160], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Last Input Step", &_lastInputStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    registerGameProperty("Current Level", &_workRAM[0x0149], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level Transition", &_workRAM[0x014A], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Level 1 Exit Door Open", &_workRAM[0x04FD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Exit State", &_workRAM[0x01BD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 11 Tile 1 State", &_workRAM[0x04ED], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 11 Tile 2 State", &_workRAM[0x04EE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level 11 Tile 3 State", &_workRAM[0x04EF], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _frameType = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Type")]->getPointer();
    _kidFrame  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Frame")]->getPointer();
    _kidPosX   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Pos X")]->getPointer();
    _kidPosY   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Pos Y")]->getPointer();

    _kidRoom      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Room")]->getPointer();
    _kidHP        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Kid HP")]->getPointer();
    _kidDirection = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Direction")]->getPointer();
    _kidAction    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Action")]->getPointer();
    _guardColumn  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Column")]->getPointer();
    _guardHP      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Guard HP")]->getPointer();

    // Initializing values
    updateCharActualPos();
    _currentStep     = 0;
    _lastInputStep   = 0;
    _lastInputIdx    = _nullInputIdx;
    _previousKidRoom = *_kidRoom;
    _actualKidRoom   = *_kidRoom;
  }

  __INLINE__ void updateCharActualPos()
  {
    _kidActualPosX = (float)*_kidPosX;
    _kidActualPosY = (float)*_kidPosY;

    // Compensating for kid's actions
    if (*_kidAction == 100) _kidActualPosY -= 1.0f; // Climbing 1 Against Wall
    if (*_kidAction == 101) _kidActualPosY -= 2.0f; // Climbing 2 Against Wall
    if (*_kidAction == 102) _kidActualPosY -= 2.5f; // Climbing 2 Against Wall
    if (*_kidAction == 107) _kidActualPosY -= 1.0f; // Climbing 1
    if (*_kidAction == 108) _kidActualPosY -= 2.0f; // Climbing 2
    if (*_kidAction == 93) _kidActualPosY -= 3.0f;  // Going up 1
    if (*_kidAction == 94) _kidActualPosY -= 3.0f;  // Going up 1
    if (*_kidAction == 95) _kidActualPosY -= 4.0f;  // Going up 1

    _guardActualPosX = (float)*_guardColumn;

    if (_kidActualPosX > 170) _kidActualPosX -= 170.0;
    if (_guardActualPosX > 170) _guardActualPosX -= 170.0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // If frame type, update previous kid room
    if (*_frameType == 0) _actualKidRoom = _previousKidRoom;

    // If frame type, update previous kid room
    if (*_frameType == 0) _previousKidRoom = *_kidRoom;

    // Running emulator
    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    // Remembering last input
    _lastInputIdx = input;

    // Advancing current step
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    uint8_t                              stateData[65536];
    jaffarCommon::serializer::Contiguous s(stateData);
    _emulator->serializeState(s);
    _emulator->advanceState(_lastInputIdx);
    // _emulator->advanceState(_lastInputIdx);
    // _emulator->advanceState(_lastInputIdx);
    // _emulator->advanceState(_lastInputIdx);
    // _emulator->advanceState(_lastInputIdx);

    // // Proper game information
    //  hashEngine.Update(&_workRAM[0x0000], 0x0100);
    //  hashEngine.Update(&_workRAM[0x0103], 0x0164 - 0x0103);

    // // Frame status (lag, transition)
    //  hashEngine.Update(&_workRAM[0x0DE0], 0x0020 - 0x0DE0);

    hashEngine.Update(&_workRAM[0x0000], 0x0100);
    hashEngine.Update(_workRAM[0x0101]);
    hashEngine.Update(&_workRAM[0x0103], 0x0164 - 0x0103);
    hashEngine.Update(&_workRAM[0x0166], 0x1100 - 0x0166);

    // hashEngine.Update(&_workRAM[0x0000], 0x8000);
    // hashEngine.Update(&_objectAttributeRAM[0x0000], 0xA0);

    jaffarCommon::deserializer::Contiguous d(stateData);
    _emulator->deserializeState(d);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override { updateCharActualPos(); }

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

    _kidDirectionMagnet.room      = 0;
    _kidDirectionMagnet.intensity = 0.0;
    _kidDirectionMagnet.direction = 0;

    _kidHPMagnet.room      = 0;
    _kidHPMagnet.intensity = 0;

    _guardHPMagnet.room      = 0;
    _guardHPMagnet.intensity = 0;

    _lastInputMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _kidDistanceToPointX = std::abs((float)_kidPointXMagnet.pos - _kidActualPosX);
    _kidDistanceToPointY = std::abs((float)_kidPointYMagnet.pos - _kidActualPosY);

    _guardDistanceToPointX = std::abs((float)_guardPointXMagnet.pos - _guardActualPosX);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.push(&_currentStep, sizeof(_currentStep));
    serializer.push(&_previousKidRoom, sizeof(_previousKidRoom));
    serializer.push(&_actualKidRoom, sizeof(_actualKidRoom));
    serializer.push(&_lastInputStep, sizeof(_lastInputStep));
    serializer.push(&_kidActualPosX, sizeof(_kidActualPosX));
    serializer.push(&_kidActualPosY, sizeof(_kidActualPosY));
    serializer.push(&_guardActualPosX, sizeof(_guardActualPosX));
    serializer.push(&_lastInputIdx, sizeof(_lastInputIdx));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.pop(&_currentStep, sizeof(_currentStep));
    deserializer.pop(&_previousKidRoom, sizeof(_previousKidRoom));
    deserializer.pop(&_actualKidRoom, sizeof(_actualKidRoom));
    deserializer.pop(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.pop(&_kidActualPosX, sizeof(_kidActualPosX));
    deserializer.pop(&_kidActualPosY, sizeof(_kidActualPosY));
    deserializer.pop(&_guardActualPosX, sizeof(_guardActualPosX));
    deserializer.pop(&_lastInputIdx, sizeof(_lastInputIdx));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputMagnet * _lastInputStep;

    // Distance to point magnets
    if (_kidPointXMagnet.room == _actualKidRoom && _kidPointXMagnet.room == *_kidRoom) reward += -1.0 * _kidPointXMagnet.intensity * (_kidDistanceToPointX - 255.0);
    if (_kidPointYMagnet.room == _actualKidRoom && _kidPointXMagnet.room == *_kidRoom) reward += -1.0 * _kidPointYMagnet.intensity * (_kidDistanceToPointY - 255.0);

    if (_kidDirectionMagnet.room == _actualKidRoom && _kidPointXMagnet.room == *_kidRoom && *_kidDirection == _kidDirectionMagnet.direction)
      reward += _kidDirectionMagnet.intensity;

    if (*_guardHP > 0)
      if (_guardPointXMagnet.room == _actualKidRoom && _kidPointXMagnet.room == *_kidRoom) reward += -1.0 * _guardPointXMagnet.intensity * (_guardDistanceToPointX - 255.0);

    if (_kidHPMagnet.room == _actualKidRoom && _kidHPMagnet.room == *_kidRoom) reward += _kidHPMagnet.intensity * (3.0 - (float)*_kidHP);
    if (_guardHPMagnet.room == _actualKidRoom && _guardHPMagnet.room == *_kidRoom) reward += _guardHPMagnet.intensity * (9.0 - (float)*_guardHP);

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Frame Type                                 %3u\n", *_frameType);
    jaffarCommon::logger::log("[J+]  + Guard HP                                   %3u\n", *_guardHP);
    jaffarCommon::logger::log("[J+]  + Guard Pos X                                %3u\n", *_guardColumn);

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
        jaffarCommon::logger::log("[J+]  + Kid Direction Magnet                     Intensity: %.5f, Direction: %s\n", _kidDirectionMagnet.intensity,
                                  _kidDirectionMagnet.direction == KID_DIRECTION_LEFT ? "Left" : "Right");
      }

    if (*_kidRoom == _guardPointXMagnet.room)
      if (std::abs(_guardPointXMagnet.intensity) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Guard Point X Magnet                     Intensity: %.5f, Pos: %3.3f\n", _guardPointXMagnet.intensity, _guardPointXMagnet.pos);
        jaffarCommon::logger::log("[J+]    + Distance                               %3.3f (Actual Pos X: %.0f)\n", _guardDistanceToPointX, _guardActualPosX);
      }

    if (_kidHPMagnet.room == *_kidRoom)
      if (std::abs(_kidHPMagnet.intensity) > 0.0f) jaffarCommon::logger::log("[J+]  + Kid HP Magnet                            Intensity: %.5f\n", _kidHPMagnet.intensity);

    if (_guardHPMagnet.room == *_kidRoom)
      if (std::abs(_guardHPMagnet.intensity) > 0.0f) jaffarCommon::logger::log("[J+]  + Guard HP Magnet                          Intensity: %.5f\n", _guardHPMagnet.intensity);

    if (std::abs(_lastInputMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Last Input Magnet                      Intensity: %.5f\n", _lastInputMagnet);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Kid Point X Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_kidPointXMagnet = point1DMagnet_t{.room = room, .intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Kid Point Y Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_kidPointYMagnet = point1DMagnet_t{.room = room, .intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Kid Direction Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto direction = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Direction");
      rule.addAction([=, this]() { this->_kidDirectionMagnet = directionMagnet_t{.room = room, .intensity = intensity, .direction = direction}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Guard Point X Magnet")
    {
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      if (pos != KID_DIRECTION_LEFT && pos != KID_DIRECTION_RIGHT) JAFFAR_THROW_LOGIC("Wrong value for kid direction magnet: %u", pos);
      rule.addAction([=, this]() { this->_guardPointXMagnet = point1DMagnet_t{.room = room, .intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Kid HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      rule.addAction([=, this]() { this->_kidHPMagnet = {.room = room, .intensity = intensity}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Guard HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
      rule.addAction([=, this]() { this->_guardHPMagnet = {.room = room, .intensity = intensity}; });
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

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override { return jaffarCommon::hash::hash_t({0, *_kidAction}); }

  // Datatype to describe a 1D point magnet
  struct point1DMagnet_t
  {
    uint8_t room      = 0;   // The room this magnet affects
    float   intensity = 0.0; // How strong the magnet is
    float   pos       = 0.0; // What is the point of attraction
  };

  struct directionMagnet_t
  {
    uint8_t room      = 0;   // The room this magnet affects
    float   intensity = 0.0; // How strong the magnet is
    uint8_t direction = 0;   // Which direction to face
  };

  struct hpMagnet_t
  {
    uint8_t room      = 0;   // The room this magnet affects
    float   intensity = 0.0; // How strong the magnet is
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  point1DMagnet_t   _kidPointXMagnet;
  point1DMagnet_t   _kidPointYMagnet;
  point1DMagnet_t   _guardPointXMagnet;
  directionMagnet_t _kidDirectionMagnet;
  hpMagnet_t        _kidHPMagnet;
  hpMagnet_t        _guardHPMagnet;
  float             _lastInputMagnet = 0.0;

  uint8_t* _frameType;

  uint8_t* _kidFrame;
  uint8_t* _kidRoom;
  uint8_t* _kidPosX;
  uint8_t* _kidPosY;
  uint8_t* _kidHP;
  uint8_t* _kidAction;
  uint8_t* _kidDirection;

  uint8_t* _guardColumn;
  uint8_t* _guardHP;
  uint8_t  _previousKidRoom;
  uint8_t  _actualKidRoom;

  // Game-Specific values
  float _kidDistanceToPointX;
  float _kidDistanceToPointY;
  float _kidActualPosX;
  float _kidActualPosY;
  float _guardDistanceToPointX;
  float _guardActualPosX;

  // Pointer to emulator's low memory storage
  uint8_t* _workRAM;
  uint8_t* _objectAttributeRAM;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _lastInputIdx;
};

} // namespace gbc

} // namespace games

} // namespace jaffarPlus