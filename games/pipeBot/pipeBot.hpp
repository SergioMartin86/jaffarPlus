#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <atomic>
#include <jaffarCommon/json.hpp>

namespace jaffarPlus
{

namespace games
{

namespace pipeBot
{

class PipeBot final : public jaffarPlus::Game
{
public:

  struct piecePath_t
  {
    emulator::PipeBot::piecePos_t pos;
    emulator::PipeBot::direction_t direction;
  };

  static __INLINE__ std::string getName() { return "PipeBot / PipeBot"; }

  PipeBot(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) 
    : jaffarPlus::Game(std::move(emulator), config),
      _pipeBot((emulator::PipeBot*)_emulator.get())
  {
    _rowCount = _pipeBot->getRowCount();
    _colCount = _pipeBot->getColCount();

    const auto distanceLimiterJs = jaffarCommon::json::getObject(config, "Distance Limiter");
    _distanceLimiterMaxRowDistance = jaffarCommon::json::getNumber<uint8_t>(distanceLimiterJs, "Max Row Distance");
    _distanceLimiterMaxColDistance = jaffarCommon::json::getNumber<uint8_t>(distanceLimiterJs, "Max Col Distance");
  }

  struct possibleInput_t
  {
    std::string inputString;
    InputSet::inputIndex_t inputIndex;
    uint8_t row;
    uint8_t col;
  };

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _grid = _emulator->getProperty("Grid").pointer;

    registerGameProperty("Forward Depth"            ,&_forwardDepth, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Pieces On Board"          ,&_piecesOnBoard, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Target Score"             ,&_targetScore, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Lingering Pieces"         ,&_lingeringPieces, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Replaced Pieces"         ,&_piecesReplaced, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Last Crossing Piece Distance To Goal",&_lastCrossingPieceDistanceToGoal, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Remaining Free Places",  &_remainingFreePlaces, Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    registerGameProperty("Score",  &_score, Property::datatype_t::dt_int32 , Property::endianness_t::little);

    // Getting initial pieces
    for (const auto& piece : _pipeBot->getInitialPieces()) _initialPieces.insert({piece.pos.row, piece.pos.col});

    // Looking for starter piece
    _startPieceFound = false;
    for (uint8_t i = 0; i < _rowCount; i++)
     for (uint8_t j = 0; j < _colCount; j++)
     {
      const auto pieceType = _pipeBot->getPiece({i,j});
      
      // Start Pieces
      if (pieceType == 0x01) // Up Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::up;
        _playerInitialRow = i-1;
        _playerInitialCol = j;
        _startPieceFound = true;
      }

      if (pieceType == 0x02) // Down Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::down;
        _playerInitialRow = i+1;
        _playerInitialCol = j;
        _startPieceFound = true;
      }

      if (pieceType == 0x08) // Right Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::right;
        _playerInitialRow = i;
        _playerInitialCol = j+1;
        _startPieceFound = true;
      }

      if (pieceType == 0x04) // Left Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::left;
        _playerInitialRow = i;
        _playerInitialCol = j-1;
        _startPieceFound = true;
      }
     }
    if (_startPieceFound == false) JAFFAR_THROW_LOGIC("Could not find starter piece");

    _connectivity = 0;
    _piecesReplaced = 0;
    _targetScore = _pipeBot->getTargetScore();
    _lastCrossingPieceStep = 0;
    _lastCrossingPieceDistanceToGoal = _targetScore;

    // Creating list of inputs
    _possibleInputs.resize(_rowCount);
    for (uint8_t i = 0; i < _rowCount; i++)
    {
      _possibleInputs[i].resize(_colCount);
      for (uint8_t j = 0; j < _colCount; j++)
      {
        char inputBuffer[256];
        sprintf(inputBuffer, "|%3u,%3u|", i, j);
        const std::string inputString(inputBuffer);
        const auto inputCode = _emulator->registerInput(inputString);

        const auto newInput = possibleInput_t({inputString, inputCode, i, j});
        _possibleInputs[i][j] = newInput;
        _inputMap[inputCode] = newInput;
      }
    }

    _playerCurrentRow = _playerInitialRow;
    _playerCurrentCol = _playerInitialCol;
    _pieceReplacedChecksum = 0;
    _crossingPieceCount = 0;
    _score = 0;
    _remainingFreePlaces = 80;
  }

  // Function to report what all the possible input that the game might require
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    std::set<std::string> possibleInputs;
    for (size_t i = 0; i < _possibleInputs.size(); i++)
     for (size_t j = 0; j < _possibleInputs[i].size(); j++)
      possibleInputs.insert(_possibleInputs[i][j].inputString);

    return possibleInputs;
  }
  
  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator

    const auto& piece = _inputMap[input];
    if (_pipeBot->getPiece({piece.row, piece.col}) != 0x00)
    {
      _piecesReplaced++;
      _pieceReplacedChecksum = (_pieceReplacedChecksum + piece.col) * piece.row ;
    } 
    
    _pipeBot->placeNextPiece({piece.row, piece.col});

    // Advancing current step
    _currentStep++;

    // Setting current column and row
    _playerCurrentRow = piece.row;
    _playerCurrentCol = piece.col;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_grid, _rowCount * _colCount * sizeof(uint8_t));
    // hashEngine.Update(_piecesReplaced);
    // hashEngine.Update(_currentStep);
    // hashEngine.Update(_playerCurrentRow);
    // hashEngine.Update(_playerCurrentCol);
    // hashEngine.Update(_pieceReplacedChecksum);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    std::set<std::pair<uint8_t, uint8_t>> piecesInPath;

    const auto forwardPath = calculatePipePath();
    for (auto& piece : forwardPath) piecesInPath.insert({piece.pos.row, piece.pos.col});
    _forwardDepth = forwardPath.size();

    _piecesOnBoard = 0;
    _lingeringPieces = 0;
    _remainingFreePlaces = 0;
    for (uint8_t i = 0; i < _rowCount; i++)
     for (uint8_t j = 0; j < _colCount; j++)
     { 
      const auto piece = _pipeBot->getPiece({i,j});
      if (piece == 0x00) _remainingFreePlaces++;
      if (piece != 0x00)
      {
        _piecesOnBoard++;
        if (piecesInPath.contains({i,j}) == false && _initialPieces.contains({i,j}) == false)
        {
          _lingeringPieces++;
        } 
      } 
     }      

    _distanceToReward = std::abs((int8_t)_targetScore - (int8_t)_forwardDepth);
    _connectivity = calculateConnectivity();

    // _lastCrossingPieceStep = getLastCrossingPiece(forwardPath);
    // _lastCrossingPieceDistanceToGoal = (uint8_t)std::max(0, (int16_t)_targetScore - (int16_t)_lastCrossingPieceStep);

    _crossingPieceCount = getCrossingPieceCount(forwardPath);

    _score = (100 * _forwardDepth) + (10000 * _crossingPieceCount) - (10 * _piecesReplaced) - (50 * _lingeringPieces) - (10 * _remainingFreePlaces) + (1 * _connectivity);
    
    // If the end piece is an end piece, the score is doubled
    const auto& endPiecePos = forwardPath.rbegin()->pos;
    const auto endPieceType = _pipeBot->getPiece(endPiecePos);
    if (endPieceType == 0x10 || endPieceType == 0x20 || endPieceType == 0x40 || endPieceType == 0x80) _score = _score * 2;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {

  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(&_currentStep, sizeof(_currentStep));
     serializer.push(&_forwardDepth, sizeof(_forwardDepth));
     serializer.push(&_piecesOnBoard, sizeof(_piecesOnBoard));
     serializer.push(&_distanceToReward, sizeof(_distanceToReward));
     serializer.push(&_lingeringPieces, sizeof(_lingeringPieces));
     serializer.push(&_connectivity, sizeof(_connectivity));
     serializer.push(&_playerCurrentRow, sizeof(_playerCurrentRow));
     serializer.push(&_playerCurrentCol, sizeof(_playerCurrentCol));
     serializer.push(&_piecesReplaced, sizeof(_piecesReplaced));
    //  serializer.push(&_pieceReplacedChecksum, sizeof(_pieceReplacedChecksum));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_forwardDepth, sizeof(_forwardDepth));
     deserializer.pop(&_piecesOnBoard, sizeof(_piecesOnBoard));
     deserializer.pop(&_distanceToReward, sizeof(_distanceToReward));
     deserializer.pop(&_lingeringPieces, sizeof(_lingeringPieces));
     deserializer.pop(&_connectivity, sizeof(_connectivity));
     deserializer.pop(&_playerCurrentRow, sizeof(_playerCurrentRow));
     deserializer.pop(&_playerCurrentCol, sizeof(_playerCurrentCol));
     deserializer.pop(&_piecesReplaced, sizeof(_piecesReplaced));
    //  deserializer.pop(&_pieceReplacedChecksum, sizeof(_pieceReplacedChecksum));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // reward += - (float)_distanceToReward + 0.001f * (float)_connectivity - (float)_lingeringPieces * 0.001f;
    reward += (float)_score;

    // Returning reward
    return reward;
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    for (size_t i = 0; i < _possibleInputs.size(); i++)
     for (size_t j = 0; j < _possibleInputs[i].size(); j++)
      // 
      if (_initialPieces.contains({i,j}) == false)
      {
        // Only replace like with like
        // if (_pipeBot->getPiece(i,j) != 0x00 && _pipeBot->getNextPiece() != _pipeBot->getPiece(i,j)) continue;

        // Don't try starter pieces
        allowedInputSet.push_back(_possibleInputs[i][j].inputIndex);
        
          // uint8_t rowDistance =  std::abs((int16_t)_playerCurrentRow - (int16_t)i);
          // uint8_t colDistance =  std::abs((int16_t)_playerCurrentCol - (int16_t)j);
          // if (rowDistance <= _distanceLimiterMaxRowDistance && colDistance <= _distanceLimiterMaxColDistance)
            // allowedInputSet.push_back(_possibleInputs[i][j].inputIndex);
      }
  }


  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:            %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Forward Depth:           %02u\n", _forwardDepth);
    jaffarCommon::logger::log("[J+]  + Current Score:           %d\n", _score);
    jaffarCommon::logger::log("[J+]  + Crossing Piece Count:    %02u\n", _crossingPieceCount);
    jaffarCommon::logger::log("[J+]  + Remaining Free Places    %02u\n", _remainingFreePlaces);
    //jaffarCommon::logger::log("[J+]  + Target Score:            %02u (Distance: %02u)\n", _targetScore, _distanceToReward);
    jaffarCommon::logger::log("[J+]  + Connectivity:            %04u\n", _connectivity);
    jaffarCommon::logger::log("[J+]  + Pieces Lingering:        %02u\n", _lingeringPieces );
    jaffarCommon::logger::log("[J+]  + Pieces On Board:         %02u\n", _piecesOnBoard );
    jaffarCommon::logger::log("[J+]  + Pieces Replaced:         %02u Checksum: 0x%04X\n", _piecesReplaced, _pieceReplacedChecksum );
    jaffarCommon::logger::log("[J+]  + Player Initial Position: %02u %02u\n", _playerInitialRow, _playerInitialCol );
    jaffarCommon::logger::log("[J+]  + Player Current Position: %02u %02u\n", _playerCurrentRow, _playerCurrentCol );
    jaffarCommon::logger::log("[J+]  + Initial Pieces:          %02lu\n", _initialPieces.size());
    // jaffarCommon::logger::log("[J+]  + Last Crossing Piece:     %02u (Distance to Goal: %02u)\n", _lastCrossingPieceStep, _lastCrossingPieceDistanceToGoal );
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ uint8_t getLastCrossingPiece(const std::vector<piecePath_t>& piecePath) const
  {
    uint8_t lastCrossingPiece = 0; 

    std::set<std::pair<uint8_t, uint8_t>> visitedPieces;
    for (uint8_t i = 0; i < piecePath.size(); i++)
    {
      const auto& piece = piecePath[i];
      if (visitedPieces.contains({piece.pos.row, piece.pos.col})) lastCrossingPiece = i;
      visitedPieces.insert({piece.pos.row, piece.pos.col});
    }

    return lastCrossingPiece;
  }

  __INLINE__ uint8_t getCrossingPieceCount(const std::vector<piecePath_t>& piecePath) const
  {
    uint8_t crossingPieceCount = 0; 

    std::set<std::pair<uint8_t, uint8_t>> visitedPieces;
    for (uint8_t i = 0; i < piecePath.size(); i++)
    {
      const auto& piece = piecePath[i];
      if (visitedPieces.contains({piece.pos.row, piece.pos.col})) crossingPieceCount++;
      visitedPieces.insert({piece.pos.row, piece.pos.col});
    }

    return crossingPieceCount;
  }

  __INLINE__ std::vector<piecePath_t> calculatePipePath() const
  {
    uint8_t currentPieceRow = _startPieceRow;
    uint8_t currentPieceCol = _startPieceCol;
    emulator::PipeBot::direction_t currentDirection = _startPieceDirection;

    std::vector<piecePath_t> path;
    path.push_back({currentPieceRow, currentPieceCol, currentDirection});

    while(true)
    {
      // Checking boundaries
      if (currentDirection == emulator::PipeBot::direction_t::left  && _pipeBot->hasLeftPiece ({currentPieceRow, currentPieceCol}) == false)  break;
      if (currentDirection == emulator::PipeBot::direction_t::right && _pipeBot->hasRightPiece({currentPieceRow, currentPieceCol}) == false)  break;
      if (currentDirection == emulator::PipeBot::direction_t::up    && _pipeBot->hasUpPiece   ({currentPieceRow, currentPieceCol}) == false)  break;
      if (currentDirection == emulator::PipeBot::direction_t::down  && _pipeBot->hasDownPiece ({currentPieceRow, currentPieceCol}) == false)  break;

      // Getting next piece's position
      emulator::PipeBot::piecePos_t nextPiecePos;
      if (currentDirection == emulator::PipeBot::direction_t::left)  nextPiecePos = _pipeBot->getLeftPiecePos( {currentPieceRow, currentPieceCol});
      if (currentDirection == emulator::PipeBot::direction_t::right) nextPiecePos = _pipeBot->getRightPiecePos({currentPieceRow, currentPieceCol});
      if (currentDirection == emulator::PipeBot::direction_t::up)    nextPiecePos = _pipeBot->getUpPiecePos(   {currentPieceRow, currentPieceCol});
      if (currentDirection == emulator::PipeBot::direction_t::down)  nextPiecePos = _pipeBot->getDownPiecePos( {currentPieceRow, currentPieceCol});

      // Getting next piece's information
      const uint8_t nextPiece = _pipeBot->getPiece(nextPiecePos);
      const auto& nextPieceType = _pipeBot->getPieceType(nextPiece);

      // Checking if it accepts the incoming stream
      if (currentDirection == emulator::PipeBot::direction_t::left  && nextPieceType.RInConnectivity == false) break;  
      if (currentDirection == emulator::PipeBot::direction_t::right && nextPieceType.LInConnectivity == false) break;
      if (currentDirection == emulator::PipeBot::direction_t::up    && nextPieceType.DInConnectivity == false) break;
      if (currentDirection == emulator::PipeBot::direction_t::down  && nextPieceType.UInConnectivity == false) break;

      // Checking if direction changes
      auto nextDirection = currentDirection;
      if (currentDirection == emulator::PipeBot::direction_t::left  && nextPieceType.LRedirection != emulator::PipeBot::direction_t::none) nextDirection = nextPieceType.LRedirection;
      if (currentDirection == emulator::PipeBot::direction_t::right && nextPieceType.RRedirection != emulator::PipeBot::direction_t::none) nextDirection = nextPieceType.RRedirection;
      if (currentDirection == emulator::PipeBot::direction_t::up    && nextPieceType.URedirection != emulator::PipeBot::direction_t::none) nextDirection = nextPieceType.URedirection;
      if (currentDirection == emulator::PipeBot::direction_t::down  && nextPieceType.DRedirection != emulator::PipeBot::direction_t::none) nextDirection = nextPieceType.DRedirection;
      
      // Updating values for the next iteration
      currentPieceRow = nextPiecePos.row;
      currentPieceCol = nextPiecePos.col;
      currentDirection = nextDirection;

      // Increasing Depth
      path.push_back({currentPieceRow, currentPieceCol, currentDirection});
    }

    return path;
  }

  __INLINE__ uint16_t calculateConnectivity() const
  {
    uint16_t connectivity = 0;

    for (uint8_t i = 0; i < _rowCount; i++)
     for (uint8_t j = 0; j < _colCount; j++)
     { 
      const auto piece = _pipeBot->getPiece({i, j});
      const auto& pieceType = _pipeBot->getPieceType(piece);

      if (piece != 0x00)
      {
        // Up
        if (i > 0)
        {
          const auto piecePos = _pipeBot->getUpPiecePos({i,j});
          const auto boundaryPiece = _pipeBot->getPiece(piecePos);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.UOutConnectivity == true && boundaryPieceType.DInConnectivity  == true) connectivity++;
          if (pieceType.UInConnectivity  == true && boundaryPieceType.DOutConnectivity == true) connectivity++;
        }

        // Down
        if (i < (_rowCount - 1))
        {
          const auto piecePos = _pipeBot->getDownPiecePos({i,j});
          const auto boundaryPiece = _pipeBot->getPiece(piecePos);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.DOutConnectivity == true && boundaryPieceType.UInConnectivity  == true) connectivity++;
          if (pieceType.DInConnectivity  == true && boundaryPieceType.UOutConnectivity == true) connectivity++;
        }

        // Left
        if (j > 0)
        {
          const auto piecePos = _pipeBot->getLeftPiecePos({i,j});
          const auto boundaryPiece = _pipeBot->getPiece(piecePos);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.LOutConnectivity == true && boundaryPieceType.RInConnectivity  == true) connectivity++;
          if (pieceType.LInConnectivity  == true && boundaryPieceType.ROutConnectivity == true) connectivity++;
        }

        // Right
        if (j < (_colCount - 1))
        {
          const auto piecePos = _pipeBot->getRightPiecePos({i,j});
          const auto boundaryPiece = _pipeBot->getPiece(piecePos);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.ROutConnectivity == true && boundaryPieceType.LInConnectivity  == true) connectivity++;
          if (pieceType.RInConnectivity  == true && boundaryPieceType.LOutConnectivity == true) connectivity++;
        }
      }
     }    

    return connectivity;
  }

  __INLINE__ void playerPrintCommands() const override
  {
    jaffarCommon::logger::log("[J+] t: Print Initial Info\n");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
     if (command == 't')
     {
      jaffarCommon::logger::log("Player Initial Row: %u\n", _playerInitialRow);
      jaffarCommon::logger::log("Player Initial Col: %u\n", _playerInitialCol);
      return true;
     }

     return false;
  };

  uint16_t _currentStep;

  uint8_t _startPieceRow;
  uint8_t _startPieceCol;
  emulator::PipeBot::direction_t _startPieceDirection;
  bool _startPieceFound;

  uint8_t _forwardDepth;
  uint8_t _piecesOnBoard;
  uint8_t _targetScore;
  uint8_t _lingeringPieces;
  uint8_t _distanceToReward;
  uint8_t _piecesReplaced;
  uint16_t _connectivity;
  uint8_t _crossingPieceCount;

  uint16_t _pieceReplacedChecksum;

  int32_t _score;
  uint8_t _remainingFreePlaces;

  emulator::PipeBot* const _pipeBot;
  uint8_t _rowCount;
  uint8_t _colCount; 

  uint8_t* _grid;

  uint8_t _playerInitialRow;
  uint8_t _playerInitialCol;
  uint8_t _distanceLimiterMaxRowDistance;
  uint8_t _distanceLimiterMaxColDistance;
  uint8_t _playerCurrentRow;
  uint8_t _playerCurrentCol;
  
  std::vector<std::vector<possibleInput_t>> _possibleInputs;
  std::map<InputSet::inputIndex_t, possibleInput_t> _inputMap;

  std::set<std::pair<uint8_t, uint8_t>> _initialPieces;
  uint8_t _lastCrossingPieceStep;
  uint8_t _lastCrossingPieceDistanceToGoal;
};

} // namespace pipeBot

} // namespace games

} // namespace jaffarPlus
