/**
 * @file coroute.cpp
 * @brief Interactive co-routing helper for hand-routing an Ice Climber climb one segment at a time.
 *
 * The whole-level automated router (router.cpp --climb) stalls on mtn6's intricate multi-screen climb.
 * This tool lets a human drive: from a checkpoint (in-process runner state -- the file-based emulator
 * "Initial State File Path" is broken for this game's packed serializer), apply a short input segment,
 * print the resulting player/cloud/brick state, and save a new checkpoint. Accumulate the accepted
 * segments into a route file, then screenshot/verify it with jaffar-player. Input tokens: a name +
 * count, space-separated, e.g. "L12 A1 .20 LA3" = 12 left, 1 jump, 20 idle, 3 left+jump. Names:
 * . (idle) A (jump) L (left) R (right) LA (left+jump) RA (right+jump).
 */

#include "emulator.hpp"
#include "game.hpp"
#include "runner.hpp"
#include <argparse/argparse.hpp>
#include <emulatorList.hpp>
#include <gameList.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <cstdio>
#include <string>
#include <vector>

using namespace jaffarPlus;
using idx_t = InputSet::inputIndex_t;

int main(int argc, char* argv[])
{
  setvbuf(stdout, NULL, _IONBF, 0);
  argparse::ArgumentParser program("iceClimber-coroute");
  program.add_argument("configFile").help("Config (Initial Sequence reaches the prior mountain's bonus-end; used with --start).").required();
  program.add_argument("--start").help("Fresh: replay init seq, then advance to the injected mountain's climb bottom.").default_value(false).implicit_value(true);
  program.add_argument("--load").help("Load an in-process checkpoint written by --save (skips init).").default_value(std::string(""));
  program.add_argument("--inputs").help("Segment to apply, e.g. \"L12 A1 .20\".").default_value(std::string(""));
  program.add_argument("--save").help("Write the resulting checkpoint here.").default_value(std::string(""));
  program.add_argument("--route").help("APPEND the applied per-frame inputs here (the accumulating route).").default_value(std::string(""));
  try { program.parse_args(argc, argv); }
  catch (const std::exception& e) { fprintf(stderr, "%s\n", e.what()); return 1; }

  const bool        start   = program.get<bool>("--start");
  const std::string loadF   = program.get<std::string>("--load");
  const std::string inputs  = program.get<std::string>("--inputs");
  const std::string saveF   = program.get<std::string>("--save");
  const std::string routeF  = program.get<std::string>("--route");

  std::string configStr;
  if (jaffarCommon::file::loadStringFromFile(configStr, program.get<std::string>("configFile")) == false) { fprintf(stderr, "cannot read config\n"); return 1; }
  auto config = nlohmann::json::parse(configStr);
  auto emulatorConfig = jaffarCommon::json::getObject(config, "Emulator Configuration");
  auto gameConfig     = jaffarCommon::json::getObject(config, "Game Configuration");
  auto runnerConfig   = jaffarCommon::json::getObject(config, "Runner Configuration");
  runnerConfig["Frameskip"]["Rate"] = 0;
  if (loadF.empty() == false) emulatorConfig["Initial Sequence File Path"] = ""; // loading a checkpoint: no replay

  auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);
  r->initialize();
  auto* lram = (uint8_t*)r->getGame()->getEmulator()->getProperty("LRAM").pointer;
  const size_t stateSize = r->getStateSize();
  const idx_t NOOP = r->getInputIndex("|..|........|");

  std::string routeAdd; // per-frame input strings applied this run

  if (loadF.empty() == false)
  {
    std::string b; if (jaffarCommon::file::loadStringFromFile(b, loadF) == false) { fprintf(stderr, "cannot read state %s\n", loadF.c_str()); return 1; }
    jaffarCommon::deserializer::Contiguous d((void*)b.data(), b.size()); r->deserializeState(d);
  }
  else if (start)
  {
    // Advance through the prior bonus-end + level transition + load until settled on the ground at the
    // climb bottom (posY>=150). ICE_INJECT_LEVEL (env) pins $59 through the transition.
    for (int g = 0; g < 400 && (lram[0x00E0] != 0 || lram[0x66] < 150); g++) { r->advanceState(NOOP); routeAdd += "|..|........|\n"; }
  }

  // Apply the input segment.
  auto nameToStr = [](const std::string& n) -> std::string {
    if (n == ".")  return "|..|........|";
    if (n == "A")  return "|..|.......A|";
    if (n == "L")  return "|..|..L.....|";
    if (n == "R")  return "|..|...R....|";
    if (n == "LA") return "|..|..L....A|";
    if (n == "RA") return "|..|...R...A|";
    return "";
  };
  std::string tok; std::vector<std::string> toks;
  for (char c : inputs + " ") { if (c == ' ') { if (!tok.empty()) toks.push_back(tok); tok.clear(); } else tok += c; }
  for (auto& t : toks)
  {
    size_t p = 0; while (p < t.size() && (t[p] < '0' || t[p] > '9')) p++;
    std::string name = t.substr(0, p); int cnt = (p < t.size()) ? std::stoi(t.substr(p)) : 1;
    std::string is = nameToStr(name);
    if (is.empty()) { fprintf(stderr, "bad token '%s'\n", t.c_str()); return 1; }
    idx_t idx = r->getInputIndex(is);
    for (int k = 0; k < cnt; k++) { r->advanceState(idx); routeAdd += is + "\n"; }
  }

  // Report the resulting state.
  printf("posX=%d posY=%d onGround=%d E0=%d scroll=%d D7=%d bricksBroken=%d bird@X=%d grab=%d\n",
         lram[0x64], lram[0x66], lram[0x00E0] == 0, lram[0x00E0], lram[0x13], lram[0x00D7], lram[0x0364], lram[0x0243], lram[0x004D]);
  printf("clouds (slot: screenY, X, vel):");
  for (int i = 0; i < 4; i++) if (lram[0x0786 + i]) printf(" [%d: y=%d x=%d v=%d]", i, lram[0x06BE + i] + 32, lram[0x0682 + i], (int8_t)lram[0x07B7 + i]);
  printf("\n");

  if (saveF.empty() == false)
  {
    std::string out; out.resize(stateSize); jaffarCommon::serializer::Contiguous s(out.data(), out.size()); r->serializeState(s);
    jaffarCommon::file::saveStringToFile(out, saveF);
  }
  if (routeF.empty() == false)
  {
    std::string existing; jaffarCommon::file::loadStringFromFile(existing, routeF);
    jaffarCommon::file::saveStringToFile(existing + routeAdd, routeF);
  }
  return 0;
}
