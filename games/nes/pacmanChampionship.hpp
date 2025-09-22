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

const uint8_t _mapBlockXCount = 47;
const uint8_t _mapBlockYCount = 26;

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
    registerGameProperty("Player Block X", &_lowMem[0x04C9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Block Y", &_lowMem[0x048E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
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
    registerGameProperty("Bonus Multiplier", &_lowMem[0x0455], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bonus Multiplier Timer", &_lowMem[0x04B3], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Fruit 1 Status", &_lowMem[0x0481], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Fruit 2 Status", &_lowMem[0x0482], Property::datatype_t::dt_uint8, Property::endianness_t::little);

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
    _playerBlockX  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Block X")]->getPointer();
    _playerBlockY  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Block Y")]->getPointer();
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
    _bonusMultiplier  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bonus Multiplier")]->getPointer();
    _bonusMultiplierTimer  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Bonus Multiplier Timer")]->getPointer();
    _fruit1Status  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Fruit 1 Status")]->getPointer();
    _fruit2Status  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Fruit 2 Status")]->getPointer();

    registerGameProperty("Score", &_score, Property::datatype_t::dt_uint32, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_playerPosX, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_playerPosY, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Prev Pos X", &_playerPrevPosX, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player Prev Pos Y", &_playerPrevPosY, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Current Step", &_currentStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Prev Ghost Capture Timer 1", &_prevGhostCaptureTimer1, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Prev Bonus Multiplier", &_prevBonusMultiplier, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    _input_U      = _emulator->registerInput("|..|U.......|");
    _input_D      = _emulator->registerInput("|..|.D......|");
    _input_L      = _emulator->registerInput("|..|..L.....|");
    _input_R      = _emulator->registerInput("|..|...R....|");

    _mapTileMemPosUC = 0;
    _mapTileMemPosDC = 0;
    _mapTileMemPosCL = 0;
    _mapTileMemPosCR = 0;
    _mapTileBlockXUC  = 0;
    _mapTileBlockXDC  = 0;
    _mapTileBlockXCL  = 0;
    _mapTileBlockXCR  = 0;
    _mapTileBlockYUC  = 0;
    _mapTileBlockYDC  = 0;
    _mapTileBlockYCL  = 0;
    _mapTileBlockYCR  = 0;
    _mapTileTypeUC = 0;
    _mapTileTypeDC = 0;
    _mapTileTypeCL = 0;
    _mapTileTypeCR = 0;

   _mapTileMemPosUL  = 0;
   _mapTileMemPosUR  = 0;
   _mapTileMemPosDL  = 0;
   _mapTileMemPosDR  = 0;
   _mapTileBlockXUL  = 0;
   _mapTileBlockXUR  = 0;
   _mapTileBlockXDL  = 0;
   _mapTileBlockXDR  = 0;
   _mapTileBlockYUL  = 0;
   _mapTileBlockYUR  = 0;
   _mapTileBlockYDL  = 0;
   _mapTileBlockYDR  = 0;
   _mapTileTypeUL = 0;
   _mapTileTypeUR = 0;
   _mapTileTypeDL = 0;
   _mapTileTypeDR = 0;

   _playerPrevPosX = 0;
   _playerPrevPosY = 0;
   _isKeyFrame = true;
   _prevScore = _score;

   _prevGhost1State = *_ghost1State;
   _prevGhost2State = *_ghost2State;
   _prevGhost3State = *_ghost3State;
   _prevGhost4State = *_ghost4State;
   _prevFruit1Status = *_fruit1Status;
   _prevFruit2Status = *_fruit2Status;

    _currentStep = 0;
    _prevGhostCaptureTimer1 = 0;
    _prevBonusMultiplier = *_bonusMultiplier;
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _playerPrevPosX = _playerPosX;
    _playerPrevPosY = _playerPosY;
    _prevGhostCaptureTimer1 = *_ghostCaptureTimer1;
    _prevScore = _score;
    _prevBonusMultiplier = *_bonusMultiplier;
    _prevGhost1State = *_ghost1State;
    _prevGhost2State = *_ghost2State;
    _prevGhost3State = *_ghost3State;
    _prevGhost4State = *_ghost4State;
    _prevFruit1Status = *_fruit1Status;
    _prevFruit2Status = *_fruit2Status;

    // Running emulator
    _emulator->advanceState(input);

    _currentStep++;
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
    hashEngine.Update(*_playerBlockX   );
    hashEngine.Update(*_playerBlockY   );
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

    // Lag Frame Accounting
     hashEngine.Update(&_lowMem[0x0000], 0x10);
     hashEngine.Update(_lowMem[0x001E]);
     hashEngine.Update(_lowMem[0x003C]);
    //  hashEngine.Update(_lowMem[0x02C6]);
    //  hashEngine.Update(_lowMem[0x0242]);
    //  hashEngine.Update(_lowMem[0x049C]);
    //  hashEngine.Update(_lowMem[0x04BD]);
    // hashEngine.Update(&_lowMem[0x0000], 0x001F);
    // hashEngine.Update(&_srmMem[0x0000], 0x0580); // The stage's situation (pebbles, walls)
  }

  __INLINE__ uint8_t adjustMapBlockPosX(const uint8_t blockX, const int8_t offset) { uint16_t blockVal = ((uint16_t)_mapBlockXCount + (uint16_t)blockX + offset) % _mapBlockXCount; return (uint8_t) blockVal;}
  __INLINE__ uint8_t adjustMapBlockPosY(const uint8_t blockY, const int8_t offset) { uint16_t blockVal = ((uint16_t)_mapBlockYCount + (uint16_t)blockY + offset) % _mapBlockYCount; return (uint8_t) blockVal;}
  __INLINE__ uint16_t getMapMemPos(const uint8_t blockX, const uint8_t blockY) { return (uint16_t)blockY * (uint16_t)_mapBlockXCount + (uint16_t)blockX; }

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

    _isKeyFrame = false;
    if (_prevScore != _score) _isKeyFrame = true;
    if (_prevGhost1State != *_ghost1State) _isKeyFrame = true;
    if (_prevGhost2State != *_ghost2State) _isKeyFrame = true;
    if (_prevGhost3State != *_ghost3State) _isKeyFrame = true;
    if (_prevGhost4State != *_ghost4State) _isKeyFrame = true;
    if (_prevFruit1Status != *_fruit1Status) _isKeyFrame = true;
    if (_prevFruit2Status != *_fruit2Status) _isKeyFrame = true;
    if (*_ghostCaptureTimer1 == 1) _isKeyFrame = true;
    // if (_currentStep % 8 == 0) _isKeyFrame = true;

    _playerPosX = (uint16_t)*_playerPosX1 + (uint16_t)*_playerPosX2;
    _playerPosY = (uint16_t)*_playerPosY2;

    _mapTileBlockXUC = adjustMapBlockPosX(*_playerBlockX, 16 + 0);
    _mapTileBlockXDC = adjustMapBlockPosX(*_playerBlockX, 16 + 0);
    _mapTileBlockXCL = adjustMapBlockPosX(*_playerBlockX, 16 - 1);
    _mapTileBlockXCR = adjustMapBlockPosX(*_playerBlockX, 16 + 1);
    _mapTileBlockXCC = adjustMapBlockPosX(*_playerBlockX, 16 + 0);
    _mapTileBlockXUL = adjustMapBlockPosX(*_playerBlockX, 16 - 1);
    _mapTileBlockXUR = adjustMapBlockPosX(*_playerBlockX, 16 + 1);
    _mapTileBlockXDL = adjustMapBlockPosX(*_playerBlockX, 16 - 1);
    _mapTileBlockXDR = adjustMapBlockPosX(*_playerBlockX, 16 + 1);

    _mapTileBlockYUC = adjustMapBlockPosY(*_playerBlockY, -1);
    _mapTileBlockYDC = adjustMapBlockPosY(*_playerBlockY, +1);
    _mapTileBlockYCL = adjustMapBlockPosY(*_playerBlockY, 0);
    _mapTileBlockYCR = adjustMapBlockPosY(*_playerBlockY, 0);
    _mapTileBlockYCC = adjustMapBlockPosY(*_playerBlockY, 0);
    _mapTileBlockYUL = adjustMapBlockPosY(*_playerBlockY, -1);
    _mapTileBlockYUR = adjustMapBlockPosY(*_playerBlockY, -1);
    _mapTileBlockYDL = adjustMapBlockPosY(*_playerBlockY, +1);
    _mapTileBlockYDR = adjustMapBlockPosY(*_playerBlockY, +1);

    _mapTileMemPosUC = getMapMemPos(_mapTileBlockXUC, _mapTileBlockYUC);
    _mapTileMemPosDC = getMapMemPos(_mapTileBlockXDC, _mapTileBlockYDC);
    _mapTileMemPosCL = getMapMemPos(_mapTileBlockXCL, _mapTileBlockYCL);
    _mapTileMemPosCR = getMapMemPos(_mapTileBlockXCR, _mapTileBlockYCR);
    _mapTileMemPosUL = getMapMemPos(_mapTileBlockXUL, _mapTileBlockYUL);
    _mapTileMemPosUR = getMapMemPos(_mapTileBlockXUR, _mapTileBlockYUR);
    _mapTileMemPosDL = getMapMemPos(_mapTileBlockXDL, _mapTileBlockYDL);
    _mapTileMemPosDR = getMapMemPos(_mapTileBlockXDR, _mapTileBlockYDR);
    _mapTileMemPosCC = getMapMemPos(_mapTileBlockXCC, _mapTileBlockYCC);

    _mapTileTypeUC = _srmMem[_mapTileMemPosUC];
    _mapTileTypeDC = _srmMem[_mapTileMemPosDC];
    _mapTileTypeCL = _srmMem[_mapTileMemPosCL];
    _mapTileTypeCR = _srmMem[_mapTileMemPosCR];
    _mapTileTypeUL = _srmMem[_mapTileMemPosUL];
    _mapTileTypeUR = _srmMem[_mapTileMemPosUR];
    _mapTileTypeDL = _srmMem[_mapTileMemPosDL];
    _mapTileTypeDR = _srmMem[_mapTileMemPosDR];
    _mapTileTypeCC = _srmMem[_mapTileMemPosCC];
   }

  __INLINE__ void ruleUpdatePreHook() override
  {
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_score, sizeof(_score));
    serializer.pushContiguous(&_prevScore, sizeof(_prevScore));
    serializer.pushContiguous(&_isKeyFrame, sizeof(_isKeyFrame));
    serializer.pushContiguous(&_playerPosX, sizeof(_playerPosX));
    serializer.pushContiguous(&_playerPosY, sizeof(_playerPosY));
    serializer.pushContiguous(&_playerPrevPosX, sizeof(_playerPrevPosX));
    serializer.pushContiguous(&_playerPrevPosY, sizeof(_playerPrevPosY));
    serializer.pushContiguous(&_prevBonusMultiplier, sizeof(_prevBonusMultiplier));
    serializer.pushContiguous(&_prevGhost1State, sizeof(_prevGhost1State));
    serializer.pushContiguous(&_prevGhost2State, sizeof(_prevGhost2State));
    serializer.pushContiguous(&_prevGhost3State, sizeof(_prevGhost3State));
    serializer.pushContiguous(&_prevGhost4State, sizeof(_prevGhost4State));
    serializer.pushContiguous(&_prevFruit1Status, sizeof(_prevFruit1Status));
    serializer.pushContiguous(&_prevFruit2Status, sizeof(_prevFruit2Status));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_score, sizeof(_score));
    deserializer.popContiguous(&_prevScore, sizeof(_prevScore));
    deserializer.popContiguous(&_isKeyFrame, sizeof(_isKeyFrame));
    deserializer.popContiguous(&_playerPosX, sizeof(_playerPosX));
    deserializer.popContiguous(&_playerPosY, sizeof(_playerPosY));
    deserializer.popContiguous(&_playerPrevPosX, sizeof(_playerPrevPosX));
    deserializer.popContiguous(&_playerPrevPosY, sizeof(_playerPrevPosY));
    deserializer.popContiguous(&_prevBonusMultiplier, sizeof(_prevBonusMultiplier));
    deserializer.popContiguous(&_prevGhost1State, sizeof(_prevGhost1State));
    deserializer.popContiguous(&_prevGhost2State, sizeof(_prevGhost2State));
    deserializer.popContiguous(&_prevGhost3State, sizeof(_prevGhost3State));
    deserializer.popContiguous(&_prevGhost4State, sizeof(_prevGhost4State));
    deserializer.popContiguous(&_prevFruit1Status, sizeof(_prevFruit1Status));
    deserializer.popContiguous(&_prevFruit2Status, sizeof(_prevFruit2Status));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from score
    float reward = (float) _score;

    // Rewarding time passing after capturing a ghost
    reward -= (float)*_ghostCaptureTimer2 * 0.001f;

    // Punishing time remaining for score
    // if (*_bonusMultiplierTimer > 0) 
    // {
    //   const float multiplier = 50.0f;
    //   const float maxTime = 600.0f;
    //   reward += multiplier * (float)*_bonusMultiplierTimer;
    // }

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:            0x%04X\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Score:                   %lu (Prev: %lu)\n", _score, _prevScore);
    jaffarCommon::logger::log("[J+]  + Is Key Frame:            %s\n", _isKeyFrame ? "True" : "False");
    jaffarCommon::logger::log("[J+]  + Bonus Multiplier:        %02u (Timer: %05u)\n", *_bonusMultiplier, *_bonusMultiplierTimer);
    jaffarCommon::logger::log("[J+]  + Ghost Capture Timer1:    0x%03X (Prev: 0x%03X)\n", *_ghostCaptureTimer1, _prevGhostCaptureTimer1);
    jaffarCommon::logger::log("[J+]  + Ghost Capture Timer2:    0x%03X\n", *_ghostCaptureTimer2);
    jaffarCommon::logger::log("[J+]  + Ghost States:            [ %02u %02u %02u %02u ]\n", *_ghost1State, *_ghost2State, *_ghost3State, *_ghost4State);
    jaffarCommon::logger::log("[J+]  + Prev Ghost States:       [ %02u %02u %02u %02u ]\n", _prevGhost1State, _prevGhost2State, _prevGhost3State, _prevGhost4State);
    jaffarCommon::logger::log("[J+]  + Fruit States:            [ %02u %02u ]\n", *_fruit1Status, *_fruit2Status);
    jaffarCommon::logger::log("[J+]  + Prev Fruit States:       [ %02u %02u ]\n", _prevFruit1Status, _prevFruit2Status);
    jaffarCommon::logger::log("[J+]  + Player Pos X:            %05u (Prev: %05u)\n", _playerPosX, _playerPrevPosX);
    jaffarCommon::logger::log("[J+]  + Player Pos Y:            %05u (Prev: %05u)\n", _playerPosY, _playerPrevPosY);

    jaffarCommon::logger::log("[J+]  + Tile Mem:       0x%04X    0x%04X    0x%04X\n", _mapTileMemPosUL, _mapTileMemPosUC, _mapTileMemPosUR);
    jaffarCommon::logger::log("[J+]  + Tile Mem:       0x%04X    0x%04X    0x%04X\n", _mapTileMemPosCL, _mapTileMemPosCC, _mapTileMemPosCR);
    jaffarCommon::logger::log("[J+]  + Tile Mem:       0x%04X    0x%04X    0x%04X\n", _mapTileMemPosDL, _mapTileMemPosDC, _mapTileMemPosDR);
    
    jaffarCommon::logger::log("[J+]  + Tile Pos:      (%02u, %02u)     (%02u, %02u)     (%02u, %02u)\n", _mapTileBlockXUL, _mapTileBlockYUL, _mapTileBlockXUC, _mapTileBlockYUC, _mapTileBlockXUR, _mapTileBlockYUR);
    jaffarCommon::logger::log("[J+]  + Tile Pos:      (%02u, %02u)     (%02u, %02u)     (%02u, %02u)\n", _mapTileBlockXCL, _mapTileBlockYCL, _mapTileBlockXCC, _mapTileBlockYCC, _mapTileBlockXCR, _mapTileBlockYCR);
    jaffarCommon::logger::log("[J+]  + Tile Pos:      (%02u, %02u)     (%02u, %02u)     (%02u, %02u)\n", _mapTileBlockXDL, _mapTileBlockYDL, _mapTileBlockXDC, _mapTileBlockYDC, _mapTileBlockXDR, _mapTileBlockYDR);

    jaffarCommon::logger::log("[J+]  + Tile Type:     %02u    %02u    %02u\n", _mapTileTypeUL, _mapTileTypeUC, _mapTileTypeUR);
    jaffarCommon::logger::log("[J+]  + Tile Type:     %02u    %02u    %02u\n", _mapTileTypeCL, _mapTileTypeCC, _mapTileTypeCR);
    jaffarCommon::logger::log("[J+]  + Tile Type:     %02u    %02u    %02u\n", _mapTileTypeDL, _mapTileTypeDC, _mapTileTypeDR);
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {

      // If capturing a ghost, only option is to press the current direction
      if (*_ghostCaptureTimer1 > 1)
      {
          if (*_playerDirection == 1) allowedInputSet.insert(allowedInputSet.end(), {_input_U});
          if (*_playerDirection == 2) allowedInputSet.insert(allowedInputSet.end(), {_input_D});
          if (*_playerDirection == 3) allowedInputSet.insert(allowedInputSet.end(), {_input_L});
          if (*_playerDirection == 4) allowedInputSet.insert(allowedInputSet.end(), {_input_R});
          return;
      }

      // Gauntlet
      bool allowU = false;
      bool allowD = false;
      bool allowL = false;
      bool allowR = false;

      // Only change directions at specific coordinates
      if (_mapTileTypeUC <= 3) allowU = true;
      if (_mapTileTypeDC <= 3) allowD = true;
      if (_mapTileTypeCL <= 3) allowL = true;
      if (_mapTileTypeCR <= 3) allowR = true;

      // Consider corners, when going up
      if (*_playerDirection == 1)  if (_mapTileTypeUR <= 3) { allowR = true; }
      if (*_playerDirection == 1)  if (_mapTileTypeUL <= 3) { allowL = true; }

      // Consider corners, when going down
      if (*_playerDirection == 2)  if (_mapTileTypeDR <= 3) { allowR = true; }
      if (*_playerDirection == 2)  if (_mapTileTypeDL <= 3) { allowL = true; }

      // Consider corners, when going left
      if (*_playerDirection == 3)  if (_mapTileTypeUL <= 3) { allowU = true; }
      if (*_playerDirection == 3)  if (_mapTileTypeDL <= 3) { allowD = true; }

      // Consider corners, when going Right
      if (*_playerDirection == 4)  if (_mapTileTypeUR <= 3) { allowU = true; }
      if (*_playerDirection == 4)  if (_mapTileTypeDR <= 3) { allowD = true; }

      // If we are already heading in that direction, allow to continue in any condition
      if (*_playerDirection == 1) allowU = true; // Up
      if (*_playerDirection == 2) allowD = true; // Down
      if (*_playerDirection == 3) allowL = true; // Left
      if (*_playerDirection == 4) allowR = true; // Right

      // Only allow to 180 degree turns on key frames
      if (*_playerDirection == 1 && _isKeyFrame == false) allowD = false;
      if (*_playerDirection == 2 && _isKeyFrame == false) allowU = false;
      if (*_playerDirection == 3 && _isKeyFrame == false) allowR = false;
      if (*_playerDirection == 4 && _isKeyFrame == false) allowL = false;

      // Adding the command
      if (allowU == true) allowedInputSet.insert(allowedInputSet.end(), {_input_U});
      if (allowD == true) allowedInputSet.insert(allowedInputSet.end(), {_input_D});
      if (allowL == true) allowedInputSet.insert(allowedInputSet.end(), {_input_L});
      if (allowR == true) allowedInputSet.insert(allowedInputSet.end(), {_input_R});

      /////// Method 1
      // Gauntlet
      // bool allowU = true;
      // bool allowD = true;
      // bool allowL = true;
      // bool allowR = true;

      // // Only allow to turn on key frames
      // if (*_playerDirection == 1 && _isKeyFrame == false) allowD = false;
      // if (*_playerDirection == 2 && _isKeyFrame == false) allowU = false;
      // if (*_playerDirection == 3 && _isKeyFrame == false) allowR = false;
      // if (*_playerDirection == 4 && _isKeyFrame == false) allowL = false;

      // // Only change directions at specific coordinates
      // if (_mapTileTypeUC > 3) allowU = false;
      // if (_mapTileTypeDC > 3) allowD = false;
      // if (_mapTileTypeCL > 3) allowL = false;
      // if (_mapTileTypeCR > 3) allowR = false;
      
      // // If we are already heading in that direction, allow to continue in any condition
      // if (*_playerDirection == 1) allowU = true; // Up
      // if (*_playerDirection == 2) allowD = true; // Down
      // if (*_playerDirection == 3) allowL = true; // Left
      // if (*_playerDirection == 4) allowR = true; // Right

      // // Adding the command
      // if (allowU == true) allowedInputSet.insert(allowedInputSet.end(), {_input_U});
      // if (allowD == true) allowedInputSet.insert(allowedInputSet.end(), {_input_D});
      // if (allowL == true) allowedInputSet.insert(allowedInputSet.end(), {_input_L});
      // if (allowR == true) allowedInputSet.insert(allowedInputSet.end(), {_input_R});

      /////// Method X

      // // Up
      // if (*_playerDirection == 1) allowedInputSet.insert(allowedInputSet.end(), {_input_U, _input_L, _input_R});
      // if (*_playerDirection == 1 && _isKeyFrame) allowedInputSet.insert(allowedInputSet.end(), {_input_D});

      // // Down
      // if (*_playerDirection == 2) allowedInputSet.insert(allowedInputSet.end(), {_input_D, _input_L, _input_R});
      // if (*_playerDirection == 2 && _isKeyFrame) allowedInputSet.insert(allowedInputSet.end(), {_input_U});

      // // Left
      // if (*_playerDirection == 3) allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_U, _input_D});
      // if (*_playerDirection == 3 && _isKeyFrame) allowedInputSet.insert(allowedInputSet.end(), {_input_R});

      // // Right
      // if (*_playerDirection == 4) allowedInputSet.insert(allowedInputSet.end(), {_input_R, _input_U, _input_D});
      // if (*_playerDirection == 4 && _isKeyFrame) allowedInputSet.insert(allowedInputSet.end(), {_input_L});

      // return;

      /////// Method 2
      // allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R, _input_U, _input_D});
      // return;

      /////// Method 3
      // // Up
      // if (*_playerDirection == 1)
      // {
      //   allowedInputSet.insert(allowedInputSet.end(), {_input_U });
      //   if (_playerPosY % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_D});
      //   if (_playerPosY % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R});
      // }

      // // Down
      // if (*_playerDirection == 2)
      // {
      //   allowedInputSet.insert(allowedInputSet.end(), {_input_D });
      //   if (_playerPosY % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_U});
      //   if (_playerPosY % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_L, _input_R});
      // }

      // // Left
      // if (*_playerDirection == 3)
      // {
      //   allowedInputSet.insert(allowedInputSet.end(), {_input_L });
      //   if (_playerPosX % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_R});
      //   if (_playerPosX % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_U, _input_D});
      // }

      // // Right
      // if (*_playerDirection == 4)
      // {
      //   allowedInputSet.insert(allowedInputSet.end(), {_input_R});
      //   if (_playerPosX % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_L});
      //   if (_playerPosX % 6 == 0) allowedInputSet.insert(allowedInputSet.end(), {_input_U, _input_D});
      // }
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
  uint8_t*  _playerBlockX;
  uint8_t*  _playerBlockY;
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

  uint8_t* _bonusMultiplier;
  uint16_t* _bonusMultiplierTimer;

  uint8_t* _fruit1Status;
  uint8_t* _fruit2Status;

  uint32_t _score;
  uint32_t _prevScore;
  uint16_t _playerPosX;
  uint16_t _playerPosY;
  uint16_t _playerPrevPosX;
  uint16_t _playerPrevPosY;
  uint8_t _prevBonusMultiplier;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
  uint8_t* _srmMem;

  uint16_t _mapTileMemPosUC ;
  uint16_t _mapTileMemPosDC ;
  uint16_t _mapTileMemPosCL ;
  uint16_t _mapTileMemPosCR ;
  uint16_t _mapTileMemPosUL ;
  uint16_t _mapTileMemPosUR ;
  uint16_t _mapTileMemPosDL ;
  uint16_t _mapTileMemPosDR ;
  uint16_t _mapTileMemPosCC ;

  uint8_t _mapTileBlockXUC ;
  uint8_t _mapTileBlockXDC ;
  uint8_t _mapTileBlockXCL ;
  uint8_t _mapTileBlockXCR ;
  uint8_t _mapTileBlockXUL ;
  uint8_t _mapTileBlockXUR ;
  uint8_t _mapTileBlockXDL ;
  uint8_t _mapTileBlockXDR ;
  uint8_t _mapTileBlockXCC ;
  
  uint8_t _mapTileBlockYUC ;
  uint8_t _mapTileBlockYDC ;
  uint8_t _mapTileBlockYCL ;
  uint8_t _mapTileBlockYCR ;
  uint8_t _mapTileBlockYUL ;
  uint8_t _mapTileBlockYUR ;
  uint8_t _mapTileBlockYDL ;
  uint8_t _mapTileBlockYDR ;
  uint8_t _mapTileBlockYCC ;

  uint8_t _mapTileTypeUC;
  uint8_t _mapTileTypeDC;
  uint8_t _mapTileTypeCL;
  uint8_t _mapTileTypeCR;
  uint8_t _mapTileTypeUL;
  uint8_t _mapTileTypeUR;
  uint8_t _mapTileTypeDL;
  uint8_t _mapTileTypeDR;
  uint8_t _mapTileTypeCC;

  InputSet::inputIndex_t _input_U;
  InputSet::inputIndex_t _input_D;
  InputSet::inputIndex_t _input_L;
  InputSet::inputIndex_t _input_R;

  uint16_t _currentStep;
  uint8_t _prevGhostCaptureTimer1;
  bool _isKeyFrame;

  uint8_t _prevGhost1State;
  uint8_t _prevGhost2State;
  uint8_t _prevGhost3State;
  uint8_t _prevGhost4State;
  uint8_t _prevFruit1Status;
  uint8_t _prevFruit2Status;

};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
