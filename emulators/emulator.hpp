#pragma once
#include <utils.hpp>

namespace jaffarPlus
{

class Emulator
{
  public:

  Emulator() = default;
  virtual ~Emulator() = default;

  inline void advanceState(const std::string &move)
  {
   
  };

  inline void loadStateFile(const std::string &stateFilePath)
  {
  }

  inline void saveStateFile(const std::string &stateFilePath) const
  {
  }
};

} // namespace jaffarPlus