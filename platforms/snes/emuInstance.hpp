#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "gfx.h"
#include "snapshot.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "logger.h"
#include "display.h"
#include "conffile.h"
#include "statemanager.h"

#include "unix.hpp"
extern thread_local bool doRendering;

class EmuInstance : public EmuInstanceBase
{
 public:

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  int argc = 2;
  char romPath[4096];
  strcpy(romPath, romFilePath.c_str());
  char* argv[2] = { "./snes9x", romPath };

  doRendering = true;
  initSnes9x(argc, argv);
 }

 void loadStateFile(const std::string& stateFilePath) override
 {

 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
 }

 void serializeState(uint8_t* state) const override
 {
 }

 void deserializeState(const uint8_t* state) override
 {
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
 }

};
