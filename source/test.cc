#include <stdio.h>
#include <Nes_Emu.h>
#include <Nes_State.h>
#include <blargg_errors.h>
#include <source/utils.h>
#include <argparse.hpp>

int main(int argc, char *argv[])
{
 printf("Testing...\n");

 // Parsing command line arguments
 argparse::ArgumentParser program("jaffar-nes", "1.0.0");

 program.add_argument("romFile")
   .help("Specifies the path to the NES rom to emulate.")
   .required();

 program.add_argument("stateFile")
   .help("Specifies the path to the NES state to load.")
   .required();

 // Try to parse arguments
 try { program.parse_args(argc, argv);  }
 catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

 // Getting rom  and state path
 auto romFilePath = program.get<std::string>("romFile");
 auto stateFilePath = program.get<std::string>("stateFile");

 // Loading rom data
 std::string romData;
 loadStringFromFile(romData, romFilePath.c_str());
 Mem_File_Reader romReader(romData.data(), (int)romData.size());
 Auto_File_Reader romFile(romReader);

 // Loading state data
 std::string stateData;
 loadStringFromFile(stateData, stateFilePath.c_str());
 Mem_File_Reader stateReader(stateData.data(), (int)stateData.size());
 Auto_File_Reader stateFile(stateReader);

 // Initializing Rom into emu
 Nes_Emu emu;
 auto result = emu.load_ines(romFile);
 printf("result: %d\n", result);

 // Loading state into emu
 Nes_State state;
 state.read(stateFile);
 emu.load_state(state);

 // Getting pointer to low memory
 auto lowMem = emu.low_mem();

 // Controller inputs
 // 0 - A / 1
 // 1 - B / 2
 // 2 - Select / 4
 // 3 - Start / 8
 // 4 - Up / 16
 // 5 - Down / 32
 // 6 - Left / 64
 // 7 - Right / 128
// printf("%u\n", lowMem[0x0086]);
// for (size_t i = 0; i < 220; i++)
// {
//  if (i == 43)
//  {
//   emu.emulate_frame(8,0);
//   printf("%u X\n", lowMem[0x0086]);
//  }
//  else
//  {
//   emu.emulate_frame(0,0);
//   printf("%u t:%1u%1u%1u\n", lowMem[0x0086], lowMem[0x07F8], lowMem[0x07F9], lowMem[0x07FA]);
//  }
// }
 printf("%u t:%1u%1u%1u\n", lowMem[0x0086], lowMem[0x07F8], lowMem[0x07F9], lowMem[0x07FA]);
 // Saving state
// emu.save_state(&state);
// Auto_File_Writer stateWriter("out.State");
// state.write(stateWriter);
}
