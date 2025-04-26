#pragma once

#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>
#include <numeric>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class IceClimber final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "NES / Ice Climber"; }

  IceClimber(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {}

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties

    registerGameProperty("Current Level", &_lowMem[0x0059], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_lowMem[0x0064], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x0066], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y", &_lowMem[0x0013], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y1", &_lowMem[0x008A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y2", &_lowMem[0x008D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y3", &_lowMem[0x0090], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Floor", &_lowMem[0x0343], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Blocks Hit", &_lowMem[0x0361], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Vegetables Collected", &_lowMem[0x0362], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Nitpickers Hit", &_lowMem[0x0363], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bricks Broken", &_lowMem[0x0364], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 1", &_lowMem[0x0014], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 2", &_lowMem[0x0015], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global Timer", &_lowMem[0x0050], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("SubTimer", &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("SubTimer", &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bonus Stage Start", &_lowMem[0x0027], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Grabbed Bird", &_lowMem[0x004D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Jumping", &_lowMem[0x00E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Is Dead", &_lowMem[0x0382], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Bird State", &_lowMem[0x00D3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bird Pos X", &_lowMem[0x0243], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bird Direction", &_lowMem[0x00D5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Is Bird Detected", &_isBirdDetected, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Player Real Pos Y", &_playerRealPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Best Pos Y", &_playerBestPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Real/Best Pos Y Diff", &_playerRealBestPosYDiff, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _currentLevel        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Current Level")]->getPointer();
    _playerPosX          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY          = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _screenScrollY       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y")]->getPointer();
    _screenScrollY1      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y1")]->getPointer();
    _screenScrollY2      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y2")]->getPointer();
    _screenScrollY3      = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y3")]->getPointer();
    _playerFloor         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Floor")]->getPointer();
    _playerJumping       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Jumping")]->getPointer();
    _blocksHit           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Blocks Hit")]->getPointer();
    _vegetablesCollected = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Vegetables Collected")]->getPointer();
    _nitpickersHit       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Nitpickers Hit")]->getPointer();
    _bricksBroken        = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bricks Broken")]->getPointer();
    _playerInputs1       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 1")]->getPointer();
    _playerInputs2       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 2")]->getPointer();
    _globalTimer         = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _subTimer            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("SubTimer")]->getPointer();
    _birdState           = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bird State")]->getPointer();
    _birdPosX            = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bird Pos X")]->getPointer();
    _birdDirection       = (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Bird Direction")]->getPointer();

    _pastBricksBroken  = 0;
    _isBirdDetected    = 0;
    _fullScreenScrolls = 0;
    _playerBestPosY    = std::numeric_limits<float>::infinity();
    updateRealPosY();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void updateRealPosY()
  {
    _playerRealPosY = (float)*_playerPosY;
    if (_playerRealPosY > 245.0) _playerRealPosY -= 255.0;
    _playerRealPosY += (float)*_screenScrollY;
    _playerRealPosY -= (float)_fullScreenScrolls * 239.0;
    _playerRealBestPosYDiff = _playerRealPosY - _playerBestPosY;

    // If player is standing and real player pos Y is less than before update
    if (*_playerJumping == 0)
      if (_playerRealPosY < _playerBestPosY)
        {
          _playerBestPosY   = _playerRealPosY;
          _pastBricksBroken = *_bricksBroken;
      }
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Remembering current screen scroll
    auto curScroll = *_screenScrollY;

    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;

    // Getting new scroll
    auto newScroll = *_screenScrollY;

    // If new scroll is larger than current, a new cycle is made
    if (newScroll > curScroll) _fullScreenScrolls++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x0000], 0x000A - 0x0000); //
    hashEngine.Update(_lowMem[0x000F]);                   // No idea but seems important
    hashEngine.Update(&_lowMem[0x0051], 0x009D - 0x0051); // Skipping timers
    hashEngine.Update(&_lowMem[0x00A9], 0x00F6 - 0x00A9);
    hashEngine.Update(&_lowMem[0x0100], 0x01F7 - 0x0100);
    hashEngine.Update(&_lowMem[0x01FD], 0x0217 - 0x01FD);
    hashEngine.Update(&_lowMem[0x0400], 0x04C0 - 0x0400); // Brick states
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    if (*_birdState > 0) _isBirdDetected = 1;
    updateRealPosY();

    _distanceToBirdX = std::abs((float)*_playerPosX - (float)*_birdPosX);
    _distanceToBirdY = std::abs(36.0f - _playerRealPosY);
    _distanceToBird  = sqrt(_distanceToBirdX * _distanceToBirdX + _distanceToBirdY * _distanceToBirdY);

    // Modular math: calculate distance regarding the distance regarding wrap around position
    if (*_birdDirection == 1)
      {
        float distanceToBirdX = std::abs((float)*_playerPosX - ((float)*_birdPosX - 256.0f));
        float distanceToBird  = sqrt(distanceToBirdX * distanceToBirdX + _distanceToBirdY * _distanceToBirdY);

        if (distanceToBird < _distanceToBird)
          {
            _distanceToBirdX = distanceToBirdX;
            _distanceToBird  = distanceToBird;
        }
    }

    if (*_birdDirection == 0)
      {
        float distanceToBirdX = std::abs((float)*_playerPosX - ((float)*_birdPosX + 256.0f));
        float distanceToBird  = sqrt(distanceToBirdX * distanceToBirdX + _distanceToBirdY * _distanceToBirdY);

        if (distanceToBird < _distanceToBird)
          {
            _distanceToBirdX = distanceToBirdX;
            _distanceToBird  = distanceToBird;
        }
    }
  }

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_isBirdDetected, sizeof(_isBirdDetected));
    serializer.pushContiguous(&_playerRealPosY, sizeof(_playerRealPosY));
    serializer.pushContiguous(&_fullScreenScrolls, sizeof(_fullScreenScrolls));
    serializer.pushContiguous(&_playerBestPosY, sizeof(_playerBestPosY));
    serializer.pushContiguous(&_pastBricksBroken, sizeof(_pastBricksBroken));
    serializer.pushContiguous(&_playerRealBestPosYDiff, sizeof(_playerRealBestPosYDiff));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_isBirdDetected, sizeof(_isBirdDetected));
    deserializer.popContiguous(&_playerRealPosY, sizeof(_playerRealPosY));
    deserializer.popContiguous(&_fullScreenScrolls, sizeof(_fullScreenScrolls));
    deserializer.popContiguous(&_playerBestPosY, sizeof(_playerBestPosY));
    deserializer.popContiguous(&_pastBricksBroken, sizeof(_pastBricksBroken));
    deserializer.popContiguous(&_playerRealBestPosYDiff, sizeof(_playerRealBestPosYDiff));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    reward -= _playerBestPosY + 0.01 * _playerRealPosY;
    reward -= 0.1 * (float)*_vegetablesCollected;

    // Reward new blocks while we're climbing
    reward += 0.1 * (float)*_bricksBroken;

    // Punish past blocks
    reward -= 0.2 * (float)_pastBricksBroken;

    // If there is a bird, chase it
    if (_isBirdDetected == 1) reward += 362.0 - _distanceToBird;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Player Real Pos Y:                  %.0f\n", _playerRealPosY);
    jaffarCommon::logger::log("[J+]  + Player Best Pos Y:                  %.0f\n", _playerBestPosY);
    jaffarCommon::logger::log("[J+]  + Player Best / Real Pos Y Diff:      %.0f\n", _playerRealBestPosYDiff);
    jaffarCommon::logger::log("[J+]  + Past Bricks Broken:                 %u\n", _pastBricksBroken);
    jaffarCommon::logger::log("[J+]  + Full Screen Scrolls:                %u\n", _fullScreenScrolls);
    if (_isBirdDetected == 1) jaffarCommon::logger::log("[J+]  + Distance To Bird:                  %.2f (%.2f, %.2f)\n", _distanceToBird, _distanceToBirdX, _distanceToBirdY);
  }

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

  uint8_t *_currentLevel;
  uint8_t *_playerPosX;
  uint8_t *_playerPosY;
  uint8_t *_screenScrollY;
  uint8_t *_screenScrollY1;
  uint8_t *_screenScrollY2;
  uint8_t *_screenScrollY3;
  uint8_t *_playerFloor;
  uint8_t *_blocksHit;
  uint8_t *_vegetablesCollected;
  uint8_t *_nitpickersHit;
  uint8_t *_bricksBroken;
  uint8_t *_playerInputs1;
  uint8_t *_playerInputs2;
  uint8_t *_globalTimer;
  uint8_t *_subTimer;
  uint8_t *_birdState;
  uint8_t *_birdPosX;
  uint8_t *_lowMem;
  uint8_t  _isBirdDetected;
  uint8_t *_playerJumping;
  uint8_t *_birdDirection;

  float _distanceToBirdX;
  float _distanceToBirdY;
  float _distanceToBird;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;
  uint8_t  _fullScreenScrolls;
  uint8_t  _pastBricksBroken;
  float    _playerRealPosY;
  float    _playerBestPosY;
  float    _playerRealBestPosYDiff;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus