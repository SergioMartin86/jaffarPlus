#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <cstdint>

/**
 * Buttons found on a standard controller.
 */
enum ControllerButton
{
    BUTTON_A      = 0,
    BUTTON_B      = 1,
    BUTTON_SELECT = 2,
    BUTTON_START  = 3,
    BUTTON_UP     = 4,
    BUTTON_DOWN   = 5,
    BUTTON_LEFT   = 6,
    BUTTON_RIGHT  = 7
};

/**
 * Emulates an NES game controller device.
 */
class Controller
{
public:
    inline Controller()
    {
     for( auto& b : buttonStates )
     {
         b = false;
     }
     buttonIndex = 0;
     strobe = 1;
    }

    /**
     * Read from the controller register.
     */
    inline uint8_t readByte()
    {
     uint8_t value = 1;

     if( buttonIndex < 8 )
     {
         value = (buttonStates[buttonIndex] ? 0x41 : 0x40);
     }

     if( (strobe & (1 << 0)) == 0 )
     {
         buttonIndex++;
     }

     return value;
    }

    /**
     * Set the state of a button on the controller.
     */
    inline void setButtonState( ControllerButton button, bool state )
    {
     buttonStates[(int)button] = state;
    }

    inline void setButtonStates(uint8_t states)
    {
     *(uint8_t*)buttonStates = states;
    }

    /**
     * Write a byte to the controller register.
     */
    inline void writeByte( uint8_t value )
    {
     if( (value & (1 << 0)) == 0 && (strobe & (1 << 0)) == 1 )
     {
         buttonIndex = 0;
     }
     strobe = value;
    }

private:
    bool    buttonStates[8];
    uint8_t buttonIndex;
    uint8_t strobe;
};

#endif // CONTROLLER_HPP
