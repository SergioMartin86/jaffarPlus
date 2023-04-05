/* Raw - Another World Interpreter
 * Copyright (C) 2004 Gregory Montoir
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "memBuffer.h"

memBuffer::memBuffer(uint8_t* buf) {
 buffer = buf;
 pos = 0;
}

int64_t memBuffer::getSize()
{
 return pos;
}

void memBuffer::reset() {
 pos = 0;
}

void memBuffer::seek(int32_t off) {
 pos += off;
}

void memBuffer::read(void *ptr, uint32_t size) {
 memcpy(ptr, &buffer[pos], size);
 pos += size;
}

uint8_t memBuffer::readByte() {
 uint8_t b = 0;
 read(&b, 1);
 return b;
}

uint16_t memBuffer::readUint16BE() {
 uint8_t hi = readByte();
 uint8_t lo = readByte();
 return (hi << 8) | lo;
}

uint32_t memBuffer::readUint32BE() {
 uint16_t hi = readUint16BE();
 uint16_t lo = readUint16BE();
 return (hi << 16) | lo;
}

void memBuffer::write(void *ptr, uint32_t size) {
 memcpy(&buffer[pos], ptr, size);
 pos += size;
}

void memBuffer::writeByte(uint8_t b) {
 write(&b, 1);
}

void memBuffer::writeUint16BE(uint16_t n) {
 writeByte(n >> 8);
 writeByte(n & 0xFF);
}

void memBuffer::writeUint32BE(uint32_t n) {
 writeUint16BE(n >> 16);
 writeUint16BE(n & 0xFFFF);
}

thread_local std::map<std::string, memBuffer*> _fileBuffers;
