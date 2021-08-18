#include "frame.h"

size_t _moveListStorageSize;
size_t _ruleCount;
size_t _maxSteps;
bool _storeMoveList;

Frame::Frame()
{
  // Two moves fit in one byte
  moveHistory.resize(_moveListStorageSize);

  // Setting i nitial value of the restart flag
  isRestart = false;
}

size_t Frame::getSerializationSize()
{
  size_t size = 0;

  // Adding move history
  if (_storeMoveList == true)
   size += _moveListStorageSize * sizeof(char);

  // Adding frame state data
  size += _STATE_DATA_SIZE * sizeof(uint8_t);

  // Adding rule status information
  size += _ruleCount * sizeof(char);

  // Adding restart condition
  size += sizeof(bool);

  // Adding reward
  size += sizeof(float);

  return size;
}

void Frame::serialize(char *output)
{
  size_t currentPos = 0;

  // Adding move history
  if (_storeMoveList == true)
  {
   memcpy(&output[currentPos], moveHistory.data(), _moveListStorageSize * sizeof(char));
   currentPos += _moveListStorageSize * sizeof(char);
  }

  // Adding frame state data
  memcpy(&output[currentPos], frameStateData, _STATE_DATA_SIZE * sizeof(uint8_t));
  currentPos += _STATE_DATA_SIZE * sizeof(uint8_t);

  // Copying Rule status information
  memcpy(&output[currentPos], rulesStatus.data(), _ruleCount * sizeof(char));
  currentPos += _ruleCount * sizeof(char);

  // Copying restart flag
  memcpy(&output[currentPos], &isRestart, sizeof(bool));
  currentPos += sizeof(bool);

  // Copying reward
  memcpy(&output[currentPos], &reward, sizeof(float));
  currentPos += sizeof(float);
}

void Frame::deserialize(const char *input)
{
  size_t currentPos = 0;

  // Copying move history information
  if (_storeMoveList == true)
  {
   moveHistory.resize(_moveListStorageSize);
   memcpy(moveHistory.data(), &input[currentPos], _moveListStorageSize * sizeof(char));
   currentPos += _moveListStorageSize * sizeof(char);
  }

  // Adding frame state data
  memcpy(frameStateData, &input[currentPos], _STATE_DATA_SIZE * sizeof(uint8_t));
  currentPos += _STATE_DATA_SIZE * sizeof(uint8_t);

  // Copying Rule status information
  rulesStatus.resize(_ruleCount);
  memcpy(rulesStatus.data(), &input[currentPos], _ruleCount * sizeof(char));
  currentPos += _ruleCount * sizeof(char);

  // Copying restart flag
  memcpy(&isRestart, &input[currentPos], sizeof(bool));
  currentPos += sizeof(bool);

  // Copying reward flag
  memcpy(&reward, &input[currentPos], sizeof(float));
  currentPos += sizeof(float);
}

Frame &Frame::operator=(Frame sourceFrame)
{
  if (_storeMoveList == true) moveHistory = sourceFrame.moveHistory;
  memcpy(frameStateData, sourceFrame.frameStateData, _STATE_DATA_SIZE * sizeof(uint8_t));
  rulesStatus = sourceFrame.rulesStatus;
  isRestart = sourceFrame.isRestart;
  reward = sourceFrame.reward;
  return *this;
}

// Move r/w operations
void Frame::setMove(const size_t idx, const uint8_t move)
{
  size_t basePos = idx / 2;
  uint8_t baseVal = moveHistory[basePos];
  uint8_t newVal;

  if (idx % 2 == 0) newVal = (baseVal & 0xF0) | (move & 0x0F);
  if (idx % 2 == 1) newVal = (baseVal & 0x0F) | (move << 4);

  moveHistory[basePos] = newVal;
}

uint8_t Frame::getMove(const size_t idx)
{
 size_t basePos = idx / 2;
 uint8_t val = moveHistory[basePos];
 if (idx % 2 == 0) val = val & 0x0F;
 if (idx % 2 == 1) val = val >> 4;
 return val;
}

