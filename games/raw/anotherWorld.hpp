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

  static __INLINE__ std::string getName() { return "A2600 / AnotherWorld"; }

  AnotherWorld(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    _ram = (int16_t*)_emulator->getProperty("RAM").pointer;
    _threadsData     = (int16_t*)_emulator->getProperty("Threads Data").pointer;
    _threadsDataSize = _emulator->getProperty("Threads Data").size;
    _scriptStackData = (int16_t*)_emulator->getProperty("Script Stack Data").pointer;
    _scriptStackSize = _emulator->getProperty("Script Stack Data").size;

    // Getting emulator save state size
    _tempStorageSize = _emulator->getStateSize();
    _tempStorage = (uint8_t*)malloc(_tempStorageSize);

    // Registering native game properties
    registerGameProperty("Lester Swim State",    &_ram[0xE5], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Pos X",         &_ram[0x01], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Pos Y",         &_ram[0x02], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Room",          &_ram[0x66], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Action",        &_ram[0xFA], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester State",         &_ram[0x63], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Game Script State",    &_ram[0x2A], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Game Animation State", &_ram[0x0F], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Dead State",    &_ram[0x03], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Momentum1",     &_ram[0x15], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Momentum2",     &_ram[0x16], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Momentum3",     &_ram[0x17], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Has Gun",       &_ram[0x0A], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Gun Ammo",      &_ram[0x06], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Gun Load",      &_ram[0x0F], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Lester Direction",     &_ram[0x63], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Alien State",          &_ram[0x6B], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Alien Room",           &_ram[0x6A], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Alien Pos X",          &_ram[0x68], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Game Timer",           &_ram[0x31], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Elevator Pos Y",       &_ram[0x14], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Fumes State",          &_ram[0xE8], Property::datatype_t::dt_int16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _lesterSwimState         = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Swim State")]->getPointer();
    _lesterPosX              = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Pos X")]->getPointer();
    _lesterPosY              = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Pos Y")]->getPointer();
    _lesterRoom              = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Room")]->getPointer();
    _lesterAction            = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Action")]->getPointer();
    _lesterState             = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester State")]->getPointer();
    _gameScriptState         = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Game Script State")]->getPointer();
    _gameAnimState           = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Game Animation State")]->getPointer();
    _lesterDeadState         = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Dead State")]->getPointer();
    _lesterMomentum1         = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Momentum1")]->getPointer();
    _lesterMomentum2         = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Momentum2")]->getPointer();
    _lesterMomentum3         = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Momentum3")]->getPointer();
    _lesterHasGun            = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Has Gun")]->getPointer();
    _lesterGunAmmo           = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Gun Ammo")]->getPointer();
    _lesterGunLoad           = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Gun Load")]->getPointer();
    _lesterDirection         = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Lester Direction")]->getPointer();
    _alienState              = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Alien State")]->getPointer();
    _alienRoom               = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Alien Room")]->getPointer();
    _alienPosX               = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Alien Pos X")]->getPointer();
    _gameTimer               = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Game Timer")]->getPointer();
    _elevatorPosY            = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Elevator Pos Y")]->getPointer();
    _fumesState              = (int16_t *)_propertyMap[jaffarCommon::hash::hashString("Fumes State")]->getPointer();
  }

  __INLINE__ void advanceStateImpl(const std::string &input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
   {
    // Storing current state
     jaffarCommon::serializer::Contiguous s(_tempStorage, _tempStorageSize);
     _emulator->serializeState(s);

     // Advancing emulator state
     _emulator->advanceState(".....");

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
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) {}

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Distance to point magnet
    reward += 256.0 * *_score + *_subDistance;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override {}

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  // Temporary storage for the emulator state for calculating hash
  uint8_t* _tempStorage;
  size_t _tempStorageSize;

  uint8_t *_score;
  uint8_t *_subDistance;

  // Pointer to emulator's low memory storage
  int16_t *_ram;
  int16_t* _threadsData;
  size_t _threadsDataSize;
  int16_t* _scriptStackData;
  size_t _scriptStackSize;

  int16_t* _lesterSwimState;
  int16_t* _lesterPosX     ;
  int16_t* _lesterPosY     ;
  int16_t* _lesterRoom     ;
  int16_t* _lesterAction   ;
  int16_t* _lesterState    ;
  int16_t* _gameScriptState;
  int16_t* _gameAnimState  ;
  int16_t* _lesterDeadState;
  int16_t* _lesterMomentum1;
  int16_t* _lesterMomentum2;
  int16_t* _lesterMomentum3;
  int16_t* _lesterHasGun   ;
  int16_t* _lesterGunAmmo  ;
  int16_t* _lesterGunLoad  ;
  int16_t* _lesterDirection;
  int16_t* _alienState     ;
  int16_t* _alienRoom      ;
  int16_t* _alienPosX      ;
  int16_t* _gameTimer      ;
  int16_t* _elevatorPosY   ;
  int16_t* _fumesState     ;
};

} // namespace raw

} // namespace games

} // namespace jaffarPlus