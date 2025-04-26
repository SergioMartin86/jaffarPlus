#pragma once

#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

#define _SAINT_SEIYA_FIGHT_COUNT 34

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class SaintSeiyaOugonDensetsu final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "NES / Saint Seiya Ougon Densetsu"; }

  SaintSeiyaOugonDensetsu(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
    // Parsing configuration
    _lastInputStepReward = jaffarCommon::json::getNumber<float>(config, "Last Input Step Reward");
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Player Pos X", &_lowMem[0x0000], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x0000], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Menu Type", &_lowMem[0x0026], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Menu Option", &_lowMem[0x00C8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Fights Completed", &_fightsCompleted, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Previous Input", &_lowMem[0x0032], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Menu Input Frame", &_lowMem[0x0038], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Adding fight properties
    _fightVector = &_lowMem[0x06D1];
    for (size_t i = 0; i < _SAINT_SEIYA_FIGHT_COUNT; i++)
      registerGameProperty(std::string("Fight[") + std::to_string(i) + std::string("]"), &_fightVector[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _menuType       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Menu Type")]->getPointer();
    _menuOption     = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Menu Option")]->getPointer();
    _playerPosX     = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY     = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _menuInputFrame = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Menu Input Frame")]->getPointer();

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

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    uint8_t ramData[0x800];

    // Storing current RAM
    memcpy(ramData, _lowMem, 0x800);

    // Advancing state
    _emulator->advanceState(_nullInputIdx);

    for (size_t i = 685; i < 0x700; i++) hashEngine.Update(_lowMem[i]);
    hashEngine.Update(*_menuOption);

    for (size_t i = 0; i < 0x100; i++)
      if (i != 0x0013 && i != 0x0014 && i != 0x001E && i != 0x0032 && i != 0x0033 && i != 0x0034 && i != 0x0036 && i != 0x004C && i != 0x004E && i != 0x00CF)
        hashEngine.Update(_lowMem[i]);

    memcpy(_lowMem, ramData, 0x800);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _fightsCompleted = 0;
    for (size_t i = 0; i < _SAINT_SEIYA_FIGHT_COUNT; i++)
      if (_fightVector[i] > 127) _fightsCompleted++;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;
    _fightsCompletedMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs((float)_pointMagnet.x - (float)*_playerPosX);
    _player1DistanceToPointY = std::abs((float)_pointMagnet.y - (float)*_playerPosY);
    _player1DistanceToPoint  = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.pushContiguous(&_fightsCompleted, sizeof(_fightsCompleted));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.popContiguous(&_fightsCompleted, sizeof(_fightsCompleted));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputStepReward * _lastInputStep;

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Rewarding fights disabled
    uint8_t maxFightValue = 0;
    for (size_t i = 0; i < _SAINT_SEIYA_FIGHT_COUNT; i++)
      if (_fightVector[i] < 200)
        if (_fightVector[i] > maxFightValue) maxFightValue = _fightVector[i];
    reward += maxFightValue * 0.001 * _fightsCompletedMagnet;
    for (size_t i = 0; i < _SAINT_SEIYA_FIGHT_COUNT; i++)
      if (_fightVector[i] > 0) reward += _fightsCompletedMagnet * .00001;
    for (size_t i = 0; i < _SAINT_SEIYA_FIGHT_COUNT; i++) reward += _fightsCompletedMagnet * 0.000001 * std::min((uint8_t)128, _fightVector[i]);
    reward += _fightsCompletedMagnet * (double)_fightsCompleted;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Fights Vector:\n");
    jaffarCommon::logger::log("     + [ ");
    for (size_t i = 0; i < _SAINT_SEIYA_FIGHT_COUNT; i++) jaffarCommon::logger::log("%4u", i);
    jaffarCommon::logger::log(" ]\n");
    jaffarCommon::logger::log("     + [ ");
    for (size_t i = 0; i < _SAINT_SEIYA_FIGHT_COUNT; i++) jaffarCommon::logger::log("%4u", _fightVector[i]);
    jaffarCommon::logger::log(" ]\n");

    if (std::abs(_pointMagnet.intensity) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
        jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
        jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
        jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }

    if (std::abs(_fightsCompletedMagnet) > 0.0f) { jaffarCommon::logger::log("[J+]  + Fights Completed Magnet                   Intensity: %.5f\n", _fightsCompletedMagnet); }
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
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

    if (actionType == "Set Fights Completed Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        rule.addAction([=, this]() { this->_fightsCompletedMagnet = intensity; });
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
    float x         = 0.0; // What is the x point of attraction
    float y         = 0.0; // What is the y point of attraction
  };

  // Counter for fights completed
  uint8_t  _fightsCompleted;
  uint8_t *_fightVector;

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  pointMagnet_t _pointMagnet;
  double        _fightsCompletedMagnet;

  uint8_t *_menuType;
  uint8_t *_menuOption;
  uint8_t *_menuInputFrame;

  uint8_t *_playerPosX;
  uint8_t *_playerPosY;

  // Additions to make the last input as soon as possible
  uint16_t _lastInputStep = 0;
  uint16_t _currentStep   = 0;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;

  // Pointer to emulator's low memory storage
  uint8_t *_lowMem;

  // Reward for the last time an input was made (for early termination)
  float _lastInputStepReward;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus