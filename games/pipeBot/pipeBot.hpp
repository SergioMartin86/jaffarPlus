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
    uint8_t row;
    uint8_t col;
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
    _distanceLimiterInitialRow = jaffarCommon::json::getNumber<uint8_t>(distanceLimiterJs, "Initial Row");
    _distanceLimiterInitialCol = jaffarCommon::json::getNumber<uint8_t>(distanceLimiterJs, "Initial Col");
    _distanceLimiterMaxRowDistance = jaffarCommon::json::getNumber<uint8_t>(distanceLimiterJs, "Max Row Distance");
    _distanceLimiterMaxColDistance = jaffarCommon::json::getNumber<uint8_t>(distanceLimiterJs, "Max Col Distance");

    _targetScore = jaffarCommon::json::getNumber<uint8_t>(config, "Target Score");
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
    registerGameProperty("Backward Depth"           ,&_backwardDepth, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Pieces On Board"          ,&_piecesOnBoard, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Target Score"             ,&_targetScore, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Lingering Pieces"         ,&_lingeringPieces, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Ends Have Met"            ,&_endsHaveMet, Property::datatype_t::dt_bool , Property::endianness_t::little);

    // Looking for starter piece
    _startPieceFound = false;
    _endPieceFound = false;
    for (uint8_t i = 0; i < _rowCount; i++)
     for (uint8_t j = 0; j < _colCount; j++)
     {
      const auto pieceType = _pipeBot->getPiece(i,j);

      // Gathering initial pieces
      if (pieceType != 0x00) _initialPieces.insert({i,j});
      
      // Start Pieces
      if (pieceType == 0x01) // Up Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::up;
        _startPieceFound = true;
        break;
      }

      if (pieceType == 0x02) // Down Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::down;
        _startPieceFound = true;
        break;
      }

      if (pieceType == 0x08) // Right Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::right;
        _startPieceFound = true;
        break;
      }

      if (pieceType == 0x04) // Left Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = emulator::PipeBot::direction_t::left;
        _startPieceFound = true;
        break;
      }

      // End Pieces
      if (pieceType == 0x10) // Up Facing
      {
        _endPieceRow = i;
        _endPieceCol = j;
        _endPieceFound = true;
        _endPieceDirection = emulator::PipeBot::direction_t::down;
        break;
      }

      if (pieceType == 0x20) // Down Facing
      {
        _endPieceRow = i;
        _endPieceCol = j;
        _endPieceFound = true;
        _endPieceDirection = emulator::PipeBot::direction_t::up;
        break;
      }

      if (pieceType == 0x80) // Right Facing
      {
        _endPieceRow = i;
        _endPieceCol = j;
        _endPieceFound = true;
        _endPieceDirection = emulator::PipeBot::direction_t::left;
        break;
      }

      if (pieceType == 0x40) // Left Facing
      {
        _endPieceRow = i;
        _endPieceCol = j;
        _endPieceFound = true;
        _endPieceDirection = emulator::PipeBot::direction_t::right;
        break;
      }

     }
    if (_startPieceFound == false) JAFFAR_THROW_LOGIC("Could not find starter piece");

    _connectivity = 0;
    _piecesReplaced = 0;

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

    _distanceLimiterCurrentRow = _distanceLimiterInitialRow;
    _distanceLimiterCurrentCol = _distanceLimiterInitialCol;
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
    if (_pipeBot->getPiece(piece.row, piece.col) != 0x00) _piecesReplaced++;
    
    _pipeBot->placeNextPiece(piece.row, piece.col);

    // Advancing current step
    _currentStep++;

    // Setting current column and row
    _distanceLimiterCurrentRow = piece.row;
    _distanceLimiterCurrentCol = piece.col;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_grid, _rowCount * _colCount * sizeof(uint8_t));
    hashEngine.Update(_piecesReplaced);
    hashEngine.Update(_distanceLimiterCurrentRow);
    hashEngine.Update(_distanceLimiterCurrentCol);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    std::set<std::pair<uint8_t, uint8_t>> piecesInPath;

    const auto forwardPath = calculatePipePath();
    for (auto& piece : forwardPath) piecesInPath.insert({piece.row, piece.col});
    _forwardDepth = forwardPath.size();

    _backwardDepth = 0;
    _distanceBetweenEnds = 0;
    _endsHaveMet = false;

    if (_endPieceFound == true)
    {
      const auto inversePath = calculateInversePipePath();
      for (auto& piece : inversePath) piecesInPath.insert({piece.row, piece.col});
      _backwardDepth = inversePath.size();

      // Checking if ends have met
      const auto& lastForwardPiecePos = *forwardPath.rbegin();

      if (lastForwardPiecePos.row == _endPieceRow && lastForwardPiecePos.col == _endPieceCol) _endsHaveMet = true;

      // If they haven't met, check distance between both ends
      if (_endsHaveMet == false)
      {
        const auto& lastInversePiecePos = *inversePath.rbegin();

        // Making adjustments based on direction
        auto forwardRow = lastForwardPiecePos.row;
        auto forwardCol = lastForwardPiecePos.col;
        const auto forwardDir = lastForwardPiecePos.direction;
        if (forwardDir == emulator::PipeBot::direction_t::up)    forwardRow = forwardRow - 1;
        if (forwardDir == emulator::PipeBot::direction_t::down)  forwardRow = forwardRow + 1;
        if (forwardDir == emulator::PipeBot::direction_t::left)  forwardCol = forwardCol - 1;
        if (forwardDir == emulator::PipeBot::direction_t::right) forwardCol = forwardCol + 1;

        auto inverseRow = lastInversePiecePos.row;
        auto inverseCol = lastInversePiecePos.col;
        const auto inverseDir = lastInversePiecePos.direction;
        if (inverseDir == emulator::PipeBot::direction_t::up)    inverseRow = inverseRow + 1;
        if (inverseDir == emulator::PipeBot::direction_t::down)  inverseRow = inverseRow - 1;
        if (inverseDir == emulator::PipeBot::direction_t::left)  inverseCol = inverseCol + 1;
        if (inverseDir == emulator::PipeBot::direction_t::right) inverseCol = inverseCol - 1;

        _distanceBetweenEnds = std::abs((int16_t)forwardRow - (int16_t)inverseRow) + std::abs((int16_t)forwardCol - (int16_t)inverseCol);
      }
    }

    _piecesOnBoard = 0;
    _lingeringPieces = 0;
    for (uint8_t i = 0; i < _rowCount; i++)
     for (uint8_t j = 0; j < _colCount; j++)
     { 
      const auto piece = _pipeBot->getPiece(i,j);
      if (piece != 0)
      {
        _piecesOnBoard++;
        if (piecesInPath.contains({i,j}) == false)
        {
          _lingeringPieces++;
        } 
      } 
     }      

    //  if (_endsHaveMet == false) _distanceToReward = std::abs((int8_t)_targetScore - (int8_t)_forwardDepth - (int8_t)_backwardDepth);
    //  if (_endsHaveMet == true) _distanceToReward = std::abs((int8_t)_targetScore - (int8_t)_forwardDepth);
    _distanceToReward = std::abs((int8_t)_targetScore - (int8_t)_forwardDepth);
    _connectivity = calculateConnectivity();

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
     serializer.push(&_backwardDepth, sizeof(_backwardDepth));
     serializer.push(&_piecesOnBoard, sizeof(_piecesOnBoard));
     serializer.push(&_distanceToReward, sizeof(_distanceToReward));
     serializer.push(&_lingeringPieces, sizeof(_lingeringPieces));
     serializer.push(&_connectivity, sizeof(_connectivity));
     serializer.push(&_endsHaveMet, sizeof(_endsHaveMet));
     serializer.push(&_distanceBetweenEnds, sizeof(_distanceBetweenEnds));
     serializer.push(&_distanceLimiterCurrentRow, sizeof(_distanceLimiterCurrentRow));
     serializer.push(&_distanceLimiterCurrentCol, sizeof(_distanceLimiterCurrentCol));
     serializer.push(&_piecesReplaced, sizeof(_piecesReplaced));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_forwardDepth, sizeof(_forwardDepth));
     deserializer.pop(&_backwardDepth, sizeof(_backwardDepth));
     deserializer.pop(&_piecesOnBoard, sizeof(_piecesOnBoard));
     deserializer.pop(&_distanceToReward, sizeof(_distanceToReward));
     deserializer.pop(&_lingeringPieces, sizeof(_lingeringPieces));
     deserializer.pop(&_connectivity, sizeof(_connectivity));
     deserializer.pop(&_endsHaveMet, sizeof(_endsHaveMet));
     deserializer.pop(&_distanceBetweenEnds, sizeof(_distanceBetweenEnds));
     deserializer.pop(&_distanceLimiterCurrentRow, sizeof(_distanceLimiterCurrentRow));
     deserializer.pop(&_distanceLimiterCurrentCol, sizeof(_distanceLimiterCurrentCol));
     deserializer.pop(&_piecesReplaced, sizeof(_piecesReplaced));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // reward += - (float)_distanceBetweenEnds - (float)_distanceToReward + 0.001f * (float)_connectivity;
    reward += - (float)_distanceToReward + 0.001f * (float)_connectivity - (float)_lingeringPieces * 0.001f;

    // Returning reward
    return reward;
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    for (size_t i = 0; i < _possibleInputs.size(); i++)
     for (size_t j = 0; j < _possibleInputs[i].size(); j++)
      // if (_pipeBot->getPiece(i,j) == 0x00)
      if (_initialPieces.contains({i,j}) == false)
      {
        uint8_t rowDistance =  std::abs((int16_t)_distanceLimiterCurrentRow - (int16_t)i);
        uint8_t colDistance =  std::abs((int16_t)_distanceLimiterCurrentCol - (int16_t)j);
        if (rowDistance <= _distanceLimiterMaxRowDistance && colDistance <= _distanceLimiterMaxColDistance)
           allowedInputSet.push_back(_possibleInputs[i][j].inputIndex);
      }
  }


  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:            %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Forward Depth:           %02u\n", _forwardDepth);
    jaffarCommon::logger::log("[J+]  + Backward Depth:          %02u\n", _backwardDepth);
    jaffarCommon::logger::log("[J+]  + Ends Have Met:           %s (Distance: %02u)\n", _endsHaveMet ? "True" : "False", _distanceBetweenEnds);
    jaffarCommon::logger::log("[J+]  + Target Score:            %02u (Distance: %02u)\n", _targetScore, _distanceToReward);
    jaffarCommon::logger::log("[J+]  + Connectivity:            %04u\n", _connectivity);
    jaffarCommon::logger::log("[J+]  + Pieces Lingering:        %02u\n", _lingeringPieces );
    jaffarCommon::logger::log("[J+]  + Pieces On Board:         %02u\n", _piecesOnBoard );
    jaffarCommon::logger::log("[J+]  + Pieces Replaced:         %02u\n", _piecesReplaced );
    jaffarCommon::logger::log("[J+]  + Current Position:        %02u %02u\n", _distanceLimiterCurrentRow, _distanceLimiterCurrentCol );
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
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
      if (currentPieceCol == 0                          && currentDirection == emulator::PipeBot::direction_t::left)  break;
      if (currentPieceCol == _colCount - 1 && currentDirection == emulator::PipeBot::direction_t::right) break;
      if (currentPieceRow == 0                          && currentDirection == emulator::PipeBot::direction_t::up)    break;
      if (currentPieceRow == _rowCount - 1 && currentDirection == emulator::PipeBot::direction_t::down)  break;

      // Getting next piece's position
      uint8_t nextPieceCol = currentPieceCol;
      if (currentDirection == emulator::PipeBot::direction_t::left)  nextPieceCol--;
      if (currentDirection == emulator::PipeBot::direction_t::right) nextPieceCol++;

      uint8_t nextPieceRow = currentPieceRow;
      if (currentDirection == emulator::PipeBot::direction_t::up)   nextPieceRow--;
      if (currentDirection == emulator::PipeBot::direction_t::down) nextPieceRow++;

      // Getting next piece's information
      const uint8_t nextPiece = _pipeBot->getPiece(nextPieceRow, nextPieceCol);
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
      currentPieceRow = nextPieceRow;
      currentPieceCol = nextPieceCol;
      currentDirection = nextDirection;

      // Increasing Depth
      path.push_back({currentPieceRow, currentPieceCol, currentDirection});
    }

    return path;
  }

  __INLINE__ emulator::PipeBot::direction_t getOppositeDirection(const emulator::PipeBot::direction_t direction) const
  {
     if (direction == emulator::PipeBot::direction_t::up) return emulator::PipeBot::direction_t::down;
     if (direction == emulator::PipeBot::direction_t::down) return emulator::PipeBot::direction_t::up;
     if (direction == emulator::PipeBot::direction_t::left) return emulator::PipeBot::direction_t::right;
     if (direction == emulator::PipeBot::direction_t::right) return emulator::PipeBot::direction_t::left;
     return emulator::PipeBot::direction_t::none;
  }

  __INLINE__ std::vector<piecePath_t> calculateInversePipePath() const
  {
    uint8_t currentPieceRow = _endPieceRow;
    uint8_t currentPieceCol = _endPieceCol;
    emulator::PipeBot::direction_t currentDirection = _endPieceDirection;

    std::vector<piecePath_t> path;
    path.push_back({currentPieceRow, currentPieceCol, currentDirection});

    while(true)
    {
      // Checking boundaries
      if (currentPieceCol == 0              && currentDirection == emulator::PipeBot::direction_t::right) break;
      if (currentPieceCol == _colCount - 1  && currentDirection == emulator::PipeBot::direction_t::left)  break;
      if (currentPieceRow == 0              && currentDirection == emulator::PipeBot::direction_t::down)  break;
      if (currentPieceRow == _rowCount - 1  && currentDirection == emulator::PipeBot::direction_t::up)    break;

      // Getting next piece's position
      uint8_t nextPieceCol = currentPieceCol;
      if (currentDirection == emulator::PipeBot::direction_t::right) nextPieceCol--;
      if (currentDirection == emulator::PipeBot::direction_t::left)  nextPieceCol++;

      uint8_t nextPieceRow = currentPieceRow;
      if (currentDirection == emulator::PipeBot::direction_t::down)  nextPieceRow--;
      if (currentDirection == emulator::PipeBot::direction_t::up)    nextPieceRow++;

      // Getting next piece's information
      const uint8_t nextPiece = _pipeBot->getPiece(nextPieceRow, nextPieceCol);
      const auto& nextPieceType = _pipeBot->getPieceType(nextPiece);

      // Checking if it accepts the incoming stream
      if (currentDirection == emulator::PipeBot::direction_t::left  && nextPieceType.LOutConnectivity == false) break;  
      if (currentDirection == emulator::PipeBot::direction_t::right && nextPieceType.ROutConnectivity == false) break;
      if (currentDirection == emulator::PipeBot::direction_t::up    && nextPieceType.UOutConnectivity == false) break;
      if (currentDirection == emulator::PipeBot::direction_t::down  && nextPieceType.DOutConnectivity == false) break;

      // Checking if direction changes
      auto nextDirection = currentDirection;
      if (currentDirection == emulator::PipeBot::direction_t::left  && nextPieceType.RRedirection != emulator::PipeBot::direction_t::none) nextDirection = getOppositeDirection(nextPieceType.RRedirection);
      if (currentDirection == emulator::PipeBot::direction_t::right && nextPieceType.LRedirection != emulator::PipeBot::direction_t::none) nextDirection = getOppositeDirection(nextPieceType.LRedirection);
      if (currentDirection == emulator::PipeBot::direction_t::up    && nextPieceType.DRedirection != emulator::PipeBot::direction_t::none) nextDirection = getOppositeDirection(nextPieceType.DRedirection);
      if (currentDirection == emulator::PipeBot::direction_t::down  && nextPieceType.URedirection != emulator::PipeBot::direction_t::none) nextDirection = getOppositeDirection(nextPieceType.URedirection);
      
      // Updating values for the next iteration
      currentPieceRow = nextPieceRow;
      currentPieceCol = nextPieceCol;
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
      const auto piece = _pipeBot->getPiece(i, j);
      const auto& pieceType = _pipeBot->getPieceType(piece);

      if (piece != 0x00)
      {
        // Up
        if (i > 0)
        {
          const auto boundaryPiece = _pipeBot->getPiece(i-1, j);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.UOutConnectivity == true && boundaryPieceType.DInConnectivity  == true) connectivity++;
          if (pieceType.UInConnectivity  == true && boundaryPieceType.DOutConnectivity == true) connectivity++;
        }

        // Down
        if (i < (_rowCount - 1))
        {
          const auto boundaryPiece = _pipeBot->getPiece(i+1, j);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.DOutConnectivity == true && boundaryPieceType.UInConnectivity  == true) connectivity++;
          if (pieceType.DInConnectivity  == true && boundaryPieceType.UOutConnectivity == true) connectivity++;
        }

        // Left
        if (j > 0)
        {
          const auto boundaryPiece = _pipeBot->getPiece(i, j-1);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.LOutConnectivity == true && boundaryPieceType.RInConnectivity  == true) connectivity++;
          if (pieceType.LInConnectivity  == true && boundaryPieceType.ROutConnectivity == true) connectivity++;
        }

        // Right
        if (j < (_colCount - 1))
        {
          const auto boundaryPiece = _pipeBot->getPiece(i, j+1);
          const auto& boundaryPieceType = _pipeBot->getPieceType(boundaryPiece);
          if (pieceType.ROutConnectivity == true && boundaryPieceType.LInConnectivity  == true) connectivity++;
          if (pieceType.RInConnectivity  == true && boundaryPieceType.LOutConnectivity == true) connectivity++;
        }
      }
     }    

    return connectivity;
  }

  uint16_t _currentStep;

  uint8_t _startPieceRow;
  uint8_t _startPieceCol;
  emulator::PipeBot::direction_t _startPieceDirection;
  bool _startPieceFound;

  uint8_t _endPieceRow; 
  uint8_t _endPieceCol;
  emulator::PipeBot::direction_t _endPieceDirection;
  bool _endPieceFound;

  uint8_t _distanceBetweenEnds;
  bool _endsHaveMet;

  uint8_t _forwardDepth;
  uint8_t _backwardDepth;
  uint8_t _piecesOnBoard;
  uint8_t _targetScore;
  uint8_t _lingeringPieces;
  uint8_t _distanceToReward;
  uint8_t _piecesReplaced;
  uint16_t _connectivity;

  emulator::PipeBot* const _pipeBot;
  uint8_t _rowCount;
  uint8_t _colCount; 

  uint8_t* _grid;

  uint8_t _distanceLimiterInitialRow;
  uint8_t _distanceLimiterInitialCol;
  uint8_t _distanceLimiterMaxRowDistance;
  uint8_t _distanceLimiterMaxColDistance;
  uint8_t _distanceLimiterCurrentRow;
  uint8_t _distanceLimiterCurrentCol;
  
  std::vector<std::vector<possibleInput_t>> _possibleInputs;
  std::map<InputSet::inputIndex_t, possibleInput_t> _inputMap;

  std::set<std::pair<uint8_t, uint8_t>> _initialPieces;

};

} // namespace pipeBot

} // namespace games

} // namespace jaffarPlus
