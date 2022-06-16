#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>

#include "shared.h"
#include "sms_ntsc.h"
#include "md_ntsc.h"

class EmuInstance : public EmuInstanceBase
{
 public:

 uint8_t* _68KRam;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  FILE *fp;

  /* set default config */
  error_init();
  set_config_defaults();

  /* mark all BIOS as unloaded */
  system_bios = 0;

  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();
  if(!load_rom(romFilePath.c_str())) EXIT_WITH_ERROR("Could not initialize emulator with rom file: %s\n", romFilePath.c_str());

  /* initialize system hardware */
  //#define SOUND_FREQUENCY 48000
  //#define SOUND_SAMPLES_SIZE  2048
  //audio_init(SOUND_FREQUENCY, 0);
  system_init();
  system_reset();

  // Getting pointer to 68K Ram
  _68KRam = work_ram;
 }

 void loadStateFile(const std::string& stateFilePath) override
 {
  // Loading state data
  std::string stateData;
  if (loadStringFromFile(stateData, stateFilePath.c_str()) == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());

  // Loading state object into the emulator
  state_load((uint8_t*)stateData.c_str());
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  std::string stateBuffer;
  stateBuffer.resize(STATE_SIZE);
  size_t stateSize = state_save((uint8_t*)stateBuffer.c_str());
  stateBuffer.resize(stateSize);
  saveStringToFile(stateBuffer, stateFilePath.c_str());
 }

 void serializeState(uint8_t* state) const override
 {
  state_save(state);
 }

 void deserializeState(const uint8_t* state) override
 {
  state_load(state);
 }

 static uint8_t moveStringToCode(const std::string& move)
 {
  uint8_t moveCode = 0;


  return moveCode;
 }

 static std::string moveCodeToString(const uint8_t move)
 {
  std::string moveString;

  return moveString;
 }

 void advanceState(const std::string& move) override
 {
  advanceState(moveStringToCode(move));
 }

 void advanceState(const uint8_t move) override
 {
  input.pad[0] = move;
  system_frame_gen(1);
 }

};
