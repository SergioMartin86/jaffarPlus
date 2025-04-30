#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/string.hpp>
#include <jaffarCommon/file.hpp>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class MarbleMadness final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Marble Madness"; }

  MarbleMadness(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Parsing configuration
    _lastInputStepReward = jaffarCommon::json::getNumber<float>(config, "Last Input Step Reward");

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
        _trace.push_back(traceEntry_t{.x = x, .y = y});
      }
    }
  }

private:

  struct traceEntry_t
  {
    float x;
    float y;
  };

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Game Timer",           &_lowMem[0x000A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game Cycle",           &_lowMem[0x07DD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Win Flag",             &_lowMem[0x008B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble State",         &_lowMem[0x0019], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Flags",         &_lowMem[0x0408], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Pos X1",        &_lowMem[0x0398], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Pos X2",        &_lowMem[0x0390], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Pos X3",        &_lowMem[0x0388], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Pos Y1",        &_lowMem[0x03B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Pos Y2",        &_lowMem[0x03A8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Pos Y3",        &_lowMem[0x03A0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Pos Z1",        &_lowMem[0x03C0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Airtime",       &_lowMem[0x03C8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Vel X",         &_lowMem[0x03D0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Vel Y",         &_lowMem[0x03E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Dead Flag",     &_lowMem[0x0400], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Marble Surface Angle", &_lowMem[0x0428], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Marble Pos X",        &_marblePosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Marble Pos Y",        &_marblePosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Marble Pos Z",        &_marblePosZ, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _gameTimer             = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Timer"           )]->getPointer();
    _gameCycle             = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Cycle"           )]->getPointer();
    _winFlag               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Win Flag"             )]->getPointer();
    _marbleState           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble State"         )]->getPointer();
    _marbleFlags           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Flags"         )]->getPointer();
    _marblePosX1           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Pos X1"        )]->getPointer();
    _marblePosX2           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Pos X2"        )]->getPointer();
    _marblePosX3           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Pos X3"        )]->getPointer();
    _marblePosY1           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Pos Y1"        )]->getPointer();
    _marblePosY2           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Pos Y2"        )]->getPointer();
    _marblePosY3           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Pos Y3"        )]->getPointer();
    _marblePosZ1           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Pos Z1"        )]->getPointer();
    _marbleAirtime         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Airtime"       )]->getPointer();
    _marbleVelX            = (int8_t*) _propertyMap[jaffarCommon::hash::hashString("Marble Vel X"         )]->getPointer();
    _marbleVelY            = (int8_t*) _propertyMap[jaffarCommon::hash::hashString("Marble Vel Y"         )]->getPointer();
    _marbleDeadFlag        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Dead Flag"     )]->getPointer();
    _marbleSurfaceAngle    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Marble Surface Angle" )]->getPointer();

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x0001], 0x0018);
    hashEngine.Update(&_lowMem[0x001C], 0x0050);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;
    _stopProcessingReward  = false;

    _traceMagnet.intensity = 0.0;
    _traceMagnet.offset = 0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    _marblePosX = (float)*_marblePosX1 * 256.0f + (float)*_marblePosX2 + (float)*_marblePosX3 / 256.0f;
    _marblePosY = (float)*_marblePosY1 * 256.0f + (float)*_marblePosY2 + (float)*_marblePosY3 / 256.0f;
    _marblePosZ = (float)*_marblePosZ1;
   
    _surfaceAngleX = 1.0; _surfaceAngleY = 1.0;
    if (*_marbleSurfaceAngle == 3) { _surfaceAngleX = 1.0; _surfaceAngleY = 1.0; }
    if (*_marbleSurfaceAngle == 0) { _surfaceAngleX = 0.5; _surfaceAngleY = 0.5; }
    if (*_marbleSurfaceAngle == 1) { _surfaceAngleX = 0.5; _surfaceAngleY = 1.0; }
    if (*_marbleSurfaceAngle == 4) { _surfaceAngleX = 0.75; _surfaceAngleY = 1.0; }
    if (*_marbleSurfaceAngle == 2) { _surfaceAngleX = 1.0; _surfaceAngleY = 0.5; }
    if (*_marbleSurfaceAngle == 255) { _surfaceAngleX = -5.0; _surfaceAngleY = -5.0; }
    if (*_marbleSurfaceAngle == 128) { _surfaceAngleX = -5.0; _surfaceAngleY = -5.0; }
   
    // Updating distance to user-defined point
    _marbleDistanceToPointX = std::abs(_pointMagnet.x - _marblePosX);
    _marbleDistanceToPointY = std::abs(_pointMagnet.y - _marblePosY);
    _marbleDistanceToPoint  = sqrtf(_marbleDistanceToPointX * _marbleDistanceToPointX + _marbleDistanceToPointY * _marbleDistanceToPointY);

    // Updating trace stuff
    if (_useTrace == true)
    {
      _traceStep = (uint16_t) std::max(std::min( (int)_currentStep + _traceMagnet.offset, (int) _trace.size() - 1), 0);
      _traceTargetX = _trace[_traceStep].x;
      _traceTargetY = _trace[_traceStep].y;

      // Updating distance to trace point
      _traceDistanceX = std::abs(_traceTargetX - _marblePosX);
      _traceDistanceY = std::abs(_traceTargetY - _marblePosY);
      _traceDistance  = sqrtf(_traceDistanceX * _traceDistanceX + _traceDistanceY * _traceDistanceY);
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputStepReward * _lastInputStep;

    // If this is a win state, then evaluate only w.r.t. how long since the last input
    if (_stopProcessingReward) return reward;

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _marbleDistanceToPoint;

    // If trace is used, compute its magnet's effect
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.intensity * _traceDistance;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[Jaffar]  + Game Timer:             %02u\n", *_gameTimer);
    jaffarCommon::logger::log("[Jaffar]  + Game Cycle:             %02u\n", *_gameCycle);
    jaffarCommon::logger::log("[Jaffar]  + Marble State:           %02u\n", *_marbleState);
    jaffarCommon::logger::log("[Jaffar]  + Marble Flags:           %02u\n", *_marbleFlags);
    jaffarCommon::logger::log("[Jaffar]  + Marble Pos X:           %f, (%02u, %02u, %02u)\n", _marblePosX, *_marblePosX1, *_marblePosX2, *_marblePosX3);
    jaffarCommon::logger::log("[Jaffar]  + Marble Pos Y:           %f, (%02u, %02u, %02u)\n", _marblePosY, *_marblePosY1, *_marblePosY2, *_marblePosY3);
    jaffarCommon::logger::log("[Jaffar]  + Marble Pos Z:           %f\n", _marblePosZ);
    jaffarCommon::logger::log("[Jaffar]  + Marble Vel X:           %02d\n", *_marbleVelX);
    jaffarCommon::logger::log("[Jaffar]  + Marble Vel Y:           %02d\n", *_marbleVelY);
    jaffarCommon::logger::log("[Jaffar]  + Marble Airtime:         %02u\n", *_marbleAirtime);
    jaffarCommon::logger::log("[Jaffar]  + Marble Surface Angle:   %02u\n", *_marbleSurfaceAngle);
    
    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _marbleDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _marbleDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _marbleDistanceToPoint);
    }

    if (_useTrace == true)
    {
      if (std::abs(_traceMagnet.intensity) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Trace Magnet                             Intensity: %.5f, Step: %u, X: %3.3f, Y: %3.3f\n", _traceMagnet.intensity, _traceStep, _traceTargetX, _traceTargetY);
        jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _traceDistanceX);
        jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _traceDistanceY);
        jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _traceDistance);
      }
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Trace Magnet")
    {
      if (_useTrace == false) JAFFAR_THROW_LOGIC("Specified Trace Magnet, but no trace file was provided.");
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto offset    = jaffarCommon::json::getNumber<int>(actionJs, "Offset");
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{.intensity = intensity, .offset = offset }; });
      recognizedActionType = true;
    }

    if (actionType == "Stop Processing Reward")
    {
      rule.addAction([this]() { _stopProcessingReward = true; });
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
  struct traceMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    int offset      = 0; // Which entry (step) to look at wrt the current emulation step
  };

  traceMagnet_t _traceMagnet;

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the x point of attraction
    float y         = 0.0; // What is the y point of attraction
  };


  bool _isDumpingTrace = false;
  std::string _traceDumpString;

  __INLINE__ void playerPrintCommands() const override
  {
    jaffarCommon::logger::log("[J+] t: start/stop trace dumping (%s)\n", _isDumpingTrace ? "On" : "Off");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
    // If storing a trace, do it here
    if (_isDumpingTrace == true) _traceDumpString += std::to_string(_marblePosX) + std::string(" ") + std::to_string(_marblePosY) + std::string("\n");
    
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

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _pointMagnet;

  // Container for game-specific values
  uint8_t*  _gameTimer;
  uint8_t*  _gameCycle;
  uint8_t*  _winFlag;
  uint8_t*  _marbleState;
  uint8_t*  _marbleFlags;
  uint8_t*  _marblePosX1;
  uint8_t*  _marblePosX2;
  uint8_t*  _marblePosX3;
  uint8_t*  _marblePosY1;
  uint8_t*  _marblePosY2;
  uint8_t*  _marblePosY3;
  uint8_t*  _marblePosZ1;
  uint8_t*  _marbleAirtime;
  int8_t*   _marbleVelX;
  int8_t*   _marbleVelY;
  uint8_t*  _marbleDeadFlag;
  uint8_t*  _marbleSurfaceAngle;

  float  _marbleDistanceToPointX;
  float  _marbleDistanceToPointY;
  float _marbleDistanceToPoint;

  float  _marblePosX;
  float  _marblePosY;
  float  _marblePosZ;
  float  _surfaceAngleX;
  float  _surfaceAngleY;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;

  // Reward for the last time an input was made (for early termination)
  float _lastInputStepReward;

  // Specifies whether the reward should continue to be processed (for early termination)
  bool _stopProcessingReward;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;

  // Additions to make the last input as soon as possible
  uint16_t _lastInputStep = 0;
  uint16_t _currentStep   = 0;

  // Whether we use a trace
  bool _useTrace = false;

  // Location of the trace file
  std::string _traceFilePath;

  // Trace contents
  std::vector<traceEntry_t> _trace;

  // Current trace target
  uint16_t _traceStep;
  float _traceTargetX;
  float _traceTargetY;
  float _traceDistanceX;
  float _traceDistanceY;
  float _traceDistance;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
