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

#ifndef TIMER_MANAGER_HXX
#define TIMER_MANAGER_HXX

#include <algorithm>
#include <functional>
#include <chrono>
#include <unordered_map>
#include <set>
#include <cstdint>
#include <mutex>
#include <condition_variable>

#include "bspf.hxx"

class TimerManager
{
  public:
    // Each Timer is assigned a unique ID of type TimerId
    using TimerId = uInt64;

    // Function object we actually use
    using TFunction = std::function<void()>;

    // Values that are a large-range millisecond count
    using millisec = uInt64;

    // Constructor does not start worker until there is a Timer.
    explicit TimerManager();

    ~TimerManager();

    /**
      Create a new timer using milliseconds, and add it to the internal queue.

      @param msDelay  Callback starts firing this many milliseconds from now
      @param msPeriod If non-zero, callback is fired again after this period
      @param func     The callback to run at the specified interval

      @return  Id used to identify the timer for later use
    */
    TimerId addTimer(millisec msDelay, millisec msPeriod, const TFunction& func);

    /**
      Convenience function; setInterval API like browser javascript.

      Call function every 'period' ms, starting 'period' ms from now.
    */
    TimerId setInterval(const TFunction& func, millisec period) {
      return addTimer(period, period, func);
    }

    /**
      Convenience function; setTimeout API like browser javascript.

      Call function once 'timeout' ms from now.
    */
    TimerId setTimeout(const TFunction& func, millisec timeout) {
      return addTimer(timeout, 0, func);
    }

    bool clear(TimerId id);

    /**
      Destroy all timers, but preserve id uniqueness.
      This carefully makes sure every timer is not executing its callback
      before destructing it.
    */
    void clear();

    // Peek at current state
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    // Returns lazily initialized singleton
    static TimerManager& global();

    /**
      This method returns number of ticks in microseconds since some
      pre-defined time in the past.  *NOTE*: it is necessary that this
      pre-defined time exists between runs of the application, and must
      be (relatively) unique.  For example, the time since the system
      started running is not a good choice, since it can be duplicated.
      The current implementation uses time since the UNIX epoch.

      @return Current time in microseconds.
    */
    static uInt64 getTicks() {
      using namespace std::chrono;
      return duration_cast<duration<uInt64, std::ratio<1, 1000000> > >
        (system_clock::now().time_since_epoch()).count();
    }

  private:
    using Lock = std::mutex;
    using ScopedLock = std::unique_lock<Lock>;
    using ConditionVar = std::condition_variable;

    using Clock = std::chrono::steady_clock;
    using Timestamp = std::chrono::time_point<Clock>;
    using Duration = std::chrono::milliseconds;

    struct Timer
    {
      explicit Timer(TimerId tid = 0) : id{tid} { }
      Timer(Timer&& r) noexcept;

      Timer(TimerId id, Timestamp next, Duration period, const TFunction& func) noexcept;

      // Never called
      Timer() = default;
      ~Timer() = default;
      Timer(Timer const& r) = delete;
      Timer& operator=(Timer const& r) = delete;
      Timer& operator=(Timer&& r) = delete;

      TimerId id{0};
      Timestamp next;
      Duration period{0};
      TFunction handler;

      // You must be holding the 'sync' lock to assign waitCond
      std::unique_ptr<ConditionVar> waitCond;

      bool running{false};
    };

    // Comparison functor to sort the timer "queue" by Timer::next
    struct NextActiveComparator
    {
      bool operator()(Timer const& a, Timer const& b) const noexcept
      {
        return a.next < b.next;
      }
    };

    // Queue is a set of references to Timer objects, sorted by next
    using QueueValue = std::reference_wrapper<Timer>;
    using Queue = std::multiset<QueueValue, NextActiveComparator>;
    using TimerMap = std::unordered_map<TimerId, Timer>;

    bool destroy_impl(ScopedLock& lock, TimerMap::iterator i, bool notify);

    // Inexhaustible source of unique IDs
    TimerId nextId;

    // The Timer objects are physically stored in this map
    TimerMap active;

    // The ordering queue holds references to items in 'active'
    Queue queue;

    mutable Lock sync;
    ConditionVar wakeUp;
    bool done{false};

    // Valid IDs are guaranteed not to be this value
    static TimerId constexpr no_timer = TimerId(0);

  private:
    // Following constructors and assignment operators not supported
    TimerManager(const TimerManager&) = delete;
    TimerManager(TimerManager&&) = delete;
    TimerManager& operator=(const TimerManager&) = delete;
    TimerManager& operator=(TimerManager&&) = delete;
};

#endif
