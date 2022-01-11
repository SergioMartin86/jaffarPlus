#include <stdio.h>
#include <quickNESInstance.h>
#include <utils.h>
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

 quickNESInstance nes(romFilePath);
 nes.loadStateFile(stateFilePath);

 nes.advanceFrame(2);

 for (size_t i = 0; i < 5; i++)
 {
  nes.advanceFrame(2);
  nes.printFrameInfo();
 }
}
