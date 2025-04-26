#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace raw
{

class AnotherWorld final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "RAW / AnotherWorld"; }

  AnotherWorld(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    _ram             = (int16_t *)_emulator->getProperty("RAM").pointer;
    _threadsData     = (int16_t *)_emulator->getProperty("Threads Data").pointer;
    _threadsDataSize = _emulator->getProperty("Threads Data").size;
    _scriptStackData = (int16_t *)_emulator->getProperty("Script Stack Data").pointer;
    _scriptStackSize = _emulator->getProperty("Script Stack Data").size;

    // Getting emulator save state size
    _tempStorageSize = _emulator->getStateSize();
    _tempStorage     = (uint8_t *)malloc(_tempStorageSize);

    // Registering native game properties
    registerGameProperty("Lester Swim State", &_ram[0xE5], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Pos X", &_ram[0x01], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Pos Y", &_ram[0x02], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Room", &_ram[0x66], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Action", &_ram[0xFA], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester State", &_ram[0x63], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Game Script State", &_ram[0x2A], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Game Script State 2", &_ram[0x2B], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Game Animation State", &_ram[0x0F], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Dead State", &_ram[0x03], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Momentum1", &_ram[0x15], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Momentum2", &_ram[0x16], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Momentum3", &_ram[0x17], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Has Gun", &_ram[0x0A], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Gun Ammo", &_ram[0x06], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Gun Load", &_ram[0x0F], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Direction", &_ram[0x63], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Alien State", &_ram[0x6B], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Alien Room", &_ram[0x6A], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Alien Pos X", &_ram[0x68], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Game Timer", &_ram[0x31], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Elevator Pos Y", &_ram[0x14], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Fumes State", &_ram[0xE8], Property::datatype_t::dt_int16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _lesterSwimState = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Swim State")]->getPointer();
    _lesterPosX      = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Pos X")]->getPointer();
    _lesterPosY      = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Pos Y")]->getPointer();
    _lesterRoom      = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Room")]->getPointer();
    _lesterAction    = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Action")]->getPointer();
    _lesterState     = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester State")]->getPointer();
    _gameScriptState = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Game Script State")]->getPointer();
    _gameAnimState   = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Game Animation State")]->getPointer();
    _lesterDeadState = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Dead State")]->getPointer();
    _lesterMomentum1 = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Momentum1")]->getPointer();
    _lesterMomentum2 = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Momentum2")]->getPointer();
    _lesterMomentum3 = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Momentum3")]->getPointer();
    _lesterHasGun    = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Has Gun")]->getPointer();
    _lesterGunAmmo   = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Gun Ammo")]->getPointer();
    _lesterGunLoad   = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Gun Load")]->getPointer();
    _lesterDirection = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Direction")]->getPointer();
    _alienState      = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Alien State")]->getPointer();
    _alienRoom       = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Alien Room")]->getPointer();
    _alienPosX       = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Alien Pos X")]->getPointer();
    _gameTimer       = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Game Timer")]->getPointer();
    _elevatorPosY    = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Elevator Pos Y")]->getPointer();
    _fumesState      = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Fumes State")]->getPointer();

    for (size_t i = 0; i < VM_NUM_VARIABLES; i++)
      {
        auto propertyName = std::string("VM Value [") + std::to_string(i) + std::string("]");
        registerGameProperty(propertyName, &_ram[i], Property::datatype_t::dt_int16, Property::endianness_t::little);
      }

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("......");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    // Storing current state
    jaffarCommon::serializer::Contiguous s(_tempStorage, _tempStorageSize);
    _emulator->serializeState(s);

    //  Advancing emulator state
    _emulator->advanceState(_nullInputIdx);

    // Hashing state now
    hashEngine.Update((uint8_t *)_threadsData, _threadsDataSize);
    hashEngine.Update((uint8_t *)_scriptStackData, _scriptStackSize);

    for (int i = 0x00; i < 0x20; i++) hashEngine.Update(_ram[i]);
    for (int i = 0x60; i < 0x70; i++) hashEngine.Update(_ram[i]);

    // Recovering emulator state
    jaffarCommon::deserializer::Contiguous d(_tempStorage, _tempStorageSize);
    _emulator->deserializeState(d);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override { _lesterFullMomentum = std::max(*_lesterMomentum1, *_lesterMomentum2) + std::abs(*_lesterMomentum3); }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _lesterHorizontalMagnet.room      = 0;
    _lesterHorizontalMagnet.intensity = 0.0;
    _lesterHorizontalMagnet.center    = 0.0;

    _lesterVerticalMagnet.room      = 0;
    _lesterVerticalMagnet.intensity = 0.0;
    _lesterVerticalMagnet.center    = 0.0;

    _alienHorizontalMagnet.room      = 0;
    _alienHorizontalMagnet.intensity = 0.0;
    _alienHorizontalMagnet.center    = 0.0;

    _elevatorVerticalMagnet.room      = 0;
    _elevatorVerticalMagnet.intensity = 0.0;
    _elevatorVerticalMagnet.center    = 0.0;

    _lesterGunLoadMagnet         = 0;
    _lesterAngularMomentumMagnet = 0;
  }

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override { serializer.push(&_lesterFullMomentum, sizeof(_lesterFullMomentum)); }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) { deserializer.pop(&_lesterFullMomentum, sizeof(_lesterFullMomentum)); }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    if (_lesterHorizontalMagnet.room == *_lesterRoom)
      reward += _lesterHorizontalMagnet.intensity * (512.0f - 1.0f * std::abs((float)_lesterHorizontalMagnet.center - (float)*_lesterPosX));
    if (_lesterVerticalMagnet.room == *_lesterRoom)
      reward += _lesterVerticalMagnet.intensity * (512.0f - 1.0f * std::abs((float)_lesterVerticalMagnet.center - (float)*_lesterPosY));
    if (_alienHorizontalMagnet.room == *_alienRoom)
      reward += _alienHorizontalMagnet.intensity * (512.0f - 1.0f * std::abs((float)_alienHorizontalMagnet.center - (float)*_alienPosX));
    if (_elevatorVerticalMagnet.room == *_lesterRoom)
      reward += _elevatorVerticalMagnet.intensity * (512.0f - 1.0f * std::abs((float)_elevatorVerticalMagnet.center - (float)*_elevatorPosY));

    // Rewarding angular momentum (for lvl02)
    reward += _lesterAngularMomentumMagnet * _lesterFullMomentum;
    reward += _lesterGunLoadMagnet * (float)*_lesterGunLoad;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+] + RAM Contents\n");
    for (size_t i = 0; i < 16; i++)
      {
        jaffarCommon::logger::log("[J+]    ");
        for (size_t j = 0; j < 16; j++) jaffarCommon::logger::log(" %04X", (uint16_t)_ram[i * 16 + j]);
        jaffarCommon::logger::log("\n");
      }

    if (_lesterHorizontalMagnet.room == *_lesterRoom)
      if (std::abs(_lesterHorizontalMagnet.intensity) > 0.0f)
        jaffarCommon::logger::log("[J+]  + Lester Horizontal Magnet                 Room: %u, Intensity: %.5f, Center: %3.3f\n",
                                  _lesterHorizontalMagnet.room,
                                  _lesterHorizontalMagnet.intensity,
                                  _lesterHorizontalMagnet.center);

    if (_lesterVerticalMagnet.room == *_lesterRoom)
      if (std::abs(_lesterVerticalMagnet.intensity) > 0.0f)
        jaffarCommon::logger::log("[J+]  + Lester Vertical Magnet                   Room: %u, Intensity: %.5f, Center: %3.3f\n",
                                  _lesterVerticalMagnet.room,
                                  _lesterVerticalMagnet.intensity,
                                  _lesterVerticalMagnet.center);

    if (_alienHorizontalMagnet.room == *_alienRoom)
      if (std::abs(_alienHorizontalMagnet.intensity) > 0.0f)
        jaffarCommon::logger::log("[J+]  + Alien Horizontal Magnet                  Room: %u, Intensity: %.5f, Center: %3.3f\n",
                                  _alienHorizontalMagnet.room,
                                  _alienHorizontalMagnet.intensity,
                                  _alienHorizontalMagnet.center);

    if (_elevatorVerticalMagnet.room == *_lesterRoom)
      if (std::abs(_elevatorVerticalMagnet.intensity) > 0.0f)
        jaffarCommon::logger::log("[J+]  + Elevator Vertical Magnet                 Room: %u, Intensity: %.5f, Center: %3.3f\n",
                                  _elevatorVerticalMagnet.room,
                                  _elevatorVerticalMagnet.intensity,
                                  _elevatorVerticalMagnet.center);

    if (std::abs(_lesterGunLoadMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Lester Gun Load Magnet                   Intensity: %.5f\n", _lesterGunLoadMagnet);

    if (std::abs(_lesterAngularMomentumMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Angular Momentum Magnet                  Intensity: %.5f\n", _lesterAngularMomentumMagnet);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Lester Horizontal Magnet")
      {
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
        rule.addAction([=, this]() { this->_lesterHorizontalMagnet = pointMagnet_t{.room = room, .intensity = intensity, .center = center}; });
        recognizedActionType = true;
    }

    if (actionType == "Set Lester Vertical Magnet")
      {
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
        rule.addAction([=, this]() { this->_lesterVerticalMagnet = pointMagnet_t{.room = room, .intensity = intensity, .center = center}; });
        recognizedActionType = true;
    }

    if (actionType == "Set Alien Horizontal Magnet")
      {
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
        rule.addAction([=, this]() { this->_alienHorizontalMagnet = pointMagnet_t{.room = room, .intensity = intensity, .center = center}; });
        recognizedActionType = true;
    }

    if (actionType == "Set Elevator Vertical Magnet")
      {
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto center    = jaffarCommon::json::getNumber<float>(actionJs, "Center");
        rule.addAction([=, this]() { this->_elevatorVerticalMagnet = pointMagnet_t{.room = room, .intensity = intensity, .center = center}; });
        recognizedActionType = true;
    }

    if (actionType == "Set Lester Gun Load Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        rule.addAction([=, this]() { this->_lesterGunLoadMagnet = intensity; });
        recognizedActionType = true;
    }

    if (actionType == "Set Angular Momentum Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        rule.addAction([=, this]() { this->_lesterAngularMomentumMagnet = intensity; });
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
    uint8_t room      = 0;
    float   intensity = 0.0; // How strong the magnet is
    float   center    = 0.0; // What is the point of attraction
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _lesterHorizontalMagnet;
  pointMagnet_t _lesterVerticalMagnet;
  pointMagnet_t _alienHorizontalMagnet;
  pointMagnet_t _elevatorVerticalMagnet;

  float _lesterAngularMomentumMagnet;
  float _lesterGunLoadMagnet;

  // Temporary storage for the emulator state for calculating hash
  uint8_t *_tempStorage;
  size_t   _tempStorageSize;

  // Pointer to emulator's low memory storage
  int16_t *_ram;
  int16_t *_threadsData;
  size_t   _threadsDataSize;
  int16_t *_scriptStackData;
  size_t   _scriptStackSize;

  int16_t *_lesterSwimState;
  int16_t *_lesterPosX;
  int16_t *_lesterPosY;
  int16_t *_lesterRoom;
  int16_t *_lesterAction;
  int16_t *_lesterState;
  int16_t *_gameScriptState;
  int16_t *_gameAnimState;
  int16_t *_lesterDeadState;
  int16_t *_lesterMomentum1;
  int16_t *_lesterMomentum2;
  int16_t *_lesterMomentum3;
  int16_t *_lesterHasGun;
  int16_t *_lesterGunAmmo;
  int16_t *_lesterGunLoad;
  int16_t *_lesterDirection;
  int16_t *_alienState;
  int16_t *_alienRoom;
  int16_t *_alienPosX;
  int16_t *_gameTimer;
  int16_t *_elevatorPosY;
  int16_t *_fumesState;

  float _lesterFullMomentum;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace raw

} // namespace games

} // namespace jaffarPlus