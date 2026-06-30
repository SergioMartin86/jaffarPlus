/**
 * @file router.cpp
 * @brief Standalone bonus-stage ROUTER for NES Ice Climber.
 *
 * The mountain-top bonus stage is a vertical scrolling climb over MOVING cloud platforms, topped by a
 * condor to grab. It is fully deterministic (clouds at known positions move at fixed velocities; the
 * player's jumps are deterministic physics), so it is a ROUTING problem, not a reward-search problem
 * (a reward-priority beam cannot do it: it dedups on near-exact pixel state and drops the long,
 * gradient-less ride). This tool plans it directly: from the bonus entry (reached by the config's
 * Initial Sequence), it runs a macro-action search where the only decisions are HOW LONG TO RIDE the
 * current cloud and WHICH JUMP to take (steer none/left/right). Transitions are simulated by the real
 * emulator (exact physics, exact cloud motion), so landings are always correct. States are deduped on
 * the coarse routing key (scroll wraps + bucketed player pos + bucketed active-cloud phases), which
 * collapses the pixel space to the small cloud graph -- exhaustively solvable. Output: the per-frame
 * bonus input sequence (to append after the climb). Generalizes: the cloud model is read from RAM, so
 * the same router runs on any mountain's bonus layout.
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
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

using namespace jaffarPlus;
using idx_t = InputSet::inputIndex_t;

int main(int argc, char* argv[])
{
  setvbuf(stderr, NULL, _IONBF, 0); // unbuffered: progress shows even when piped/killed

  argparse::ArgumentParser program("jaffar-router");
  program.add_argument("configFile").help("Jaffar config (its Initial Sequence must reach the bonus entry).").required();
  program.add_argument("--output").help("Output .sol for the bonus input sequence.").default_value(std::string("bonus.sol"));
  program.add_argument("--maxNodes").help("Node-expansion cap.").default_value(std::string("60000"));
  program.add_argument("--maxWait").help("Max ride frames before a jump.").default_value(std::string("200"));
  program.add_argument("--maxAir").help("Max airborne frames per jump.").default_value(std::string("70"));
  program.add_argument("--posBucket").help("Dedup bucket size for player X/Y (smaller=finer).").default_value(std::string("2"));
  program.add_argument("--cloudBucket").help("Dedup bucket size for cloud X (smaller=finer).").default_value(std::string("4"));
  try { program.parse_args(argc, argv); }
  catch (const std::exception& e) { fprintf(stderr, "%s\n", e.what()); return 1; }

  const std::string configFile = program.get<std::string>("configFile");
  const std::string outputFile = program.get<std::string>("--output");
  const size_t MAXNODES = std::stoul(program.get<std::string>("--maxNodes"));
  const int    MAXWAIT  = std::stoi(program.get<std::string>("--maxWait"));
  const int    MAXAIR   = std::stoi(program.get<std::string>("--maxAir"));
  const int    PB       = std::stoi(program.get<std::string>("--posBucket"));
  const int    CB       = std::stoi(program.get<std::string>("--cloudBucket"));

  std::string configStr;
  if (jaffarCommon::file::loadStringFromFile(configStr, configFile) == false) { fprintf(stderr, "cannot read %s\n", configFile.c_str()); return 1; }
  auto config = nlohmann::json::parse(configStr);
  auto emulatorConfig = jaffarCommon::json::getObject(config, "Emulator Configuration");
  auto gameConfig     = jaffarCommon::json::getObject(config, "Game Configuration");
  auto runnerConfig   = jaffarCommon::json::getObject(config, "Runner Configuration");
  runnerConfig["Frameskip"]["Rate"] = 0; // frame-precise (input is read every frame)

  auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);
  r->initialize(); // replays the Initial Sequence -> bonus entry
  auto* lram = (uint8_t*)r->getGame()->getEmulator()->getProperty("LRAM").pointer;
  const size_t stateSize = r->getStateSize();

  // Inputs
  const idx_t NOOP = r->getInputIndex("|..|........|");
  const idx_t IA   = r->getInputIndex("|..|.......A|");
  const idx_t IL   = r->getInputIndex("|..|..L.....|");
  const idx_t IR   = r->getInputIndex("|..|...R....|");
  const idx_t ILA  = r->getInputIndex("|..|..L....A|");
  const idx_t IRA  = r->getInputIndex("|..|...R...A|");

  auto saveState = [&]() { std::string b; b.resize(stateSize); jaffarCommon::serializer::Contiguous s(b.data(), b.size()); r->serializeState(s); return b; };
  auto restoreState = [&](const std::string& b) { jaffarCommon::deserializer::Contiguous d((void*)b.data(), b.size()); r->deserializeState(d); };

  auto onGround = [&]() { return lram[0x00E0] == 0; };
  auto inBonus  = [&]() { return lram[0x00D7] == 21; };
  auto grabbed  = [&]() { return lram[0x004D] > 0 && lram[0x0243] != 112; };
  auto key = [&](int wraps) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "%d|%d|%d|%d|%d|", wraps, lram[0x64] / PB, lram[0x66] / PB, lram[0x13] / 8, lram[0xE0]);
    for (int i = 0; i < 4; i++)
      if (lram[0x0786 + i] && len > 0 && len < (int)sizeof(buf) - 16)
        len += snprintf(buf + len, sizeof(buf) - len, "%d:%d,", i, lram[0x0682 + i] / CB);
    return std::string(buf);
  };

  // Skip the bonus INTRO: the player is uncontrollable for the first ~35 frames (a jump press has no
  // effect). Advance NOOP until a test A-press actually launches a jump; collect those frames to
  // prepend to the final solution.
  std::vector<idx_t> introPath;
  { int g = 0;
    while (inBonus() && g++ < 90)
    {
      if (onGround())
      {
        std::string t = saveState(); int y0 = lram[0x66];
        r->advanceState(IA);
        bool jumped = (lram[0x66] < y0) || (lram[0xE0] != 0);
        restoreState(t);
        if (jumped) break;
      }
      r->advanceState(NOOP); introPath.push_back(NOOP);
    }
  }
  fprintf(stderr, "[router] entry (post-intro %zu frames): posX=%d posY=%d onGround=%d inBonus=%d scroll=%d\n",
          introPath.size(), lram[0x64], lram[0x66], (int)onGround(), (int)inBonus(), lram[0x13]);
  if (!inBonus()) { fprintf(stderr, "[router] not in bonus stage after init (D7=%d)\n", lram[0x00D7]); return 1; }

  struct Node { std::string state; int parent; std::vector<idx_t> macro; int frames; int wraps; uint8_t prevScroll; };
  std::vector<Node> pool;
  std::priority_queue<std::pair<int,int>, std::vector<std::pair<int,int>>, std::greater<>> pq; // (frames, idx)
  std::unordered_set<std::string> visited;

  // A*-style priority (min-first): frames spent MINUS a strong height bonus, so the search climbs
  // toward the top/grab instead of exhausting low states. heightScore rises monotonically with the
  // climb (each scroll-wrap = +240; lower scroll within a screen = higher).
  auto priorityOf = [&](int frames, int w, uint8_t ps, uint8_t posY) {
    // Weight COMMITTED climb (scroll-wraps + low scroll) far above transient jump height (posY): a
    // high jump must not look like real progress, or the A* chases jumps instead of scroll-downs.
    int heightScore = (w * 240 - (int)ps) * 4 + (144 - (int)posY);
    return frames - 6 * heightScore;
  };

  pool.push_back({saveState(), -1, {}, 0, 0, lram[0x13]});
  visited.insert(key(0));
  pq.push({priorityOf(0, 0, lram[0x13], lram[0x66]), 0});

  int solNode = -1; std::vector<idx_t> solMacro; long expansions = 0;

  while (!pq.empty() && pool.size() < MAXNODES)
  {
    int ni = pq.top().second; pq.pop();
    std::string baseState = pool[ni].state; int baseFrames = pool[ni].frames;
    int baseWraps = pool[ni].wraps; uint8_t baseScroll = pool[ni].prevScroll;
    restoreState(baseState);
    if (!onGround() || !inBonus()) continue;
    expansions++;
    if (expansions % 500 == 0) fprintf(stderr, "[router] expanded=%ld nodes=%zu frontier=%zu best-wraps=%d scroll=%d\n", expansions, pool.size(), pq.size(), baseWraps, baseScroll);

    auto tryAdd = [&](const std::vector<idx_t>& macro, int w, uint8_t ps) {
      std::string k = key(w);
      if (visited.count(k)) return;
      visited.insert(k);
      int fr = baseFrames + (int)macro.size();
      pool.push_back({saveState(), ni, macro, fr, w, ps});
      pq.push({priorityOf(fr, w, ps, lram[0x66]), (int)pool.size() - 1});
    };

    // GROUND MOVES: walk-left, walk-right, wait -- each advanced UNTIL the coarse key changes (so a
    // walk/wait actually crosses a position/phase bucket instead of deduping in place), or it leaves
    // the ground / the bonus ends. Lets the player reposition and ride moving clouds.
    std::string baseKey = key(baseWraps);
    for (idx_t g : {IL, IR, NOOP})
    {
      restoreState(baseState); int w = baseWraps; uint8_t ps = baseScroll;
      std::vector<idx_t> mp; bool solved = false, settled = false;
      for (int t = 0; t < 24; t++)
      {
        r->advanceState(g); mp.push_back(g);
        if ((int)lram[0x13] - (int)ps > 120) w++; ps = lram[0x13];
        if (grabbed()) { solved = true; break; }
        if (!inBonus() || !onGround()) break;
        if (key(w) != baseKey) { settled = true; break; }
      }
      if (solved) { solNode = ni; solMacro = mp; break; }
      if (settled && inBonus() && onGround()) tryAdd(mp, w, ps);
    }
    if (solNode >= 0) break;

    // JUMPS (macro to next landing): every LAUNCH (straight/left/right) x every held AIR-STEER
    // (none/left/right) = 9 variants. This covers e.g. "launch-left then RELEASE" (LA then noop, drift
    // on momentum), which real solutions use and a held-steer-only model misses.
    idx_t launches[3] = {IA, ILA, IRA};
    idx_t airs[3]     = {NOOP, IL, IR};
    for (int li = 0; li < 3 && solNode < 0; li++)
    for (int ai = 0; ai < 3 && solNode < 0; ai++)
    {
      restoreState(baseState); int w = baseWraps; uint8_t ps = baseScroll;
      idx_t first = launches[li], air = airs[ai];
      std::vector<idx_t> jp; jp.push_back(first);
      r->advanceState(first);
      if ((int)lram[0x13] - (int)ps > 120) w++; ps = lram[0x13];
      bool solved = false, landed = false;
      for (int t = 0; t < MAXAIR; t++)
      {
        if (grabbed()) { solved = true; break; }
        if (!inBonus()) break;
        if (onGround() && t > 0) { landed = true; break; }
        r->advanceState(air); jp.push_back(air);
        if ((int)lram[0x13] - (int)ps > 120) w++; ps = lram[0x13];
      }
      if (solved) { solNode = ni; solMacro = jp; break; }
      if (landed && inBonus()) tryAdd(jp, w, ps);
    }
    if (solNode >= 0) break;
  }

  if (solNode < 0) { fprintf(stderr, "[router] NO SOLUTION (expanded=%ld nodes=%zu)\n", expansions, pool.size()); return 2; }

  // Reconstruct: intro frames, then root..solNode macros, then solMacro
  std::vector<std::vector<idx_t>> chain;
  for (int cur = solNode; cur != -1; cur = pool[cur].parent) chain.push_back(pool[cur].macro);
  std::vector<idx_t> full = introPath;
  for (auto it = chain.rbegin(); it != chain.rend(); ++it) full.insert(full.end(), it->begin(), it->end());
  full.insert(full.end(), solMacro.begin(), solMacro.end());

  std::string out;
  for (auto i : full) out += r->getInputStringFromIndex(i) + "\n";
  jaffarCommon::file::saveStringToFile(out, outputFile);
  fprintf(stderr, "[router] SOLVED! bonus length=%zu frames, wrote %s (expanded=%ld nodes=%zu)\n", full.size(), outputFile.c_str(), expansions, pool.size());
  return 0;
}
