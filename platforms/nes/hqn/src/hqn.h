#ifndef __HQN_H__
#define __HQN_H__

#include <Nes_Emu.h>
#include <cstdint>

#define BLIT_SIZE 65536

namespace hqn
{

typedef const char *error_t;

class HQNState;

class HQNListener
{
public:
    HQNListener() = default;
    virtual ~HQNListener() = default;

    virtual void onLoadROM(HQNState *state, const char *filename) = 0;
    virtual void onAdvanceFrame(HQNState *state) = 0;
    virtual void onLoadState(HQNState *state) = 0;
};

/*
State which is maintained by the emulator driver.

This should normally be obtained using hqn_get_state() if you are in a lua
function.
*/
class HQNState
{
public:

    /* A reference to the emulator instance. */
    Nes_Emu *m_emu;

    static const int32_t *NES_VIDEO_PALETTE;

    /**
     * Constructor.
     */
    HQNState();
    ~HQNState();

    /*
    The joypad data for the two joypads available to an NES.
    This is directly available because I'm lazy.
    */
    uint32_t joypad[2];

    /* Get the emulator this state uses. */
    inline Nes_Emu *emu() const
    { return m_emu; }

    /*
    Load a NES rom from the named file.
    Returns NULL or error string.
    */
	error_t loadROM(const char *filename);

    /*
    Advance the emulator by one frame. If sleep is true and there is a frame
    limit set advanceFrame() will sleep in order to limit the framerate.
    Returns NULL or error string.
    */
	error_t advanceFrame(bool sleep=true);

    /*
    Save the game state. This can be restored at any time.
    */
    error_t saveState(void *dest, size_t size, size_t *size_out);

    /*
    Get the size (bytes) of a savestate.
    Use this to allocate enough space for the saveState method.
    */
    error_t saveStateSize(size_t *size) const;

    /*
    Load the emulator state from data.
    This should be data produced by saveState().
    */
    error_t loadState(const char *data, size_t size);


	error_t setSampleRate(int rate);

    /**
     * Set a limit to the framerate.
     * 0 means no limit.
     */
    void setFramerate(int fps);

    /**
     * Get the current framerate limit.
     */
    int getFramerate() const;

    /**
     * Get the calculated frames per second.
     */
    double getFPS() const;

    /**
     * Blit the contents of the NES screen to dest. Dest should be able to hold 256*240 ints.
     * Unless you want to change the color palette you should use NES_VIDEO_PALETTE for colors.
     */
    void blit(int32_t *dest, const int32_t *colors, int cropleft, int croptop, int cropright, int cropbottom) const;

    inline HQNListener *getListener() const
    { return m_listener; }

    inline void setListener(HQNListener *l)
    { m_listener = l; }

    /**
     * Get the state of the keyboard. Use this to update the keyboard state.
     */
    inline uint8_t *getKeyboard()
    { return m_keyboard; }

private:
    // Safely unload the currently loaded rom
    void unloadRom();

    /* ROM file stored in memory because reasons */
    uint8_t *m_romData;
    size_t m_romSize;
    /* Minimum milliseconds between each frame. */
    uint32_t m_msPerFrame;
    /* time value of previous frame. */
    uint32_t m_prevFrame;
    /* The listener */
    HQNListener *m_listener;
    /* Keyboard state */
    uint8_t m_keyboard[256];
    /* Number of frames we've processed */
    uint32_t m_frames;
    /* How long it took to process the previous frame */
    uint32_t m_frameTime;
    uint32_t m_initialFrame;
};

/*
Print the usage message.
@param filename used to specify the name of the exe file, may be NULL.
*/
void printUsage(const char *filename);

} // end namespace hqn


#endif /* __HQN_H__ */
