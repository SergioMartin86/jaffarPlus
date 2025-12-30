#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <atomic>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class ExciteBike final : public jaffarPlus::Game
{
public:

  static __INLINE__ std::string getName() { return "NES / Excite Bike"; }

  ExciteBike(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    _bikePosX1           = (uint8_t*)  registerGameProperty("Bike Pos X1"          ,&_lowMem[0x0050], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikePosX2           = (uint8_t*)  registerGameProperty("Bike Pos X2"          ,&_lowMem[0x0394], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _intraLoopAdvance    = (uint8_t*)  registerGameProperty("Intra Loop Advance"   ,&_lowMem[0x00ED], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _loopsRemaining      = (uint8_t*)  registerGameProperty("Loops Remaining"      ,&_lowMem[0x0057], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _currentLoop         = (uint8_t*)  registerGameProperty("Current Loop"         ,&_lowMem[0x03A4], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _raceOverFlag        = (uint8_t*)  registerGameProperty("Race Over Flag"       ,&_lowMem[0x0052], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeMoving          = (uint8_t*)  registerGameProperty("Bike Moving"          ,&_lowMem[0x000E], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeAngle           = (uint8_t*)  registerGameProperty("Bike Angle"           ,&_lowMem[0x00AC], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeAirMode         = (uint8_t*)  registerGameProperty("Bike Air Mode"        ,&_lowMem[0x00B0], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeVelX2           = (uint8_t*)  registerGameProperty("Bike Vel X2"          ,&_lowMem[0x0094], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeVelX1           = (uint8_t*)  registerGameProperty("Bike Vel X1"          ,&_lowMem[0x0090], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeEngineTemp      = (uint8_t*)  registerGameProperty("Bike Engine Temp"     ,&_lowMem[0x03B6], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeVelZ            = (uint8_t*)  registerGameProperty("Bike Vel Z"           ,&_lowMem[0x00DC], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikePosZ1           = (uint8_t*)  registerGameProperty("Bike Pos Z1"          ,&_lowMem[0x0070], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikePosZ2           = (uint8_t*)  registerGameProperty("Bike Pos Z2"          ,&_lowMem[0x00B8], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikePosY            = (uint8_t*)  registerGameProperty("Bike Pos Y"           ,&_lowMem[0x008C], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeVelY1           = (uint8_t*)  registerGameProperty("Bike Vel Y1"          ,&_lowMem[0x0270], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeVelY2           = (uint8_t*)  registerGameProperty("Bike Vel Y2"          ,&_lowMem[0x0274], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeFlightMode1     = (uint8_t*)  registerGameProperty("Bike Flight Mode 1"   ,&_lowMem[0x00F6], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeFlightMode2     = (uint8_t*)  registerGameProperty("Bike Flight Mode 2"   ,&_lowMem[0x0380], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _bikeFlightMode3     = (uint8_t*)  registerGameProperty("Bike Flight Mode 3"   ,&_lowMem[0x0384], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _gameCycle           = (uint8_t*)  registerGameProperty("Game Cycle"           ,&_lowMem[0x004C], Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    _currBlockX          = (uint8_t*)  registerGameProperty("Curr Block X"         ,&_lowMem[0x004E], Property::datatype_t::dt_uint8,  Property::endianness_t::little);

    registerGameProperty("Block X Transitions"  , &_blockXTransitions, Property::datatype_t::dt_uint8,  Property::endianness_t::little);
    registerGameProperty("Bike Pos X"           , &_bikePosX, Property::datatype_t::dt_float32,  Property::endianness_t::little);
    registerGameProperty("Cur Vel"              , &_curVel, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Max Vel"              , &_maxVel, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Prev Block X"         , &_prevBlockX, Property::datatype_t::dt_uint8,  Property::endianness_t::little);

    _currentStep = 0;
    _nullInputIdx      = _emulator->registerInput("|..|........|");
    _lastInput = _nullInputIdx;
    _blockXTransitions = 0;
    _curVel = 0;
    _maxVel = 0;

    stateUpdatePostHook();
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;

    _lastInput = input;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // Storage for hash calculation
    hashEngine.Update(*_bikePosX1         );
    hashEngine.Update(*_bikePosX2         );
    hashEngine.Update(*_intraLoopAdvance  );
    hashEngine.Update(*_loopsRemaining    );
    hashEngine.Update(*_currentLoop       );
    hashEngine.Update(*_raceOverFlag      );
    hashEngine.Update(*_bikeMoving        );
    if (*_bikeAirMode > 0) hashEngine.Update(*_bikeAngle         );
    hashEngine.Update(*_bikeAirMode       );
    hashEngine.Update(*_bikeVelX2         );
    hashEngine.Update(*_bikeVelX1         );
    // hashEngine.Update(*_bikeEngineTemp    );
    hashEngine.Update(*_bikeVelZ          );
    hashEngine.Update(*_bikePosZ1         );
    hashEngine.Update(*_bikePosZ2         );
    hashEngine.Update(*_bikePosY          );
    hashEngine.Update(*_bikeVelY1         );
    hashEngine.Update(*_bikeVelY2         );
    hashEngine.Update(*_bikeFlightMode1   );
    hashEngine.Update(*_bikeFlightMode2   );
    hashEngine.Update(*_bikeFlightMode3   );
    hashEngine.Update(*_gameCycle         );
    hashEngine.Update(*_currBlockX        );
    // hashEngine.Update(*_prevBlockX        );
    // hashEngine.Update(_blockXTransitions );
    // hashEngine.Update(_curVel          );
    // hashEngine.Update(_maxVel          );

    //  for (size_t i = 0; i < 0x800; i++)
    //   if (i != 0x0014)
    //   if (i != 0x0015)
    //   if (i != 0x005C)
    //   if (i != 0x00FC)
    //   if (i != 0x00FE)
    //   if (i != 0x0020)
    //   if (i != 0x003F)
    //   if (i != 0x004C)
    //   if (i != 0x006A)
    //   if (i != 0x006B)
    //   if (i != 0x031B)
    //   if (i != 0x03A9)
    //   if (i != 0x03B5)
    //   if (i != 0x03D7)
    //   if (i != 0x03DF)
    //    hashEngine.Update(_emu->_baseMem[i]);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    if (_prevBlockX != *_currBlockX) _blockXTransitions++;
    _prevBlockX = *_currBlockX;

    // _emu->_baseMem[0x0081] = 255;
    // _emu->_baseMem[0x0082] = 255;
    // _emu->_baseMem[0x0083] = 255;

    _curVel =  256.0 * (uint16_t)*_bikeVelX1 + (uint16_t)*_bikeVelX2;
    if (_curVel > _maxVel) _maxVel = _curVel;

    _bikePosX = (float)_blockXTransitions * 256.0f + (float)*_bikePosX1 + (float)*_bikePosX2 / 256.0;
    
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    if (_bypassEmulatorState == true)
    {
     serializer.push(&_lowMem[0x0000], 0x005D0);
     serializer.push(&_lowMem[0x07E0], 0x00020);
    }

     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_lastInput, sizeof(_lastInput));
     serializer.push(&_curVel, sizeof(_curVel));
     serializer.push(&_maxVel, sizeof(_maxVel));
     serializer.push(&_blockXTransitions, sizeof(_blockXTransitions));
     serializer.push(&_prevBlockX, sizeof(_prevBlockX));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    if (_bypassEmulatorState == true)
    {
     deserializer.pop(&_lowMem[0x0000], 0x005D0);
     deserializer.pop(&_lowMem[0x07E0], 0x00020);
    }

     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_lastInput, sizeof(_lastInput));
     deserializer.pop(&_curVel, sizeof(_curVel));
     deserializer.pop(&_maxVel, sizeof(_maxVel));
     deserializer.pop(&_blockXTransitions, sizeof(_blockXTransitions));
     deserializer.pop(&_prevBlockX, sizeof(_prevBlockX));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Reward is how much we advanced
    reward += _bikePosX;

    // Returning reward
    return reward;
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
  }

    // Function to report what all the possible input that the game might require
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    return {};
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:                     %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Game Cycle:                       %02u\n", *_gameCycle);
    jaffarCommon::logger::log("[J+]  + Current / Remaining Loop:         %02u/%02u\n", *_currentLoop, *_loopsRemaining);
    jaffarCommon::logger::log("[J+]  + Loop Advance:                     %02u\n", *_intraLoopAdvance);
    jaffarCommon::logger::log("[J+]  + Block X:                          Curr: %02u, Prev: %02u, Count: %02u\n", *_currBlockX, _prevBlockX, _blockXTransitions);
    jaffarCommon::logger::log("[J+]  + Bike Pos X:                       %.3f (%02u, %02u)\n", _bikePosX, *_bikePosX1, *_bikePosX2);
    jaffarCommon::logger::log("[J+]  + Bike Pos Y:                       %02u\n", *_bikePosY);
    jaffarCommon::logger::log("[J+]  + Bike Pos Z:                       %02u (%02u)\n", *_bikePosZ1, *_bikePosZ2);
    jaffarCommon::logger::log("[J+]  + Bike Vel:                         %05u (Max: %05u)\n", _curVel, _maxVel);
    jaffarCommon::logger::log("[J+]  + Bike Vel X:                       %02u (%02u)\n", *_bikeVelX1, *_bikeVelX2);
    jaffarCommon::logger::log("[J+]  + Bike Vel Y:                       %02u (%02u)\n", *_bikeVelY1, *_bikeVelY2);
    jaffarCommon::logger::log("[J+]  + Bike Vel Z:                       %02u\n", *_bikeVelZ);
    jaffarCommon::logger::log("[J+]  + Bike Air Mode:                    %02u\n", *_bikeAirMode);
    jaffarCommon::logger::log("[J+]  + Bike Flight:                      %02u %02u %02u\n", *_bikeFlightMode1, *_bikeFlightMode2, *_bikeFlightMode3);
    jaffarCommon::logger::log("[J+]  + Bike Engine Temp:                 %02u\n", *_bikeEngineTemp);
    jaffarCommon::logger::log("[J+]  + Bike Angle:                       %02u\n", *_bikeAngle);
    jaffarCommon::logger::log("[J+]  + Bike Moving:                      %02u\n", *_bikeMoving);
    jaffarCommon::logger::log("[J+]  + Race Over Flag:                   %02u\n", *_raceOverFlag);

  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;


    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    return {0, 0};
  }


  __INLINE__ void playerPrintCommands() const override
  {
    // jaffarCommon::logger::log("[J+] t: Print Initial Info\n");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
     return false;
  };

  uint8_t* _lowMem;
  uint16_t _currentStep;
  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _lastInput;

  uint8_t* _bikePosX1          ;
  uint8_t* _bikePosX2          ;
  uint8_t* _intraLoopAdvance   ;
  uint8_t* _loopsRemaining     ;
  uint8_t* _currentLoop        ;
  uint8_t* _raceOverFlag       ;
  uint8_t* _bikeMoving         ;
  uint8_t* _bikeAngle          ;
  uint8_t* _bikeAirMode        ;
  uint8_t* _bikeVelX2          ;
  uint8_t* _bikeVelX1          ;
  uint8_t* _bikeEngineTemp     ;
  uint8_t* _bikeVelZ           ;
  uint8_t* _bikePosZ1          ;
  uint8_t* _bikePosZ2          ;
  uint8_t* _bikePosY           ;
  uint8_t* _bikeVelY1          ;
  uint8_t* _bikeVelY2          ;
  uint8_t* _bikeFlightMode1    ;
  uint8_t* _bikeFlightMode2    ;
  uint8_t* _bikeFlightMode3    ;
  uint8_t* _gameCycle          ;
  uint8_t* _currBlockX         ;


  uint8_t _blockXTransitions    ;
  uint16_t _curVel            ;
  uint16_t _maxVel            ;
  float _bikePosX;
  uint8_t _prevBlockX         ;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
