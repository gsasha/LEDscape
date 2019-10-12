package(default_visibility = ["//visibility:public"])


cc_library(
  name="rate-data",
  srcs=["rate-data.cc"],
  hdrs=["rate-data.h"],
)

cc_library(
  name="server-pru",
  srcs=["server-pru.cc"],
  hdrs=["server-pru.h"],
)

cc_library(
  name="driver",
  srcs = [
    "animation.cc",
    "driver.cc",
    "effect.cc",
    "render.cc",
  ],
  hdrs=[
    "animation.h",
    "driver.h",
    "effect.h",
    "render.h",
  ],
  deps = [
    "@system//:libfltk",
    "@system//:libyaml",
    ":rate-data",
  ],
  linkopts = [
    "-pthread",
  ],
)

cc_library(
  name="fltk_driver",
  srcs = [
    "fltk-driver.cc",
  ],
  hdrs = [
    "fltk-driver.h",
  ],
  deps = [
    ":driver",
  ]
)

cc_library(
  name="ledscape_driver",
  srcs = [
    "ledscape_driver.cc",
    "server-config.cc",
    "server-error.cc",
    "server-pru.cc",
    "color.cc",
  ],
  hdrs = [
    "ledscape_driver.h",
    "server-config.h",
    "server-error.h",
    "server-pru.h",
    "color.h",
  ],
  deps = [
    "@cesanta_frozen//:frozen",
    ":driver",
    "//ledscape:ledscape",
  ]
)

cc_binary(
  name="screen_runner",
  srcs=["screen_runner.cc"],
  deps=[
    "@com_google_absl//absl/flags:flag",
    "@com_google_absl//absl/flags:parse",
    "@system//:libyaml",
    ":fltk_driver",
  ],
  linkopts = [
    '-lyaml-cpp',
  ],
)
