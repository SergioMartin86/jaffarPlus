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

class DonkeyKong final : public jaffarPlus::Game
{
public:

  static __INLINE__ std::string getName() { return "NES / Donkey Kong"; }

  DonkeyKong(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;
    
     _globalTimer  =  (uint8_t *)registerGameProperty("Global Timer", &_lowMem[0x06F0], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _levelTimer   =  (uint8_t *)registerGameProperty("Level Timer" , &_lowMem[0x0034], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _marioPosX    =  (uint8_t *)registerGameProperty("Mario Pos X" , &_lowMem[0x0046], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _marioPosY    =  (uint8_t *)registerGameProperty("Mario Pos Y" , &_lowMem[0x0047], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _gameState    =  (uint8_t *)registerGameProperty("Game State"  , &_lowMem[0x004F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _marioAlive   =  (uint8_t *)registerGameProperty("Mario Alive" , &_lowMem[0x00FC], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _marioAlive2  =  (uint8_t *)registerGameProperty("Mario Alive2", &_lowMem[0x00AE], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     
     _piece0State = (uint8_t*) 	registerGameProperty("Piece 0 State", &_lowMem[0x00C1], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece1State = (uint8_t*) 	registerGameProperty("Piece 1 State", &_lowMem[0x00C2], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece2State = (uint8_t*) 	registerGameProperty("Piece 2 State", &_lowMem[0x00C3], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece3State = (uint8_t*) 	registerGameProperty("Piece 3 State", &_lowMem[0x00C4], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece4State = (uint8_t*) 	registerGameProperty("Piece 4 State", &_lowMem[0x00C5], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece5State = (uint8_t*) 	registerGameProperty("Piece 5 State", &_lowMem[0x00C6], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece6State = (uint8_t*) 	registerGameProperty("Piece 6 State", &_lowMem[0x00C7], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece7State = (uint8_t*) 	registerGameProperty("Piece 7 State", &_lowMem[0x00C8], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece8State = (uint8_t*) 	registerGameProperty("Piece 8 State", &_lowMem[0x00C9], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
     _piece9State = (uint8_t*) 	registerGameProperty("Piece 9 State", &_lowMem[0x00CA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

     registerGameProperty("Pieces Picked Up", &_piecesPickedUp, Property::datatype_t::dt_uint8 , Property::endianness_t::little);

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
    hashEngine.Update(*_marioPosX);
    hashEngine.Update(*_marioPosY);
    hashEngine.Update(*_gameState);
    hashEngine.Update(*_marioAlive);
    hashEngine.Update(*_marioAlive2);

     hashEngine.Update(*_piece0State);
     hashEngine.Update(*_piece1State);
     hashEngine.Update(*_piece2State);
     hashEngine.Update(*_piece3State);
     hashEngine.Update(*_piece4State);
     hashEngine.Update(*_piece5State);
     hashEngine.Update(*_piece6State);
     hashEngine.Update(*_piece7State);
     hashEngine.Update(*_piece8State);
     hashEngine.Update(*_piece9State);

    hashEngine.Update(&_lowMem[0x0050], 0x0050);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _piecesPickedUp = 0;
    _piecesPickedUp += *_piece0State;
    _piecesPickedUp += *_piece1State;
    _piecesPickedUp += *_piece2State;
    _piecesPickedUp += *_piece3State;
    _piecesPickedUp += *_piece4State;
    _piecesPickedUp += *_piece5State;
    _piecesPickedUp += *_piece6State;
    _piecesPickedUp += *_piece7State;
    _piecesPickedUp += *_piece8State;
    _piecesPickedUp += *_piece9State;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    _marioDistanceToPointX = std::abs((float)_marioPosXMagnet.pos - (float)*_marioPosX);
    _marioDistanceToPointY = std::abs((float)_marioPosYMagnet.pos - (float)*_marioPosY);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_piecesPickedUp, sizeof(_piecesPickedUp));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_piecesPickedUp, sizeof(_piecesPickedUp));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Evaluating point magnet
    reward += -1.0 * _marioPosXMagnet.intensity * _marioDistanceToPointX;
    reward += -1.0 * _marioPosYMagnet.intensity * _marioDistanceToPointY;

    reward += _piecesPickedUp * 1000.0f;

    // Returning reward
    return reward;
  }

      // Function to report what all the possible input that the game might require
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    return {
      "|..|........|",
      "|..|...R....|",
      "|..|..L.....|",
      "|..|U.......|",
      "|..|.D......|",
      "|..|........|",
      "|..|...R...A|",
      "|..|.......A|",
      "|..|.D.....A|",
      "|..|U......A|"
    };
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
      allowedInputSet.insert(allowedInputSet.end(), {
          _nullInputIdx,
          _input_U, 
          _input_D, 
          _input_L, 
          _input_R, 
          _input_A,
          _input_AR,
          _input_DA,
          _input_UA
         });
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:                      %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Game State:                        %02u\n", *_gameState);
    jaffarCommon::logger::log("[J+]  + Mario Alive:                       %02u\n", *_marioAlive);
    jaffarCommon::logger::log("[J+]  + Mario Alive2:                      %02u\n", *_marioAlive2);
    jaffarCommon::logger::log("[J+]  + Mario Position X:                  %02u\n", *_marioPosX);
    jaffarCommon::logger::log("[J+]  + Mario Position Y:                  %02u\n", *_marioPosY);
    jaffarCommon::logger::log("[J+]  + Pieces:                            %02u [ %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u ]\n", _piecesPickedUp, *_piece0State, *_piece1State, *_piece2State, *_piece3State, *_piece4State, *_piece5State, *_piece6State, *_piece7State, *_piece8State, *_piece9State);


    if (std::abs(_marioPosXMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Mario Pos X Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _marioPosXMagnet.intensity, _marioPosXMagnet.pos, _marioDistanceToPointX);
    if (std::abs(_marioPosYMagnet.intensity) > 0.0f)  
       jaffarCommon::logger::log("[J+]  + Mario Pos Y Magnet                 Intensity: %.5f, Pos: %.5f, Distance: %.5f\n", _marioPosYMagnet.intensity, _marioPosYMagnet.pos, _marioDistanceToPointY);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Mario Pos X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_marioPosXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Mario Pos Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      auto action    = [=, this]() { this->_marioPosYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; };
      rule.addAction(action);
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

  pointMagnet_t _marioPosXMagnet;
  pointMagnet_t _marioPosYMagnet;
  float _marioDistanceToPointX;
  float _marioDistanceToPointY;

  uint8_t* _marioPosX;
  uint8_t* _marioPosY;
  uint8_t* _marioAlive;
  uint8_t* _marioAlive2;
  uint8_t* _gameState;
  uint8_t* _globalTimer;
  uint8_t* _levelTimer;

  uint8_t*  _piece0State;
  uint8_t*  _piece1State;
  uint8_t*  _piece2State;
  uint8_t*  _piece3State;
  uint8_t*  _piece4State;
  uint8_t*  _piece5State;
  uint8_t*  _piece6State;
  uint8_t*  _piece7State;
  uint8_t*  _piece8State;
  uint8_t*  _piece9State;

  uint8_t _piecesPickedUp;

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
