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

  static __INLINE__ std::string getName() { return "PipeBot / PipeBot"; }

  PipeBot(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) 
    : jaffarPlus::Game(std::move(emulator), config),
      _pipeBot((emulator::PipeBot*)_emulator.get())
  {
    _rowCount = _pipeBot->getRowCount();
    _colCount = _pipeBot->getColCount();
  }

  struct possibleInput_t
  {
    std::string inputString;
    InputSet::inputIndex_t inputIndex;
    uint8_t pieceType;
  };

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _grid = _emulator->getProperty("Grid").pointer;

    registerGameProperty("Forward Depth"            ,&_forwardDepth, Property::datatype_t::dt_uint8 , Property::endianness_t::little);
    registerGameProperty("Score",  &_score, Property::datatype_t::dt_int32 , Property::endianness_t::little);
    registerGameProperty("End Is Met",  &_endIsMet, Property::datatype_t::dt_bool , Property::endianness_t::little);

    // Getting piece inventory
    auto nextPiece = _pipeBot->getNextPiece();
    size_t pieceCount = 0;
    while (nextPiece != 0x00 && pieceCount < 120)
    {
     _pieceInventory[nextPiece]++;
     nextPiece = _pipeBot->getNextPiece();
     pieceCount++;
    }

    // Creating list of inputs
    for (const auto& piece : _pieceInventory)
    {
        const auto pieceType = piece.first;
        char inputBuffer[256];
        sprintf(inputBuffer, "|%3u|", pieceType);
        const std::string inputString(inputBuffer);
        const auto inputCode = _emulator->registerInput(inputString);
        const auto newInput = possibleInput_t({inputString, inputCode, pieceType});
        _possibleInputs.push_back(newInput);
        _inputMap[inputCode] = newInput;
    }

    _score = 0;
    _endIsMet = false;
  }

  // Function to report what all the possible input that the game might require
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    std::set<std::string> possibleInputSet;
    for (const auto& input : _possibleInputs) possibleInputSet.insert(input.inputString);
    return possibleInputSet;
  }
  
  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    const auto& possibleInput = _inputMap[input];

    // Running emulator
    _emulator->advanceState(input);

    // Reducing inventory
    _pieceInventory[possibleInput.pieceType]--;

    // Advancing current step
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_grid, _rowCount * _colCount * sizeof(uint8_t));
    for (auto& piece : _pieceInventory) hashEngine.Update(piece.second);
    hashEngine.Update(_pipeBot->getCurrentPos());
    // hashEngine.Update(_currentStep);
    // hashEngine.Update(_playerCurrentRow);
    // hashEngine.Update(_playerCurrentCol);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    std::set<std::pair<uint8_t, uint8_t>> piecesInPath;

    const auto forwardPath = _pipeBot->calculatePipePath();
    for (auto& piece : forwardPath) piecesInPath.insert({piece.pos.row, piece.pos.col});
    _forwardDepth = forwardPath.size();
    _crossingPieceCount = getCrossingPieceCount(forwardPath);
    _score = (100 * _forwardDepth) + (10000 * _crossingPieceCount);
    
    // If the end piece is an end piece, the score is doubled
    const auto& endPiecePos = forwardPath.rbegin()->pos;
    const auto endPieceType = _pipeBot->getPiece(endPiecePos);

    _endIsMet = false;
    if (endPieceType == 0x10 || endPieceType == 0x20 || endPieceType == 0x40 || endPieceType == 0x80) _endIsMet = true;
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
     serializer.push(&_endIsMet, sizeof(_endIsMet));
     for (auto& piece : _pieceInventory) serializer.push(&piece.second, sizeof(uint8_t));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
     deserializer.pop(&_currentStep, sizeof(_currentStep));
     deserializer.pop(&_forwardDepth, sizeof(_forwardDepth));
     deserializer.pop(&_endIsMet, sizeof(_endIsMet));
     for (auto& piece : _pieceInventory) deserializer.pop(&piece.second, sizeof(uint8_t));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    reward += (float)_score;

    // Returning reward
    return reward;
  }

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    const auto currentPos = _pipeBot->getCurrentPos();
    if (_pipeBot->getPiece(currentPos) != 0x00) return;

    const auto currentDirection = _pipeBot->getCurrentDirection();

    for (const auto& input : _possibleInputs)
     if (_pieceInventory[input.pieceType] > 0)
      {
        const auto type = _pipeBot->getPieceType(input.pieceType);

        if (currentDirection == emulator::PipeBot::direction_t::right && type.LInConnectivity == false) continue;
        if (currentDirection == emulator::PipeBot::direction_t::left  && type.RInConnectivity == false) continue;
        if (currentDirection == emulator::PipeBot::direction_t::down  && type.UInConnectivity == false) continue;
        if (currentDirection == emulator::PipeBot::direction_t::up    && type.DInConnectivity == false) continue;

        allowedInputSet.push_back(input.inputIndex);
      } 
  }


  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:            %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Forward Depth:           %02u\n", _forwardDepth);
    jaffarCommon::logger::log("[J+]  + Current Score:           %d\n", _score);
    jaffarCommon::logger::log("[J+]  + End is Met:              %s\n", _endIsMet ? "True" : "False");
    jaffarCommon::logger::log("[J+]  + Crossing Piece Count:    %02u\n", _crossingPieceCount);
    jaffarCommon::logger::log("[J+]  + Piece Inventory: \n");
    for (const auto& piece : _pieceInventory)
    {
      const auto pieceType = piece.first;
      const auto pieceCount = piece.second;
      const auto& pieceShape = _pipeBot->getPieceType(pieceType).shape;
      jaffarCommon::logger::log("[J+]     + %03u - '%s': %u\n", pieceType, pieceShape.c_str(), pieceCount);
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ uint8_t getLastCrossingPiece(const std::vector<emulator::PipeBot::piecePath_t>& piecePath) const
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

  __INLINE__ uint8_t getCrossingPieceCount(const std::vector<emulator::PipeBot::piecePath_t>& piecePath) const
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

  uint16_t _currentStep;

  uint8_t _forwardDepth;
  uint8_t _crossingPieceCount;
  int32_t _score;

  emulator::PipeBot* const _pipeBot;
  uint8_t _rowCount;
  uint8_t _colCount; 
  uint8_t* _grid;
  bool _endIsMet;

  std::vector<possibleInput_t> _possibleInputs;
  std::map<InputSet::inputIndex_t, possibleInput_t> _inputMap;
  std::map<uint8_t, uint8_t> _pieceInventory;
};

} // namespace pipeBot

} // namespace games

} // namespace jaffarPlus
