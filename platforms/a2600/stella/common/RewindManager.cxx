//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2023 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include <cmath>

#include "OSystem.hxx"
#include "Console.hxx"
#include "Serializer.hxx"
#include "StateManager.hxx"
#include "TIA.hxx"
#include "EventHandler.hxx"

#include "RewindManager.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
RewindManager::RewindManager(OSystem& system, StateManager& statemgr)
  : myOSystem{system},
    myStateManager{statemgr}
{
  setup();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RewindManager::setup()
{
  myLastTimeMachineAdd = false;

  const string& prefix = myOSystem.settings().getBool("dev.settings") ? "dev." : "plr.";

  // TODO - Add proper bounds checking (define constexpr variables for this)
  //        Use those bounds in DeveloperDialog too
  mySize = std::min<uInt32>(
      myOSystem.settings().getInt(prefix + "tm.size"), MAX_BUF_SIZE);

  myUncompressed = std::min<uInt32>(
      myOSystem.settings().getInt(prefix + "tm.uncompressed"), MAX_BUF_SIZE);

  myInterval = INTERVAL_CYCLES[0];
  for(int i = 0; i < NUM_INTERVALS; ++i)
    if(INT_SETTINGS[i] == myOSystem.settings().getString(prefix + "tm.interval"))
      myInterval = INTERVAL_CYCLES[i];

  myHorizon = HORIZON_CYCLES[NUM_HORIZONS-1];
  for(int i = 0; i < NUM_HORIZONS; ++i)
    if(HOR_SETTINGS[i] == myOSystem.settings().getString(prefix + "tm.horizon"))
      myHorizon = HORIZON_CYCLES[i];

  // calc interval growth factor for compression
  // this factor defines the backward horizon
  constexpr double MAX_FACTOR = 1E8;
  double minFactor = 0, maxFactor = MAX_FACTOR;
  myFactor = 1;

  while(myUncompressed < mySize)
  {
    double interval = myInterval;
    double cycleSum = interval * (myUncompressed + 1);
    // calculate nextCycles factor
    myFactor = (minFactor + maxFactor) / 2;
    // horizon not reachable?
    if(myFactor == MAX_FACTOR)
      break;
    // sum up interval cycles (first state is not compressed)
    for(uInt32 i = myUncompressed + 1; i < mySize; ++i)
    {
      interval *= myFactor;
      cycleSum += interval;
    }
    const double diff = cycleSum - myHorizon;

    // exit loop if result is close enough
    if(std::abs(diff) < myHorizon * 1E-5)
      break;
    // define new boundary
    if(cycleSum < myHorizon)
      minFactor = myFactor;
    else
      maxFactor = myFactor;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool RewindManager::addState(string_view message, bool timeMachine)
{
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 RewindManager::rewindStates(uInt32 numStates)
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 RewindManager::unwindStates(uInt32 numStates)
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 RewindManager::windStates(uInt32 numStates, bool unwind)
{
  if(unwind)
    return unwindStates(numStates);
  else
    return rewindStates(numStates);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string RewindManager::saveAllStates()
{
    return "Error saving all states";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string RewindManager::loadAllStates()
{
 return "";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RewindManager::compressStates()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string RewindManager::loadState(Int64 startCycles, uInt32 numStates)
{

  return "";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string RewindManager::getUnitString(Int64 cycles)
{
  constexpr size_t NTSC_FREQ = 1193182; // ~76*262*60
  constexpr size_t PAL_FREQ  = 1182298; // ~76*312*50
  const size_t scanlines = std::max<size_t>(
      myOSystem.console().tia().scanlinesLastFrame(), 240);
  const bool isNTSC = scanlines <= 287;
  const size_t freq = isNTSC ? NTSC_FREQ : PAL_FREQ; // = cycles/second

  constexpr size_t NUM_UNITS = 5;
  const std::array<string, NUM_UNITS> UNIT_NAMES = {
    "cycle", "scanline", "frame", "second", "minute"
  };
  const std::array<uInt64, NUM_UNITS+1> UNIT_CYCLES = {
    1, 76, 76 * scanlines, freq, freq * 60, Int64{1} << 62
  };

  stringstream result;
  size_t i = 0;

  const uInt64 u_cycles = std::abs(cycles);

  for(i = 0; i < NUM_UNITS - 1; ++i)
  {
    // use the lower unit up to twice the nextCycles unit, except for an exact match of the nextCycles unit
    // TODO: does the latter make sense, e.g. for ROMs with changing scanlines?
    if(u_cycles == 0 || (u_cycles < UNIT_CYCLES[i + 1] * 2 && u_cycles % UNIT_CYCLES[i + 1] != 0))
      break;
  }
  result << (u_cycles / UNIT_CYCLES[i]) << " " << UNIT_NAMES[i];
  if(u_cycles / UNIT_CYCLES[i] != 1)
    result << "s";

  return result.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt64 RewindManager::getFirstCycles() const
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt64 RewindManager::getCurrentCycles() const
{
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt64 RewindManager::getLastCycles() const
{
  return  0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
IntArray RewindManager::cyclesList() const
{
  IntArray arr;

  return arr;
}
