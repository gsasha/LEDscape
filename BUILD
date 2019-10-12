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

load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar")

genrule(
  name="pru_bin",
  srcs = glob([
    "Makefile",
    "am335x/pasm/*",
    "pru/*",
    "pru/mappings/*",
    "pru/templates/*",
  ]),
  outs = [
    "ws281x-original-ledscape-pru0.bin",
    "ws281x-original-ledscape-pru1.bin",
  ],
  cmd = "find .; make -f $(location Makefile) all_pru_templates;\
 cp pru/bin/ws281x-original-ledscape-pru0.bin $(location ws281x-original-ledscape-pru0.bin);\
 cp pru/bin/ws281x-original-ledscape-pru1.bin $(location ws281x-original-ledscape-pru1.bin);\
 find .",
)

cc_binary(
  name="opc-server",
  srcs=[
    "opc-server.cc",
  ],
  deps = [
    "@yaml-cpp//:yaml-cpp",
    "@cesanta_frozen//:frozen",
    "@cesanta_mongoose//:mongoose",
    "@org_llvm_libcxx//:libcxx",
    "//ledscape:ledscape",
    "//opc:ledscape_driver",
  ],
  linkstatic=1,
)
# run with /home/gsasha/bin/bazel build --config=pi :deploy
pkg_tar(
    name = "deploy",
    extension = "tar",
    include_runfiles = True,
    package_dir = "ledagent",
    srcs = [
        ":opc-server",
        ":pru_bin",
    ],
    files = {
        "//opc:emulator-layout-rectangle.yaml": "opc/emulator-layout-rectangle.yaml",
        "//opc:emulator-layout-screens.yaml": "opc/emulator-layout-screens.yaml",
#        "//:pru_bin/ws281x-original-ledscape-pru0.bin": "pru/bin/ws281x-original-ledscape-pru0.bin",
#        "//:pru_bin/ws281x-original-ledscape-pru1.bin": "pru/bin/ws281x-original-ledscape-pru1.bin",
    },
)

