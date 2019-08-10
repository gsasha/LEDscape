
new_local_repository(
    name = "fltk",
    # pkg-config --variable=libdir x11
    path = "/usr/lib/x86_64-linux-gnu",
    build_file_content = """
cc_library(
    name = "libfltk",
    srcs = ["libfltk.so"],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "libyaml-cpp",
    srcs = ["libyaml-cpp.so"],
    visibility = ["//visibility:public"],
)
""",
)
