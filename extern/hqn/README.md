# Introduction
HeadlessQuickNes (HQN) uses the QuickNes emulator to emulate NES games. It also includes Lua scripting support with LuaJIT.
HQN is designed for distributed computing and to that end it includes the Lua sockets library as well.

# Building
Build scripts are provided for Windows and Linux. For other platforms it shouldn't be too hard.

## Required Libraries
HQN depends on three libraies and their dependancies: SDL2, SDL2_ttf, and LuaJIT. This means HQN
also depends on the freetype library and zlib but those don't have to be linked in the build.

## All
You will need LuaJIT. Make sure to extract it and rename the folder to LuaJIT (remove the version).
The LuaJIT folder should be in the root of the HeadlessQuickNes folder. You will need to compile
LuaJIT for your platform in this folder.

## Linux
You will need to install libsdl2-dev and libsdl2_ttf-dev (or whatever the library name it). You
should be able to install these with your package manager. The makefile for Linux can be found
in `linux/Makefile` and it should automatically build libquicknes for you.

## Windows
Before trying to build HQN you need to set up SDL2 correctly. Download the development libraries
for SDL2 and extract them so a folder named `SDL2` is in the same folder as `src`. You also
need to download SDL2_ttf development libraries. Inside the `SDL2_ttf` zip you will find two folders
`include` and `lib`. Copy both of those folders into your `SDL2` folder. It will say something about
'merging' the folders and that is what we want.
To compile use the batch file `win32\msvcbuild.bat`. This will compile both the quicknes library
and HQN. This requires the Visual Studio command prompt.

# Running
## Linux
When HQN is run it will say it is missing a required library. Copy LuaJIT/src/libluajit.so
to the directory where hqnes is and rename it to the name of the required library. This will
usually be something like `libluajit-5.1.so.2`.

## Windows
Copy lua51.dll, SDL2.dll, SDL2_ttf.dll, freetype.dll and zlib1.dll to the directory where hqnes.exe is located.
SDL2, SDL2_ttf, freetype and zlib1 can be found in `SDL2/lib/x86`.

# Lua Sockets
## Linux
Run the command `sudo apt-get install luarocks && sudo luarocks install luasocket` or the appropriate
command for your Linux distribution.

## Windows
Download the luasockets distribution from [LuaPower](https://luapower.com/socket).
