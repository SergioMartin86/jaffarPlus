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

#include "engine.h"
#include "memBuffer.h"
#include "serializer.h"
#include "sys.h"
#include "parts.h"
#include <vector>
#include "utils.hpp"

Engine::Engine(System *paramSys, const char *dataDir, const char *saveDir)
	: sys(paramSys), vm(&res, &video, sys), res(&video, dataDir), video(&res, sys), _dataDir(dataDir), _saveDir(saveDir), _stateSlot(0) {
}

void Engine::run() {

	while (!sys->input.quit) {

		vm.checkThreadRequests();

		vm.inp_updatePlayer();

		processInput();

		vm.hostFrame();
	}

}

Engine::~Engine(){

	finish();
	sys->destroy();
}


void Engine::init() {

 // Caching files
 std::vector<std::string> fileList;
 fileList.push_back("MEMLIST.BIN");
 fileList.push_back("BANK01");
 fileList.push_back("BANK02");
 fileList.push_back("BANK03");
 fileList.push_back("BANK04");
 fileList.push_back("BANK05");
 fileList.push_back("BANK06");
 fileList.push_back("BANK07");
 fileList.push_back("BANK08");
 fileList.push_back("BANK09");
 fileList.push_back("BANK0A");
 fileList.push_back("BANK0B");
 fileList.push_back("BANK0C");
 fileList.push_back("BANK0D");

#pragma omp single
 for(const auto& f : fileList)
 {
  std::string path = std::string(_dataDir) + std::string("/") + f;
  auto s = new std::string;
  if (loadStringFromFile(*s, path.c_str()) == false) EXIT_WITH_ERROR("Could not load file %s\n", path.c_str());
  _fileBuffers[f] = new memBuffer((uint8_t*)s->data());
 }

#pragma omp barrier

	//Init system
	sys->init("Out Of This World");

	video.init();

	res.allocMemBlock();

	res.readEntries();

	vm.init();

	uint16_t part = GAME_PART1;  // This game part is the protection screen
//#ifdef BYPASS_PROTECTION
  part = GAME_PART2;
//#endif
  vm.initForPart(part);

  // Try to cheat here. You can jump anywhere but the VM crashes afterward.
	// Starting somewhere is probably not enough, the variables and calls return are probably missing.
	//vm.initForPart(GAME_PART2); // Skip protection screen and go directly to intro
	//vm.initForPart(GAME_PART3); // CRASH
	//vm.initForPart(GAME_PART4); // Start directly in jail but then crash
	//vm.initForPart(GAME_PART5);   //CRASH
	//vm.initForPart(GAME_PART6);   // Start in the battlechar but CRASH afteward
	//vm.initForPart(GAME_PART7); //CRASH
	//vm.initForPart(GAME_PART8); //CRASH
	//vm.initForPart(GAME_PART9); // Green screen not doing anything
}

void Engine::finish() {
	res.freeMemBlock();
}

void Engine::processInput() {
}

void Engine::makeGameStateName(uint8_t slot, char *buf) {
	sprintf(buf, "raw.s%02d", slot);
}

void Engine::saveGameState(uint8_t* buffer)
{
  memBuffer m(buffer);
  saveGameState(&m);
}

void Engine::saveGameState(memBuffer* m)
{
  Serializer s(m, Serializer::SM_SAVE, res._memPtrStart);
  processState(s);
}

size_t Engine::getGameStateSize()
{
  uint8_t* b = (uint8_t*) malloc (1048576);
  memBuffer m(b);
  saveGameState(&m);
  free(b);
  return m.getSize();
}

void Engine::loadGameState(uint8_t* buffer)
{
 memBuffer m(buffer);
 loadGameState(&m);
}

void Engine::loadGameState(memBuffer* m)
{
 memset((uint8_t *)vm.threadsData, 0xFF, sizeof(vm.threadsData));
 memset((uint8_t *)vm.vmIsChannelActive, 0, sizeof(vm.vmIsChannelActive));
 int firstThreadId = 0;
 vm.threadsData[PC_OFFSET][firstThreadId] = 0;

 Serializer s(m, Serializer::SM_LOAD, res._memPtrStart, 0);
 processState(s);
}

void Engine::processState(Serializer& s)
{
 vm.saveOrLoad(s);
 res.saveOrLoad(s);

#ifdef _JAFFAR_PLAY
 video.saveOrLoad(s);
#endif
}
