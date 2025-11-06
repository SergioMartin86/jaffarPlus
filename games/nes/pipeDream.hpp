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

#define __PIPE_DREAM_GRID_ROWS 8
#define __PIPE_DREAM_GRID_COLS 10

class PipeDream final : public jaffarPlus::Game
{
public:

  typedef std::pair<uint8_t, uint8_t> piecePos_t;

  enum direction_t
  {
    none = 0,
    left = 1,
    right = 2,
    up = 3,
    down = 4
  };

  struct pieceType_t {
    std::string shape;

    bool LInConnectivity;
    bool RInConnectivity;
    bool UInConnectivity;
    bool DInConnectivity;

    bool LOutConnectivity;
    bool ROutConnectivity;
    bool UOutConnectivity;
    bool DOutConnectivity;

    direction_t LRedirection;
    direction_t RRedirection;
    direction_t URedirection;
    direction_t DRedirection;
  };

  static __INLINE__ std::string getName() { return "NES / Pipe Dream"; }

  PipeDream(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    _pieceTypes = 
    {  
      //             LIn    RIn    UIn    DIn     LOut   ROut   UOut   DOut   LRedirect          RRedirect          URedirect           DRedirect
      { 0x00, { ".", false, false, false, false,  false, false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x01, { "╨", false, false, false, false,  false, false, true,  false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x02, { "╥", false, false, false, false,  false, false, false, true,  direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x08, { "╞", false, false, false, false,  false, true,  false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x04, { "╡", false, false, false, false,  true,  false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x12, { "↓", false, false, true,  false,  false, false, false, true , direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x21, { "↑", false, false, false, true,   false, false, true,  false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x48, { "→", true,  false, false, false,  false, true,  false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x84, { "←", false,  true, false, false,  true,  false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0xCC, { "═", true,  true,  false, false,  true,  true,  false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x99, { "╚", false, true,  true,  false,  false, true,  true,  false, direction_t::up,   direction_t::none, direction_t::none,  direction_t::right,  } },
      { 0x55, { "╝", true,  false, true,  false,  true,  false, true,  false, direction_t::none, direction_t::up,   direction_t::none,  direction_t::left,   } },
      { 0x33, { "║", false, false, true,  true ,  false, false, true,  true , direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0xFF, { "╬", true,  true,  true,  true ,  true,  true,  true,  true , direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x66, { "╗", true,  false, false, true ,  true,  false, false, true , direction_t::none, direction_t::down, direction_t::left,  direction_t::none,   } },
      { 0xAA, { "╔", false, true,  false, true ,  false, true,  false, true , direction_t::down, direction_t::none, direction_t::right, direction_t::none,   } },
    };
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    registerGameProperty("Player Input 1"           ,&_lowMem[0x000E], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Input 2"           ,&_lowMem[0x0010], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Input 3"           ,&_lowMem[0x009B], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Input 4"           ,&_lowMem[0x009D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Input 5"           ,&_lowMem[0x00CE], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Input 6"           ,&_lowMem[0x01FA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Input 7"           ,&_lowMem[0x0486], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos X"             ,&_lowMem[0x00AA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Player Pos Y"             ,&_lowMem[0x00A8], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Piece Placing Status"     ,&_lowMem[0x0079], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Piece Placing Timer 1"    ,&_lowMem[0x007B], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Piece Placing Timer 2"    ,&_lowMem[0x007D], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Piece Placing Pos X"      ,&_lowMem[0x007F], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Piece Placing Pos Y"      ,&_lowMem[0x0081], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Grid Start"               ,&_lowMem[0x05B0], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Grid End"                 ,&_lowMem[0x05FF], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Pieces Placed"            ,&_lowMem[0x00F2], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Game State"               ,&_lowMem[0x00FA], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Pending Score"            ,&_lowMem[0x0447], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Piece Replacement State"  ,&_lowMem[0x00C4], Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Piece Replacement Timer"  ,&_lowMem[0x00EB], Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    _playerInput1        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 1"            )]->getPointer();
    _playerInput2        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 2"            )]->getPointer();
    _playerInput3        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 3"            )]->getPointer();
    _playerInput4        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 4"            )]->getPointer();
    _playerInput5        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 5"            )]->getPointer();
    _playerInput6        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 6"            )]->getPointer();
    _playerInput7        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Input 7"            )]->getPointer();
    _playerPosX          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos X"              )]->getPointer();
    _playerPosY          =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y"              )]->getPointer();
    _piecePlacingStatus  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Status"      )]->getPointer();
    _piecePlacingTimer1  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Timer 1"     )]->getPointer();
    _piecePlacingTimer2  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Timer 2"     )]->getPointer();
    _piecePlacingPosX    =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Pos X"       )]->getPointer();
    _piecePlacingPosY    =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Placing Pos Y"       )]->getPointer();
    _gridStart           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Grid Start"                )]->getPointer();
    _gridEnd             =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Grid End"                  )]->getPointer();
    _piecesPlaced        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Pieces Placed"             )]->getPointer();  
    _gameState           =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Game State"                )]->getPointer(); 
    _pendingScore        =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Pending Score"             )]->getPointer(); 
    _pieceReplacementState  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Replacement State"              )]->getPointer(); 
    _pieceReplacementTimer  =  (uint8_t *)_propertyMap[jaffarCommon::hash::hashString("Piece Replacement Timer"              )]->getPointer(); 

    registerGameProperty("Current Depth"            ,&_currentDepth, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Pieces On Board"          ,&_piecesOnBoard, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Target Score"             ,&_targetScore, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Lingering Pieces"         ,&_lingeringPieces, Property::datatype_t::dt_uint8 , Property::endianness_t::little);

    _nullInputIdx      = _emulator->registerInput("|..|........|");
    _buttonAInputIdx   = _emulator->registerInput("|..|.......A|");
    _buttonUInputIdx   = _emulator->registerInput("|..|U.......|");
    _buttonDInputIdx   = _emulator->registerInput("|..|.D......|");
    _buttonLInputIdx   = _emulator->registerInput("|..|..L.....|");
    _buttonRInputIdx   = _emulator->registerInput("|..|...R....|");
    // _buttonULInputIdx  = _emulator->registerInput("|..|U.L.....|");
    // _buttonURInputIdx  = _emulator->registerInput("|..|U..R....|");
    // _buttonDLInputIdx  = _emulator->registerInput("|..|.DL.....|");
    // _buttonDRInputIdx  = _emulator->registerInput("|..|.D.R....|");

    _targetScore = *_pendingScore;

    // Looking for starter piece
    bool startPieceFound = false;
    for (uint8_t i = 0; i < __PIPE_DREAM_GRID_ROWS; i++)
     for (uint8_t j = 0; j < __PIPE_DREAM_GRID_COLS; j++)
     {
      const auto pieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + j];
      
      // Storing initial pieces (fixed)
      if (pieceType != 0x00) _initialPieces.insert({i, j});

      if (pieceType == 0x01) // Up Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = direction_t::up;
        startPieceFound = true;
        break;
      }

      if (pieceType == 0x02) // Down Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = direction_t::down;
        startPieceFound = true;
        break;
      }

      if (pieceType == 0x08) // Right Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = direction_t::right;
        startPieceFound = true;
        break;
      }

      if (pieceType == 0x04) // Left Facing
      {
        _startPieceRow = i;
        _startPieceCol = j;
        _startPieceDirection = direction_t::left;
        startPieceFound = true;
        break;
      }

     }
    if (startPieceFound == false) JAFFAR_THROW_LOGIC("Could not find starter piece");

    _inputAPressCount = 0;
    _connectivity = 0;

  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);

    // Advancing current step
    _currentStep++;

    if (input == _buttonAInputIdx) _inputAPressCount++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_currentStep % 2      ); // Keep timer since we are only placing pieces on even turns
    hashEngine.Update(*_playerInput1 > 0    );
    hashEngine.Update(*_playerPosX          );
    hashEngine.Update(*_playerPosY          );
    hashEngine.Update(*_piecePlacingStatus  );
    hashEngine.Update(*_piecePlacingTimer1  );
    hashEngine.Update(*_piecePlacingTimer2  );
    hashEngine.Update(*_piecePlacingPosX    );
    hashEngine.Update(*_piecePlacingPosY    );
    hashEngine.Update(_gridStart, __PIPE_DREAM_GRID_ROWS * __PIPE_DREAM_GRID_COLS );
    hashEngine.Update(*_piecesPlaced        );
    hashEngine.Update(*_gameState);
    hashEngine.Update(_inputAPressCount);
    hashEngine.Update(_lingeringPieces);

    hashEngine.Update(*_pieceReplacementState); 
    if (*_pieceReplacementState != 255 && *_pieceReplacementState != 7) hashEngine.Update(*_pieceReplacementTimer); 
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    const auto path = calculatePipePath();
    std::set<piecePos_t> piecesInPath;
    for (auto& piece : path) piecesInPath.insert(piece);

    _currentDepth = path.size();
    _piecesOnBoard = 0;
    _lingeringPieces = 0;
    for (uint8_t i = 0; i < __PIPE_DREAM_GRID_ROWS; i++)
     for (uint8_t j = 0; j < __PIPE_DREAM_GRID_COLS; j++)
     { 
      const auto pieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + j];
      if (pieceType != 0)
      {
        _piecesOnBoard++;
        if (piecesInPath.contains({i,j}) == false)
        {
          _lingeringPieces++;

          // Clearing lingering pieces, if not fixed
          // if (_initialPieces.contains({i,j}) == false) _gridStart[i * __PIPE_DREAM_GRID_COLS + j] = 0x00;
        } 
      } 
     }      

    _distanceToReward = std::abs((int8_t)_targetScore - (int8_t)_currentDepth);

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
     serializer.push(&_currentDepth, sizeof(_currentDepth));
     serializer.push(&_piecesOnBoard, sizeof(_piecesOnBoard));
     serializer.push(&_distanceToReward, sizeof(_distanceToReward));
     serializer.push(&_inputAPressCount, sizeof(_inputAPressCount));
     serializer.push(&_lingeringPieces, sizeof(_lingeringPieces));
     serializer.push(&_connectivity, sizeof(_connectivity));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_currentDepth, sizeof(_currentDepth));
     deserializer.pop(&_piecesOnBoard, sizeof(_piecesOnBoard));
     deserializer.pop(&_distanceToReward, sizeof(_distanceToReward));
     deserializer.pop(&_inputAPressCount, sizeof(_inputAPressCount));
     deserializer.pop(&_lingeringPieces, sizeof(_lingeringPieces));
     deserializer.pop(&_connectivity, sizeof(_connectivity));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Adding depth to reward
    reward += - (float)_distanceToReward + (float)_connectivity * 0.01f;

    // Returning reward
    return reward;
  }

  __INLINE__ bool canPlacePiece() const 
  {
     if (*_piecePlacingStatus > 0) return false;
     if (*_pieceReplacementState != 255 && *_pieceReplacementState != 7) return false;
     return true;
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    // The gauntlet
    bool allowN = true;
    bool allowA = true;
    bool allowU = true;
    bool allowD = true;
    bool allowL = true;
    bool allowR = true;
    // bool allowUL = true;
    // bool allowUR = true;
    // bool allowDL = true;
    // bool allowDR = true;

    if (*_playerPosX == 0)                          { allowL = false; } // allowUL = false; allowDL = false; }
    if (*_playerPosX == __PIPE_DREAM_GRID_COLS - 1) { allowR = false; } // allowUR = false; allowDR = false; } 
    if (*_playerPosY == 0)                          { allowU = false; } // allowUL = false; allowUR = false; }
    if (*_playerPosY == __PIPE_DREAM_GRID_ROWS - 1) { allowD = false; } // allowDL = false; allowDR = false; }

    // Do not place piece if there is one there already. Don't even linger
    const uint8_t playerPosIdx = *_playerPosY * __PIPE_DREAM_GRID_COLS + *_playerPosX;
    const uint8_t tileType = _gridStart[playerPosIdx];
    if (tileType != 0x00)
    {
      allowN = false;
      allowA = false;
    } 

    // Do not place piece if already placing one
    if (canPlacePiece() == false) allowA = false;

    // Force placing a piece if you can
    if (canPlacePiece() == true && *_piecesPlaced > 0)
    {
      allowN  = false;
      allowA  = true;
      allowU  = false;
      allowD  = false;
      allowL  = false;
      allowR  = false;
      // allowUL = false;
      // allowUR = false;
      // allowDL = false;
      // allowDR = false;
    }

    // Only add inputs if on even steps
    // if (_currentStep % 2 == 1)
    // {
    //   allowN = true;
    //   allowA = false;
    //   allowU = false;
    //   allowD = false;
    //   allowL = false;
    //   allowR = false;
    //   // allowUL = false;
    //   // allowUR = false;
    //   // allowDL = false;
    //   // allowDR = false;
    // } 

    // // If a movement has been made in the previous frame, do not press anything now
    // if (*_playerInput1 > 0)
    // {
    //   allowN  = true;
    //   allowA  = false;
    //   allowU  = false;
    //   allowD  = false;
    //   allowL  = false;
    //   allowR  = false;
    //   // allowUL = false;
    //   // allowUR = false;
    //   // allowDL = false;
    //   // allowDR = false;
    // } 


    if (allowN  == true) allowedInputSet.push_back(_nullInputIdx    );
    if (allowA  == true) allowedInputSet.push_back(_buttonAInputIdx );
    if (allowU  == true) allowedInputSet.push_back(_buttonUInputIdx );
    if (allowD  == true) allowedInputSet.push_back(_buttonDInputIdx );
    if (allowL  == true) allowedInputSet.push_back(_buttonLInputIdx );
    if (allowR  == true) allowedInputSet.push_back(_buttonRInputIdx );
    //if (allowUL == true) allowedInputSet.push_back(_buttonULInputIdx );
    //if (allowUR == true) allowedInputSet.push_back(_buttonURInputIdx );
    //if (allowDL == true) allowedInputSet.push_back(_buttonDLInputIdx );
    //if (allowDR == true) allowedInputSet.push_back(_buttonDRInputIdx );
  }


  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:            %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Game State:              %02u\n", *_gameState);
    jaffarCommon::logger::log("[J+]  + Current Depth:           %02u / %02u (Dist: %02u)\n", _currentDepth, _targetScore, _distanceToReward);
    jaffarCommon::logger::log("[J+]  + Pending Score:           %02u\n", *_pendingScore);
    jaffarCommon::logger::log("[J+]  + Connectivity:            %04u\n", _connectivity);
    jaffarCommon::logger::log("[J+]  + Player Input:            %02u\n", *_playerInput1);
    jaffarCommon::logger::log("[J+]  + Player Pos X:            %02u\n", *_playerPosX);
    jaffarCommon::logger::log("[J+]  + Player Pos Y:            %02u\n", *_playerPosY);
    jaffarCommon::logger::log("[J+]  + Piece Placing Status:    %02u\n", *_piecePlacingStatus);
    jaffarCommon::logger::log("[J+]  + Piece Placing Timer 1:   %02u\n", *_piecePlacingTimer1);
    jaffarCommon::logger::log("[J+]  + Piece Placing Timer 2:   %02u\n", *_piecePlacingTimer2);
    jaffarCommon::logger::log("[J+]  + Piece Placing Pos X:     %02u\n", *_piecePlacingPosX);
    jaffarCommon::logger::log("[J+]  + Piece Placing Pos Y:     %02u\n", *_piecePlacingPosY);
    jaffarCommon::logger::log("[J+]  + Pieces Placed:           %02u\n", *_piecesPlaced );
    jaffarCommon::logger::log("[J+]  + Pieces Lingering:        %02u\n", _lingeringPieces );
    jaffarCommon::logger::log("[J+]  + Pieces On Board:         %02u\n", _piecesOnBoard );
    jaffarCommon::logger::log("[J+]  + Initial Pieces:          %02u\n", _initialPieces.size() );
    jaffarCommon::logger::log("[J+]  + Input A Pressed Count:   %02u\n", _inputAPressCount );
    
    
    jaffarCommon::logger::log("[J+]  + Piece Replacement:       %02u (%03u)\n", *_pieceReplacementState, *_pieceReplacementTimer );
    jaffarCommon::logger::log("[J+]  + Grid:\n");

    for (size_t i = 0; i < __PIPE_DREAM_GRID_ROWS; i++)
    {
      jaffarCommon::logger::log("[J+]    "); 
      for (size_t j = 0; j < __PIPE_DREAM_GRID_COLS; j++)
      {
        const auto pieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + j];
        if (_pieceTypes.contains(pieceType) == false) { printf("Did not recognize next piece type: 0%02X", pieceType); break; }
        const auto& piece = _pieceTypes.at(pieceType);
        jaffarCommon::logger::log("%s", piece.shape.c_str());
      }
      jaffarCommon::logger::log("\n");
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ std::vector<piecePos_t> calculatePipePath() const
  {
    uint8_t currentPieceRow = _startPieceRow;
    uint8_t currentPieceCol = _startPieceCol;
    uint8_t currentDirection = _startPieceDirection;

    std::vector<piecePos_t> path;
    path.push_back({currentPieceRow, currentPieceCol});

    while(true)
    {
      // Checking boundaries
      if (currentPieceCol == 0                          && currentDirection == direction_t::left)  break;
      if (currentPieceCol == __PIPE_DREAM_GRID_COLS - 1 && currentDirection == direction_t::right) break;
      if (currentPieceRow == 0                          && currentDirection == direction_t::up)    break;
      if (currentPieceRow == __PIPE_DREAM_GRID_ROWS - 1 && currentDirection == direction_t::down)  break;

      // Getting next piece's position
      uint8_t nextPieceCol = currentPieceCol;
      if (currentDirection == direction_t::left)  nextPieceCol--;
      if (currentDirection == direction_t::right) nextPieceCol++;

      uint8_t nextPieceRow = currentPieceRow;
      if (currentDirection == direction_t::up)   nextPieceRow--;
      if (currentDirection == direction_t::down) nextPieceRow++;

      // Getting next piece's information
      const uint8_t nextPieceIdx = nextPieceRow * __PIPE_DREAM_GRID_COLS + nextPieceCol;
      const uint8_t nextPieceType = _gridStart[nextPieceIdx];
      if (_pieceTypes.contains(nextPieceType) == false) { printf("Did not recognize next piece type: 0%02X", nextPieceType); break; }
      const auto& nextPiece = _pieceTypes.at(nextPieceType);

      // Checking if it accepts the incoming stream
      if (currentDirection == direction_t::left  && nextPiece.RInConnectivity == false) break;  
      if (currentDirection == direction_t::right && nextPiece.LInConnectivity == false) break;
      if (currentDirection == direction_t::up    && nextPiece.DInConnectivity == false) break;
      if (currentDirection == direction_t::down  && nextPiece.UInConnectivity == false) break;

      // Checking if direction changes
      auto nextDirection = currentDirection;
      if (currentDirection == direction_t::left  && nextPiece.LRedirection != direction_t::none) nextDirection = nextPiece.LRedirection;
      if (currentDirection == direction_t::right && nextPiece.RRedirection != direction_t::none) nextDirection = nextPiece.RRedirection;
      if (currentDirection == direction_t::up    && nextPiece.URedirection != direction_t::none) nextDirection = nextPiece.URedirection;
      if (currentDirection == direction_t::down  && nextPiece.DRedirection != direction_t::none) nextDirection = nextPiece.DRedirection;
      
      // Updating values for the next iteration
      currentPieceRow = nextPieceRow;
      currentPieceCol = nextPieceCol;
      currentDirection = nextDirection;

      // Increasing Depth
      path.push_back({currentPieceRow, currentPieceCol});
    }

    return path;
  }

  __INLINE__ uint16_t calculateConnectivity() const
  {
    uint16_t connectivity = 0;

    for (uint8_t i = 0; i < __PIPE_DREAM_GRID_ROWS; i++)
     for (uint8_t j = 0; j < __PIPE_DREAM_GRID_COLS; j++)
     { 
      const auto pieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + j];
      const auto& piece = _pieceTypes.at(pieceType);

      if (pieceType != 0x00)
      {
        // Up
        if (i > 0)
        {
          const auto boundaryPieceType = _gridStart[(i - 1) * __PIPE_DREAM_GRID_COLS + j];
          const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
          if (piece.UOutConnectivity == true && boundaryPiece.DInConnectivity  == true) connectivity++;
          if (piece.UInConnectivity  == true && boundaryPiece.DOutConnectivity == true) connectivity++;
        }

        // Down
        if (i < (__PIPE_DREAM_GRID_ROWS - 1))
        {
          const auto boundaryPieceType = _gridStart[(i + 1) * __PIPE_DREAM_GRID_COLS + j];
          const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
          if (piece.DOutConnectivity == true && boundaryPiece.UInConnectivity  == true) connectivity++;
          if (piece.DInConnectivity  == true && boundaryPiece.UOutConnectivity == true) connectivity++;
        }

        // Left
        if (j > 0)
        {
          const auto boundaryPieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + (j - 1)];
          const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
          if (piece.LOutConnectivity == true && boundaryPiece.RInConnectivity  == true) connectivity++;
          if (piece.LInConnectivity  == true && boundaryPiece.ROutConnectivity == true) connectivity++;
        }

        // Right
        if (j < (__PIPE_DREAM_GRID_COLS - 1))
        {
          const auto boundaryPieceType = _gridStart[i * __PIPE_DREAM_GRID_COLS + (j + 1)];
          const auto& boundaryPiece = _pieceTypes.at(boundaryPieceType);
          if (piece.ROutConnectivity == true && boundaryPiece.LInConnectivity  == true) connectivity++;
          if (piece.RInConnectivity  == true && boundaryPiece.LOutConnectivity == true) connectivity++;
        }
      }
     }    

    return connectivity;
  }

  uint8_t* _playerInput1        ;
  uint8_t* _playerInput2        ;
  uint8_t* _playerInput3        ;
  uint8_t* _playerInput4        ;
  uint8_t* _playerInput5        ;
  uint8_t* _playerInput6        ;
  uint8_t* _playerInput7        ;
  uint8_t* _playerPosX          ;
  uint8_t* _playerPosY          ;
  uint8_t* _piecePlacingStatus  ;
  uint8_t* _piecePlacingTimer1  ;
  uint8_t* _piecePlacingTimer2  ;
  uint8_t* _piecePlacingPosX    ;
  uint8_t* _piecePlacingPosY    ;
  uint8_t* _gridStart           ;
  uint8_t* _gridEnd             ;
  uint8_t* _piecesPlaced        ;
  uint8_t* _gameState;
  uint8_t* _pendingScore;
  uint8_t* _pieceReplacementState;
  uint8_t* _pieceReplacementTimer;

  uint16_t _currentStep;
  uint8_t* _lowMem;

  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _buttonAInputIdx;
  InputSet::inputIndex_t _buttonUInputIdx;
  InputSet::inputIndex_t _buttonDInputIdx;
  InputSet::inputIndex_t _buttonLInputIdx;
  InputSet::inputIndex_t _buttonRInputIdx;
  // InputSet::inputIndex_t _buttonULInputIdx;
  // InputSet::inputIndex_t _buttonURInputIdx;
  // InputSet::inputIndex_t _buttonDLInputIdx;
  // InputSet::inputIndex_t _buttonDRInputIdx;

  std::map<uint8_t, pieceType_t> _pieceTypes;

  std::set<piecePos_t> _initialPieces;

  uint8_t _startPieceRow;
  uint8_t _startPieceCol;
  direction_t _startPieceDirection;
  uint8_t _currentDepth;
  uint8_t _piecesOnBoard;
  uint8_t _targetScore;
  uint8_t _lingeringPieces;
  uint8_t _distanceToReward;
  uint8_t _inputAPressCount;
  uint16_t _connectivity;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
