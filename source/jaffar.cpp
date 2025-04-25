#include <argparse/argparse.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/string.hpp>
#include "driver.hpp"

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar", "1.0");

  program.add_argument("configFile").help("path to the Jaffar configuration script (.jaffar) file to run.").required();
  program.add_argument("--dryRun").help("Only check for configuration without initializing or running the engine").default_value(false).implicit_value(true);

  // Try to parse arguments
  try
    {
      program.parse_args(argc, argv);
    }
  catch (const std::runtime_error &err)
    {
      JAFFAR_THROW_LOGIC("%s\n%s", err.what(), program.help().str().c_str());
    }

  // Getting config file name
  const std::string configFile = program.get<std::string>("configFile");

  // Getting dry run option
  const bool isDryRun = program.get<bool>("--dryRun");

  // Reporting script file
  jaffarCommon::logger::log("[J+] Loading script file: '%s'\n", configFile.c_str());

  // Loading script file contents
  std::string configFileString;
  if (jaffarCommon::file::loadStringFromFile(configFileString, configFile) == false)
    JAFFAR_THROW_RUNTIME("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  // Parsing JSON from script file
  nlohmann::json config;
  try
    {
      config = nlohmann::json::parse(configFileString);
    }
  catch (const std::exception &err)
    {
      JAFFAR_THROW_LOGIC("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what());
    }

  // Creating driver to run the Jaffar engine
  auto d = jaffarPlus::Driver::getDriver(configFile, config);

  // Returning now if dry running
  if (isDryRun) 
  {
    jaffarCommon::logger::log("[J+] Finished dry run successfully.\n");
    return 0;
  }
  
  // Initializing driver
  d->initialize();

  // Running driver
  auto exitReason = d->run();

  // Printing exit reason
  std::string exitReasonString;
  if (exitReason == jaffarPlus::Driver::exitReason_t::winStateFound) exitReasonString = "Solution found.";
  if (exitReason == jaffarPlus::Driver::exitReason_t::outOfStates) exitReasonString = "Engine ran out of states.";
  if (exitReason == jaffarPlus::Driver::exitReason_t::maximumStepReached) exitReasonString = "Maximum step count reached.";

  // Getting current step
  auto finalStep = d->getCurrentStep();

  // Printing exit message
  jaffarCommon::logger::log("[J+] Step %lu - Exit Reason: %s\n", finalStep, exitReasonString.c_str());

  // Return exit reason
  return exitReason;
}
