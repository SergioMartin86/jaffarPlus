#include <emulators/emulator.hpp>
#include <emulators/quickerNES/quickerNES.hpp>

namespace jaffarPlus
{

 Emulator* Emulator::getEmulator(const std::string& emulatorName, const nlohmann::json& config)
 {
  if (emulatorName == "QuickerNES") return new jaffarPlus::QuickerNES(config);

  EXIT_WITH_ERROR("Emulator '%s' not recognized\n", emulatorName.c_str());
 }


}
