#include <sfc/sfc.hpp>

namespace SuperFamicom {

#include "serialization.cpp"
thread_local SufamiTurboCartridge sufamiturboA;
thread_local SufamiTurboCartridge sufamiturboB;

auto SufamiTurboCartridge::unload() -> void {
  rom.reset();
  ram.reset();
}

auto SufamiTurboCartridge::power() -> void {
}

}
