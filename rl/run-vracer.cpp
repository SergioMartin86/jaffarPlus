#include "_environment/environment.hpp"
#include "korali.hpp"
#include "argparse.hpp"
#include "utils.hpp"
#include <string>

int main(int argc, char *argv[])
{
  ////// Initializing Game

  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-rl", "1.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Parsing config file
  std::string configFile = program.get<std::string>("configFile");
  std::string configFileString;
  auto statusConfig = loadStringFromFile(configFileString, configFile.c_str());
  if (statusConfig == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  nlohmann::json config;
  try { config = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // Checking whether it contains the rules field
  if (isDefined(config, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rules' key.\n");

  // Checking whether it contains the emulator configuration field
  if (isDefined(config, "Emulator Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Emulator Configuration' key.\n");

  // Checking whether it contains the Game configuration field
  if (isDefined(config, "Game Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Configuration' key.\n");

  // Initializing emulator
  emuInstance = new EmuInstance(config["Emulator Configuration"]);
  std::string stateFile = config["Emulator Configuration"]["State File"].get<std::string>();

  // Initializing game state
  gameInstance = new GameInstance(emuInstance, config["Game Configuration"]);
  gameInstance->parseRules(config["Rules"]);

  // Saving initial state
  gameInstance->loadStateFile(stateFile);
  gameInstance->updateDerivedValues();
  gameInstance->popState(initialGameState);

  // Initializing Jaffar trainer
  _train = new Train(config);

  ////// Initializing Experiment

  auto e = korali::Experiment();
  std::string optimizer = "Adam";
  float learningRate = 1e-6f;

  ////// Checking if existing results are there and continuing them

  std::string _resultDir = "_korali_result";
  //auto found = e.loadState(_resultDir + std::string("/latest"));
  //if (found == true) printf("Continuing execution from previous run...\n");

  ////// Defining problem configuration

  e["Problem"]["Type"] = "Reinforcement Learning / Continuous";
  e["Problem"]["Environment Function"] = &runEnvironment;

  //// Setting state variables

  size_t vIdx = 0;
  e["Variables"][vIdx]["Name"] = "Marble Pos X"; vIdx++;
  e["Variables"][vIdx]["Name"] = "Marble Pos Y"; vIdx++;
  e["Variables"][vIdx]["Name"] = "Marble Pos Z"; vIdx++;
  e["Variables"][vIdx]["Name"] = "Marble Vel X"; vIdx++;
  e["Variables"][vIdx]["Name"] = "Marble Vel Y"; vIdx++;

  //// Setting action variables

  e["Variables"][vIdx]["Name"] = "Up Button";
  e["Variables"][vIdx]["Type"] = "Action";
  e["Variables"][vIdx]["Initial Exploration Noise"] = 0.447f;
  e["Variables"][vIdx]["Lower Bound"] = 0.0;
  e["Variables"][vIdx]["Upper Bound"] = 1.0;
  vIdx++;

  e["Variables"][vIdx]["Name"] = "Down Button";
  e["Variables"][vIdx]["Type"] = "Action";
  e["Variables"][vIdx]["Initial Exploration Noise"] = 0.447f;
  e["Variables"][vIdx]["Lower Bound"] = 0.0;
  e["Variables"][vIdx]["Upper Bound"] = 1.0;
  vIdx++;

  e["Variables"][vIdx]["Name"] = "Left Button";
  e["Variables"][vIdx]["Type"] = "Action";
  e["Variables"][vIdx]["Initial Exploration Noise"] = 0.447f;
  e["Variables"][vIdx]["Lower Bound"] = 0.0;
  e["Variables"][vIdx]["Upper Bound"] = 1.0;
  vIdx++;

  e["Variables"][vIdx]["Name"] = "Right Button";
  e["Variables"][vIdx]["Type"] = "Action";
  e["Variables"][vIdx]["Initial Exploration Noise"] = 0.447f;
  e["Variables"][vIdx]["Lower Bound"] = 0.0;
  e["Variables"][vIdx]["Upper Bound"] = 1.0;
  vIdx++;

  e["Variables"][vIdx]["Name"] = "A Button";
  e["Variables"][vIdx]["Type"] = "Action";
  e["Variables"][vIdx]["Initial Exploration Noise"] = 0.447f;
  e["Variables"][vIdx]["Lower Bound"] = 0.0;
  e["Variables"][vIdx]["Upper Bound"] = 1.0;
  vIdx++;

  /// Defining Agent Configuration

  e["Solver"]["Type"] = "Agent / Continuous / VRACER";
  e["Solver"]["Mode"] = "Training";
  e["Solver"]["Episodes Per Generation"] = 1;
  e["Solver"]["Experiences Between Policy Updates"] = 1;
  e["Solver"]["Learning Rate"] = learningRate;
  e["Solver"]["Discount Factor"] = 0.995;

  e["Solver"]["Policy"]["Distribution"] = "Normal";
  e["Solver"]["State Rescaling"]["Enabled"] = false;
  e["Solver"]["Reward"]["Rescaling"]["Enabled"] = false;

  /// Defining the configuration of replay memory

  e["Solver"]["Experience Replay"]["Start Size"] = 131072;
  e["Solver"]["Experience Replay"]["Maximum Size"] = 262144;
  e["Solver"]["Experience Replay"]["Off Policy"]["Cutoff Scale"] = 4.0;
  e["Solver"]["Experience Replay"]["Off Policy"]["Target"] = 0.1;
  e["Solver"]["Experience Replay"]["Off Policy"]["Annealing Rate"] = 0.0;
  e["Solver"]["Experience Replay"]["Off Policy"]["REFER Beta"] = 0.3;

  /// Configuring Mini Batch

  e["Solver"]["Mini Batch"]["Size"] = 128;

  /// Configuring the neural network and its hidden layers

  e["Solver"]["Neural Network"]["Engine"] = "OneDNN";
  e["Solver"]["Neural Network"]["Optimizer"] = optimizer;

  e["Solver"]["Neural Network"]["Hidden Layers"][0]["Type"] = "Layer/Linear";
  e["Solver"]["Neural Network"]["Hidden Layers"][0]["Output Channels"] = 128;

  e["Solver"]["Neural Network"]["Hidden Layers"][1]["Type"] = "Layer/Activation";
  e["Solver"]["Neural Network"]["Hidden Layers"][1]["Function"] = "Elementwise/Tanh";

  e["Solver"]["Neural Network"]["Hidden Layers"][2]["Type"] = "Layer/Linear";
  e["Solver"]["Neural Network"]["Hidden Layers"][2]["Output Channels"] = 128;

  e["Solver"]["Neural Network"]["Hidden Layers"][3]["Type"] = "Layer/Activation";
  e["Solver"]["Neural Network"]["Hidden Layers"][3]["Function"] = "Elementwise/Tanh";

  ////// Setting termination criteria
  
  //e["Solver"]["Termination Criteria"]["Max Experiences"] = 10e6;
  
  ////// Setting file output configuration

  e["Console Output"]["Verbosity"] = "Detailed";
  e["Solver"]["Experience Replay"]["Serialize"] = false;
  e["File Output"]["Enabled"] = true;
  e["File Output"]["Frequency"] = 5;
  e["File Output"]["Path"] = _resultDir;

  auto k = korali::Engine();
  k.run(e);
}
