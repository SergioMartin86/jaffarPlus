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

#ifndef M6532_HXX
#define M6532_HXX

class RiotDebug;
class System;
class Settings;

#include "bspf.hxx"
#include "Device.hxx"
#include "ConsoleIO.hxx"
#include "Switches.hxx"
#include "Control.hxx"

/**
  This class models the M6532 RAM-I/O-Timer (aka RIOT) chip in the 2600
  console.  Note that since the M6507 CPU doesn't contain an interrupt line,
  the following functionality relating to the RIOT IRQ line is not emulated:

    - A3 to enable/disable interrupt from timer to IRQ
    - A1 to enable/disable interrupt from PA7 to IRQ

  @author  Bradford W. Mott and Stephen Anthony
*/
class M6532 : public Device
{
private:
  // Reference to the console
  const ConsoleIO& myConsole;

  public:
    /**
      The RIOT debugger class is a friend who needs special access
    */
    friend class RiotDebug;

  public:
    /**
      Create a new 6532 for the specified console

      @param console  The console the 6532 is associated with
      @param settings The settings used by the system
    */
    M6532(const ConsoleIO& console, const Settings& settings);
    ~M6532() override = default;

   public:
    /**
      Reset cartridge to its power-on state
    */
    void reset() override;

    /**
      Update the entire digital and analog pin state of ports A and B.
    */
    void update();

    /**
      Install 6532 in the specified system.  Invoked by the system
      when the 6532 is attached to it.

      @param system The system the device should install itself in
    */
    void install(System& system) override;

    /**
      Install 6532 in the specified system and device.  Invoked by
      the system when the 6532 is attached to it.  All devices
      which invoke this method take responsibility for chaining
      requests back to *this* device.

      @param system The system the device should install itself in
      @param device The device responsible for this address space
    */
    void installDelegate(System& system, Device& device);

    /**
      Save the current state of this device to the given Serializer.

      @param out  The Serializer object to use
      @return  False on any errors, else true
    */
    bool save(Serializer& out) const override;

    /**
      Load the current state of this device from the given Serializer.

      @param in  The Serializer object to use
      @return  False on any errors, else true
    */
    bool load(Serializer& in) override;

   public:
    /**
      Get the byte at the specified address

      @return The byte at the specified address
    */
    inline uInt8 peek(uInt16 addr)
    {
       updateEmulation();

       // A9 distinguishes I/O registers from ZP RAM
       // A9 = 1 is read from I/O
       // A9 = 0 is read from RAM
       if((addr & 0x0200) == 0x0000)
         return myRAM[addr & 0x007f];

       switch(addr & 0x07)
       {
         case 0x00:    // SWCHA - Port A I/O Register (Joystick)
         {
           const uInt8 value = (myConsole.leftController().read() << 4) |
                                myConsole.rightController().read();

           // Each pin is high (1) by default and will only go low (0) if either
           //  (a) External device drives the pin low
           //  (b) Corresponding bit in SWACNT = 1 and SWCHA = 0
           // Thanks to A. Herbert for this info
           return (myOutA | ~myDDRA) & value;
         }

         case 0x01:    // SWACNT - Port A Data Direction Register
         {
           return myDDRA;
         }

         case 0x02:    // SWCHB - Port B I/O Register (Console switches)
         {
           return (myOutB | ~myDDRB) & (myConsole.switches().read() | myDDRB);
         }

         case 0x03:    // SWBCNT - Port B Data Direction Register
         {
           return myDDRB;
         }

         case 0x04:    // INTIM - Timer Output
         case 0x06:
         {
           // Timer Flag is always cleared when accessing INTIM
           if (!myWrappedThisCycle) myInterruptFlag &= ~TimerBit;
           return myTimer;
         }

         case 0x05:    // TIMINT/INSTAT - Interrupt Flag
         case 0x07:
         {
           // PA7 Flag is always cleared after accessing TIMINT
           const uInt8 result = myInterruptFlag;
           myInterruptFlag &= ~PA7Bit;
         #ifdef DEBUGGER_SUPPORT
           myTimReadCycles += 7;
         #endif
           return result;
         }

         default:
         {
           return 0;
         }
       }
    }

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address

      @return  True if the poke changed the device address space, else false
    */
    bool poke(uInt16 address, uInt8 value) override;

    /**
     * Update RIOT state to the current timestamp.
     */
    void updateEmulation();

    /**
      Get a pointer to the RAM contents.

      @return  Pointer to RAM array.
    */
    uInt8* getRAM() { return myRAM.data(); }

  #ifdef DEBUGGER_SUPPORT
    /**
      Query the access counters

      @return  The access counters as comma separated string
    */
    string getAccessCounters() const override;

    /**
      Reset the timer read CPU cycle counter
    */
    void resetTimReadCylces() { myTimReadCycles = 0; }
  #endif

  private:

    void setTimerRegister(uInt8 value, uInt8 interval);
    void setPinState(bool swcha);


  private:

    // Reference to the settings
    const Settings& mySettings;

    // An amazing 128 bytes of RAM
    std::array<uInt8, 128> myRAM;

    // Current value of the timer
    uInt8 myTimer{0};

    // Current number of clocks "queued" for the divider
    uInt32 mySubTimer{0};

    // The divider
    uInt32 myDivider{1};

    // Has the timer wrapped this very cycle?
    bool myWrappedThisCycle{false};

    // Cycle when the timer set. Debugging only.
    uInt64 mySetTimerCycle{0};

    // Last cycle considered in emu updates
    uInt64 myLastCycle{0};

    // Data Direction Register for Port A
    uInt8 myDDRA{0};

    // Data Direction Register for Port B
    uInt8 myDDRB{0};

    // Last value written to Port A
    uInt8 myOutA{0};

    // Last value written to Port B
    uInt8 myOutB{0};

    // Interrupt Flag Register
    uInt8 myInterruptFlag{0};

    // Used to determine whether an active transition on PA7 has occurred
    // True is positive edge-detect, false is negative edge-detect
    bool myEdgeDetectPositive{false};

    // Last value written to the timer registers
    std::array<uInt8, 4> myOutTimer{0};

    // Accessible bits in the interrupt flag register
    // All other bits are always zeroed
    static constexpr uInt8 TimerBit = 0x80, PA7Bit = 0x40;


  private:
    // Following constructors and assignment operators not supported
    M6532() = delete;
    M6532(const M6532&) = delete;
    M6532(M6532&&) = delete;
    M6532& operator=(const M6532&) = delete;
    M6532& operator=(M6532&&) = delete;
};

#endif
