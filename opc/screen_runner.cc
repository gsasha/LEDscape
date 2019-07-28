#include "opc/fltk-driver.h"
#include "opc/animation.h"

const int NUM_STRIPS = 48;
const int NUM_PIXELS_PER_STRIP = 100;
int main(int argc, char **argv) {
  FltkDriver driver(argc, argv, NUM_STRIPS, NUM_PIXELS_PER_STRIP);
  Animation animation(&driver);
  animation.StartThread();
  driver.Run();
}

