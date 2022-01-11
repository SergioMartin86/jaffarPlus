#include "frame.h"

size_t _maxFrameDiff;

Frame::Frame()
{
  // Setting initially with no differences wrt base frame
  frameDiffCount = 0;
}

