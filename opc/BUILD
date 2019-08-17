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
    "fltk-driver.cc",
    "render.cc",
  ],
  hdrs=[
    "animation.h",
    "driver.h",
    "effect.h",
    "fltk-driver.h",
    "render.h",
  ],
  deps = [
    "@fltk//:libfltk",
    "@system//:libyaml",
    ":rate-data",
  ],
  linkopts = [
    "-pthread",
  ],
)

cc_binary(
  name="screen_runner",
  srcs=["screen_runner.cc"],
  deps=[
    "@com_google_absl//absl/flags:flag",
    "@com_google_absl//absl/flags:parse",
    "@system//:libyaml",
    ":driver",
  ],
  linkopts = [
    '-lyaml-cpp',
  ],
)
