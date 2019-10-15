# This might be interesting: https://bloggerbust.ca/post/adding-a-dependency-based-on-autotools-to-a-bazel-project/

workspace(name="ledagent")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

new_local_repository(
  name = "system",
  path = "/usr/lib/x86_64-linux-gnu",
  build_file_content = """
cc_library(
    name = "libfltk",
    srcs = ["libfltk.so"],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "libyaml",
    srcs = ["libyaml-cpp.so"],
    visibility = ["//visibility:public"],
)
""",
)

#http_archive(
#   name = "rules_foreign_cc",
#   strip_prefix = "rules_foreign_cc-a3593905f73ce19c09d21f9968f1d3f5bc115157",
#   url = "https://github.com/bazelbuild/rules_foreign_cc/archive/a3593905f73ce19c09d21f9968f1d3f5bc115157.zip",
#   sha256 = "6f3484eacc172c90d605e79130f9f01ec827a98b99c499c396eddc597a9c219d"
#)

# from https://github.com/mjbots/rpi_bazel
load("//tools/workspace:default.bzl", "add_default_repositories")

add_default_repositories()
load("@rpi_bazel//tools/workspace:default.bzl",
     rpi_bazel_add = "add_default_repositories")
rpi_bazel_add()

#load("@com_github_mjbots_bazel_deps//tools/workspace:default.bzl",
#     bazel_deps_add = "add_default_repositories")
#bazel_deps_add()

