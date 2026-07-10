/**
 * @file cpuTrace.cpp
 * @brief Per-instruction CPU trace of a solution replay window, for auditing glitch inputs.
 *
 * Replays a solution on the Prince of Persia runner and, for a given step window, dumps every executed
 * CPU instruction (PC, opcode, A, X, Y, SP) to a file. Used to audit what the U+D input family actually
 * executes (the game dispatches pad input through an action-pointer table; impossible pad combinations
 * index past its end and jump through a garbage pointer) so game-level tricks can be told apart from
 * emulator artifacts (open-bus execution, KIL/JAM revival, unofficial-opcode semantics).
 *
 * Requires a build with -D_QUICKERNES_ENABLE_TRACEBACK_SUPPORT (the trace callback is compiled out
 * otherwise; the tool then produces an empty trace and says so).
 *
 * Usage: pop-cpu-trace <configFile> <solutionFile> <traceStartStep> <traceEndStep> <outputFile>
 */

#include <argparse/argparse.hpp>
#include <cstdio>
#include <emulatorList.hpp>
#include <gameList.hpp>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/string.hpp>
#include <nesInstance.hpp>
#include <runner.hpp>

// The trace callback is a plain function pointer (no closure), so the output stream and the emulator
// handle are globals. Operand bytes are peeked at execution time so PRG bank switches cannot desync
// the dump from what actually ran.
static FILE*       _traceFile = nullptr;
static emulator_t* _emu       = nullptr;

// Callback layout (see cpuPaged.cpp): scratch = { a, x, y, sp, pc, status, opcode }
static void traceCb(unsigned int* d)
{
  const uint16_t pc = (uint16_t)d[4];
  fprintf(_traceFile, "%04X %02X %02X %02X a=%02X x=%02X y=%02X sp=%02X\n", pc, d[6], _emu->peek_code(pc + 1), _emu->peek_code(pc + 2), d[0], d[1], d[2], d[3]);
}

int main(int argc, char* argv[])
{
  argparse::ArgumentParser program("pop-cpu-trace", "1.0");
  program.add_argument("configFile").help("path to the Jaffar configuration script (.jaffar) file.").required();
  program.add_argument("solutionFile").help("path to the solution sequence file (.sol) to replay.").required();
  program.add_argument("traceStart").help("first step (0-based input index) to trace.").required();
  program.add_argument("traceEnd").help("last step (inclusive) to trace.").required();
  program.add_argument("outputFile").help("path to write the instruction trace to.").required();

  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err)
  {
    JAFFAR_THROW_LOGIC("%s\n%s", err.what(), program.help().str().c_str());
  }

  const auto   configFile   = program.get<std::string>("configFile");
  const auto   solutionFile = program.get<std::string>("solutionFile");
  const size_t traceStart   = std::stoul(program.get<std::string>("traceStart"));
  const size_t traceEnd     = std::stoul(program.get<std::string>("traceEnd"));
  const auto   outputFile   = program.get<std::string>("outputFile");

  // Loading configuration
  std::string configFileString;
  if (jaffarCommon::file::loadStringFromFile(configFileString, configFile) == false) JAFFAR_THROW_LOGIC("[ERROR] Could not read Jaffar config file: %s\n", configFile.c_str());
  const auto config = nlohmann::json::parse(configFileString);

  // Loading solution
  std::string solutionString;
  if (jaffarCommon::file::loadStringFromFile(solutionString, solutionFile) == false) JAFFAR_THROW_LOGIC("[ERROR] Could not read solution file: %s\n", solutionFile.c_str());
  const auto solution = jaffarCommon::string::split(solutionString, '\n');

  // Creating the runner
  auto r = jaffarPlus::Runner::getRunner(jaffarCommon::json::getObject(config, "Emulator Configuration"), jaffarCommon::json::getObject(config, "Game Configuration"),
                                         jaffarCommon::json::getObject(config, "Runner Configuration"));

  // Initializing it (boots the emulator, loads the ROM and plays the configured initial sequence)
  r->initialize();

  // Reaching the internal quickerNES core to install the per-instruction hook
  auto* wrapper = dynamic_cast<jaffarPlus::emulator::QuickerNES*>(r->getGame()->getEmulator());
  if (wrapper == nullptr) JAFFAR_THROW_LOGIC("[ERROR] This tool requires the QuickerNES emulator.\n");
  auto* emu = (emulator_t*)wrapper->getInternalEmulatorPointer();
  _emu      = emu;

  _traceFile = fopen(outputFile.c_str(), "w");
  if (_traceFile == nullptr) JAFFAR_THROW_LOGIC("[ERROR] Could not open output file: %s\n", outputFile.c_str());

#ifndef _QUICKERNES_ENABLE_TRACEBACK_SUPPORT
  jaffarCommon::logger::log("[!] This build lacks _QUICKERNES_ENABLE_TRACEBACK_SUPPORT -- the trace will be EMPTY.\n");
#endif

  // Replaying: register each input and advance, tracing inside the window. A step marker line is
  // emitted per traced step so instructions can be attributed to their driving input.
  for (size_t i = 0; i < solution.size(); i++)
  {
    const auto& inputString = solution[i];
    if (inputString.empty()) continue;

    if (i == traceStart) emu->set_tracecb(traceCb);
    if (i >= traceStart && i <= traceEnd) fprintf(_traceFile, "=== step %lu input %s ===\n", i, inputString.c_str());

    r->advanceState(r->registerInput(inputString));

    if (i == traceEnd)
    {
      emu->set_tracecb(nullptr);
      break;
    }
  }

  fclose(_traceFile);
  jaffarCommon::logger::log("[+] Trace written to %s (steps %lu..%lu)\n", outputFile.c_str(), traceStart, traceEnd);

  // Also dump the CPU-visible code space (0x8000-0xFFFF, as banked at the end of the window) so the
  // traced routines can be disassembled offline.
  const auto codeFile = outputFile + ".code";
  FILE*      cf       = fopen(codeFile.c_str(), "wb");
  if (cf == nullptr) JAFFAR_THROW_LOGIC("[ERROR] Could not open code dump file: %s\n", codeFile.c_str());
  for (uint32_t addr = 0x8000; addr <= 0xFFFF; addr++)
  {
    const uint8_t b = emu->peek_code((uint16_t)addr);
    fwrite(&b, 1, 1, cf);
  }
  fclose(cf);
  jaffarCommon::logger::log("[+] Code dump (0x8000-0xFFFF) written to %s\n", codeFile.c_str());
  return 0;
}
