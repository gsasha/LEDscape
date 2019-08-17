#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "opc/animation.h"
#include "opc/fltk-driver.h"
#include "yaml-cpp/yaml.h"
#include <iostream>
#include <stdio.h>
#include <string>

const int NUM_STRIPS = 48;
const int NUM_PIXELS_PER_STRIP = 100;

ABSL_FLAG(std::string, screen_layout, "", "Name of the screen layout");
ABSL_FLAG(std::string, screen_layout_file, "",
          "File containing screen layout in yaml");

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  YAML::Node screen_layout;
  if (!absl::GetFlag(FLAGS_screen_layout).empty()) {
    std::cout << "--- LOADING " << absl::GetFlag(FLAGS_screen_layout) << "\n";
    screen_layout = YAML::Load(absl::GetFlag(FLAGS_screen_layout));
  } else if (!absl::GetFlag(FLAGS_screen_layout_file).empty()) {
    std::cout << "--- LOADING " << absl::GetFlag(FLAGS_screen_layout_file)
              << "\n";
    screen_layout = YAML::LoadFile(absl::GetFlag(FLAGS_screen_layout_file));
  } else {
    std::cout << "--- LAYOUT NOT DEFINED\n";
  }

  std::cout << "screen_layout\n" << screen_layout << "\n";
  FltkDriver driver(argc, argv, screen_layout["num_strips"].as<int>(),
                    screen_layout["num_pixels_per_strip"].as<int>());
  Animation animation(&driver);
  animation.StartThread();
  driver.Run();
}

