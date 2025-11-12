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

  typedef std::pair<uint8_t, uint8_t> piecePos_t;

  static __INLINE__ std::string getName() { return "PipeBot / PipeBot"; }

  PipeBot(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // // Getting emulator's low memory pointer
    const auto gridProperty = _emulator->getProperty("Grid");
    _grid = gridProperty.pointer;
    _gridSize = gridProperty.size;

    // registerGameProperty("Player Input 1"           ,&_grid[0x000E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Input 2"           ,&_grid[0x0010], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Input 3"           ,&_grid[0x009B], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Input 4"           ,&_grid[0x009D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Input 5"           ,&_grid[0x00CE], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Input 6"           ,&_grid[0x01FA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Input 7"           ,&_grid[0x0486], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Pos X"             ,&_grid[0x00AA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Player Pos Y"             ,&_grid[0x00A8], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Piece Placing Status"     ,&_grid[0x0079], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Piece Placing Timer 1"    ,&_grid[0x007B], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Piece Placing Timer 2"    ,&_grid[0x007D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Piece Placing Pos X"      ,&_grid[0x007F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Piece Placing Pos Y"      ,&_grid[0x0081], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Grid Start"               ,&_grid[0x05B0], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Grid End"                 ,&_grid[0x05FF], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Pieces Placed"            ,&_grid[0x00F2], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Game State"               ,&_grid[0x00FA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Pending Score"            ,&_grid[0x0447], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Piece Replacement State"  ,&_grid[0x00C4], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Piece Replacement Timer"  ,&_grid[0x00EB], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    // _playerInput1        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 1"            )]->getPointer();
    // _playerInput2        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 2"            )]->getPointer();
    // _playerInput3        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 3"            )]->getPointer();
    // _playerInput4        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 4"            )]->getPointer();
    // _playerInput5        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 5"            )]->getPointer();
    // _playerInput6        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 6"            )]->getPointer();
    // _playerInput7        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 7"            )]->getPointer();
    // _playerPosX          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X"              )]->getPointer();
    // _playerPosY          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y"              )]->getPointer();
    // _piecePlacingStatus  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Status"      )]->getPointer();
    // _piecePlacingTimer1  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Timer 1"     )]->getPointer();
    // _piecePlacingTimer2  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Timer 2"     )]->getPointer();
    // _piecePlacingPosX    =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Pos X"       )]->getPointer();
    // _piecePlacingPosY    =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Pos Y"       )]->getPointer();
    // _gridStart           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Grid Start"                )]->getPointer();
    // _gridEnd             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Grid End"                  )]->getPointer();
    // _piecesPlaced        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Pieces Placed"             )]->getPointer();  
    // _gameState           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game State"                )]->getPointer(); 
    // _pendingScore        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Pending Score"             )]->getPointer(); 
    // _pieceReplacementState  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Replacement State"              )]->getPointer(); 
    // _pieceReplacementTimer  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Replacement Timer"              )]->getPointer(); 

    // registerGameProperty("Forward Depth"            ,&_forwardDepth, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Backward Depth"           ,&_backwardDepth, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Pieces On Board"          ,&_piecesOnBoard, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Target Score"             ,&_targetScore, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Lingering Pieces"         ,&_lingeringPieces, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    // registerGameProperty("Ends Have Met"            ,&_endsHaveMet, Property::datatype_t::dt_bool , Property::endianness_t::little);

    // _nullInputIdx      = _emulator->registerInput("|..|........|");
    // _buttonAInputIdx   = _emulator->registerInput("|..|.......A|");
    // _buttonUInputIdx   = _emulator->registerInput("|..|U.......|");
    // _buttonDInputIdx   = _emulator->registerInput("|..|.D......|");
    // _buttonLInputIdx   = _emulator->registerInput("|..|..L.....|");
    // _buttonRInputIdx   = _emulator->registerInput("|..|...R....|");
    // // _buttonULInputIdx  = _emulator->registerInput("|..|U.L.....|");
    // // _buttonURInputIdx  = _emulator->registerInput("|..|U..R....|");
    // // _buttonDLInputIdx  = _emulator->registerInput("|..|.DL.....|");
    // // _buttonDRInputIdx  = _emulator->registerInput("|..|.D.R....|");

    // _targetScore = *_pendingScore;

    // // Looking for starter piece
    // _startPieceFound = false;
    // _endPieceFound = false;
    // for (uint8_t i = 0; i < __PIPE_DREAM_GRID_ROWS; i++)
    //  for (uint8_t j = 0; j < __PIPE_DREAM_GRID_COLS; j++)
    //  {
    //   const auto pieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + j];
      
    //   // Storing initial pieces (fixed)
    //   if (pieceType != 0x00) _initialPieces.insert({i, j});

    //   // Start Pieces
    //   if (pieceType == 0x01) // Up Facing
    //   {
    //     _startPieceRow = i;
    //     _startPieceCol = j;
    //     _startPieceDirection = direction_t::up;
    //     _startPieceFound = true;
    //     break;
    //   }

    //   if (pieceType == 0x02) // Down Facing
    //   {
    //     _startPieceRow = i;
    //     _startPieceCol = j;
    //     _startPieceDirection = direction_t::down;
    //     _startPieceFound = true;
    //     break;
    //   }

    //   if (pieceType == 0x08) // Right Facing
    //   {
    //     _startPieceRow = i;
    //     _startPieceCol = j;
    //     _startPieceDirection = direction_t::right;
    //     _startPieceFound = true;
    //     break;
    //   }

    //   if (pieceType == 0x04) // Left Facing
    //   {
    //     _startPieceRow = i;
    //     _startPieceCol = j;
    //     _startPieceDirection = direction_t::left;
    //     _startPieceFound = true;
    //     break;
    //   }

    //   // End Pieces
    //   if (pieceType == 0x10) // Up Facing
    //   {
    //     _endPieceRow = i;
    //     _endPieceCol = j;
    //     _endPieceFound = true;
    //     _endPieceDirection = direction_t::down;
    //     break;
    //   }

    //   if (pieceType == 0x20) // Down Facing
    //   {
    //     _endPieceRow = i;
    //     _endPieceCol = j;
    //     _endPieceFound = true;
    //     _endPieceDirection = direction_t::up;
    //     break;
    //   }

    //   if (pieceType == 0x80) // Right Facing
    //   {
    //     _endPieceRow = i;
    //     _endPieceCol = j;
    //     _endPieceFound = true;
    //     _endPieceDirection = direction_t::left;
    //     break;
    //   }

    //   if (pieceType == 0x40) // Left Facing
    //   {
    //     _endPieceRow = i;
    //     _endPieceCol = j;
    //     _endPieceFound = true;
    //     _endPieceDirection = direction_t::right;
    //     break;
    //   }

    //  }
    // if (_startPieceFound == false) JAFFAR_THROW_LOGIC("Could not find starter piece");

    // _inputAPressCount = 0;
    // _connectivity = 0;

  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_grid, _gridSize);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    // std::set<piecePos_t> piecesInPath;

    // const auto forwardPath = calculatePipePath();
    // for (auto& piece : forwardPath) piecesInPath.insert(piece);
    // _forwardDepth = forwardPath.size();

    // _backwardDepth = 0;
    // _distanceBetweenEnds = 0;
    // _endsHaveMet = false;

    // if (_endPieceFound == true)
    // {
    //   const auto inversePath = calculateInversePipePath();
    //   for (auto& piece : inversePath) piecesInPath.insert(piece);
    //   _backwardDepth = inversePath.size();

    //   // Checking if ends have met
    //   const auto& lastForwardPiecePos = forwardPath.rbegin();
    //   if (lastForwardPiecePos->first == _endPieceRow && lastForwardPiecePos->second == _endPieceCol) _endsHaveMet = true;

    //   // If they haven't met, check distance between both ends
    //   if (_endsHaveMet == false)
    //   {
    //     const auto& lastInversePiecePos = inversePath.rbegin();
    //     _distanceBetweenEnds = std::abs((int8_t)lastForwardPiecePos->first - (int8_t)lastInversePiecePos->first) + std::abs((int8_t)lastForwardPiecePos->second - (int8_t)lastInversePiecePos->second);
    //   }
    // }

    // _piecesOnBoard = 0;
    // _lingeringPieces = 0;
    // for (uint8_t i = 0; i < __PIPE_DREAM_GRID_ROWS; i++)
    //  for (uint8_t j = 0; j < __PIPE_DREAM_GRID_COLS; j++)
    //  { 
    //   const auto pieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + j];
    //   if (pieceType != 0)
    //   {
    //     _piecesOnBoard++;
    //     if (piecesInPath.contains({i,j}) == false)
    //     {
    //       _lingeringPieces++;

    //       // Clearing lingering pieces, if not fixed
    //       // if (_initialPieces.contains({i,j}) == false) _gridStart[i * __PIPE_DREAM_GRID_COLS + j] = 0x00;
    //     } 
    //   } 
    //  }      

    // _distanceToReward = std::abs((int8_t)_targetScore - (int8_t)_forwardDepth);
    // _connectivity = calculateConnectivity();

  }

  __INLINE__ void ruleUpdatePreHook() override
  {

  }

  __INLINE__ void ruleUpdatePostHook() override
  {
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    //  serializer.push(&_currentStep, sizeof(_currentStep));
    //  serializer.push(&_forwardDepth, sizeof(_forwardDepth));
    //  serializer.push(&_backwardDepth, sizeof(_backwardDepth));
    //  serializer.push(&_piecesOnBoard, sizeof(_piecesOnBoard));
    //  serializer.push(&_distanceToReward, sizeof(_distanceToReward));
    //  serializer.push(&_inputAPressCount, sizeof(_inputAPressCount));
    //  serializer.push(&_lingeringPieces, sizeof(_lingeringPieces));
    //  serializer.push(&_connectivity, sizeof(_connectivity));
    //  serializer.push(&_endsHaveMet, sizeof(_endsHaveMet));
    //  serializer.push(&_distanceBetweenEnds, sizeof(_distanceBetweenEnds));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    //  deserializer.pop(&_currentStep, sizeof(_currentStep));
    //  deserializer.pop(&_forwardDepth, sizeof(_forwardDepth));
    //  deserializer.pop(&_backwardDepth, sizeof(_backwardDepth));
    //  deserializer.pop(&_piecesOnBoard, sizeof(_piecesOnBoard));
    //  deserializer.pop(&_distanceToReward, sizeof(_distanceToReward));
    //  deserializer.pop(&_inputAPressCount, sizeof(_inputAPressCount));
    //  deserializer.pop(&_lingeringPieces, sizeof(_lingeringPieces));
    //  deserializer.pop(&_connectivity, sizeof(_connectivity));
    //  deserializer.pop(&_endsHaveMet, sizeof(_endsHaveMet));
    //  deserializer.pop(&_distanceBetweenEnds, sizeof(_distanceBetweenEnds));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // reward += - 10.0f * (float)_distanceBetweenEnds - (float)_distanceToReward + 0.01f * (float)_connectivity;

    // Returning reward
    return reward;
  }

  // __INLINE__ bool canPlacePiece() const 
  // {
  //    if (*_piecePlacingStatus > 0) return false;
  //    if (*_pieceReplacementState != 255 && *_pieceReplacementState != 7) return false;
  //    return true;
  // }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    // // The gauntlet
    // bool allowN = true;
    // bool allowA = true;
    // bool allowU = true;
    // bool allowD = true;
    // bool allowL = true;
    // bool allowR = true;

    // if (*_playerPosX == 0)                          allowL = false; 
    // if (*_playerPosX == __PIPE_DREAM_GRID_COLS - 1) allowR = false;  
    // if (*_playerPosY == 0)                          allowU = false; 
    // if (*_playerPosY == __PIPE_DREAM_GRID_ROWS - 1) allowD = false; 

    // // Do not place piece if there is one there already. Don't even linger
    // const uint8_t playerPosIdx = *_playerPosY * __PIPE_DREAM_GRID_COLS + *_playerPosX;
    // const uint8_t tileType = _gridStart[playerPosIdx];
    // if (tileType != 0x00)
    // {
    //   // allowN = false;
    //   allowA = false;
    // } 

    // // Do not place piece if already placing one
    // if (canPlacePiece() == false) allowA = false;

    // // Force placing a piece if you can
    // if (canPlacePiece() == true && *_piecesPlaced > 0)
    // {
    //   allowN  = false;
    //   allowA  = true;
    //   allowU  = false;
    //   allowD  = false;
    //   allowL  = false;
    //   allowR  = false;
    // }

    // // Prevent going back on our tracks (1-deep)
    // if (_lastInput == _buttonUInputIdx) allowD = false;
    // if (_lastInput == _buttonDInputIdx) allowU = false;
    // if (_lastInput == _buttonLInputIdx) allowR = false;
    // if (_lastInput == _buttonRInputIdx) allowL = false;

    // if (allowN  == true) allowedInputSet.push_back(_nullInputIdx    );
    // if (allowA  == true) allowedInputSet.push_back(_buttonAInputIdx );
    // if (allowU  == true) allowedInputSet.push_back(_buttonUInputIdx );
    // if (allowD  == true) allowedInputSet.push_back(_buttonDInputIdx );
    // if (allowL  == true) allowedInputSet.push_back(_buttonLInputIdx );
    // if (allowR  == true) allowedInputSet.push_back(_buttonRInputIdx );
  }


  void printInfoImpl() const override
  {
    // jaffarCommon::logger::log("[J+]  + Current Step:            %04u\n", _currentStep);
    // jaffarCommon::logger::log("[J+]  + Game State:              %02u\n", *_gameState);
    // jaffarCommon::logger::log("[J+]  + Forward Depth:           %02u\n", _forwardDepth);
    // jaffarCommon::logger::log("[J+]  + Backward Depth:          %02u\n", _backwardDepth);
    // jaffarCommon::logger::log("[J+]  + Ends Have Met:           %s (Distance: %02u)\n", _endsHaveMet ? "True" : "False", _distanceBetweenEnds);
    // jaffarCommon::logger::log("[J+]  + Target Score:            %02u (Distance: %02u)\n", _targetScore, _distanceToReward);
    // jaffarCommon::logger::log("[J+]  + Pending Score:           %02u\n", *_pendingScore);
    // jaffarCommon::logger::log("[J+]  + Connectivity:            %04u\n", _connectivity);
    // jaffarCommon::logger::log("[J+]  + Player Input:            %02u\n", *_playerInput1);
    // jaffarCommon::logger::log("[J+]  + Player Pos X:            %02u\n", *_playerPosX);
    // jaffarCommon::logger::log("[J+]  + Player Pos Y:            %02u\n", *_playerPosY);
    // jaffarCommon::logger::log("[J+]  + Piece Placing Status:    %02u\n", *_piecePlacingStatus);
    // jaffarCommon::logger::log("[J+]  + Piece Placing Timer 1:   %02u\n", *_piecePlacingTimer1);
    // jaffarCommon::logger::log("[J+]  + Piece Placing Timer 2:   %02u\n", *_piecePlacingTimer2);
    // jaffarCommon::logger::log("[J+]  + Piece Placing Pos X:     %02u\n", *_piecePlacingPosX);
    // jaffarCommon::logger::log("[J+]  + Piece Placing Pos Y:     %02u\n", *_piecePlacingPosY);
    // jaffarCommon::logger::log("[J+]  + Pieces Placed:           %02u\n", *_piecesPlaced );
    // jaffarCommon::logger::log("[J+]  + Pieces Lingering:        %02u\n", _lingeringPieces );
    // jaffarCommon::logger::log("[J+]  + Pieces On Board:         %02u\n", _piecesOnBoard );
    // jaffarCommon::logger::log("[J+]  + Initial Pieces:          %02u\n", _initialPieces.size() );
    // jaffarCommon::logger::log("[J+]  + Input A Pressed Count:   %02u\n", _inputAPressCount );
    
    
    // jaffarCommon::logger::log("[J+]  + Piece Replacement:       %02u (%03u)\n", *_pieceReplacementState, *_pieceReplacementTimer );

  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  // __INLINE__ std::vector<piecePos_t> calculatePipePath() const
  // {
  //   uint8_t currentPieceRow = _startPieceRow;
  //   uint8_t currentPieceCol = _startPieceCol;
  //   uint8_t currentDirection = _startPieceDirection;

  //   std::vector<piecePos_t> path;
  //   path.push_back({currentPieceRow, currentPieceCol});

  //   while(true)
  //   {
  //     // Checking boundaries
  //     if (currentPieceCol == 0                          && currentDirection == direction_t::left)  break;
  //     if (currentPieceCol == __PIPE_DREAM_GRID_COLS - 1 && currentDirection == direction_t::right) break;
  //     if (currentPieceRow == 0                          && currentDirection == direction_t::up)    break;
  //     if (currentPieceRow == __PIPE_DREAM_GRID_ROWS - 1 && currentDirection == direction_t::down)  break;

  //     // Getting next piece's position
  //     uint8_t nextPieceCol = currentPieceCol;
  //     if (currentDirection == direction_t::left)  nextPieceCol--;
  //     if (currentDirection == direction_t::right) nextPieceCol++;

  //     uint8_t nextPieceRow = currentPieceRow;
  //     if (currentDirection == direction_t::up)   nextPieceRow--;
  //     if (currentDirection == direction_t::down) nextPieceRow++;

  //     // Getting next piece's information
  //     const uint8_t nextPieceIdx = nextPieceRow * __PIPE_DREAM_GRID_COLS + nextPieceCol;
  //     const uint8_t nextPieceType = _gridStart[nextPieceIdx];
  //     if (_pieceTypes.contains(nextPieceType) == false) { printf("Did not recognize next piece type: 0%02X", nextPieceType); break; }
  //     const auto& nextPiece = _pieceTypes.at(nextPieceType);

  //     // Checking if it accepts the incoming stream
  //     if (currentDirection == direction_t::left  && nextPiece.RInConnectivity == false) break;  
  //     if (currentDirection == direction_t::right && nextPiece.LInConnectivity == false) break;
  //     if (currentDirection == direction_t::up    && nextPiece.DInConnectivity == false) break;
  //     if (currentDirection == direction_t::down  && nextPiece.UInConnectivity == false) break;

  //     // Checking if direction changes
  //     auto nextDirection = currentDirection;
  //     if (currentDirection == direction_t::left  && nextPiece.LRedirection != direction_t::none) nextDirection = nextPiece.LRedirection;
  //     if (currentDirection == direction_t::right && nextPiece.RRedirection != direction_t::none) nextDirection = nextPiece.RRedirection;
  //     if (currentDirection == direction_t::up    && nextPiece.URedirection != direction_t::none) nextDirection = nextPiece.URedirection;
  //     if (currentDirection == direction_t::down  && nextPiece.DRedirection != direction_t::none) nextDirection = nextPiece.DRedirection;
      
  //     // Updating values for the next iteration
  //     currentPieceRow = nextPieceRow;
  //     currentPieceCol = nextPieceCol;
  //     currentDirection = nextDirection;

  //     // Increasing Depth
  //     path.push_back({currentPieceRow, currentPieceCol});
  //   }

  //   return path;
  // }

  // __INLINE__ direction_t getOppositeDirection(const direction_t direction) const
  // {
  //    if (direction == direction_t::up) return direction_t::down;
  //    if (direction == direction_t::down) return direction_t::up;
  //    if (direction == direction_t::left) return direction_t::right;
  //    if (direction == direction_t::right) return direction_t::left;
  //    return direction_t::none;
  // }

  // __INLINE__ std::vector<piecePos_t> calculateInversePipePath() const
  // {
  //   uint8_t currentPieceRow = _endPieceRow;
  //   uint8_t currentPieceCol = _endPieceCol;
  //   uint8_t currentDirection = _endPieceDirection;

  //   std::vector<piecePos_t> path;
  //   path.push_back({currentPieceRow, currentPieceCol});

  //   while(true)
  //   {
  //     // Checking boundaries
  //     if (currentPieceCol == 0                          && currentDirection == direction_t::right) break;
  //     if (currentPieceCol == __PIPE_DREAM_GRID_COLS - 1 && currentDirection == direction_t::left)  break;
  //     if (currentPieceRow == 0                          && currentDirection == direction_t::down)  break;
  //     if (currentPieceRow == __PIPE_DREAM_GRID_ROWS - 1 && currentDirection == direction_t::up)    break;

  //     // Getting next piece's position
  //     uint8_t nextPieceCol = currentPieceCol;
  //     if (currentDirection == direction_t::right) nextPieceCol--;
  //     if (currentDirection == direction_t::left)  nextPieceCol++;

  //     uint8_t nextPieceRow = currentPieceRow;
  //     if (currentDirection == direction_t::down)  nextPieceRow--;
  //     if (currentDirection == direction_t::up)    nextPieceRow++;

  //     // Getting next piece's information
  //     const uint8_t nextPieceIdx = nextPieceRow * __PIPE_DREAM_GRID_COLS + nextPieceCol;
  //     const uint8_t nextPieceType = _gridStart[nextPieceIdx];
  //     if (_pieceTypes.contains(nextPieceType) == false) { printf("Did not recognize next piece type: 0%02X", nextPieceType); break; }
  //     const auto& nextPiece = _pieceTypes.at(nextPieceType);

  //     // Checking if it accepts the incoming stream
  //     if (currentDirection == direction_t::left  && nextPiece.LOutConnectivity == false) break;  
  //     if (currentDirection == direction_t::right && nextPiece.ROutConnectivity == false) break;
  //     if (currentDirection == direction_t::up    && nextPiece.UOutConnectivity == false) break;
  //     if (currentDirection == direction_t::down  && nextPiece.DOutConnectivity == false) break;

  //     // Checking if direction changes
  //     auto nextDirection = currentDirection;
  //     if (currentDirection == direction_t::left  && nextPiece.RRedirection != direction_t::none) nextDirection = getOppositeDirection(nextPiece.RRedirection);
  //     if (currentDirection == direction_t::right && nextPiece.LRedirection != direction_t::none) nextDirection = getOppositeDirection(nextPiece.LRedirection);
  //     if (currentDirection == direction_t::up    && nextPiece.DRedirection != direction_t::none) nextDirection = getOppositeDirection(nextPiece.DRedirection);
  //     if (currentDirection == direction_t::down  && nextPiece.URedirection != direction_t::none) nextDirection = getOppositeDirection(nextPiece.URedirection);
      
  //     // Updating values for the next iteration
  //     currentPieceRow = nextPieceRow;
  //     currentPieceCol = nextPieceCol;
  //     currentDirection = nextDirection;

  //     // Increasing Depth
  //     path.push_back({currentPieceRow, currentPieceCol});
  //   }

  //   return path;
  // }

  // __INLINE__ uint16_t calculateConnectivity() const
  // {
  //   uint16_t connectivity = 0;

  //   for (uint8_t i = 0; i < __PIPE_DREAM_GRID_ROWS; i++)
  //    for (uint8_t j = 0; j < __PIPE_DREAM_GRID_COLS; j++)
  //    { 
  //     const auto pieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + j];
  //     const auto& piece = _pieceTypes.at(pieceType);

  //     if (pieceType != 0x00)
  //     {
  //       // Up
  //       if (i > 0)
  //       {
  //         const auto boundaryPieceType = _gridStart[(i - 1) * __PIPE_DREAM_GRID_COLS + j];
  //         const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
  //         if (piece.UOutConnectivity == true && boundaryPiece.DInConnectivity  == true) connectivity++;
  //         if (piece.UInConnectivity  == true && boundaryPiece.DOutConnectivity == true) connectivity++;
  //       }

  //       // Down
  //       if (i < (__PIPE_DREAM_GRID_ROWS - 1))
  //       {
  //         const auto boundaryPieceType = _gridStart[(i + 1) * __PIPE_DREAM_GRID_COLS + j];
  //         const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
  //         if (piece.DOutConnectivity == true && boundaryPiece.UInConnectivity  == true) connectivity++;
  //         if (piece.DInConnectivity  == true && boundaryPiece.UOutConnectivity == true) connectivity++;
  //       }

  //       // Left
  //       if (j > 0)
  //       {
  //         const auto boundaryPieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + (j - 1)];
  //         const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
  //         if (piece.LOutConnectivity == true && boundaryPiece.RInConnectivity  == true) connectivity++;
  //         if (piece.LInConnectivity  == true && boundaryPiece.ROutConnectivity == true) connectivity++;
  //       }

  //       // Right
  //       if (j < (__PIPE_DREAM_GRID_COLS - 1))
  //       {
  //         const auto boundaryPieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + (j + 1)];
  //         const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
  //         if (piece.ROutConnectivity == true && boundaryPiece.LInConnectivity  == true) connectivity++;
  //         if (piece.RInConnectivity  == true && boundaryPiece.LOutConnectivity == true) connectivity++;
  //       }
  //     }
  //    }    

  //   return connectivity;
  // }

  // uint8_t* _playerInput1        ;
  // uint8_t* _playerInput2        ;
  // uint8_t* _playerInput3        ;
  // uint8_t* _playerInput4        ;
  // uint8_t* _playerInput5        ;
  // uint8_t* _playerInput6        ;
  // uint8_t* _playerInput7        ;
  // uint8_t* _playerPosX          ;
  // uint8_t* _playerPosY          ;
  // uint8_t* _piecePlacingStatus  ;
  // uint8_t* _piecePlacingTimer1  ;
  // uint8_t* _piecePlacingTimer2  ;
  // uint8_t* _piecePlacingPosX    ;
  // uint8_t* _piecePlacingPosY    ;
  // uint8_t* _gridStart           ;
  // uint8_t* _gridEnd             ;
  // uint8_t* _piecesPlaced        ;
  // uint8_t* _gameState;
  // uint8_t* _pendingScore;
  // uint8_t* _pieceReplacementState;
  // uint8_t* _pieceReplacementTimer;

  uint16_t _currentStep;

  // InputSet::inputIndex_t _nullInputIdx;
  // InputSet::inputIndex_t _buttonAInputIdx;
  // InputSet::inputIndex_t _buttonUInputIdx;
  // InputSet::inputIndex_t _buttonDInputIdx;
  // InputSet::inputIndex_t _buttonLInputIdx;
  // InputSet::inputIndex_t _buttonRInputIdx;
  // // InputSet::inputIndex_t _buttonULInputIdx;
  // // InputSet::inputIndex_t _buttonURInputIdx;
  // // InputSet::inputIndex_t _buttonDLInputIdx;
  // // InputSet::inputIndex_t _buttonDRInputIdx;

  // std::map<uint8_t, pieceType_t> _pieceTypes;

  // std::set<piecePos_t> _initialPieces;

  // uint8_t _startPieceRow;
  // uint8_t _startPieceCol;
  // direction_t _startPieceDirection;
  // bool _startPieceFound;

  // uint8_t _endPieceRow; 
  // uint8_t _endPieceCol;
  // direction_t _endPieceDirection;
  // bool _endPieceFound;

  // uint8_t _distanceBetweenEnds;
  // bool _endsHaveMet;

  // uint8_t _forwardDepth;
  // uint8_t _backwardDepth;

  // uint8_t _piecesOnBoard;
  // uint8_t _targetScore;
  // uint8_t _lingeringPieces;
  // uint8_t _distanceToReward;
  // uint8_t _inputAPressCount;
  // uint16_t _connectivity;

  uint8_t* _grid;
  size_t _gridSize;

};

} // namespace pipeBot

} // namespace games

} // namespace jaffarPlus
