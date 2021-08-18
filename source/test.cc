#include "argparse.hpp"
#include "nlohmann/json.hpp"
#include "quicknesInstance.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
 printf("Testing...\n");

 quicknesInstance quicknes;
 printf("Quicknes created.\n");
 quicknes.initialize(argv[1], argv[3], false, false);
 printf("Quicknes initialized.\n");

 quicknes.playFrame("L");
 uint8_t testState[_STATE_DATA_SIZE];
 quicknes.saveState(testState);

 size_t step = 0;
 while(1)
 {
  printf("Step %lu\n", step++);
  printf("-------------------------------------\n");
  quicknes.playFrame("L");
  quicknes.printState();
  getchar();
  if (step % 40 == 0)
  {
   printf("Reloading\n");
   quicknes.loadState(testState);
  }
  if (step % 10 == 0)
   quicknes.saveState(testState);
 }
}
