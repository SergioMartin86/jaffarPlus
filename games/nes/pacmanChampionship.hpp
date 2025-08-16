#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class PacManChampionship final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Pac-Man Championship"; }

  PacManChampionship(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;
    _srmMem = _emulator->getProperty("SRAM").pointer;

    // Registering native game properties
    registerGameProperty("Game Mode", &_lowMem[0x0033], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x10", &_lowMem[0x0457], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x100", &_lowMem[0x0458], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x1000", &_lowMem[0x0459], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x10000", &_lowMem[0x045A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x100000", &_lowMem[0x045B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score x1000000", &_lowMem[0x045C], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Player Direction", &_lowMem[0x049F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X1", &_lowMem[0x047B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X2", &_lowMem[0x047C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y2", &_lowMem[0x04FD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Wall Skid", &_lowMem[0x04A3], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ghost Capture Timer 1", &_lowMem[0x0446], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost Capture Timer 2", &_lowMem[0x066B], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ghost 1 State", &_lowMem[0x042A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 2 State", &_lowMem[0x042B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 3 State", &_lowMem[0x042C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 4 State", &_lowMem[0x042D], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ghost 1 Block X", &_lowMem[0x03A8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 2 Block X", &_lowMem[0x03A9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 3 Block X", &_lowMem[0x03AA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 4 Block X", &_lowMem[0x03AB], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 1 Block Y", &_lowMem[0x03D2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 2 Block Y", &_lowMem[0x03D3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 3 Block Y", &_lowMem[0x03D4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 4 Block Y", &_lowMem[0x03D5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 1 Direction", &_lowMem[0x03E7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 2 Direction", &_lowMem[0x03E8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 3 Direction", &_lowMem[0x03E9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ghost 4 Direction", &_lowMem[0x03EA], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _gameMode  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Mode")]->getPointer();
    _score10  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score x10")]->getPointer();
    _score100  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score x100")]->getPointer();
    _score1000  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score x1000")]->getPointer();
    _score10000  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score x10000")]->getPointer();
    _score100000  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score x100000")]->getPointer();
    _score1000000  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score x1000000")]->getPointer();
    _playerDirection  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Direction")]->getPointer();
    _playerPosX1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X1")]->getPointer();
    _playerPosX2  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X2")]->getPointer();
    _playerPosY2  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y2")]->getPointer();
    _playerWallSkid  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Wall Skid")]->getPointer();
        
    _ghostCaptureTimer1  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost Capture Timer 1")]->getPointer();
    _ghostCaptureTimer2  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost Capture Timer 2")]->getPointer();

    _ghost1State  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 1 State")]->getPointer();
    _ghost2State  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 2 State")]->getPointer();
    _ghost3State  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 3 State")]->getPointer();
    _ghost4State  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 4 State")]->getPointer();

    _ghost1BlockX  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 1 Block X")]->getPointer();
    _ghost2BlockX  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 2 Block X")]->getPointer();
    _ghost3BlockX  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 3 Block X")]->getPointer();
    _ghost4BlockX  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 4 Block X")]->getPointer();
    _ghost1BlockY  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 1 Block Y")]->getPointer();
    _ghost2BlockY  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 2 Block Y")]->getPointer();
    _ghost3BlockY  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 3 Block Y")]->getPointer();
    _ghost4BlockY  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 4 Block Y")]->getPointer();
    _ghost1Direction  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 1 Direction")]->getPointer();
    _ghost2Direction  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 2 Direction")]->getPointer();
    _ghost3Direction  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 3 Direction")]->getPointer();
    _ghost4Direction  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Ghost 4 Direction")]->getPointer();

    registerGameProperty("Score", &_score, Property::datatype_t::dt_uint32, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_playerPosX, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_playerPosY, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    _input_U      = _emulator->registerInput("|..|U.......|");
    _input_D      = _emulator->registerInput("|..|.D......|");
    _input_L      = _emulator->registerInput("|..|..L.....|");
    _input_R      = _emulator->registerInput("|..|...R....|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // hashEngine.Update(&_lowMem[0x0002], 0x07FD);

    hashEngine.Update(*_gameMode   );
    hashEngine.Update(*_score10  );
    hashEngine.Update(*_score100  );
    hashEngine.Update(*_score1000  );
    hashEngine.Update(*_score10000    );
    hashEngine.Update(*_score100000    );
    hashEngine.Update(*_score1000000   );
    hashEngine.Update(*_playerDirection   );
    hashEngine.Update(*_playerPosX1   );
    hashEngine.Update(*_playerPosX2   );
    hashEngine.Update(*_playerPosY2   );
    hashEngine.Update(*_ghostCaptureTimer1);
    hashEngine.Update(*_ghostCaptureTimer2);
    hashEngine.Update(*_ghost1State);
    hashEngine.Update(*_ghost2State);
    hashEngine.Update(*_ghost3State);
    hashEngine.Update(*_ghost4State);
    hashEngine.Update(*_ghost1BlockX);
    hashEngine.Update(*_ghost2BlockX);
    hashEngine.Update(*_ghost3BlockX);
    hashEngine.Update(*_ghost4BlockX);
    hashEngine.Update(*_ghost1BlockY);
    hashEngine.Update(*_ghost2BlockY);
    hashEngine.Update(*_ghost3BlockY);
    hashEngine.Update(*_ghost4BlockY);
    hashEngine.Update(*_ghost1Direction);
    hashEngine.Update(*_ghost2Direction);
    hashEngine.Update(*_ghost3Direction);
    hashEngine.Update(*_ghost4Direction);
    hashEngine.Update(*_playerWallSkid);

    hashEngine.Update(&_srmMem[0x0000], 0x0580); // The stage's situation (pebbles, walls)
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
   {
     _score =
      (uint32_t)*_score10 * 10u +
      (uint32_t)*_score100 * 100u +
      (uint32_t)*_score1000 * 1000u +
      (uint32_t)*_score10000 * 10000u +
      (uint32_t)*_score100000 * 100000u +
      (uint32_t)*_score1000000 * 1000000u;

    _playerPosX = (uint16_t)*_playerPosX1 + (uint16_t)*_playerPosX2;
    _playerPosY = (uint16_t)*_playerPosY2;
   }

  __INLINE__ void ruleUpdatePreHook() override
  {
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_score, sizeof(_score));
    serializer.pushContiguous(&_playerPosX, sizeof(_playerPosX));
    serializer.pushContiguous(&_playerPosY, sizeof(_playerPosY));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_score, sizeof(_score));
    deserializer.popContiguous(&_playerPosX, sizeof(_playerPosX));
    deserializer.popContiguous(&_playerPosY, sizeof(_playerPosY));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from score
    float reward = (float) _score;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {

      // If capturing a ghost, only option is to press the current directions
      if (*_ghostCaptureTimer1 > 0)
      {
          if (*_playerDirection == 1) allowedInputSet.insert(allowedInputSet.end(), {_input_U});
          if (*_playerDirection == 2) allowedInputSet.insert(allowedInputSet.end(), {_input_D});
          if (*_playerDirection == 3) allowedInputSet.insert(allowedInputSet.end(), {_input_L});
          if (*_playerDirection == 4) allowedInputSet.insert(allowedInputSet.end(), {_input_R});
          return;
      }


      allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R, _input_U, _input_D});
      return;

      // Up
      if (*_playerDirection == 1)
      {
        allowedInputSet.insert(allowedInputSet.end(), {_input_U, _input_D});
        if (_playerPosY % 8 == 5) allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R});
      }

      // Down
      if (*_playerDirection == 2)
      {
        allowedInputSet.insert(allowedInputSet.end(), {_input_U, _input_D});
        if (_playerPosY % 8 == 2) allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R});
      }

      // Left
      if (*_playerDirection == 3)
      {
        allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R});
        if (_playerPosX % 8 == 3) allowedInputSet.insert(allowedInputSet.end(), {_input_U, _input_D});
      }

      // Right
      if (*_playerDirection == 4)
      {
        allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R});
        if (_playerPosX % 8 == 4) allowedInputSet.insert(allowedInputSet.end(), {_input_U, _input_D});
      }
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

  uint8_t*  _gameMode ;
  uint8_t*  _score10  ;
  uint8_t*  _score100 ;
  uint8_t*  _score1000 ;
  uint8_t*  _score10000 ;
  uint8_t*  _score100000 ;
  uint8_t*  _score1000000 ;

  uint8_t*  _playerDirection ;
  uint8_t*  _playerPosX1 ;
  uint8_t*  _playerPosX2 ;
  uint8_t*  _playerPosY2 ;
  uint8_t*  _playerWallSkid;

  uint8_t* _ghostCaptureTimer1;
  uint8_t* _ghostCaptureTimer2;

  uint8_t* _ghost1State;
  uint8_t* _ghost2State;
  uint8_t* _ghost3State;
  uint8_t* _ghost4State;

  uint8_t* _ghost1BlockX;
  uint8_t* _ghost2BlockX;
  uint8_t* _ghost3BlockX;
  uint8_t* _ghost4BlockX;
  uint8_t* _ghost1BlockY;
  uint8_t* _ghost2BlockY;
  uint8_t* _ghost3BlockY;
  uint8_t* _ghost4BlockY;
  uint8_t* _ghost1Direction;
  uint8_t* _ghost2Direction;
  uint8_t* _ghost3Direction;
  uint8_t* _ghost4Direction;

  uint32_t _score;
  uint16_t _playerPosX;
  uint16_t _playerPosY;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
  uint8_t* _srmMem;

  InputSet::inputIndex_t _input_U  ;
  InputSet::inputIndex_t _input_D  ;
  InputSet::inputIndex_t _input_L  ;
  InputSet::inputIndex_t _input_R  ;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus