#include <emulators/quickerNES/quickerNES.hpp>
#include <emulators/emulator.hpp>

namespace jaffarPlus
{

 Emulator* Emulator::getEmulator(const std::string& emulatorName, const nlohmann::json& config)
 {
  if (emulatorName == "QuickerNES") return new jaffarPlus::QuickerNES(config);

  EXIT_WITH_ERROR("Emulator '%s' not recognized\n", emulatorName.c_str());
 }

}
