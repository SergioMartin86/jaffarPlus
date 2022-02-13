#include <cmath>
#include <SDL_timer.h>
#include "hqn.h"
#include "hqn_util.h"

namespace hqn
{

// Function to initalize the video palette
int32_t *_initF_VideoPalette()
{
    static int32_t VideoPalette[512];
    const Nes_Emu::rgb_t *palette = Nes_Emu::nes_colors;
    for (int i = 0; i < 512; i++)
    {
        VideoPalette[i] = palette[i].red << 16 | palette[i].green << 8
            | palette[i].blue | 0xff000000;
    }
    return VideoPalette;
}

// Initialize the video palette
const int32_t *HQNState::NES_VIDEO_PALETTE = _initF_VideoPalette();

// simulate the write so we'll know how long the buffer needs to be
class Sim_Writer : public Data_Writer
{
	long size_;
public:
	Sim_Writer() :size_(0) { }
	error_t write(const void *, long size)
	{
		size_ += size;
		return 0;
	}
	long size() const { return size_; }
};

// Constructor
HQNState::HQNState()
{
    m_emu = new Nes_Emu();
    joypad[0] = 0x00;
    joypad[1] = 0x00;

	m_listener = nullptr;
    m_romData = nullptr;
    m_romSize = 0;

	m_emu->set_tracecb(nullptr);
	m_emu->set_sample_rate(44100);

    m_prevFrame = 0;
    m_msPerFrame = 0;
    m_initialFrame = SDL_GetTicks();
}

// Destructor
HQNState::~HQNState()
{
    delete m_emu;
}

error_t HQNState::setSampleRate(int rate)
{
	const char *ret = m_emu->set_sample_rate(rate);
	if (!ret)
		m_emu->set_equalizer(Nes_Emu::nes_eq);
	return ret;
}

// Load a ROM image
error_t HQNState::loadROM(const char *filename)
{
    // unload any existing rom data
    unloadRom();

    if (!load_file(filename, (char**)(&m_romData), &m_romSize))
    {
        return "Failed to open file";
    }

    // Now finally load the rom. Ugh
    Mem_File_Reader r(m_romData, (int)m_romSize);
    Auto_File_Reader a(r);
    error_t result = m_emu->load_ines(a);
    if (m_listener)
        m_listener->onLoadROM(this, filename);
    return result;
}

error_t HQNState::saveState(void *dest, size_t size, size_t *size_out)
{
    Mem_Writer w(dest, size, 0);
    Auto_File_Writer a(w);
    blargg_err_t ret = m_emu->save_state(a);
	if (size_out)
		*size_out = w.size();
    if (!ret && w.size() != size)
        return "Buffer Underrun!";
    return ret;
}

error_t HQNState::saveStateSize(size_t *size) const
{
    Sim_Writer w;
    Auto_File_Writer a(w);
    const char *ret = m_emu->save_state(a);
    if (size)
        *size = w.size();
    return ret;
}

error_t HQNState::loadState(const char *data, size_t size)
{
    Mem_File_Reader r(data, size);
    Auto_File_Reader a(r);
    error_t result = m_emu->load_state(a);
    if (m_listener)
        m_listener->onLoadState(this);
    return result;
}

void HQNState::unloadRom()
{
    if (m_romData)
    {
        delete[] m_romData;
        m_romData = nullptr;
        m_romSize = 0;
    }
}

// Advance the emulator
error_t HQNState::advanceFrame(bool sleep)
{
    Uint32 ticks;
    ticks = SDL_GetTicks();
    Uint32 wantTicks = m_prevFrame + m_msPerFrame;
    if (wantTicks > ticks)
    {
        SDL_Delay(wantTicks - ticks);
    }
    // m_frameTime = wantTicks - m_prevFrame;
    error_t result = m_emu->emulate_frame(joypad[0], joypad[1]);
    if (m_listener)
        m_listener->onAdvanceFrame(this);
    ticks = SDL_GetTicks();
    m_frameTime = ticks - m_prevFrame;
    m_prevFrame = ticks;
    return result;
}

void HQNState::setFramerate(int fps)
{
    if (fps == 0)
    {
        m_msPerFrame = 0;
    }
    else
    {
        m_msPerFrame = (long)(1000.0 / fps);
    }   
}

int HQNState::getFramerate() const
{
    if (m_msPerFrame)
    {
        return (int)(1000.0 / m_msPerFrame);
    }
    else
    {
        return 0;
    }
}

double HQNState::getFPS() const
{
    double ft = m_frameTime ? m_frameTime : 1;
    double fps = 1000.0 / ft;
    // round to 2 decimal places
    fps = std::floor(fps * 100) / 100;
    return fps;
}

// Copied from bizinterface.cpp in BizHawk/quicknes
void HQNState::blit(int32_t *dest, const int32_t *colors, int cropleft, int croptop, int cropright, int cropbottom)
{
    // what is the point of the 256 color bitmap and the dynamic color allocation to it?
    // why not just render directly to a 512 color bitmap with static palette positions?
    Nes_Emu *e = m_emu; // e was a parameter but since this is now part of a class, it's just in here
    const int srcpitch = e->frame().pitch;
    const unsigned char *src = e->frame().pixels;
    const unsigned char *const srcend = src + (e->image_height - cropbottom) * srcpitch;

    const short *lut = e->frame().palette;

    const int rowlen = 256 - cropleft - cropright;

    src += cropleft;
    src += croptop * srcpitch;

    for (; src < srcend; src += srcpitch)
    {
        for (int i = 0; i < rowlen; i++)
        {
            *dest++ = colors[lut[src[i]]];
        }
    }
}

} // end namespace hqn
