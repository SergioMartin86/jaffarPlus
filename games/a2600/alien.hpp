#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>

namespace jaffarPlus
{

namespace games
{

namespace a2600
{

class Alien final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "A2600 / Alien"; }

  Alien(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config) {}

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Pellets Eaten", &_lowMem[0x5B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game Status", &_lowMem[0x0D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Alive", &_lowMem[0x66], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Direction", &_lowMem[0x04], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_lowMem[0x08], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x0A], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _pelletsEaten    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Pellets Eaten")]->getPointer();
    _gameStatus      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Status")]->getPointer();
    _playerAlive     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Alive")]->getPointer();
    _playerDirection = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Direction")]->getPointer();
    _playerPosX      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();

    _inputL = _emulator->registerInput("|.....|..L..|");
    _inputU = _emulator->registerInput("|.....|U....|");
    _inputD = _emulator->registerInput("|.....|.D...|");
    _inputR = _emulator->registerInput("|.....|...R.|");

    _verticalPassagesPosX.insert(_verticalPassagesPosX.end(), {2, 14, 22, 34, 46, 50, 66, 70, 82, 94, 102, 114});
    _horizontalPassagesPosY.insert(_horizontalPassagesPosY.end(), {69, 81, 93, 57, 64, 45, 33, 21});

    _prevPelletsEaten = 0;
    _keyFrame         = 0;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    _keyFrame = 0;
    if (*_pelletsEaten != _prevPelletsEaten) _keyFrame = 1;
    _prevPelletsEaten = *_pelletsEaten;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // //  hashEngine.Update(_lowMem, 0x80);
    //  for (size_t i = 0; i < 0x80; i++)
    //   if (i != 0x02) // Timer
    //   if (i != 0x3E) // Timer
    //    hashEngine.Update(_lowMem[i]);

    hashEngine.Update(*_pelletsEaten);
    hashEngine.Update(*_gameStatus);
    hashEngine.Update(*_playerAlive);
    hashEngine.Update(_lowMem[0x08]); // Player Pos X
    hashEngine.Update(_lowMem[0x34]); // Player Pos X
    hashEngine.Update(_lowMem[0x0A]); // Player Pos Y
    hashEngine.Update(_lowMem[0x04]); // Player Direction
    hashEngine.Update(_lowMem[0x06]); // Player Direction
    hashEngine.Update(_lowMem[0x22]); // Player Direction
    hashEngine.Update(_lowMem[0x3F]); // Player Direction

    // Pellets
    for (size_t i = 0x50; i <= 0x5B; i++) hashEngine.Update(_lowMem[i]);
  }

  __INLINE__ std::set<std::string> getAllPossibleInputs() { return {"|.....|..L..|", "|.....|U....|", "|.....|.D...|", "|.....|...R.|"}; }

#define A2600_ALIEN_DIRECTION_LEFT 64
#define A2600_ALIEN_DIRECTION_DOWN 32
#define A2600_ALIEN_DIRECTION_RIGHT 128
#define A2600_ALIEN_DIRECTION_UP 16
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    if (*_playerDirection == A2600_ALIEN_DIRECTION_RIGHT)
    {
      // bool tryVertical = false;
      // for (const auto posX : _verticalPassagesPosX)  if (*_playerPosX == posX-1 || *_playerPosX == posX) { tryVertical = true; break; }

      allowedInputSet.insert(allowedInputSet.end(), {_inputR});
      // if (tryVertical == true) allowedInputSet.insert(allowedInputSet.end(), { _inputU, _inputD });
      allowedInputSet.insert(allowedInputSet.end(), {_inputU, _inputD});
      if (_keyFrame == 1) allowedInputSet.insert(allowedInputSet.end(), {_inputL});

      return;
    }

    if (*_playerDirection == A2600_ALIEN_DIRECTION_LEFT)
    {
      // bool tryVertical = false;
      // for (const auto posX : _verticalPassagesPosX)  if (*_playerPosX == posX+1 || *_playerPosX == posX) { tryVertical = true; break; }

      allowedInputSet.insert(allowedInputSet.end(), {_inputL});
      // if (tryVertical == true) allowedInputSet.insert(allowedInputSet.end(), { _inputU, _inputD });
      allowedInputSet.insert(allowedInputSet.end(), {_inputU, _inputD});
      if (_keyFrame == 1) allowedInputSet.insert(allowedInputSet.end(), {_inputR});

      return;
    }

    if (*_playerDirection == A2600_ALIEN_DIRECTION_UP)
    {
      // bool tryHorizontal = false;
      // for (const auto posY : _horizontalPassagesPosY)  if (*_playerPosY == posY+1 || *_playerPosY == posY) { tryHorizontal = true; break; }

      allowedInputSet.insert(allowedInputSet.end(), {_inputU});
      // if (tryHorizontal == true) allowedInputSet.insert(allowedInputSet.end(), { _inputL, _inputR });
      allowedInputSet.insert(allowedInputSet.end(), {_inputL, _inputR});
      if (_keyFrame == 1) allowedInputSet.insert(allowedInputSet.end(), {_inputD});

      return;
    }

    if (*_playerDirection == A2600_ALIEN_DIRECTION_DOWN)
    {
      // bool tryHorizontal = false;
      // for (const auto posY : _horizontalPassagesPosY)  if (*_playerPosY == posY-1 || *_playerPosY == posY-1) { tryHorizontal = true; break; }

      allowedInputSet.insert(allowedInputSet.end(), {_inputD});
      // if (tryHorizontal == true) allowedInputSet.insert(allowedInputSet.end(), { _inputL, _inputR });
      allowedInputSet.insert(allowedInputSet.end(), {_inputL, _inputR});
      if (_keyFrame == 1) allowedInputSet.insert(allowedInputSet.end(), {_inputU});
      return;
    }

    allowedInputSet.insert(allowedInputSet.end(), {_inputL, _inputU, _inputD, _inputR});
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.push(&_prevPelletsEaten, sizeof(_prevPelletsEaten));
    serializer.push(&_keyFrame, sizeof(_keyFrame));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.pop(&_prevPelletsEaten, sizeof(_prevPelletsEaten));
    deserializer.pop(&_keyFrame, sizeof(_keyFrame));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 1.0f * (float)*_pelletsEaten;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Pellets Eaten:       %u (Prev: %u)\n", *_pelletsEaten, _prevPelletsEaten);
    jaffarCommon::logger::log("[J+]  + Key Frame:           %u\n", _keyFrame);
    jaffarCommon::logger::log("[J+]  + Game Status:         %u\n", *_gameStatus);
    jaffarCommon::logger::log("[J+]  + Player Alive:        %u\n", *_playerAlive);
    jaffarCommon::logger::log("[J+]  + Player Direction:    %u\n", *_playerDirection);
    jaffarCommon::logger::log("[J+]  + Player Pos X:        %u\n", *_playerPosX);
    jaffarCommon::logger::log("[J+]  + Player Pos Y:        %u\n", *_playerPosY);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;

  uint8_t* _pelletsEaten;
  uint8_t* _gameStatus;
  uint8_t* _playerAlive;
  uint8_t* _playerDirection;
  uint8_t* _playerPosX;
  uint8_t* _playerPosY;

  uint8_t _prevPelletsEaten;
  uint8_t _keyFrame;

  InputSet::inputIndex_t _inputL;
  InputSet::inputIndex_t _inputU;
  InputSet::inputIndex_t _inputD;
  InputSet::inputIndex_t _inputR;

  std::vector<uint8_t> _verticalPassagesPosX;
  std::vector<uint8_t> _horizontalPassagesPosY;
};

} // namespace a2600

} // namespace games

} // namespace jaffarPlus