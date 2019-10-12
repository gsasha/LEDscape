load("//tools/workspace/abseil:repository.bzl", "abseil_repository")
load("//tools/workspace/mongoose:repository.bzl", "mongoose_repository")
load("//tools/workspace/rpi_bazel:repository.bzl", "rpi_bazel_repository")
load("//tools/workspace/bazel_deps:repository.bzl", "bazel_deps_repository")
load("//tools/workspace/yaml-cpp:repository.bzl", "yaml_cpp_repository")

def add_default_repositories(excludes = []):
    if "rpi_bazel" not in excludes:
        rpi_bazel_repository(name = "rpi_bazel")
    if "bazel_deps" not in excludes:
        bazel_deps_repository(name="bazel_deps")
    if "abseil" not in excludes:
        abseil_repository(name = "com_google_absl")
    if "mongoose" not in excludes:
        mongoose_repository(name = "mongoose")
    if "yaml-cpp" not in excludes:
        yaml_cpp_repository(name = "yaml-cpp")
