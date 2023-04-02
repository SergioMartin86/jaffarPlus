#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>

#include "engine.h"
#include "sys.h"

extern System *stub;

class EmuInstance : public EmuInstanceBase
{

 Engine* _engine;

 public:

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Game Files") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Files' key.\n");
  std::string gameFilesPath = config["Game Files"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  _enableRender = false;
  _engine = new Engine(stub, gameFilesPath.c_str(), stateFilePath.c_str());
  _engine->init();

  _engine->sys->context = boost::context::callcc(
     [this](boost::context::continuation && sink)
     {
       // Return once
       sink = sink.resume();

       // Store return context
       _engine->sys->context = std::move(sink);

       // Infinite game loop
       while(true)
       {
        _engine->vm.checkThreadRequests();
        _engine->vm.inp_updatePlayer();
        _engine->processInput();
        _engine->vm.hostFrame();
       }

       return std::move(sink);
     }
    );
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


 static inline INPUT_TYPE moveStringToCode(const std::string& move)
 {
  INPUT_TYPE moveCode = 0;


  return moveCode;
 }

 static inline std::string moveCodeToString(const INPUT_TYPE move)
 {
  std::string moveString = "|..|";


  moveString += "|";
  return moveString;
 }


 void advanceState(const std::string& move)
 {
  advanceState(moveStringToCode(move));
 }

 void advanceState(const INPUT_TYPE move) override
 {
  _engine->sys->context = _engine->sys->context.resume();
 }

};
