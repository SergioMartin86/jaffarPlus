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

class KungFu final : public jaffarPlus::Game
{
public:

  static __INLINE__ std::string getName() { return "NES / Kung Fu"; }

  KungFu(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    registerGameProperty("Game Mode"          ,&_lowMem[0x0051], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("gameSubMode"        ,&_lowMem[0x0008], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("gameTimer"          ,&_lowMem[0x0049], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("heroAction"         ,&_lowMem[0x0069], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("screenScroll1"      ,&_lowMem[0x00E5], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("screenScroll2"      ,&_lowMem[0x00D4], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss HP"            ,&_lowMem[0x04A5], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Hero HP"            ,&_lowMem[0x04A6], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("currentStage"       ,&_lowMem[0x0058], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("score1"             ,&_lowMem[0x0535], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("score2"             ,&_lowMem[0x0534], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("score3"             ,&_lowMem[0x0533], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("score4"             ,&_lowMem[0x0532], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("score5"             ,&_lowMem[0x0531], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("heroActionTimer"    ,&_lowMem[0x0021], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("heroScreenPosX"     ,&_lowMem[0x0094], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("heroScreenPosY"     ,&_lowMem[0x00B6], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("heroAirMode"        ,&_lowMem[0x036A], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemyShrugCounter"  ,&_lowMem[0x0378], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Enemy Grab Counter" ,&_lowMem[0x0374], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemyPosX"          ,&_lowMem[0x008E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemyAction"        ,&_lowMem[0x0080], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemyActionTimer"   ,&_lowMem[0x002B], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Boss Pos X"         ,&_lowMem[0x0093], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    registerGameProperty("enemy1PosX"          ,&_lowMem[0x008E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy1Action"        ,&_lowMem[0x0080], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy1ActionTimer"   ,&_lowMem[0x002B], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy2PosX"          ,&_lowMem[0x008F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy2Action"        ,&_lowMem[0x0081], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy2ActionTimer"   ,&_lowMem[0x002C], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy3PosX"          ,&_lowMem[0x0090], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy3Action"        ,&_lowMem[0x0082], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy3ActionTimer"   ,&_lowMem[0x002D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy4PosX"          ,&_lowMem[0x0091], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy4Action"        ,&_lowMem[0x0083], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("enemy4ActionTimer"   ,&_lowMem[0x002E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    registerGameProperty("Hero Pos X"   ,&_heroPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Hero Pos Y"   ,&_heroPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _gameMode            =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game Mode"          )]->getPointer();
    _gameSubMode         =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("gameSubMode"        )]->getPointer();
    _gameTimer           =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("gameTimer"          )]->getPointer();
    _heroAction          =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("heroAction"         )]->getPointer();
    _screenScroll1       =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("screenScroll1"      )]->getPointer();
    _screenScroll2       =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("screenScroll2"      )]->getPointer();
    _bossHP              =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss HP"            )]->getPointer();
    _heroHP              =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Hero HP"            )]->getPointer();
    _currentStage        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("currentStage"       )]->getPointer();
    _score1              =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("score1"             )]->getPointer();
    _score2              =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("score2"             )]->getPointer();
    _score3              =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("score3"             )]->getPointer();
    _score4              =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("score4"             )]->getPointer();
    _score5              =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("score5"             )]->getPointer();
    _heroActionTimer     =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("heroActionTimer"    )]->getPointer();
    _heroScreenPosX      =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("heroScreenPosX"     )]->getPointer();
    _heroScreenPosY      =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("heroScreenPosY"     )]->getPointer();
    _heroAirMode         =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("heroAirMode"        )]->getPointer();
    _enemyShrugCounter   =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemyShrugCounter"  )]->getPointer();
    _enemyGrabCounter    =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Enemy Grab Counter" )]->getPointer();
    _bossPosX            =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Boss Pos X"         )]->getPointer();

    _enemy1PosX          =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy1PosX"          )]->getPointer();
    _enemy1Action        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy1Action"        )]->getPointer();
    _enemy1ActionTimer   =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy1ActionTimer"   )]->getPointer();

    _enemy2PosX          =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy2PosX"          )]->getPointer();
    _enemy2Action        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy2Action"        )]->getPointer();
    _enemy2ActionTimer   =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy2ActionTimer"   )]->getPointer();

    _enemy3PosX          =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy3PosX"          )]->getPointer();
    _enemy3Action        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy3Action"        )]->getPointer();
    _enemy3ActionTimer   =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy3ActionTimer"   )]->getPointer();

    _enemy4PosX          =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy4PosX"          )]->getPointer();
    _enemy4Action        =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy4Action"        )]->getPointer();
    _enemy4ActionTimer   =   (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("enemy4ActionTimer"   )]->getPointer();

    _nullInputIdx      = _emulator->registerInput("|..|........|");
    _input_U    = _emulator->registerInput("|..|U.......|");
    _input_D    = _emulator->registerInput("|..|.D......|");
    _input_L    = _emulator->registerInput("|..|..L.....|");
    _input_R    = _emulator->registerInput("|..|...R....|");
    _input_A    = _emulator->registerInput("|..|.......A|");
    _input_B    = _emulator->registerInput("|..|......B.|");
    _input_LR   = _emulator->registerInput("|..|..LR....|");
    _input_UL   = _emulator->registerInput("|..|U.L.....|");
    _input_UR   = _emulator->registerInput("|..|U..R....|");
    _input_UA   = _emulator->registerInput("|..|U......A|");
    _input_UB   = _emulator->registerInput("|..|U.....B.|");
    _input_DL   = _emulator->registerInput("|..|.DL.....|");
    _input_DR   = _emulator->registerInput("|..|.D.R....|");
    _input_DA   = _emulator->registerInput("|..|.D.....A|");
    _input_DB   = _emulator->registerInput("|..|.D....B.|");
    _input_AL   = _emulator->registerInput("|..|..L....A|");
    _input_BL   = _emulator->registerInput("|..|..L...B.|");
    _input_AR   = _emulator->registerInput("|..|...R...A|");
    _input_BR   = _emulator->registerInput("|..|...R..B.|");
    _input_ULA  = _emulator->registerInput("|..|U.L....A|");
    _input_URA  = _emulator->registerInput("|..|U..R...A|");
    _input_ULB  = _emulator->registerInput("|..|U..R..B.|");
    _input_DLA  = _emulator->registerInput("|..|.DL....A|");
    _input_DRA  = _emulator->registerInput("|..|.D.R...A|");
    _input_DLB  = _emulator->registerInput("|..|.DL...B.|");
    _input_DRB  = _emulator->registerInput("|..|.D.R..B.|");

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
    hashEngine.Update(*_gameMode);
    hashEngine.Update(*_gameSubMode);
    hashEngine.Update(*_heroAction);
    hashEngine.Update(*_screenScroll1);
    hashEngine.Update(*_screenScroll2);
    hashEngine.Update(*_bossHP);
    hashEngine.Update(*_heroHP);
    hashEngine.Update(*_currentStage);
    hashEngine.Update(*_score1);
    hashEngine.Update(*_score2);
    hashEngine.Update(*_score3);
    hashEngine.Update(*_score4);
    hashEngine.Update(*_score5);
    hashEngine.Update(*_heroActionTimer);
    hashEngine.Update(*_heroScreenPosX);
    hashEngine.Update(*_heroScreenPosY);
    hashEngine.Update(*_heroAirMode);
    hashEngine.Update(*_enemyShrugCounter);
    hashEngine.Update(*_enemyGrabCounter);
    hashEngine.Update(*_bossPosX);

    // hashEngine.Update(*_enemy1PosX       );
    // hashEngine.Update(_enemy1Action      );
    // hashEngine.Update(_enemy1ActionTimer );
    // hashEngine.Update(*_enemy2PosX       );
    // hashEngine.Update(_enemy2Action      );
    // hashEngine.Update(_enemy2ActionTimer );
    // hashEngine.Update(*_enemy3PosX       );
    // hashEngine.Update(_enemy3Action      );
    // hashEngine.Update(_enemy3ActionTimer );
    // hashEngine.Update(*_enemy4PosX       );
    // hashEngine.Update(_enemy4Action      );
    // hashEngine.Update(_enemy4ActionTimer );

    // Action states
    hashEngine.Update(&_lowMem[0x0065], 0x10);

    hashEngine.Update(_lowMem[0x0025]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x003C]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x007A]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x007C]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x00BC]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x036B]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x03A5]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x0420]); // boss fight stuff?
    hashEngine.Update(_lowMem[0x04C3]); // boss fight stuff?

  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _heroPosX = (float)*_screenScroll1 * 256.0f + (float)*_screenScroll2 + (float)*_heroScreenPosX;
    _heroPosY =  (float)*_heroScreenPosY;
    _score = (float)*_score5 * 100000.0f + (float)*_score4 * 10000.0f + (float)*_score3 * 1000.0f + (float)*_score2 * 100.0f + (float)*_score1 * 10.0f;

    _enemyPosImbalance = 0;
    if (*_enemy1ActionTimer != 0 && *_enemy1Action == 3) _enemyPosImbalance++;
    if (*_enemy2ActionTimer != 0 && *_enemy2Action == 3) _enemyPosImbalance++;
    if (*_enemy3ActionTimer != 0 && *_enemy3Action == 3) _enemyPosImbalance++;
    if (*_enemy4ActionTimer != 0 && *_enemy4Action == 3) _enemyPosImbalance++;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
      _heroPosXMagnet.intensity = 0.0f;
      _heroPosYMagnet.intensity = 0.0f;
      _bossPosXMagnet.intensity = 0.0f;
      _heroPosXMagnet.pos = 0.0f;
      _heroPosYMagnet.pos = 0.0f;
      _bossPosXMagnet.pos = 0.0f;
      _heroHPMagnet = 0.0f;
      _bossHPMagnet = 0.0f;
      _enemyPosImbalanceMagnet = 0.0f;
      _scoreMagnet = 0.0f;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    _heroDistanceToPointX = std::abs((float)_heroPosXMagnet.pos - (float)_heroPosX);
    _heroDistanceToPointY = std::abs((float)_heroPosYMagnet.pos - (float)_heroPosY);
    _bossDistanceToPointX = std::abs((float)_bossPosXMagnet.pos - (float)*_bossPosX);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_lastInput, sizeof(_lastInput));
     serializer.push(&_enemyPosImbalance, sizeof(_enemyPosImbalance));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_lastInput, sizeof(_lastInput));
     deserializer.pop(&_enemyPosImbalance, sizeof(_enemyPosImbalance));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Evaluating point magnet
    reward += -1.0 * _heroPosXMagnet.intensity * _heroDistanceToPointX;
    reward += -1.0 * _heroPosYMagnet.intensity * _heroDistanceToPointY;
    reward += -1.0 * _bossPosXMagnet.intensity * _bossDistanceToPointX;

    // Evaluating boss health magnet
    auto bossHPCorrected = *_bossHP;
    if (bossHPCorrected < 0) bossHPCorrected = 0;
    reward += _bossHPMagnet * bossHPCorrected;

    // Evaluating hero health  magnet
    reward += _heroHPMagnet * (float)*_heroHP;

    // Evaluating hero health  magnet
    reward += _scoreMagnet * (float)_score;

    // Evaluating position imbalance magnet
    reward += _enemyPosImbalanceMagnet * (float)_enemyPosImbalance;

    // Returning reward
    return reward;
  }

      // Function to report what all the possible input that the game might require
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    return {
      "|..|........|",
      "|..|U.......|",
      "|..|.D......|",
      "|..|..L.....|",
      "|..|...R....|",
      "|..|.......A|",
      "|..|......B.|",
      "|..|U.L.....|",
      "|..|U..R....|",
      "|..|U......A|",
      "|..|U.....B.|",
      "|..|.DL.....|",
      "|..|.D.R....|",
      "|..|.D.....A|",
      "|..|.D....B.|",
      "|..|..L....A|",
      "|..|..L...B.|",
      "|..|...R...A|",
      "|..|...R..B.|",
      "|..|..LR....|",
      "|..|.DL....A|",  
      "|..|.DL...B.|",
      "|..|.D.R...A|",
      "|..|.D.R..B.|",
      "|..|U.L....A|",  
      "|..|U.L...B.|",
      "|..|U..R...A|",
      "|..|U..R..B.|"  
    };
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    
      if (*_gameMode != 2) return;

      // Getting mini-hash
      auto stateMiniHash = getStateMoveHash();

      allowedInputSet.insert(allowedInputSet.end(), { _nullInputIdx });

      switch (stateMiniHash)
      {
        case 0x0000: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0001: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0002: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0003: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0004: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0005: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0006: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0010: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AL, _input_BL, _input_UL, _input_DL, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0011: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AL, _input_BL, _input_UL, _input_DL, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0012: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AL, _input_BL, _input_UL, _input_DL, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0013: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AL, _input_BL, _input_UL, _input_DL, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0014: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AL, _input_BL, _input_UL, _input_DL, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0015: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AL, _input_BL, _input_UL, _input_DL, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0016: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AL, _input_BL, _input_UL, _input_DL, _input_LR, _input_UL, _input_UR, _input_DA, _input_DB,_input_URA, _input_URB, _input_ULA, _input_ULB }); break;
        case 0x0020: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_DA, _input_DB, _input_DL, _input_UR, _input_DR, _input_DRA, _input_DRB }); break;
        case 0x0030: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_DA, _input_DB, _input_UL, _input_DL, _input_DR, _input_DLA, _input_DLB }); break;
        case 0x0041: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0042: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0043: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0044: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0045: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0046: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0047: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0048: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0049: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x004A: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x004B: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x004C: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0051: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0052: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0053: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0054: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0055: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0056: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0057: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0058: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0059: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x005A: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x005B: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x005C: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_DA, _input_DB }); break;
        case 0x0101: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0102: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0103: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0104: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0105: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_BR, _input_UR, _input_DR }); break;
        case 0x0106: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_BR, _input_UR, _input_DR }); break;
        case 0x0107: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_BR, _input_UR, _input_DR }); break;
        case 0x0108: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_BR, _input_UR, _input_DR }); break;
        case 0x0109: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_BR, _input_UR, _input_DR }); break;
        case 0x010A: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_BR, _input_UR, _input_DR }); break;
        case 0x010B: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_R, _input_BR, _input_UR, _input_DR }); break;
        case 0x0111: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0112: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0113: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0114: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0115: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_BL, _input_UL, _input_DL }); break;
        case 0x0116: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_BL, _input_UL, _input_DL }); break;
        case 0x0117: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_BL, _input_UL, _input_DL }); break;
        case 0x0118: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_BL, _input_UL, _input_DL }); break;
        case 0x0119: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_BL, _input_UL, _input_DL }); break;
        case 0x011A: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_BL, _input_UL, _input_DL }); break;
        case 0x011B: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_D, _input_L, _input_BL, _input_UL, _input_DL }); break;
        case 0x0121: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0122: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0123: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0124: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0125: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0126: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0127: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0128: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_R, _input_BR, _input_UR }); break;
        case 0x0129: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_R, _input_BR, _input_UR }); break;
        case 0x012A: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_R, _input_BR, _input_UR }); break;
        case 0x012B: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_R, _input_BR, _input_UR }); break;
        case 0x0131: allowedInputSet.insert(allowedInputSet.end(), { _input_L }); break;
        case 0x0132: allowedInputSet.insert(allowedInputSet.end(), { _input_L }); break;
        case 0x0133: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L }); break;
        case 0x0134: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_L }); break;
        case 0x0135: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L }); break;
        case 0x0136: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L }); break;
        case 0x0137: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L }); break;
        case 0x0138: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L, _input_BL, _input_UL }); break;
        case 0x0139: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L, _input_BL, _input_UL }); break;
        case 0x013A: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L, _input_BL, _input_UL }); break;
        case 0x013B: allowedInputSet.insert(allowedInputSet.end(), { _input_B, _input_U, _input_L, _input_BL, _input_UL }); break;
        case 0x0201: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0202: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0203: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0204: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0205: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_R, _input_AR, _input_UR, _input_DR }); break;
        case 0x0206: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_R, _input_AR, _input_UR, _input_DR }); break;
        case 0x0207: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_R, _input_AR, _input_UR, _input_DR }); break;
        case 0x0208: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_R, _input_AR, _input_UR, _input_DR }); break;
        case 0x0209: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_R, _input_AR, _input_UR, _input_DR }); break;
        case 0x020A: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_R, _input_AR, _input_UR, _input_DR }); break;
        case 0x020B: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_R, _input_AR, _input_UR, _input_DR }); break;
        case 0x0211: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0212: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0213: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0214: allowedInputSet.insert(allowedInputSet.end(), { _input_D, _input_L }); break;
        case 0x0215: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_L, _input_AL, _input_UL, _input_DL }); break;
        case 0x0216: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_L, _input_AL, _input_UL, _input_DL }); break;
        case 0x0217: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_L, _input_AL, _input_UL, _input_DL }); break;
        case 0x0218: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_L, _input_AL, _input_UL, _input_DL }); break;
        case 0x0219: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_L, _input_AL, _input_UL, _input_DL }); break;
        case 0x021A: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_L, _input_AL, _input_UL, _input_DL }); break;
        case 0x021B: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_D, _input_L, _input_AL, _input_UL, _input_DL }); break;
        case 0x0221: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0222: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0223: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0224: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0225: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0226: allowedInputSet.insert(allowedInputSet.end(), { _input_R }); break;
        case 0x0227: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_R, _input_DA, _input_DB,  _input_AR, _input_UR }); break;
        case 0x0228: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_R, _input_DA, _input_DB,  _input_AR, _input_UR }); break;
        case 0x0229: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_R, _input_DA, _input_DB,  _input_AR, _input_UR }); break;
        case 0x022A: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_R, _input_DA, _input_DB,  _input_AR, _input_UR }); break;
        case 0x022B: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_R, _input_DA, _input_DB,  _input_AR, _input_UR }); break;
        case 0x0231: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB }); break;
        case 0x0232: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB }); break;
        case 0x0233: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB }); break;
        case 0x0234: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB }); break;
        case 0x0235: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB }); break;
        case 0x0236: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB }); break;
        case 0x0237: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB,  _input_AL, _input_UL }); break;
        case 0x0238: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB,  _input_AL, _input_UL }); break;
        case 0x0239: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB,  _input_AL, _input_UL }); break;
        case 0x023A: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB,  _input_AL, _input_UL }); break;
        case 0x023B: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_U, _input_L, _input_DA, _input_DB,  _input_AL, _input_UL }); break;
        default: allowedInputSet.insert(allowedInputSet.end(), { _input_A, _input_B, _input_U, _input_D, _input_L, _input_R, _input_AR, _input_BR, _input_UR, _input_DR, _input_LR, _input_UL, _input_UR, _input_URA, _input_URB, _input_ULA, _input_ULB }); break;
      }
  }

  __INLINE__ uint64_t getStateMoveHash() const
  {
  return *_heroAction * 16 + *_heroActionTimer;
  }

  void printInfoImpl() const override
  {
    auto stateMiniHash = getStateMoveHash();

    jaffarCommon::logger::log("[J+]  + Current Step:                     %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Game Mode:                        %02u-%02u\n", *_gameMode, *_gameSubMode);
    jaffarCommon::logger::log("[J+]  + Score:                            %f\n", _score);
    jaffarCommon::logger::log("[J+]  + Hero HP:                          %02d\n", *_heroHP);
    jaffarCommon::logger::log("[J+]  + Hero Air Mode:                    %02u\n", *_heroAirMode);
    jaffarCommon::logger::log("[J+]  + Hero Action:                      0x%02X (0x%02X) (Hash: %02X)\n", *_heroAction, *_heroActionTimer, stateMiniHash);
    jaffarCommon::logger::log("[J+]  + Hero Position X:                  %f\n", _heroPosX);
    jaffarCommon::logger::log("[J+]  + Hero Position Y:                  %f\n", _heroPosY);
    jaffarCommon::logger::log("[J+]  + Boss HP:                          %02d\n", *_bossHP);
    jaffarCommon::logger::log("[J+]  + Boss Pos X:                       %02d\n", *_bossPosX);
    jaffarCommon::logger::log("[J+]  + Enemy Position Imbalance:         %02d\n", _enemyPosImbalance);
    jaffarCommon::logger::log("[J+]  + Enemy Shrug Counter:              %02u\n", *_enemyShrugCounter);
    jaffarCommon::logger::log("[J+]  + Enemy Grab Counter:               %02u\n", *_enemyGrabCounter);
    jaffarCommon::logger::log("[J+]  + Enemy 1                           X: %3u, Action: %3u, Timer: %3u\n", *_enemy1PosX, *_enemy1Action, *_enemy1ActionTimer);
    jaffarCommon::logger::log("[J+]  + Enemy 2                           X: %3u, Action: %3u, Timer: %3u\n", *_enemy2PosX, *_enemy2Action, *_enemy2ActionTimer);
    jaffarCommon::logger::log("[J+]  + Enemy 3                           X: %3u, Action: %3u, Timer: %3u\n", *_enemy3PosX, *_enemy3Action, *_enemy3ActionTimer);
    jaffarCommon::logger::log("[J+]  + Enemy 4                           X: %3u, Action: %3u, Timer: %3u\n", *_enemy4PosX, *_enemy4Action, *_enemy4ActionTimer);


    if (std::abs(_heroHPMagnet) > 0.0f) 
       jaffarCommon::logger::log("[J+]  + Hero HP Magnet                    Intensity: %.5f\n", _heroHPMagnet);
    if (std::abs(_bossHPMagnet) > 0.0f)
       jaffarCommon::logger::log("[J+]  + Boss HP Magnet                    Intensity: %.5f\n", _bossHPMagnet);
    if (std::abs(_scoreMagnet) > 0.0f) 
       jaffarCommon::logger::log("[J+]  + Score Magnet                      Intensity: %.5f\n", _scoreMagnet);
    if (std::abs(_enemyPosImbalanceMagnet) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Position Imbalance Magnet         Intensity: %.5f\n", _enemyPosImbalanceMagnet);
    if (std::abs(_heroPosXMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Hero Pos X Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _heroPosXMagnet.intensity, _heroPosXMagnet.pos, _heroDistanceToPointX);
    if (std::abs(_heroPosYMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Hero Pos Y Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _heroPosYMagnet.intensity, _heroPosYMagnet.pos, _heroDistanceToPointY);
    if (std::abs(_bossPosXMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Boss Pos X Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _bossPosXMagnet.intensity, _bossPosXMagnet.pos, _bossDistanceToPointX);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Hero Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_heroPosXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Hero Pos Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_heroPosYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Boss Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_bossPosXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos }; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Hero HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_heroHPMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Boss HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bossHPMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Enemy Imbalance Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_enemyPosImbalanceMagnet = intensity; });
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

  __INLINE__ void playerPrintCommands() const override
  {
    // jaffarCommon::logger::log("[J+] t: Print Initial Info\n");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
     return false;
  };

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float pos       = 0.0; // What is the point of attraction X
  };

  uint16_t _currentStep;
  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _lastInput;
  uint8_t* _lowMem;

  pointMagnet_t _heroPosXMagnet;
  pointMagnet_t _heroPosYMagnet;
  pointMagnet_t _bossPosXMagnet;
  float _heroHPMagnet;
  float _bossHPMagnet;
  float _enemyPosImbalanceMagnet;
  float _scoreMagnet;

  float _heroDistanceToPointX;
  float _heroDistanceToPointY;
  float _bossDistanceToPointX;

  uint8_t* _gameMode          ; 
  uint8_t* _gameSubMode       ; 
  uint8_t* _gameTimer         ; 
  uint8_t* _heroAction        ; 
  uint8_t* _screenScroll1     ; 
  uint8_t* _screenScroll2     ; 
  uint8_t* _bossHP            ; 
  uint8_t* _heroHP            ; 
  uint8_t* _currentStage      ; 
  uint8_t* _score1            ; 
  uint8_t* _score2            ; 
  uint8_t* _score3            ; 
  uint8_t* _score4            ; 
  uint8_t* _score5            ; 
  uint8_t* _heroActionTimer   ; 
  uint8_t* _heroScreenPosX    ; 
  uint8_t* _heroScreenPosY    ; 
  uint8_t* _heroAirMode       ; 
  uint8_t* _enemyShrugCounter ; 
  uint8_t* _enemyGrabCounter  ; 
  uint8_t* _bossPosX          ; 

  uint8_t* _enemy1PosX        ;
  uint8_t* _enemy1Action      ;
  uint8_t* _enemy1ActionTimer ;
  uint8_t* _enemy2PosX        ;
  uint8_t* _enemy2Action      ;
  uint8_t* _enemy2ActionTimer ;
  uint8_t* _enemy3PosX        ;
  uint8_t* _enemy3Action      ;
  uint8_t* _enemy3ActionTimer ;
  uint8_t* _enemy4PosX        ;
  uint8_t* _enemy4Action      ;
  uint8_t* _enemy4ActionTimer ;

  uint8_t _enemyPosImbalance;
  float _heroPosX;
  float _heroPosY;
  float _score;

  // Possible inputs
  InputSet::inputIndex_t _input_U  ;
  InputSet::inputIndex_t _input_D  ;
  InputSet::inputIndex_t _input_L  ;
  InputSet::inputIndex_t _input_R  ;
  InputSet::inputIndex_t _input_A  ;
  InputSet::inputIndex_t _input_B  ;
  InputSet::inputIndex_t _input_UL ;
  InputSet::inputIndex_t _input_UR ;
  InputSet::inputIndex_t _input_UA ;
  InputSet::inputIndex_t _input_UB ;
  InputSet::inputIndex_t _input_DL ;
  InputSet::inputIndex_t _input_DR ;
  InputSet::inputIndex_t _input_DA ;
  InputSet::inputIndex_t _input_DB ;
  InputSet::inputIndex_t _input_AL ;
  InputSet::inputIndex_t _input_BL ;
  InputSet::inputIndex_t _input_AR ;
  InputSet::inputIndex_t _input_BR ;
  InputSet::inputIndex_t _input_LR ;
  InputSet::inputIndex_t _input_ULA;
  InputSet::inputIndex_t _input_URA;
  InputSet::inputIndex_t _input_DLA;
  InputSet::inputIndex_t _input_DRA;
  InputSet::inputIndex_t _input_ULB;
  InputSet::inputIndex_t _input_URB;
  InputSet::inputIndex_t _input_DLB;
  InputSet::inputIndex_t _input_DRB;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
