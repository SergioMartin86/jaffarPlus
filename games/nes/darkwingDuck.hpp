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

class DarkwingDuck final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Darkwing Duck"; }

  DarkwingDuck(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Getting emulator name (for runtime use)
    _traceFilePath = jaffarCommon::json::getString(config, "Trace File Path");

    // Loading trace
    if (_traceFilePath != "")
    {
      _useTrace = true;
      std::string traceData;
      bool        status = jaffarCommon::file::loadStringFromFile(traceData, _traceFilePath.c_str());
      if (status == false) JAFFAR_THROW_LOGIC("Could not find/read trace file: %s\n", _traceFilePath.c_str());

      std::istringstream f(traceData);
      std::string line;
      while (std::getline(f, line))
      {
        auto coordinates = jaffarCommon::string::split(line, ' ');
        float x = std::atof(coordinates[0].c_str());
        float y = std::atof(coordinates[1].c_str());
        _trace.push_back(traceEntry_t{
          .x = x,
          .y = y,
        });
      }
    }
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    
    registerGameProperty("Game Mode"             , &_lowMem[0x00F2], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Global Timer"          , &_lowMem[0x0092], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Duck Pos X1"           , &_lowMem[0x006F], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Duck Pos X2"           , &_lowMem[0x006E], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Duck Pos Y1"           , &_lowMem[0x0440], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Duck Frame Cycle"      , &_lowMem[0x0430], Property::datatype_t::dt_uint8, Property::endianness_t::little); 
    registerGameProperty("Grabbed Weapon"        , &_lowMem[0x0054], Property::datatype_t::dt_uint8, Property::endianness_t::little); 

    _gameMode                        = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Game Mode"       )]->getPointer();
    _globalTimer                     = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Global Timer"    )]->getPointer();
    _duckPosX1                       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Duck Pos X1"     )]->getPointer();
    _duckPosX2                       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Duck Pos X2"     )]->getPointer();
    _duckPosY1                       = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Duck Pos Y1"     )]->getPointer();
    _duckFrameCycle                  = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Duck Frame Cycle")]->getPointer();
    _grabbedWeapon                   = (uint8_t*) _propertyMap[jaffarCommon::hash::hashString("Grabbed Weapon"  )]->getPointer();

    registerGameProperty("Duck Pos X"               , &_duckPosX, Property::datatype_t::dt_float32, Property::endianness_t::little); 
    registerGameProperty("Duck Pos Y"               , &_duckPosY, Property::datatype_t::dt_float32, Property::endianness_t::little); 

    stateUpdatePostHook();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");

    _inputU   = _emulator->registerInput("|..|U.......|");
    _inputD   = _emulator->registerInput("|..|.D......|");
    _inputL   = _emulator->registerInput("|..|..L.....|");
    _inputR   = _emulator->registerInput("|..|...R....|");
    _inputA   = _emulator->registerInput("|..|.......A|");
    _inputB   = _emulator->registerInput("|..|......B.|");
    _inputUL  = _emulator->registerInput("|..|U.L.....|");
    _inputUR  = _emulator->registerInput("|..|U..R....|");
    _inputUA  = _emulator->registerInput("|..|U......A|");
    _inputUB  = _emulator->registerInput("|..|U.....B.|");
    _inputDL  = _emulator->registerInput("|..|.DL.....|");
    _inputDR  = _emulator->registerInput("|..|.D.R....|");
    _inputDA  = _emulator->registerInput("|..|.D.....A|");
    _inputDB  = _emulator->registerInput("|..|.D....B.|");
    _inputAL  = _emulator->registerInput("|..|..L....A|");
    _inputBL  = _emulator->registerInput("|..|..L...B.|");
    _inputAR  = _emulator->registerInput("|..|...R...A|");
    _inputBR  = _emulator->registerInput("|..|...R..B.|");
    _inputULA = _emulator->registerInput("|..|U.L....A|");
    _inputURA = _emulator->registerInput("|..|U..R...A|");
    _inputUBA = _emulator->registerInput("|..|U.....BA|");
    _inputDLA = _emulator->registerInput("|..|.DL....A|");
    _inputDRA = _emulator->registerInput("|..|.D.R...A|");
    _inputDBA = _emulator->registerInput("|..|.D....BA|");
    _inputBLA = _emulator->registerInput("|..|..L...BA|");
    _inputBRA = _emulator->registerInput("|..|...R..BA|");
    _inputULB = _emulator->registerInput("|..|U.L...B.|");
    _inputURB = _emulator->registerInput("|..|U..R..B.|");
    _inputDLB = _emulator->registerInput("|..|.DL...B.|");
    _inputDRB = _emulator->registerInput("|..|.D.R..B.|");

    _inputBA  = _emulator->registerInput("|..|......BA|");
    _inputUBA = _emulator->registerInput("|..|U.....BA|");
    _inputLRA = _emulator->registerInput("|..|..LR...A|");
    _inputRBA = _emulator->registerInput("|..|...R..BA|");
    _inputLBA = _emulator->registerInput("|..|..L...BA|");
    _inputLR  = _emulator->registerInput("|..|..LR....|");

    _inputARS = _emulator->registerInput("|..|...RS..A|");
    _inputALS = _emulator->registerInput("|..|..L.S..A|");
    _inputRS  = _emulator->registerInput("|..|...RS...|");
    _inputLS  = _emulator->registerInput("|..|..L.S...|");

    _inputSelect  = _emulator->registerInput("|..|...s....|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _emulator->advanceState(input);
    
    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;
  }

  __INLINE__ void calculateHashValues(MetroHash128& hashEngine) const
  {
    hashEngine.Update(*_gameMode           );
    hashEngine.Update(*_duckPosX1          );
    hashEngine.Update(*_duckPosX2          );
    hashEngine.Update(*_duckPosY1          );
    hashEngine.Update(*_duckFrameCycle     );
    hashEngine.Update(*_grabbedWeapon      );

    // Duck Stats Array
    hashEngine.Update(&_lowMem[0x0400], 0x140);
    hashEngine.Update(&_lowMem[0x0040], 0x30);

    if (std::abs(_lastInputStepReward) > 0.0f) hashEngine.Update(_lastInputStep);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    calculateHashValues(hashEngine);

    // uint8_t _saveBuffer[1024*1024];
    // jaffarCommon::serializer::Contiguous s(_saveBuffer);
    // _emulator->serializeState(s);
    // _emulator->advanceState(_nullInputIdx);
    // _emulator->advanceState(_nullInputIdx);

    // calculateHashValues(hashEngine);

    // jaffarCommon::deserializer::Contiguous d(_saveBuffer);
    // _emulator->deserializeState(d);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
     _duckPosX = (float)*_duckPosX1 * 256.0f + (float)*_duckPosX2;
     _duckPosY = (float)*_duckPosY1;
  }

  __INLINE__ void ruleUpdatePreHook() override 
  {
    _pointMagnet.intensity = 0.0; 
    _pointMagnet.x = 0.0; 
    _pointMagnet.y = 0.0; 

    _traceMagnet.intensityX = 0.0;
    _traceMagnet.intensityY = 0.0;
    _traceMagnet.offset = 0;

    _lastInputStepReward = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _duckDistanceToPointX = std::abs(_pointMagnet.x - _duckPosX);
    _duckDistanceToPointY = std::abs(_pointMagnet.y - _duckPosY);
    _duckDistanceToPoint  =  sqrtf(_duckDistanceToPointX * _duckDistanceToPointX + _duckDistanceToPointY * _duckDistanceToPointY);

    // Updating trace stuff
    if (_useTrace == true)
    {
      _traceStep = (uint16_t) std::max(std::min( (int)_currentStep + _traceMagnet.offset, (int) _trace.size() - 1), 0);
      _traceTargetX = _trace[_traceStep].x;
      _traceTargetY = _trace[_traceStep].y;

      // Updating distance to trace point
      _traceDistanceX = std::abs(_traceTargetX - _duckPosX);
      _traceDistanceY = std::abs(_traceTargetY - _duckPosY);
      _traceDistance  = sqrtf(_traceDistanceX * _traceDistanceX + _traceDistanceY * _traceDistanceY);
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // If trace is used, compute its magnet's effect
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityX * _traceDistanceX;
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensityY * _traceDistanceY;

    // Evaluating point magnet
    reward += -1.0 * _pointMagnet.intensity * _duckDistanceToPoint;

    // float jumpPreparedness = 0.0;
    // if (*batmanAction == 8 && *batmanJumpTimer > 1) jumpPreparedness = 8.0 - (float)*batmanJumpTimer;
    // if (*batmanAction == 13) jumpPreparedness = 8.0;
    // float actualPosY = batmanPosY - jumpPreparedness;
    // float diff = std::abs(magnets.batmanVerticalMagnet.center - actualPosY);
    // float weightedDiffY = magnets.batmanVerticalMagnet.intensity * diff;
    // reward -= weightedDiffY;

    // reward += _batmanDistanceToBossMagnet * std::abs( (float)*_batmanPosX2 - (float)*_bossX );
    // reward += _batmanHPMagnet * (float)*_batmanHP;
    // reward += _bossHPMagnet * (float)*_bossHP;
    // reward += _batmanAmmoMagnet * (float)*_batmanAmmo;
    
    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputStepReward * _lastInputStep;

    // Returning reward
    return reward;
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    if (*_gameMode == 0x0001) allowedInputSet.insert(allowedInputSet.end(), { _inputSelect, _inputR, _inputL, _inputU, _inputA, _inputD, _inputDA, _inputUL, _inputUR, _inputUA, _inputDL, _inputDR, _inputLR, _inputAL, _inputAR, _inputULA, _inputURA, _inputLRA });
    if (*_gameMode == 0x000E) allowedInputSet.insert(allowedInputSet.end(), { _inputSelect, _inputA,  _inputR, _inputL, _inputD, _inputU, _inputAR, _inputAL, _inputUA, _inputURA, _inputULA });
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Global Timer:                     %02u\n", *_globalTimer);
    jaffarCommon::logger::log("[J+]  + Game Mode:                        %02u\n", *_gameMode);
    jaffarCommon::logger::log("[J+]  + Duck Frame Cycle:                 %02u\n", *_duckFrameCycle);
    jaffarCommon::logger::log("[J+]  + Duck Pos X:                       %f: %02u + %02u\n", _duckPosX, *_duckPosX1, *_duckPosX2);
    jaffarCommon::logger::log("[J+]  + Duck Pos Y:                       %02u\n", *_duckPosY1);
    jaffarCommon::logger::log("[J+]  + Grabbed Weapon:                   %02u\n", *_grabbedWeapon);

    jaffarCommon::logger::log("[J+]  + Rule Status: ");

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _duckDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _duckDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _duckDistanceToPoint);
    }

    if (std::abs(_lastInputStepReward) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Last Input Magnet                        Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _lastInputStepReward);
      jaffarCommon::logger::log("[J+]    + Last Input Step                        %04u\n", _lastInputStep);
    }

    if (_useTrace == true)
    {
      if (std::abs(_traceMagnet.intensityX) > 0.0f || std::abs(_traceMagnet.intensityY) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Trace Magnet                             Intensity: (X: %.5f, Y: %.5f), Step: %u (%+1u), X: %3.3f, Y: %3.3f\n", _traceMagnet.intensityX, _traceMagnet.intensityY, _traceStep, _traceMagnet.offset, _traceTargetX, _traceTargetY);
        jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _traceDistanceX);
        jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _traceDistanceY);
        jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _traceDistance);
      }
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Trace Magnet")
    {
      if (_useTrace == false) JAFFAR_THROW_LOGIC("Specified Trace Magnet, but no trace file was provided.");
      auto intensityX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity X");
      auto intensityY = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Y");
      auto offset    = jaffarCommon::json::getNumber<int>(actionJs, "Offset");
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{.intensityX = intensityX, .intensityY = intensityY, .offset = offset }; });
      recognizedActionType = true;
    }

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x       = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y       = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Last Input Step Reward")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_lastInputStepReward = intensity; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    // return jaffarCommon::hash::hash_t({0, *_duckAction});
    return jaffarCommon::hash::hash_t({0, 0});
  }

  // Datatype to describe a point magnet
  bool _isDumpingTrace = false;
  std::string _traceDumpString;

  __INLINE__ void playerPrintCommands() const override
  {
    jaffarCommon::logger::log("[J+] t: start/stop trace dumping (%s)\n", _isDumpingTrace ? "On" : "Off");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
    // If storing a trace, do it here
    if (_isDumpingTrace == true) _traceDumpString += std::to_string(_duckPosX) + std::string(" ") + std::to_string(_duckPosY) + std::string("\n");

     if (command == 't')
     {
      if (_isDumpingTrace == false)
      {
        _isDumpingTrace = true;
        _traceDumpString = "";
        return false;
      }
      else
      {
        const std::string dumpOutputFile = "jaffar.trace";
        jaffarCommon::logger::log("[J+] Dumping trace to file: '%s'", dumpOutputFile.c_str());
        jaffarCommon::file::saveStringToFile(_traceDumpString, dumpOutputFile);
        _isDumpingTrace = false;
        return true;
      }
     }

     return false;
  };


  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the point of attraction X
    float y         = 0.0; // What is the point of attraction X
  };
  pointMagnet_t _pointMagnet;
  pointMagnet_t _screenMagnet;

  float _duckDistanceToPointX;
  float _duckDistanceToPointY;
  float _duckDistanceToPoint;

  // Datatype to describe a magnet
  struct weaponMagnet_t {
    float intensity = 0.0; // How strong the magnet is
    uint8_t value = 0;  // Specifies the weapon number
  };

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;

  uint8_t* _lowMem;

  float _duckPosX;
  float _duckPosY;
  
  struct traceMagnet_t
  {
    float intensityX = 0.0; // How strong the magnet is on X
    float intensityY = 0.0; // How strong the magnet is on Y
    int offset      = 0; // Which entry (step) to look at wrt the current emulation step
  };

  // Reward for the last time an input was made (for early termination)
  float _lastInputStepReward;
  traceMagnet_t _traceMagnet;

  // Whether we use a trace
  bool _useTrace = false;

  // Location of the trace file
  std::string _traceFilePath;

  // Trace contents
  struct traceEntry_t
  {
    float x;
    float y;
  };
  std::vector<traceEntry_t> _trace;

  // Current trace target
  uint16_t _traceStep;
  float _traceTargetX;
  float _traceTargetY;
  float _traceDistanceX;
  float _traceDistanceY;
  float _traceDistance;
  
  // Possible inputs
  InputSet::inputIndex_t _inputU  ;
  InputSet::inputIndex_t _inputD  ;
  InputSet::inputIndex_t _inputL  ;
  InputSet::inputIndex_t _inputR  ;
  InputSet::inputIndex_t _inputA  ;
  InputSet::inputIndex_t _inputB  ;
  InputSet::inputIndex_t _inputUL ;
  InputSet::inputIndex_t _inputUR ;
  InputSet::inputIndex_t _inputUA ;
  InputSet::inputIndex_t _inputUB ;
  InputSet::inputIndex_t _inputDL ;
  InputSet::inputIndex_t _inputDR ;
  InputSet::inputIndex_t _inputDA ;
  InputSet::inputIndex_t _inputDB ;
  InputSet::inputIndex_t _inputAL ;
  InputSet::inputIndex_t _inputBL ;
  InputSet::inputIndex_t _inputAR ;
  InputSet::inputIndex_t _inputBR ;
  InputSet::inputIndex_t _inputULA;
  InputSet::inputIndex_t _inputURA;
  InputSet::inputIndex_t _inputUBA;
  InputSet::inputIndex_t _inputDLA;
  InputSet::inputIndex_t _inputDRA;
  InputSet::inputIndex_t _inputDBA;
  InputSet::inputIndex_t _inputBLA;
  InputSet::inputIndex_t _inputBRA;
  InputSet::inputIndex_t _inputULB;
  InputSet::inputIndex_t _inputURB;
  InputSet::inputIndex_t _inputDLB;
  InputSet::inputIndex_t _inputDRB;
  InputSet::inputIndex_t _inputLRA;
  InputSet::inputIndex_t _inputBA;
  InputSet::inputIndex_t _inputRBA;
  InputSet::inputIndex_t _inputLBA;
  InputSet::inputIndex_t _inputLR;

  InputSet::inputIndex_t _inputARS ;
  InputSet::inputIndex_t _inputALS ;
  InputSet::inputIndex_t _inputRS  ;
  InputSet::inputIndex_t _inputLS  ;
  InputSet::inputIndex_t _inputSelect;

  uint8_t* _gameMode         ;
  uint8_t* _globalTimer      ;
  uint8_t* _duckPosX1        ;
  uint8_t* _duckPosX2        ;
  uint8_t* _duckPosY1         ;
  uint8_t* _duckFrameCycle   ;
  uint8_t* _grabbedWeapon    ;

  // Game values

  uint16_t _currentStep;
  uint16_t _lastInputStep;
};


} // namespace nes

} // namespace games

} // namespace jaffarPlus
