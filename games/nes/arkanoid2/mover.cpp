#include <chrono>
#include <argparse/argparse.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/string.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <gameList.hpp>
#include <emulatorList.hpp>
#include "emulator.hpp"
#include "game.hpp"
#include "runner.hpp"
#include <random>
#include <math.h> 

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile").help("path to the Jaffar configuration script (.jaffar) file to run.").required();

  program.add_argument("solutionFile").help("path to the solution sequence file (.sol) to reproduce.").required();

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

  // Parsin solution file
  const std::string solutionFile = program.get<std::string>("solutionFile");

  // If config file defined, read it now
  std::string configFileString;
  if (jaffarCommon::file::loadStringFromFile(configFileString, configFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from Jaffar config file: %s\n", configFile.c_str());

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

  // Getting component configurations
  auto emulatorConfig = jaffarCommon::json::getObject(config, "Emulator Configuration");
  auto gameConfig     = jaffarCommon::json::getObject(config, "Game Configuration");
  auto runnerConfig   = jaffarCommon::json::getObject(config, "Runner Configuration");

  // Creating runner from the configuration
  auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);

  // Initializing runner
  r->initialize();

  // If sequence file defined, load it and play it
  std::string solutionFileString;
  if (jaffarCommon::file::loadStringFromFile(solutionFileString, solutionFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from solution sequence file: %s\n", solutionFile.c_str());

  // Getting input sequence
  const auto solutionSequence = jaffarCommon::string::split(solutionFileString, '\0');

  // Getting sequence length
  const size_t sequenceLength = solutionSequence.size();

  // Getting game pointer
  auto g = dynamic_cast<jaffarPlus::games::nes::Arkanoid2 *>(r->getGame());
  auto e = dynamic_cast<jaffarPlus::emulator::QuickerNES *>(r->getGame()->getEmulator());

  // Getting initial score
  // auto initialScore = arkState->score;
  // fprintf(stderr, "Initial Score:            %u\n", initialScore);

  // Getting state size
  jaffarCommon::serializer::Contiguous sizeEvaluator;
  g->serializeState(sizeEvaluator);
  size_t stateSize = sizeEvaluator.getOutputSize();

  // Allocating space for the state
  uint8_t* stateData = (uint8_t*) malloc (stateSize);

  // Saving initial state
  jaffarCommon::serializer::Contiguous s(stateData);
  g->serializeState(s);

  // Storing score at each step
  std::vector<uint32_t> _sequenceScores;
  std::vector<jaffarPlus::InputSet::inputIndex_t> _sequenceIdxs;
  
  // Running sequence initially
  for (size_t curStep = 0; curStep < sequenceLength; curStep++)
  {
    _sequenceScores.push_back(g->_score);
    auto inputIdx = e->registerInput(solutionSequence[curStep]);
    _sequenceIdxs.push_back(inputIdx);
    g->advanceState(inputIdx);
  }

  // Printing original final score
  auto originalFinalScore = g->_score;
  fprintf(stderr, "Final Score (Original):   %u\n", originalFinalScore);
  fprintf(stderr, "Brain Boss Health (Original):   %u\n", *g->_brainBossDamage);

  // Loading initial state
  jaffarCommon::deserializer::Contiguous d(stateData);
  g->deserializeState(d);

// Randomizers  
  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(16, 160);

  // Target paddle pos
  int targetPaddlePos = 90;
  int curDirection = 1;
  int totalHitsRemaining = g->_remainingHits;

  // Getting new inputs
  for (size_t curStep = 0; curStep < sequenceLength; curStep++) 
  {
    fprintf(stderr, "Step %lu / %lu", curStep, sequenceLength);

    // Step final input
    auto stepInputString = solutionSequence[curStep];
    auto stepInputIdx = _sequenceIdxs[curStep];

    // If it's not a dummy input, don't consider it
    if (stepInputString == "|..|........|.......|    1,F|")
    {

      // Saving state at this point
      jaffarCommon::serializer::Contiguous s(stateData);
      g->serializeState(s);

      // Candidate input
  
      ///// Randomized effect
      // targetPaddlePos = (uint8_t) distrib(gen);

      ///// Stay still effect
      targetPaddlePos = *g->_paddlePosX;

      ///// Follow one each frame effect
      // targetPaddlePos = arkState->ball[0].pos.x;
      // if (arkState->ball[1].exists && curStep % 3 == 1) targetPaddlePos = arkState->ball[1].pos.x;
      // if (arkState->ball[2].exists && curStep % 3 == 2) targetPaddlePos = arkState->ball[2].pos.x;

      if (*g->_ballCount < 8)
      {
        if (curStep % *g->_ballCount == 0) targetPaddlePos = *g->_ball1PosX - 12;
        if (curStep % *g->_ballCount == 1) targetPaddlePos = *g->_ball2PosX - 12;
        if (curStep % *g->_ballCount == 2) targetPaddlePos = *g->_ball3PosX - 12;
        if (curStep % *g->_ballCount == 3) targetPaddlePos = *g->_ball4PosX - 12;
        if (curStep % *g->_ballCount == 4) targetPaddlePos = *g->_ball5PosX - 12;
        if (curStep % *g->_ballCount == 5) targetPaddlePos = *g->_ball6PosX - 12;
        if (curStep % *g->_ballCount == 6) targetPaddlePos = *g->_ball7PosX - 12;
        if (curStep % *g->_ballCount == 7) targetPaddlePos = *g->_ball8PosX - 12;
      }

      ///// Follow the lowest one effect
      // targetPaddlePos = arkState->ball[0].pos.x;
      // if (arkState->ball[1].pos.y > arkState->ball[0].pos.y && arkState->ball[1].pos.y > arkState->ball[2].pos.y) targetPaddlePos = arkState->ball[1].pos.x;
      // if (arkState->ball[2].pos.y > arkState->ball[0].pos.y && arkState->ball[2].pos.y > arkState->ball[1].pos.y) targetPaddlePos = arkState->ball[2].pos.x;

      ///// Advancing effect
      // targetPaddlePos = targetPaddlePos + 1;
      // targetPaddlePos = targetPaddlePos - 1;
      // if (targetPaddlePos > 160) targetPaddlePos = 16;
      // if (targetPaddlePos < 16) targetPaddlePos = 160;
    
      ///// Expanding effect
      // int targetExpansion = curStep % 75;
      // if ((curStep / 75) % 2 == 1) targetExpansion = 75 - targetExpansion;
      // targetPaddlePos = curStep % 2 == 0 ? 90 + targetExpansion : 90 - targetExpansion;

      ///// Follow Ball 1 if closer to the bottom
      // if (arkState->ball[0].pos.y > 135) targetPaddlePos = arkState->ball[0].pos.x;
      // else targetPaddlePos = 90;

      ////// Advance with each broken block
      // targetPaddlePos = (uint8_t)(16.0f + 144.0f * (((float)arkState->totalBlocks - (float)arkState->currentBlocks) / (float)arkState->totalBlocks));

      ////// Retreat with each broken block
      // targetPaddlePos = (uint8_t)(160.0f - 144.0f * (((float)arkState->totalBlocks - (float)arkState->currentBlocks) / (float)arkState->totalBlocks));

      ////// Center with each broken block
      // if (curStep % 2 == 0) targetPaddlePos = (uint8_t)(160.0f - 72.0f * (((float)arkState->totalBlocks - (float)arkState->currentBlocks) / (float)arkState->totalBlocks));
      // if (curStep % 2 == 1) targetPaddlePos = (uint8_t)(16.0f + 72.0f * (((float)arkState->totalBlocks - (float)arkState->currentBlocks) / (float)arkState->totalBlocks));

      ////// De-Center with each broken block
      // if (curStep % 2 == 0) targetPaddlePos = (uint8_t)(90.0f + 72.0f * (((float)arkState->totalBlocks - (float)arkState->currentBlocks) / (float)arkState->totalBlocks));
      // if (curStep % 2 == 1) targetPaddlePos = (uint8_t)(90.0f - 72.0f * (((float)arkState->totalBlocks - (float)arkState->currentBlocks) / (float)arkState->totalBlocks));

      // ///// Bounce back and forth
      // targetPaddlePos += 20 * curDirection;
      // if (targetPaddlePos > 160) { curDirection = -1; targetPaddlePos = 155; } 
      // if (targetPaddlePos < 16) { curDirection = 1; targetPaddlePos = 24; } 

      ////// Advance with each hit
      // targetPaddlePos = (uint8_t)(16.0f + 144.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));

      ////// Retreat with each hit
      // targetPaddlePos = (uint8_t)(160.0f - 144.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));

      ////// Advance with each hit and come back
      // targetPaddlePos = (uint8_t)(16.0f + (int)(144.0f * 8.0 * (((float)totalHitsRemaining - (float)g->_remainingHits)  / (float)totalHitsRemaining)) % 144);

      ////// Retreat with each hit and come back
      // targetPaddlePos = (uint8_t)(160.0f - (int)(144.0f * 8.0 * (((float)totalHitsRemaining - (float)g->_remainingHits)  / (float)totalHitsRemaining)) % 144);

      // ////// Center with each hit block
      // if (curStep % 2 == 0) targetPaddlePos = (uint8_t)(160.0f - 72.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));
      // if (curStep % 2 == 1) targetPaddlePos = (uint8_t)(16.0f + 72.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));

      ////// Center with each hit block x 4
      // if (curStep % 4 == 0) targetPaddlePos = (uint8_t)(30.0f   + 60.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));
      // if (curStep % 4 == 1) targetPaddlePos = (uint8_t)(60.0f   + 30.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));
      // if (curStep % 4 == 2) targetPaddlePos = (uint8_t)(120.0f  - 30.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));
      // if (curStep % 4 == 3) targetPaddlePos = (uint8_t)(150.0f  - 60.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));

      ////// De-Center with each hit block x 4
      // if (curStep % 4 == 0) targetPaddlePos = (uint8_t)(90.0f  + 60.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));
      // if (curStep % 4 == 1) targetPaddlePos = (uint8_t)(90.0f  + 30.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));
      // if (curStep % 4 == 2) targetPaddlePos = (uint8_t)(90.0f  - 30.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));
      // if (curStep % 4 == 3) targetPaddlePos = (uint8_t)(90.0f  - 60.0f * (((float)totalHitsRemaining - (float)g->_remainingHits) / (float)totalHitsRemaining));

      ///// Follow the average of the 3 balls
      // targetPaddlePos = arkState->ball[0].pos.x;
      // if (arkState->ball[1].exists && arkState->ball[2].exists) targetPaddlePos = (arkState->ball[0].pos.x + arkState->ball[1].pos.x + arkState->ball[2].pos.x) / 3;

      ////// Sinus effect
      // targetPaddlePos = (uint8_t)(90.0 + 72.0 * std::sin((float)curStep * 4.0f * 3.14f / 180.0f));

      // Normalizing inputs
      uint8_t arkInputValue = (uint8_t)std::max(std::min(142, (int)targetPaddlePos - 12), 1);
      auto candidateInputString = g->paddlePositionStringLUT[arkInputValue];
      auto candidateInputIdx = g->paddlePositionIndexLUT[arkInputValue];

      // // Running candidate input
      g->advanceState(candidateInputIdx);

      // Testing candidate input
      bool success = true;

      for (size_t subStep = curStep+1; subStep < sequenceLength; subStep++)
      {
        // If current score doesn't coincide, fail earlier
        if (g->_score != _sequenceScores[subStep] ) { success = false; break; }
        auto subStepInputIdx = _sequenceIdxs[subStep];
        g->advanceState(subStepInputIdx);
      }

      if (success == true && *g->_brainBossDamage == 8)
      {
        // Replacing step input for the candidate one
        stepInputString = candidateInputString;
        stepInputIdx = candidateInputIdx;
        fprintf(stderr, " Changed ");
      }

      // Re-loading step state
      jaffarCommon::deserializer::Contiguous d(stateData);
      g->deserializeState(d);
    }

    fprintf(stderr, "\n");

    // Advancing emulator with the step input
    printf("%s\n", stepInputString.c_str());
    g->advanceState(stepInputIdx);
  }
}