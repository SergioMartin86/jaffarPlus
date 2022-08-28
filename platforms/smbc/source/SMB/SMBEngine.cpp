#include <cstring>

#include "../Configuration.hpp"

#include "../Emulation/Controller.hpp"
#include "../Emulation/PPU.hpp"

#include "SMBEngine.hpp"

#define DATA_STORAGE_OFFSET 0x8000 // Starting address for storing constant data

uint8_t* romImage;

//---------------------------------------------------------------------
// Public interface
//---------------------------------------------------------------------

size_t SMBEngine::getStateSize()
{
 size_t size = 0;
 size += sizeof(c);
 size += sizeof(z);
 size += sizeof(n);
 size += sizeof(registerA);
 size += sizeof(registerX);
 size += sizeof(registerY);
 size += sizeof(registerS);
 size += sizeof(a.constant);
 size += sizeof(x.constant);
 size += sizeof(y.constant);
 size += sizeof(s.constant);
 size += sizeof(dataStorage);
 size += sizeof(ram);
// size += sizeof(chr);
 size += sizeof(returnIndexStack);
 size += sizeof(returnIndexStackTop);
 size += sizeof(ppu->ppuCtrl);
 size += sizeof(ppu->ppuMask);
 size += sizeof(ppu->ppuStatus);
 size += sizeof(ppu->oamAddress);
 size += sizeof(ppu->ppuScrollX);
 size += sizeof(ppu->ppuScrollY);
 size += sizeof(ppu->palette);
 size += sizeof(ppu->nametable);
 size += sizeof(ppu->currentAddress);
 size += sizeof(ppu->writeToggle);
 size += sizeof(ppu->vramBuffer);
 return size;
}

void SMBEngine::saveState(uint8_t* state) const
{
 uint8_t* ptr = state;
 memcpy(ptr, &c, sizeof(c)); ptr += sizeof(c);
 memcpy(ptr, &z, sizeof(z)); ptr += sizeof(z);
 memcpy(ptr, &n, sizeof(n)); ptr += sizeof(n);
 memcpy(ptr, &registerA, sizeof(registerA)); ptr += sizeof(registerA);
 memcpy(ptr, &registerX, sizeof(registerX)); ptr += sizeof(registerX);
 memcpy(ptr, &registerY, sizeof(registerY)); ptr += sizeof(registerY);
 memcpy(ptr, &registerS, sizeof(registerS)); ptr += sizeof(registerS);
 memcpy(ptr, &a.constant, sizeof(a.constant)); ptr += sizeof(a.constant);
 memcpy(ptr, &x.constant, sizeof(x.constant)); ptr += sizeof(x.constant);
 memcpy(ptr, &y.constant, sizeof(y.constant)); ptr += sizeof(y.constant);
 memcpy(ptr, &s.constant, sizeof(s.constant)); ptr += sizeof(s.constant);
 memcpy(ptr, &dataStorage, sizeof(dataStorage)); ptr += sizeof(dataStorage);
 memcpy(ptr, &ram, sizeof(ram)); ptr += sizeof(ram);
// memcpy(ptr, &chr, sizeof(chr)); ptr += sizeof(chr);
 memcpy(ptr, &returnIndexStack, sizeof(returnIndexStack)); ptr += sizeof(returnIndexStack);
 memcpy(ptr, &returnIndexStackTop, sizeof(returnIndexStackTop)); ptr += sizeof(returnIndexStackTop);
 memcpy(ptr, &ppu->ppuCtrl, sizeof(ppu->ppuCtrl)); ptr += sizeof(ppu->ppuCtrl);
 memcpy(ptr, &ppu->ppuMask, sizeof(ppu->ppuMask)); ptr += sizeof(ppu->ppuMask);
 memcpy(ptr, &ppu->ppuStatus, sizeof(ppu->ppuStatus)); ptr += sizeof(ppu->ppuStatus);
 memcpy(ptr, &ppu->oamAddress, sizeof(ppu->oamAddress)); ptr += sizeof(ppu->oamAddress);
 memcpy(ptr, &ppu->ppuScrollX, sizeof(ppu->ppuScrollX)); ptr += sizeof(ppu->ppuScrollX);
 memcpy(ptr, &ppu->ppuScrollY, sizeof(ppu->ppuScrollY)); ptr += sizeof(ppu->ppuScrollY);
 memcpy(ptr, &ppu->palette, sizeof(ppu->palette)); ptr += sizeof(ppu->palette);
 memcpy(ptr, &ppu->nametable, sizeof(ppu->nametable)); ptr += sizeof(ppu->nametable);
 memcpy(ptr, &ppu->currentAddress, sizeof(ppu->currentAddress)); ptr += sizeof(ppu->currentAddress);
 memcpy(ptr, &ppu->writeToggle, sizeof(ppu->writeToggle)); ptr += sizeof(ppu->writeToggle);
 memcpy(ptr, &ppu->vramBuffer, sizeof(ppu->vramBuffer)); ptr += sizeof(ppu->vramBuffer);
}

void SMBEngine::loadState(const uint8_t* state)
{
 const uint8_t* ptr = state;
 memcpy(&c, ptr, sizeof(c)); ptr += sizeof(c);
 memcpy(&z, ptr, sizeof(z)); ptr += sizeof(z);
 memcpy(&n, ptr, sizeof(n)); ptr += sizeof(n);
 memcpy(&registerA, ptr, sizeof(registerA)); ptr += sizeof(registerA);
 memcpy(&registerX, ptr, sizeof(registerX)); ptr += sizeof(registerX);
 memcpy(&registerY, ptr, sizeof(registerY)); ptr += sizeof(registerY);
 memcpy(&registerS, ptr, sizeof(registerS)); ptr += sizeof(registerS);
 memcpy(&a.constant, ptr, sizeof(a.constant)); ptr += sizeof(a.constant);
 memcpy(&x.constant, ptr, sizeof(x.constant)); ptr += sizeof(x.constant);
 memcpy(&y.constant, ptr, sizeof(y.constant)); ptr += sizeof(y.constant);
 memcpy(&s.constant, ptr, sizeof(s.constant)); ptr += sizeof(s.constant);
 memcpy(&dataStorage, ptr, sizeof(dataStorage)); ptr += sizeof(dataStorage);
 memcpy(&ram, ptr, sizeof(ram)); ptr += sizeof(ram);
// memcpy(&chr, ptr, sizeof(chr)); ptr += sizeof(chr);
 memcpy(&returnIndexStack, ptr, sizeof(returnIndexStack)); ptr += sizeof(returnIndexStack);
 memcpy(&returnIndexStackTop, ptr, sizeof(returnIndexStackTop)); ptr += sizeof(returnIndexStackTop);
 memcpy(&ppu->ppuCtrl, ptr, sizeof(ppu->ppuCtrl)); ptr += sizeof(ppu->ppuCtrl);
 memcpy(&ppu->ppuMask, ptr, sizeof(ppu->ppuMask)); ptr += sizeof(ppu->ppuMask);
 memcpy(&ppu->ppuStatus, ptr, sizeof(ppu->ppuStatus)); ptr += sizeof(ppu->ppuStatus);
 memcpy(&ppu->oamAddress, ptr, sizeof(ppu->oamAddress)); ptr += sizeof(ppu->oamAddress);
 memcpy(&ppu->ppuScrollX, ptr, sizeof(ppu->ppuScrollX)); ptr += sizeof(ppu->ppuScrollX);
 memcpy(&ppu->ppuScrollY, ptr, sizeof(ppu->ppuScrollY)); ptr += sizeof(ppu->ppuScrollY);
 memcpy(&ppu->palette, ptr, sizeof(ppu->palette)); ptr += sizeof(ppu->palette);
 memcpy(&ppu->nametable, ptr, sizeof(ppu->nametable)); ptr += sizeof(ppu->nametable);
 memcpy(&ppu->currentAddress, ptr, sizeof(ppu->currentAddress)); ptr += sizeof(ppu->currentAddress);
 memcpy(&ppu->writeToggle, ptr, sizeof(ppu->writeToggle)); ptr += sizeof(ppu->writeToggle);
 memcpy(&ppu->vramBuffer, ptr, sizeof(ppu->vramBuffer)); ptr += sizeof(ppu->vramBuffer);
}

SMBEngine::SMBEngine(uint8_t* romImage) :
    a(*this, &registerA),
    x(*this, &registerX),
    y(*this, &registerY),
    s(*this, &registerS)
{
    ppu = new PPU(*this);
    controller1 = new Controller();
    controller2 = new Controller();

    // CHR Location in ROM: Header (16 bytes) + 2 PRG pages (16k each)
    chr = (romImage + 16 + (16384 * 2));

    returnIndexStackTop = 0;
}

SMBEngine::~SMBEngine()
{
    delete ppu;
    delete controller1;
    delete controller2;
}

void SMBEngine::audioCallback(uint8_t* stream, int length)
{
}

Controller& SMBEngine::getController1()
{
    return *controller1;
}

Controller& SMBEngine::getController2()
{
    return *controller2;
}

void SMBEngine::render(uint32_t* buffer)
{
    ppu->render(buffer);
}

void SMBEngine::reset()
{
    // Run the decompiled code for initialization
    code(0);
}

void SMBEngine::update()
{
    // Run the decompiled code for the NMI handler
    code(1);
}

//---------------------------------------------------------------------
// Private methods
//---------------------------------------------------------------------

void SMBEngine::compare(uint8_t value1, uint8_t value2)
{
    uint8_t result = value1 - value2;
    c = (value1 >= value2);
    setZN(result);
}

void SMBEngine::bit(uint8_t value)
{
    n = (value & (1 << 7)) != 0;
    z = (registerA & value) == 0;
}

uint8_t* SMBEngine::getCHR()
{
    return chr;
}

uint8_t* SMBEngine::getDataPointer(uint16_t address)
{
    // Constant data
    if( address >= DATA_STORAGE_OFFSET )
    {
        return dataStorage + (address - DATA_STORAGE_OFFSET);
    }
    // RAM and Mirrors
    else if( address < 0x2000 )
    {
        return ram + (address & 0x7ff);
    }

    return nullptr;
}

MemoryAccess SMBEngine::getMemory(uint16_t address)
{
    uint8_t* dataPointer = getDataPointer(address);
    if( dataPointer != nullptr )
    {
        return MemoryAccess(*this, dataPointer);
    }
    else
    {
        return MemoryAccess(*this, readData(address));
    }
}

uint16_t SMBEngine::getMemoryWord(uint8_t address)
{
    return (uint16_t)readData(address) + ((uint16_t)(readData(address + 1)) << 8);
}

void SMBEngine::pha()
{
    writeData(0x100 | (uint16_t)registerS, registerA);
    registerS--;
}

void SMBEngine::pla()
{
    registerS++;
    a = readData(0x100 | (uint16_t)registerS);
}

int SMBEngine::popReturnIndex()
{
    return returnIndexStack[returnIndexStackTop--];
}

void SMBEngine::pushReturnIndex(int index)
{
    returnIndexStack[++returnIndexStackTop] = index;
}

uint8_t SMBEngine::readData(uint16_t address)
{
    // Constant data
    if( address >= DATA_STORAGE_OFFSET )
    {
        return dataStorage[address - DATA_STORAGE_OFFSET];
    }
    // RAM and Mirrors
    else if( address < 0x2000 )
    {
        return ram[address & 0x7ff];
    }
    // PPU Registers and Mirrors
    else if( address < 0x4000 )
    {
        return ppu->readRegister(0x2000 + (address & 0x7));
    }
    // IO registers
    else if( address < 0x4020 )
    {
        switch (address)
        {
        case 0x4016:
            return controller1->readByte();
        case 0x4017:
            return controller2->readByte();
        }
    }

    return 0;
}

void SMBEngine::setZN(uint8_t value)
{
    z = (value == 0);
    n = (value & (1 << 7)) != 0;
}

void SMBEngine::writeData(uint16_t address, uint8_t value)
{
    // RAM and Mirrors
    if( address < 0x2000 )
    {
        ram[address & 0x7ff] = value;
    }
    // PPU Registers and Mirrors
    else if( address < 0x4000 )
    {
        ppu->writeRegister(0x2000 + (address & 0x7), value);
    }
    // IO registers
    else if( address < 0x4020 )
    {
        switch( address )
        {
        case 0x4014:
            ppu->writeDMA(value);
            break;
        case 0x4016:
            controller1->writeByte(value);
            controller2->writeByte(value);
            break;
        default:
            break;
        }
    }
}

void SMBEngine::writeData(uint16_t address, const uint8_t* data, size_t length)
{
    address -= DATA_STORAGE_OFFSET;

    memcpy(dataStorage + (std::ptrdiff_t)address, data, length);
}
