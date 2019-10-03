# System configuration:
# sudo apt-get install bazel
# sudo apt-get install gcc-aarch64-linux-gnu gcc-9-arm-linux-gnueabihf gcc-8-arm-linux-gnueabi
# Build command:
# bazel build --platforms=beaglebone :all
#platform(
#  name="beaglebone",
#  constraint_values = [
#     "@bazel_tools//platforms:arm",
#     "@bazel_tools//platforms:linux",
#  ],
#)

cc_library(name="pru",
  srcs=[
    "pru.c",
  ],
  hdrs = [
    "pru.h",
  ],
)
