#pragma once

#include <emulator.hpp>
#include "inputParser.hpp"
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>

namespace jaffarPlus
{

namespace emulator
{

class PipeBot final : public Emulator
{
public:

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

  struct piece_t 
  {
    uint8_t row;
    uint8_t col;
    uint8_t type;
  };

  static std::string getName() { return "PipeBot"; }

  // Constructor must only do configuration parsing
  PipeBot(const nlohmann::json& config) : Emulator(config)
  {
    _rowCount = jaffarCommon::json::getNumber<uint8_t>(config, "Row Count");
    _colCount = jaffarCommon::json::getNumber<uint32_t>(config, "Col Count");
    _grid = (uint8_t*) malloc (_rowCount * _colCount * sizeof(uint8_t));

    _pieceTypes = 
    {  
      //             LIn    RIn    UIn    DIn     LOut   ROut   UOut   DOut   LRedirect          RRedirect          URedirect           DRedirect
      { 0x00, { ".", false, false, false, false,  false, false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x01, { "╨", false, false, false, false,  false, false, true,  false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x02, { "╥", false, false, false, false,  false, false, false, true,  direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x08, { "╞", false, false, false, false,  false, true,  false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x04, { "╡", false, false, false, false,  true,  false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x10, { "╩", false, false, true,  false,  false, false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x20, { "╦", false, false, false, true,   false, false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x80, { "╠", false, true,  false, false,  false, false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
      { 0x40, { "╣", true,  false, false, false,  false, false, false, false, direction_t::none, direction_t::none, direction_t::none,  direction_t::none,   } },
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

    _nextPieceQueue = jaffarCommon::json::getArray<uint8_t>(config, "Next Piece Queue");

    const auto& initialPiecesJs = jaffarCommon::json::getArray<nlohmann::json>(config, "Initial Pieces");
    for (const auto& pieceJs : initialPiecesJs)
    {
      const uint8_t pieceType = jaffarCommon::json::getNumber<uint8_t>(pieceJs, "Type");
      if (_pieceTypes.contains(pieceType) == false) { JAFFAR_THROW_LOGIC("Did not recognize next piece type: 0%02X", pieceType); }

      const uint8_t pieceRow  = jaffarCommon::json::getNumber<uint8_t>(pieceJs, "Row"); 
      const uint8_t pieceCol  = jaffarCommon::json::getNumber<uint8_t>(pieceJs, "Col"); 
      _initialPieces.push_back(piece_t({.row = pieceRow, .col = pieceCol, .type = pieceType}));
    }


    _inputParser = std::make_unique<jaffar::InputParser>(config);
  };

  ~PipeBot() { free (_grid); }

  void initializeImpl() override
  {
    // resetting grid to initial state
    reset();
  }

  __INLINE__ uint8_t getRowCount() const { return _rowCount; }
  __INLINE__ uint8_t getColCount() const { return _colCount; }

  __INLINE__ void reset()
  {
    for (size_t i = 0; i < _rowCount; i++) for (size_t j = 0; j < _colCount; j++) setPiece(i, j, 0x00);
    for (const auto& piece : _initialPieces) setPiece(piece.row, piece.col, piece.type);
  }

  __INLINE__ void setPiece(const uint8_t row, const uint8_t col, const uint8_t type)
  {
    _grid[row * _colCount + col] = type; 
  }

  __INLINE__ void placeNextPiece(const uint8_t row, const uint8_t col)
  {
    if (_currentPieceIdx >= _nextPieceQueue.size()) JAFFAR_THROW_LOGIC("Ran out of pieces");
    const auto piece = _nextPieceQueue[_currentPieceIdx];
    setPiece(row, col, piece); 
    _currentPieceIdx++;
  }

  __INLINE__ uint8_t getPiece(const uint8_t row, const uint8_t col) const
  {
    return _grid[row * _colCount + col];
  }

  __INLINE__ const pieceType_t& getPieceType(const uint8_t typeId) const
  {
    if (_pieceTypes.contains(typeId) == false) { JAFFAR_THROW_LOGIC("Did not recognize next piece type id: 0%02X", typeId); }
    return _pieceTypes.at(typeId);
  }


  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override
  {
     return _inputParser.get();
  }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override
  {
    
  }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override
  {
     serializer.push(_grid, _rowCount * _colCount * sizeof(uint8_t)); 
     serializer.push(&_currentPieceIdx, sizeof(_currentPieceIdx));
  };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override
  {
     deserializer.pop(_grid, _rowCount * _colCount * sizeof(uint8_t));
     deserializer.pop(&_currentPieceIdx, sizeof(_currentPieceIdx));
  };

  __INLINE__ void printInfo() const override
  { 
    const auto& nextPiece = _nextPieceQueue[_currentPieceIdx];
    const auto& nextPieceType = _pieceTypes.at(nextPiece);
    jaffarCommon::logger::log("[J+]  + Next Piece: %s, %u (0x%X)\n", nextPieceType.shape.c_str(), nextPiece, nextPiece); 
    jaffarCommon::logger::log("[J+]  + Grid:\n");
    for (size_t i = 0; i < _rowCount; i++)
    {
      jaffarCommon::logger::log("[J+]    "); 
      for (size_t j = 0; j < _colCount; j++)
      {
        const auto pieceType = getPiece(i,j);
        if (_pieceTypes.contains(pieceType) == false) { printf("Did not recognize next piece type: 0%02X", pieceType); break; }
        const auto& piece = _pieceTypes.at(pieceType);
        jaffarCommon::logger::log("%s", piece.shape.c_str());
      }
      jaffarCommon::logger::log("\n");
    }
  }

  property_t getProperty(const std::string& propertyName) const override
  {
    if (propertyName == "Grid") return property_t(_grid, _rowCount * _colCount * sizeof(uint8_t));

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override {}

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override {}

  __INLINE__ void enableRendering() override {}

  __INLINE__ void disableRendering() override {}

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base& serializer) const override { serializeState(serializer); }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override { deserializeState(deserializer); }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override {}

  __INLINE__ void doSoftReset() { };

private:

  uint8_t _rowCount;
  uint8_t _colCount;
  uint8_t* _grid;

  std::map<uint8_t, pieceType_t> _pieceTypes;
  std::vector<piece_t> _initialPieces;
  std::unique_ptr<jaffar::InputParser> _inputParser;


  std::vector<uint8_t> _nextPieceQueue;
  size_t _currentPieceIdx;

};

} // namespace emulator

} // namespace jaffarPlus
