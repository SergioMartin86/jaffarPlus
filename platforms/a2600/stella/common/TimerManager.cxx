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

#include <cassert>
#include "TimerManager.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TimerManager::TimerManager()
  : nextId{no_timer + 1}
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TimerManager::~TimerManager()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TimerManager::TimerId TimerManager::addTimer(
    millisec msDelay,
    millisec msPeriod,
    const TFunction& func)
{

  return nextId++;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool TimerManager::clear(TimerId id)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TimerManager::clear()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::size_t TimerManager::size() const noexcept
{
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool TimerManager::empty() const noexcept
{
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TimerManager& TimerManager::global()
{
  static TimerManager singleton;
  return singleton;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// NOTE: if notify is true, returns with lock unlocked
bool TimerManager::destroy_impl(ScopedLock& lock, TimerMap::iterator i,
                                bool notify)
{

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TimerManager::Timer implementation
//

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TimerManager::Timer::Timer(Timer&& r) noexcept
  : id{r.id},
    next{r.next},
    period{r.period},
    handler{std::move(r.handler)},
    running{r.running}
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TimerManager::Timer::Timer(TimerId tid, Timestamp tnext, Duration tperiod,
                           const TFunction& func) noexcept
  : id{tid},
    next{tnext},
    period{tperiod},
    handler{func}
{
}
