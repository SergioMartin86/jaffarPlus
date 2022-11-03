#include "bsnes.hpp"
#include <sfc/interface/interface.hpp>
thread_local Video video;
thread_local Audio audio;
thread_local Input input;
thread_local unique_pointer<Emulator::Interface> emulator;

auto locate(string name) -> string {
  string location = {Path::program(), name};
  if(inode::exists(location)) return location;

  location = {Path::userData(), "bsnes/", name};
  if(inode::exists(location)) return location;

  directory::create({Path::userSettings(), "bsnes/"});
  return {Path::userSettings(), "bsnes/", name};
}

#include <nall/main.hpp>
auto nall::_MAIN(Arguments arguments) -> void {
  settings.location = locate("settings.bml");

  for(auto argument : arguments) {
    if(argument == "--fullscreen") {
      program.startFullScreen = true;
    } else if(argument.beginsWith("--locale=")) {
      Application::locale().scan(locate("Locale/"));
      Application::locale().select(argument.trimLeft("--locale=", 1L));
    } else if(argument.beginsWith("--settings=")) {
      settings.location = argument.trimLeft("--settings=", 1L);
    } else if(inode::exists(argument)) {
      //game without option
      program.gameQueue.append({"Auto;", argument});
    } else if(argument.find(";")) {
      //game with option
      auto game = argument.split(";", 1L);
      if(inode::exists(game.last())) program.gameQueue.append(argument);
    }
  }

  settings.load();
  Application::setName("bsnes");
  Application::setScreenSaver(settings.general.screenSaver);
  Application::setToolTips(settings.general.toolTips);

  Instances::presentation.construct();
  Instances::settingsWindow.construct();
  Instances::cheatDatabase.construct();
  Instances::cheatWindow.construct();
  Instances::stateWindow.construct();
  Instances::toolsWindow.construct();

  emulator = new SuperFamicom::Interface;
  program.create();
  program.load();

  //Application::run();

  while(true)
  {
   program.updateStatus();
   video.poll();
//   program.main();
//   auto state = emulator->serialize(0);
//   printf("State Size: %u\n", state.size());
//   state.setMode(serializer::Mode::Load);
//   emulator->unserialize(state);

   emulator->run();
  }

  Instances::presentation.destruct();
  Instances::settingsWindow.destruct();
  Instances::cheatDatabase.destruct();
  Instances::cheatWindow.destruct();
  Instances::stateWindow.destruct();
  Instances::toolsWindow.destruct();
}
