load("//tools/workspace/rpi_bazel:repository.bzl", "rpi_bazel_repository")
#load("//tools/workspace/bazel_deps:repository.bzl", "bazel_deps_repository")

def add_default_repositories(excludes = []):
    if "rpi_bazel" not in excludes:
        rpi_bazel_repository(name = "rpi_bazel")
    #if "bazel_deps" not in excludes:
    #    bazel_deps_repository(name="bazel_deps")
