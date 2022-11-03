#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>

class EmuInstance : public EmuInstanceBase
{
 public:

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {

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
