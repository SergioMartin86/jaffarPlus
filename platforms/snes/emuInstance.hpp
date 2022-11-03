#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>
#include "bsnes.hpp"
#include <sfc/interface/interface.hpp>

class EmuInstance : public EmuInstanceBase
{
 public:

 SuperFamicom::Interface* _emu;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  settings.load();
  _emu = new SuperFamicom::Interface;
  emulator = _emu;
  Emulator::platform = &program;

  emulator->configure("System/Serialization/Method", settings.emulator.serialization.method);
  emulator->configure("Hacks/Hotfixes", settings.emulator.hack.hotfixes);
  emulator->configure("Hacks/Entropy", settings.emulator.hack.entropy);
  emulator->configure("Hacks/CPU/Overclock", settings.emulator.hack.cpu.overclock);
  emulator->configure("Hacks/CPU/FastMath", settings.emulator.hack.cpu.fastMath);
  emulator->configure("Hacks/PPU/Fast", settings.emulator.hack.ppu.fast);
  emulator->configure("Hacks/PPU/Deinterlace", settings.emulator.hack.ppu.deinterlace);
  emulator->configure("Hacks/PPU/NoSpriteLimit", settings.emulator.hack.ppu.noSpriteLimit);
  emulator->configure("Hacks/PPU/NoVRAMBlocking", settings.emulator.hack.ppu.noVRAMBlocking);
  emulator->configure("Hacks/PPU/Mode7/Scale", settings.emulator.hack.ppu.mode7.scale);
  emulator->configure("Hacks/PPU/Mode7/Perspective", settings.emulator.hack.ppu.mode7.perspective);
  emulator->configure("Hacks/PPU/Mode7/Supersample", settings.emulator.hack.ppu.mode7.supersample);
  emulator->configure("Hacks/PPU/Mode7/Mosaic", settings.emulator.hack.ppu.mode7.mosaic);
  emulator->configure("Hacks/DSP/Fast", settings.emulator.hack.dsp.fast);
  emulator->configure("Hacks/DSP/Cubic", settings.emulator.hack.dsp.cubic);
  emulator->configure("Hacks/DSP/EchoShadow", settings.emulator.hack.dsp.echoShadow);
  emulator->configure("Hacks/Coprocessor/DelayedSync", settings.emulator.hack.coprocessor.delayedSync);
  emulator->configure("Hacks/Coprocessor/PreferHLE", settings.emulator.hack.coprocessor.preferHLE);
  emulator->configure("Hacks/SuperFX/Overclock", settings.emulator.hack.superfx.overclock);
  if(!emulator->load()) EXIT_WITH_ERROR("[ERROR] Could not load emulator.\n");


  program.superFamicom.location = "/home/jaffar/pop.sfc";
  program.superFamicom.option = "Auto";
  if(program.loadSuperFamicom(program.superFamicom.location) == false) EXIT_WITH_ERROR("[ERROR] Could not load state file '%s'.\n", stateFilePath.c_str());
  emulator->power();
 }

 void loadStateFile(const std::string& stateFilePath) override
 {

 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
 }

 void serializeState(uint8_t* state) const override
 {
  auto gameState = emulator->serialize(0);
//  printf("State Size: %u\n", gameState.size());
  memcpy(state, gameState.data(), _STATE_DATA_SIZE_TRAIN);
 }

 void deserializeState(const uint8_t* state) override
 {
  auto gameState = serializer(state, _STATE_DATA_SIZE_TRAIN);
  emulator->unserialize(gameState);
 }

 static INPUT_TYPE moveStringToCode(const std::string& move)
 {
  INPUT_TYPE moveCode = 0;

  return moveCode;
 }

 static std::string moveCodeToString(const INPUT_TYPE move)
 {
  std::string moveString;

  return moveString;
 }

 void advanceState(const INPUT_TYPE move) override
 {
  emulator->run();
 }

};
