cc_library(
  name = "pasmlib",
  srcs = [
    "pasm.c",
    "pasmdot.c",
    "pasmexp.c",
    "pasmmacro.c",
    "pasmop.c",
    "pasmpp.c",
    "pasmstruct.c",
  ],
  hdrs = [
    "pasmdbg.h",
    "pasm.h",
    "pru_ins.h",
  ]
)

cc_binary(
  name = "pasm",
  srcs = [
    "pasm.c",
  ],
  deps = [
    ":pasmlib",
  ],
)
