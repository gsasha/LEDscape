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
  name="effect",
  srcs=["effect.cc"],
  hdrs=["effect.h"],
)

cc_library(
  name="driver",
  hdrs=["driver.h"],
)

cc_binary(
  name="screen_runner",
  srcs=["screen_runner.cc"],
  deps=[
    "@fltk//:libfltk",
    ":driver",
  ],
)
