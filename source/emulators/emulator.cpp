#include <emulators/quickerNES/quickerNES.hpp>
#include <emulators/emulator.hpp>

namespace jaffarPlus
{
 #define DETECT_EMULATOR(E) if (emulatorName == E::getName()) return std::make_unique<E>(config);
  
 std::unique_ptr<Emulator> Emulator::getEmulator(const std::string& emulatorName, const nlohmann::json& config)
 {
  DETECT_EMULATOR(emulator::QuickerNES);

  EXIT_WITH_ERROR("Emulator '%s' not recognized\n", emulatorName.c_str());
 }

} // namespace jaffarPlus
