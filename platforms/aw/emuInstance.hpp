#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <boost/context/continuation.hpp>
#include "engine.h"
#include "sysImplementation.hpp"
#include "sys.h"

class EmuInstance : public EmuInstanceBase
{
 std::string _gameFilesPath;
 boost::context::continuation gameContext;

 public:

 Engine* _engine;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Game Files") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Files' key.\n");
  _gameFilesPath = config["Game Files"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  _enableRender = false;
  SDLStub* sysImplementation = new SDLStub();
  _engine = new Engine(sysImplementation, _gameFilesPath.c_str(), "");
  _engine->init();

  if (stateFilePath != "") loadStateFile(stateFilePath);

  gameContext = boost::context::callcc(
    [this](boost::context::continuation && sink)
    {
      // Return once
      sink = sink.resume();

      // Store return context
      gameContext = std::move(sink);

      // Set return state
      _engine->sys->_doReturn = false;

      // Infinite game loop
      while(true)
      {
       _engine->vm.checkThreadRequests();
       _engine->vm.inp_updatePlayer();
       _engine->processInput();

       if (_engine->sys->_doReturn == true)
       {
        // Returning to Jaffar
        gameContext = gameContext.resume();
        _engine->sys->_doReturn = false;
       }

       _engine->vm.hostFrame();
      }

      return std::move(sink);
    }
   );

    #ifdef _JAFFAR_PLAY
     _engine->_enableVideoLoad = true;
     _engine->_enableResourceLoad = true;
    #else
     _engine->_enableVideoLoad = false;
     _engine->_enableResourceLoad = false;
    #endif

//  size_t stateSize = _engine->getGameStateSize();
//  printf("State Size: %lu\n", stateSize); fflush(stdout);
//  exit(0);

 }

 void loadStateFile(const std::string& stateFilePath) override
 {
  _engine->_enableVideoLoad = true;
  _engine->_enableResourceLoad = true;
  std::string stateData;
  if (loadStringFromFile(stateData, stateFilePath.c_str()) == false) EXIT_WITH_ERROR("Could not find/read state file: '%s'\n", stateFilePath.c_str());

  _engine->_enableVideoLoad = true;
  _engine->_enableResourceLoad = true;

  uint8_t buf[_STATE_DATA_SIZE_PLAY];
  memcpy(buf, (uint8_t*)stateData.data(), _STATE_DATA_SIZE_PLAY);
  _engine->loadGameState(buf);

  _engine->lastInput0 = 0;
  _engine->lastInput1 = 0;
  _engine->lastInput2 = 0;
  _engine->lastInput3 = 0;
  _engine->buttonPressCount = 0;
  _engine->diffInputCount = 0;
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  _engine->_enableVideoLoad = true;
  _engine->_enableResourceLoad = true;
  std::string stateData;
  stateData.resize(_STATE_DATA_SIZE_PLAY);
  _engine->saveGameState((uint8_t*)stateData.data());
  saveStringToFile(stateData, stateFilePath.c_str());
 }

 void serializeState(uint8_t* state) const override
 {
  _engine->saveGameState(state);
 }

 void deserializeState(const uint8_t* state) override
 {
   #ifdef _JAFFAR_PLAY
    uint8_t buf[_STATE_DATA_SIZE_PLAY];
    memcpy(buf, state, _STATE_DATA_SIZE_PLAY);
   #else
    uint8_t buf[_STATE_DATA_SIZE_TRAIN];
    memcpy(buf, state, _STATE_DATA_SIZE_TRAIN);
   #endif

  _engine->loadGameState(buf);
 }


 // Controller input bits
 // 0 - Up / 1
 // 1 - Down / 2
 // 2 - Left / 4
 // 3 - Right / 8
 // 4 - B / 16

 // Move Format:
 // ---BRLDU

 static inline INPUT_TYPE moveStringToCode(const std::string& move)
 {
  INPUT_TYPE moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'U': moveCode |= 0b00000001; break;
    case 'D': moveCode |= 0b00000010; break;
    case 'L': moveCode |= 0b00000100; break;
    case 'R': moveCode |= 0b00001000; break;
    case 'B': moveCode |= 0b00010000; break;
    case '.': break;
    default: EXIT_WITH_ERROR("Move provided cannot be parsed: '%s', unrecognized character: '%c'\n", move.c_str(), move[i]);
  }

  return moveCode;
 }

 static inline std::string moveCodeToString(const INPUT_TYPE move)
 {
  std::string moveString;

  if (move & 0b00000001) moveString += 'U'; else moveString += '.';
  if (move & 0b00000010) moveString += 'D'; else moveString += '.';
  if (move & 0b00000100) moveString += 'L'; else moveString += '.';
  if (move & 0b00001000) moveString += 'R'; else moveString += '.';
  if (move & 0b00010000) moveString += 'B'; else moveString += '.';

  return moveString;
 }


 void advanceState(const std::string& move)
 {
  advanceState(moveStringToCode(move));
 }

 void advanceState(const INPUT_TYPE move) override
 {
  _engine->sys->input.dirMask  = 0;
  _engine->sys->input.button = false;

  if (move & 0b00000001) _engine->sys->input.dirMask |= PlayerInput::DIR_UP;
  if (move & 0b00000010) _engine->sys->input.dirMask |= PlayerInput::DIR_DOWN;
  if (move & 0b00000100) _engine->sys->input.dirMask |= PlayerInput::DIR_LEFT;
  if (move & 0b00001000) _engine->sys->input.dirMask |= PlayerInput::DIR_RIGHT;
  if (move & 0b00010000) _engine->sys->input.button = true;

  gameContext = gameContext.resume();
 }

};
