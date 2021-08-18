#pragma once

#define _MAX_MOVE_SIZE 4

#include "blastemInstance.h"
#include "nlohmann/json.hpp"
#include "rule.h"
#include <string>
#include <vector>

class Frame
{
  public:
  Frame();

  // Stores the entire move history of the frame
  std::vector<uint8_t> moveHistory;

  // The score calculated for this frame
  float reward;

  // Stores the game state data
  uint8_t frameStateData[_STATE_DATA_SIZE];

  // Rule status vector
  std::vector<char> rulesStatus;

  // Serialization functions
  static size_t getSerializationSize();
  void serialize(char *output);
  void deserialize(const char *input);

  // Additional local metadata
  bool isRestart;

  // Move r/w operations
  void setMove(const size_t idx, const uint8_t move);
  uint8_t getMove(const size_t idx);

  Frame &operator=(Frame sourceFrame);
};

extern size_t _ruleCount;
extern size_t _maxSteps;
extern size_t _moveListStorageSize;
