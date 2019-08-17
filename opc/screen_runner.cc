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

ABSL_FLAG(std::string, layout, "", "Name of the screen layout");
ABSL_FLAG(std::string, layout_file, "",
          "File containing screen layout in yaml");

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  YAML::Node layout;
  if (!absl::GetFlag(FLAGS_layout).empty()) {
    std::cout << "--- LOADING " << absl::GetFlag(FLAGS_layout) << "\n";
    layout = YAML::Load(absl::GetFlag(FLAGS_layout));
  } else if (!absl::GetFlag(FLAGS_layout_file).empty()) {
    std::cout << "--- LOADING " << absl::GetFlag(FLAGS_layout_file)
              << "\n";
    layout = YAML::LoadFile(absl::GetFlag(FLAGS_layout_file));
  } else {
    std::cout << "--- LAYOUT NOT DEFINED\n";
  }

  std::cout << "screen layout\n" << layout << "\n";
  FltkDriver driver(argc, argv, layout["num_strips"].as<int>(),
                    layout["num_pixels_per_strip"].as<int>());
  // TODO(gsasha): switch to exceptions.
  if (!driver.LoadLayout(layout["layout"])) {
    std::cerr << "Failed loading layout";
    return 1;
  }

  Animation animation(&driver);
  animation.StartThread();
  driver.Run();
}

