cc_library(
  name="ledscape",
  visibility=["//visibility:public"],
  srcs = [
    "ledscape.c",
  ],
  hdrs = [
    "ledscape.h",
  ],
  deps = [
    ":pru"
  ],
)

cc_library(name="pru",
  srcs=[
    "pru.c",
    "util.c",
  ],
  hdrs = [
    "pru.h",
    "util.h",
  ],
  deps = [
    "//am335x/app_loader:prussdrv",
  ],
)

