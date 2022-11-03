#pragma once

#include <nall/platform.hpp>
#include <nall/arguments.hpp>
#include <nall/string.hpp>

#ifndef _NO_MAIN
#define _MAIN main
#else
#define _MAIN __main
#endif

namespace nall {
  auto _MAIN(Arguments arguments) -> void;

  auto _MAIN(int argc, char** argv) -> int {
    #if defined(PLATFORM_WINDOWS)
    CoInitialize(0);
    WSAData wsaData{0};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    _setmode(_fileno(stdin ), O_BINARY);
    _setmode(_fileno(stdout), O_BINARY);
    _setmode(_fileno(stderr), O_BINARY);
    #endif

    _MAIN(move(Arguments{argc, argv}));

    //when a program is running, input on the terminal queues in stdin
    //when terminating the program, the shell proceeds to try and execute all stdin data
    //this is annoying behavior: this code tries to minimize the impact as much as it can
    //we can flush all of stdin up to the last line feed, preventing spurious commands from executing
    //however, even with setvbuf(_IONBF), we can't stop the last line from echoing to the terminal
    #if !defined(PLATFORM_WINDOWS)
    auto flags = fcntl(fileno(stdin), F_GETFL, 0);
    fcntl(fileno(stdin), F_SETFL, flags | O_NONBLOCK);  //don't allow read() to block when empty
    char buffer[4096], data = false;
    while(read(fileno(stdin), buffer, sizeof(buffer)) > 0) data = true;
    fcntl(fileno(stdin), F_SETFL, flags);  //restore original flags for the terminal
    if(data) putchar('\r');  //ensures PS1 is printed at the start of the line
    #endif

    return EXIT_SUCCESS;
  }
}

auto _MAIN(int argc, char** argv) -> int {
  return nall::_MAIN(argc, argv);
}
