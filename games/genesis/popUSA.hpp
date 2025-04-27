#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>

namespace jaffarPlus
{

namespace games
{

namespace genesis
{

class PrinceOfPersiaUSA final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "Genesis / Prince Of Persia (USA)"; }

  PrinceOfPersiaUSA(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    _hashLevelProperties = jaffarCommon::json::getBoolean(config, "Hash Level Properties");
    _hashGuardProperties = jaffarCommon::json::getBoolean(config, "Hash Guard Properties");
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Kid Pos X", &_workRAM[0x001EA4], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Kid Pos Y", &_workRAM[0x001EA2], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Room Pos X", &_workRAM[0x003B18], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Room Pos Y", &_workRAM[0x003B20], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Direction", &_workRAM[0x001FCC], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Frame", &_workRAM[0x001EA8], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid HP", &_workRAM[0x001EB4], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Pending Action", &_workRAM[0x001EAC], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Input", &_workRAM[0x000F46], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Frame Type", &_workRAM[0x000D62], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard Pos X", &_workRAM[0x000D0A], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Guard Pos Y", &_workRAM[0x000C54], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard HP", &_workRAM[0x000D18], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard Direction", &_workRAM[0x000C62], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("General Timer", &_workRAM[0x000D60], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Feather Fall State",  &_workRAM[0x00430A], Property::datatype_t::dt_uint16, Property::endianness_t::little);

    registerGameProperty("Exit Door State", &_workRAM[0x000D6D], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Exit Door State2",     &_workRAM[0x000D7C], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 3 Door 1",            &_workRAM[0x000F1C], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 4 Gate Tile State",   &_workRAM[0x00172E], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 6 Gate 1 State",   &_workRAM[0x0F1C], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 6 Gate 2 State",   &_workRAM[0x0F40], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 6 Loose Tile State",   &_workRAM[0x1D2A], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 7 Potion State",   &_workRAM[0x42B8], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 10 Loose Tile State", &_workRAM[0x1D24], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 11 Gate 1 State", &_workRAM[0x0F10], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 11 Gate 2 State", &_workRAM[0x0F1C], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 16 Loose Tile 1 State", &_workRAM[0x1E14], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // registerGameProperty("Level 16 Gate 1 State", &_workRAM[0x0F10], Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _kidPosX       = (int16_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Pos X")]->getPointer();
    _kidPosY       = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Pos Y")]->getPointer();
    _kidRoomPosX   = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Room Pos X")]->getPointer();
    _kidRoomPosY   = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Room Pos Y")]->getPointer();
    _kidDirection  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Direction")]->getPointer();
    _kidFrame      = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Kid Frame")]->getPointer();
    _kidHP         = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Kid HP")]->getPointer();
    _pendingAction = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Pending Action")]->getPointer();
    _input         = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Input")]->getPointer();
    _frameType     = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Frame Type")]->getPointer();
    _exitDoorState = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Exit Door State")]->getPointer();
    _guardPosX     = (int16_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Pos X")]->getPointer();
    _guardPosY     = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Guard Pos Y")]->getPointer();
    _guardHP       = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Guard HP")]->getPointer();
    _generalTimer  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("General Timer")]->getPointer();

    _frameCounter      = 0;
    _transitionCounter = 0;
    _kidRealPosY       = *_kidPosY;

    // Getting index for a non input
    _nullInputIdx  = _emulator->registerInput("|..|........|");
    _lastInputStep = 0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _frameCounter;

    _emulator->advanceState(input);
    _frameCounter++;

    // If not in transition state, reset transition counter
    if (*_frameType == 0x0000) _transitionCounter = 0;

    // If this is a transition state, advance transition counter
    if (*_frameType == 0xFFFF) _transitionCounter++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_workRAM[0x1F50], 0x2170 - 0x1F50); // Kid stuff
    if (_hashLevelProperties) hashEngine.Update(&_workRAM[0x1000], 0x4000 - 0x1000);
    if (_hashGuardProperties) hashEngine.Update(&_workRAM[0x0D00], 0x0D50 - 0x0D00);
    if (_hashLevelProperties) hashEngine.Update(&_workRAM[0x0F10], 0x0F40 - 0x0F10);

    // Function Stack
    hashEngine.Update(&_workRAM[0x3BD0], 0x3CD0 - 0x3BD0);

    // Video/Frame info
    //  hashEngine.Update(&_workRAM[0xFF40], 0xFFFF - 0xFF40);

    // If transitioning room, hash the generael counter
    if (*_frameType == 0xFFFF) hashEngine.Update(_frameCounter);

    // for (size_t i = 0x0000; i < 0xFFA0; i++) if (i != 0x0FDE && i != 0x0FDF)  hashEngine.Update(_workRAM[i]);
    {
      uint64_t start = 0x0D70;
      uint64_t end   = 0x0F40;
      hashEngine.Update(&_workRAM[start], end - start);
    }
    {
      uint64_t start = 0x0F50;
      uint64_t end   = 0x1000;
      hashEngine.Update(&_workRAM[start], end - start);
    }
    {
      uint64_t start = 0x0020;
      uint64_t end   = 0x0100;
      hashEngine.Update(&_workRAM[start], end - start);
    }

    // Exit Door State
    hashEngine.Update(*_exitDoorState);

    // Frame type
    hashEngine.Update(*_frameType);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _kidRealPosY = *_kidPosY;

    // Climbing
    if (*_kidFrame >= 56 && *_kidFrame <= 69) _kidRealPosY -= 0.5f * (*_kidFrame - 56);
    if (*_kidFrame == 213) _kidRealPosY -= 7.0f;

    // Counteracting jumping
    if (*_kidFrame == 51) _kidRealPosY += 3.0f;
    if (*_kidFrame == 53) _kidRealPosY += 12.0f;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointXMagnet.intensity = 0.0;
    _pointXMagnet.pos       = 0.0;
    _pointYMagnet.intensity = 0.0;
    _pointYMagnet.pos       = 0.0;

    _guardXMagnet.intensity = 0.0;
    _guardXMagnet.pos       = 0.0;
    _guardYMagnet.intensity = 0.0;
    _guardYMagnet.pos       = 0.0;

    _guardHPMagnet   = 0.0;
    _lastInputMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _kidDistanceToPointX = std::abs((float)_pointXMagnet.pos - (float)*_kidPosX);
    _kidDistanceToPointY = std::abs((float)_pointYMagnet.pos - (float)_kidRealPosY);

    _guardDistanceToPointX = std::abs((float)_guardXMagnet.pos - (float)*_guardPosX);
    _guardDistanceToPointY = std::abs((float)_guardYMagnet.pos - (float)*_guardPosY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    // serializer.push(&_workRAM[0xFF50], 0xFFFF - 0xFF50);
    // serializer.push(&_workRAM[0x3B00], 0x3D00 - 0x3B00);
    // serializer.push(&_workRAM[0x4400], 0x4400 - 0x3E00);
    serializer.push(&_frameCounter, sizeof(_frameCounter));
    serializer.push(&_transitionCounter, sizeof(_transitionCounter));
    serializer.push(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    // deserializer.pop(&_workRAM[0xFF50], 0xFFFF - 0xFF50);
    // deserializer.pop(&_workRAM[0x3B00], 0x3D00 - 0x3B00);
    // deserializer.pop(&_workRAM[0x4400], 0x4400 - 0x3E00);
    deserializer.pop(&_frameCounter, sizeof(_frameCounter));
    deserializer.pop(&_transitionCounter, sizeof(_transitionCounter));
    deserializer.pop(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnets
    reward += -1.0 * _pointXMagnet.intensity * _kidDistanceToPointX;
    reward += -1.0 * _pointYMagnet.intensity * _kidDistanceToPointY;

    // Distance to point magnets
    reward += -1.0 * _guardXMagnet.intensity * _guardDistanceToPointX;
    reward += -1.0 * _guardYMagnet.intensity * _guardDistanceToPointY;

    // If transitioning room, reward the passage of time
    if (*_frameType == 0xFFFF) reward += (float)_transitionCounter * 0.01;

    // Rewarding climbing exit
    if (*_kidFrame >= 201 && *_kidFrame <= 212) reward += 212.0 - (float)*_kidFrame;

    // Rewarding guard HP magnet
    reward += _guardHPMagnet * (float)*_guardHP;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputMagnet * _lastInputStep;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+] + Frame Counter (Transition / Last Input)     %u (%u)\n", _frameCounter, _lastInputStep);
    jaffarCommon::logger::log("[J+] + Kid Real Pos Y                              %f\n", _kidRealPosY);

    if (std::abs(_pointXMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point X Magnet                           Intensity: %.5f, Pos: %3.3f\n", _pointXMagnet.intensity, _pointXMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _kidDistanceToPointX);
    }

    if (std::abs(_pointYMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Y Magnet                           Intensity: %.5f, Pos: %3.3f\n", _pointYMagnet.intensity, _pointYMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _kidDistanceToPointY);
    }

    if (std::abs(_guardXMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Guard X Magnet                           Intensity: %.5f, Pos: %3.3f\n", _guardXMagnet.intensity, _guardXMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _guardDistanceToPointX);
    }

    if (std::abs(_guardYMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Guard Y Magnet                           Intensity: %.5f, Pos: %3.3f\n", _guardYMagnet.intensity, _guardYMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _guardDistanceToPointY);
    }

    if (std::abs(_guardHPMagnet) > 0.0f) { jaffarCommon::logger::log("[J+]  + Guard HP Magnet                           Intensity: %.5f\n", _guardHPMagnet); }

    jaffarCommon::logger::log("[J+]  + %2X %2X %2X %2X %2X %2X %2X %2X \n", _workRAM[0], _workRAM[1], _workRAM[2], _workRAM[3], _workRAM[4], _workRAM[5], _workRAM[6], _workRAM[7]);

    // jaffarCommon::logger::log("[J+]  + Exit Door State                             %u\n", *_exitDoorState);

    // jaffarCommon::logger::log("[J+]  + Memview:\n");
    // for (size_t i = 0; i < 16; i++)
    // {
    //  for (size_t j = 0; j < 16; j++) jaffarCommon::logger::log("%2X ", _workRAM[0x000D00 + 16*i + j]);
    //  jaffarCommon::logger::log("\n");
    // }
    // jaffarCommon::logger::log("\n");
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

    if (actionType == "Set Guard X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_guardXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Guard Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_guardYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Guard HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_guardHPMagnet = intensity; });
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

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override { return std::make_pair(0ul, *_kidFrame); }

  // Datatype to describe a 1D point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float pos       = 0.0; // What is the point of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _pointXMagnet;
  pointMagnet_t _pointYMagnet;
  float         _kidDistanceToPointX;
  float         _kidDistanceToPointY;

  pointMagnet_t _guardXMagnet;
  pointMagnet_t _guardYMagnet;
  float         _guardDistanceToPointX;
  float         _guardDistanceToPointY;

  float _guardHPMagnet;
  float _lastInputMagnet;

  // Game-Specific values
  int16_t*  _kidPosX;
  uint16_t* _kidPosY;
  uint16_t* _kidRoomPosX;
  uint16_t* _kidRoomPosY;
  uint16_t* _kidDirection;
  uint16_t* _kidFrame;
  uint16_t* _kidHP;
  uint16_t* _pendingAction;
  uint16_t* _input;
  uint16_t* _frameType;
  int16_t*  _guardPosX;
  uint16_t* _guardPosY;
  uint16_t* _guardHP;
  uint16_t* _generalTimer;
  uint16_t* _exitDoorState;

  float _kidRealPosY;

  // Pointer to emulator's low memory storage
  uint8_t* _workRAM;

  uint16_t _frameCounter;
  uint16_t _transitionCounter;

  bool _hashLevelProperties;
  bool _hashGuardProperties;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace genesis

} // namespace games

} // namespace jaffarPlus