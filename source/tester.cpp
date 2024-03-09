#include <chrono>
#include <argparse/argparse.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/deserializers/differential.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/serializers/differential.hpp>
#include <jaffarCommon/string.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "emulator.hpp"
#include "game.hpp"
#include "runner.hpp"

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  program.add_argument("--cycleType")
    .help("Specifies the emulation actions to be performed per each input. Possible values: 'Simple': performs only advance state, 'Rerecord': performs load/advance/save, and 'Full': performs load/advance/save/advance.")
    .default_value(std::string("Simple"));

  program.add_argument("solutionFile")
    .help("path to the solution sequence file (.sol) to reproduce.")
    .required();

  // Try to parse arguments
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
    JAFFAR_THROW_LOGIC("%s\n%s", err.what(), program.help().str().c_str());
  }

  // Parsing config file
  const std::string configFile = program.get<std::string>("configFile");
  std::string configFileString;
  if (jaffarCommon::file::loadStringFromFile(configFileString, configFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  // If sequence file defined, load it and play it
  const std::string solutionFile = program.get<std::string>("solutionFile");
  std::string solutionFileString;
  if (jaffarCommon::file::loadStringFromFile(solutionFileString, solutionFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from solution sequence file: %s\n%s \n", solutionFile.c_str(), program.help().str().c_str());

  // Getting reproduce flag
  const std::string cycleType = program.get<std::string>("--cycleType");

  // Parsing cycleType
  bool recognizedCycleType = false;
  if (cycleType == "Simple") recognizedCycleType = true;
  if (cycleType == "Rerecord") recognizedCycleType = true;
  if (cycleType == "Full") recognizedCycleType = true;
  if (recognizedCycleType == false) JAFFAR_THROW_LOGIC("[ERROR] Unrecognized cycle type '%s'");

  // Parsing configuration file
  nlohmann::json config;
  try
  {
    config = nlohmann::json::parse(configFileString);
  }
  catch (const std::exception &err)
  {
    JAFFAR_THROW_LOGIC("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what());
  }

  // Parsing state compression configuration
  const auto &stateCompressionJs = jaffarCommon::json::getObject(config, "State Compression");
  const auto useDifferentialCompression = jaffarCommon::json::getBoolean(stateCompressionJs, "Use Differential Compression");
  const auto maximumDifferences = jaffarCommon::json::getNumber<size_t>(stateCompressionJs, "Max Difference (bytes)");
  const auto useZlibCompression = jaffarCommon::json::getBoolean(stateCompressionJs, "Use Zlib Compression");

  // Getting initial state file from the configuration
  const auto initialStateFilePath = jaffarCommon::json::getString(config, "Initial State File Path");

  // Creating runner from the configuration
  auto r = jaffarPlus::Runner::getRunner(
    jaffarCommon::json::getObject(config, "Emulator Configuration"),
    jaffarCommon::json::getObject(config, "Game Configuration"),
    jaffarCommon::json::getObject(config, "Runner Configuration"));

  // Disabling rendering
  r->getGame()->getEmulator()->disableRendering();

  // Getting game state size
  const auto stateSize = r->getStateSize();

  // Getting input sequence
  const auto solutionSequence = jaffarCommon::string::split(solutionFileString, ' ');

  // Getting sequence length
  const auto sequenceLength = solutionSequence.size();

  // Getting differential state size
  size_t differentialStateSize = r->getDifferentialStateSize(maximumDifferences);

  // Printing information
  jaffarCommon::logger::log("[J+] Emulator Name:                   '%s'\n", r->getGame()->getEmulator()->getName().c_str());
  jaffarCommon::logger::log("[J+] Game Name:                       '%s'\n", r->getGame()->getName().c_str());
  jaffarCommon::logger::log("[J+] Config File:                     '%s'\n", configFile.c_str());
  jaffarCommon::logger::log("[J+] Solution File:                   '%s'\n", solutionFile.c_str());
  jaffarCommon::logger::log("[J+] Cycle Type:                      '%s'\n", cycleType.c_str());
  jaffarCommon::logger::log("[J+] State Size:                       %lu\n", stateSize);
  jaffarCommon::logger::log("[J+] Use Differential Compression:     %s\n", useDifferentialCompression ? "true" : "false");
  if (useDifferentialCompression == true)
  {
    jaffarCommon::logger::log("[J+]  + Using Zlib Compression:        %s\n", useZlibCompression ? "true" : "false");
    jaffarCommon::logger::log("[J+]  + Differential State Size:       %lu\n", differentialStateSize);
  }
  jaffarCommon::logger::log("[J+] Sequence Length:                  %lu\n", sequenceLength);
  jaffarCommon::logger::log("[J+] ********** Running Test **********\n");

  // Serializing initial state
  auto currentState = (uint8_t *)malloc(stateSize);
  jaffarCommon::serializer::Contiguous sc(currentState, stateSize);
  r->serializeState(sc);

  // Storage for diffential compression
  auto diffentialStateData = (uint8_t *)malloc(differentialStateSize);
  jaffarCommon::serializer::Differential sd(diffentialStateData, differentialStateSize, currentState, stateSize, useZlibCompression);
  r->serializeState(sd);
  size_t maxDifferentialStateSize = sd.getOutputSize();

  // Check whether to perform each action
  const bool doPreAdvance = cycleType == "Full";
  const bool doDeserialize = cycleType == "Rerecord" || cycleType == "Full";
  const bool doSerialize = cycleType == "Rerecord" || cycleType == "Full";

  // Actually running the sequence
  auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto &input : solutionSequence)
  {
    // Getting input index
    const auto inputIndex = r->getInputIndex(input);

    if (doPreAdvance == true) r->advanceState(inputIndex);

    if (doDeserialize == true)
    {
      if (useDifferentialCompression == false)
      {
        jaffarCommon::deserializer::Contiguous dc(currentState, stateSize);
        r->deserializeState(dc);
      }

      if (useDifferentialCompression == true)
      {
        jaffarCommon::deserializer::Differential dd(diffentialStateData, differentialStateSize, currentState, stateSize, useZlibCompression);
        r->deserializeState(dd);
      }
    }

    r->advanceState(inputIndex);

    if (doSerialize == true)
    {
      if (useDifferentialCompression == false)
      {
        jaffarCommon::serializer::Contiguous sc(currentState, stateSize);
        r->serializeState(sc);
      }

      if (useDifferentialCompression == true)
      {
        jaffarCommon::serializer::Differential sd(diffentialStateData, differentialStateSize, currentState, stateSize, useZlibCompression);
        r->serializeState(sd);
        maxDifferentialStateSize = std::max(maxDifferentialStateSize, sd.getOutputSize());
      }
    }
  }
  auto tf = std::chrono::high_resolution_clock::now();

  // Calculating running time
  auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
  double elapsedTimeSeconds = (double)dt * 1.0e-9;

  // Printing time information
  jaffarCommon::logger::log("[J+] Elapsed time:                  %3.3fs\n", (double)dt * 1.0e-9);
  jaffarCommon::logger::log("[J+] Performance:                   %.3f inputs / s\n", (double)sequenceLength / elapsedTimeSeconds);
  if (useDifferentialCompression == true)
  {
    jaffarCommon::logger::log("[J+] Max Differential State Size:   %lu\n", maxDifferentialStateSize);
  }

  // Printing Information
  jaffarCommon::logger::log("[J+] Runner Information:\n");
  r->printInfo();

  jaffarCommon::logger::log("[J+] Emulator Information:\n");
  r->getGame()->getEmulator()->printInfo();

  jaffarCommon::logger::log("[J+] Game Information:\n");
  r->getGame()->printInfo();
}
