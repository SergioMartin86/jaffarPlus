#include <emulator.hpp>
#include "quickerNES/quickerNES.hpp"

namespace jaffarPlus
{
 #define DETECT_EMULATOR(EMULATOR) if (emulatorName == EMULATOR::getName()) return std::make_unique<EMULATOR>(config);
  
 std::unique_ptr<Emulator> Emulator::getEmulator(const std::string& emulatorName, const nlohmann::json& config)
 {
  DETECT_EMULATOR(emulator::QuickerNES);

  EXIT_WITH_ERROR("Emulator '%s' not recognized\n", emulatorName.c_str());
 }

} // namespace jaffarPlus
