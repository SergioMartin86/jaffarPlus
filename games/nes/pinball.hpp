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

class Pinball final : public jaffarPlus::Game
{
public:

  static __INLINE__ std::string getName() { return "NES / Pinball"; }

  Pinball(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

  	_springCharge  = (uint8_t*) registerGameProperty("Spring Charge"  ,&_lowMem[0x00E3], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _ballPosX      = (uint8_t*) registerGameProperty("Ball Pos X"     ,&_lowMem[0x0007], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _ballPosY      = (uint8_t*) registerGameProperty("Ball Pos Y"     ,&_lowMem[0x0009], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _ballDirY      = (uint8_t*) registerGameProperty("Ball Dir Y"     ,&_lowMem[0x000D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _lagFrame      = (uint8_t*) registerGameProperty("Lag Frame"      ,&_lowMem[0x001D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _score06       = (uint8_t*) registerGameProperty("Score06"        ,&_lowMem[0x0100], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _score05       = (uint8_t*) registerGameProperty("Score05"        ,&_lowMem[0x0101], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _score04       = (uint8_t*) registerGameProperty("Score04"        ,&_lowMem[0x0102], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _score03       = (uint8_t*) registerGameProperty("Score03"        ,&_lowMem[0x0103], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _score02       = (uint8_t*) registerGameProperty("Score02"        ,&_lowMem[0x0104], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _currentScreen = (uint8_t*) registerGameProperty("Current Screen" ,&_lowMem[0x00BF], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _warpState     = (uint8_t*) registerGameProperty("Warp State"     ,&_lowMem[0x0023], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    _paulinePosX   = (uint8_t*) registerGameProperty("Pauline Pos X"  ,&_lowMem[0x00ED], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _paulinePosY   = (uint8_t*) registerGameProperty("Pauline Pos Y"  ,&_lowMem[0x011C], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    _platform1State   = (uint8_t*) registerGameProperty("Platform 1 State"     ,&_lowMem[0x0121], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _platform2State   = (uint8_t*) registerGameProperty("Platform 2 State"     ,&_lowMem[0x0137], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    _platform3State   = (uint8_t*) registerGameProperty("Platform 3 State"     ,&_lowMem[0x013C], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    registerGameProperty("Ball Launched"    ,&_ballLaunched, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Score"    ,&_score, Property::datatype_t::dt_uint32, Property::endianness_t::little);
    registerGameProperty("Prev Score"    ,&_prevScore, Property::datatype_t::dt_uint32, Property::endianness_t::little);
    _nullInputIdx      = _emulator->registerInput("|..|........|");
    _ballLaunched = false;
    _prevScore = 0;

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
    hashEngine.Update(*_springCharge);
    hashEngine.Update(*_ballPosX    );
    hashEngine.Update(*_ballPosY    );
    hashEngine.Update(*_ballDirY    );
    hashEngine.Update(*_lagFrame    );
    hashEngine.Update(*_score06     );
    hashEngine.Update(*_score05     );
    hashEngine.Update(*_score04     );
    hashEngine.Update(*_score03     );
    hashEngine.Update(*_score02     );

    hashEngine.Update(&_lowMem[0x0000], 0x140);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _prevScore = _score;
    _score = 100000 * (*_score06) + 10000 * (*_score05) + 1000 * (*_score04) + 100 * (*_score03) + 10 * (*_score02); 
    if (*_ballDirY != 0) _ballLaunched = true;

    _sumPlatform = 0;
    _sumPlatform += *_platform1State;
    _sumPlatform += *_platform2State;
    _sumPlatform += *_platform3State;

  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    _ballPosXMagnet.intensity = 0.0f;
    _ballPosYMagnet.intensity = 0.0f;
    _ballPosXMagnet.pos = 0.0f;
    _ballPosYMagnet.pos = 0.0f;

    _paulinePosXMagnet.intensity = 0.0f;
    _paulinePosYMagnet.intensity = 0.0f;
    _paulinePosXMagnet.pos = 0.0f;
    _paulinePosYMagnet.pos = 0.0f;

    _scoreMagnet = 0.0f;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    _ballDistanceToPointX = std::abs((float)_ballPosXMagnet.pos - (float)*_ballPosX);
    _ballDistanceToPointY = std::abs((float)_ballPosYMagnet.pos - (float)*_ballPosY);

    _paulineDistanceToPointX = std::abs((float)_paulinePosXMagnet.pos - (float)*_paulinePosX);
    _paulineDistanceToPointY = std::abs((float)_paulinePosYMagnet.pos - (float)*_paulinePosY);

  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_lastInput, sizeof(_lastInput));
     serializer.push(&_ballLaunched, sizeof(_ballLaunched));
     serializer.push(&_prevScore, sizeof(_prevScore));
     serializer.push(&_sumPlatform, sizeof(_sumPlatform));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_lastInput, sizeof(_lastInput));
     deserializer.pop(&_ballLaunched, sizeof(_ballLaunched));
     deserializer.pop(&_prevScore, sizeof(_prevScore));
     deserializer.pop(&_sumPlatform, sizeof(_sumPlatform));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    if (_ballLaunched == true) reward += 100.0;
    reward += *_springCharge * 0.0001f;

    reward += -1.0 * _ballPosXMagnet.intensity * _ballDistanceToPointX;
    reward += -1.0 * _ballPosYMagnet.intensity * _ballDistanceToPointY;

    reward += -1.0 * _paulinePosXMagnet.intensity * _paulineDistanceToPointX;
    reward += -1.0 * _paulinePosYMagnet.intensity * _paulineDistanceToPointY;

    reward += 1000.0f * _sumPlatform; 

    // Evaluating position imbalance magnet
    reward += (float)_score * _scoreMagnet; 

    // Returning reward
    return reward;
  }

      // Function to report what all the possible input that the game might require
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    return {};
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
  }

  __INLINE__ uint64_t getStateMoveHash() const
  {
    return 0;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:                     %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Score:                            %u (Prev: %u)\n", _score, _prevScore);
    jaffarCommon::logger::log("[J+]  + Current Screen:                   %02u\n", _currentScreen);
    jaffarCommon::logger::log("[J+]  + Spring Charge:                    %02u\n", *_springCharge);
    jaffarCommon::logger::log("[J+]  + Ball Launched:                    %s\n", _ballLaunched ? "True" : "False");
    jaffarCommon::logger::log("[J+]  + Ball Pos X:                       %02u\n", *_ballPosX);
    jaffarCommon::logger::log("[J+]  + Ball Pos Y:                       %02u\n", *_ballPosY);
    jaffarCommon::logger::log("[J+]  + Warp State:                       %02u\n", *_warpState);
    jaffarCommon::logger::log("[J+]  + Pauline Pos X:                    %02u\n", *_paulinePosX);
    jaffarCommon::logger::log("[J+]  + Pauline Pos Y:                    %02u\n", *_paulinePosY);
    
    jaffarCommon::logger::log("[J+]  + Platforms:                        Sum: %02u [ %02u, %02u, %02u ]\n", _sumPlatform, *_platform1State, *_platform2State, *_platform3State);

    if (std::abs(_ballPosXMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Ball Pos X Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _ballPosXMagnet.intensity, _ballPosXMagnet.pos, _ballDistanceToPointX);
    if (std::abs(_ballPosYMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Ball Pos Y Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _ballPosYMagnet.intensity, _ballPosYMagnet.pos, _ballDistanceToPointY);
    if (std::abs(_scoreMagnet) > 0.0f)

    if (std::abs(_paulinePosXMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Pauline Pos X Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _paulinePosXMagnet.intensity, _paulinePosXMagnet.pos, _paulineDistanceToPointX);
    if (std::abs(_paulinePosYMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Pauline Pos Y Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _paulinePosYMagnet.intensity, _paulinePosYMagnet.pos, _paulineDistanceToPointY);
    if (std::abs(_scoreMagnet) > 0.0f)

       jaffarCommon::logger::log("[J+]  + Score Magnet                      Intensity: %.5f\n", _scoreMagnet);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Ball Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_ballPosXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Ball Pos Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_ballPosYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Pauline Pos Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_paulinePosYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Pauline Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_paulinePosYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Score Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_scoreMagnet = intensity; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float pos       = 0.0; // What is the point of attraction X
  };

  float _scoreMagnet;
  pointMagnet_t _ballPosXMagnet;
  pointMagnet_t _ballPosYMagnet;
  float _ballDistanceToPointX;
  float _ballDistanceToPointY;

  pointMagnet_t _paulinePosXMagnet;
  pointMagnet_t _paulinePosYMagnet;
  float _paulineDistanceToPointX;
  float _paulineDistanceToPointY;

  InputSet::inputIndex_t _lastInput;
  InputSet::inputIndex_t _nullInputIdx;
  uint8_t* _lowMem;
  uint32_t _score;
  bool _ballLaunched;
  uint8_t* _springCharge ;
  uint8_t* _ballPosX     ;
  uint8_t* _ballPosY     ;
  uint8_t* _ballDirY     ;
  uint8_t* _lagFrame     ;
  uint8_t* _score06      ;
  uint8_t* _score05      ;
  uint8_t* _score04      ;
  uint8_t* _score03      ;
  uint8_t* _score02      ;
  uint8_t* _currentScreen;
  uint8_t* _warpState;
  uint8_t* _platform1State;
  uint8_t* _platform2State;
  uint8_t* _platform3State;
  uint8_t* _paulinePosX;
  uint8_t* _paulinePosY;

  uint32_t _currentStep;
  uint32_t _prevScore;
  uint8_t _sumPlatform;
  
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
