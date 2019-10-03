load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


def rpi_bazel_repository(name):
    commit = "000b9c7ff87e58454fbb96849c3be5ac7461ca5a"
    http_archive(
        name = name,
        url = "https://github.com/mjbots/rpi_bazel/archive/{}.zip".format(commit),
        sha256 = "5e567936abefe572a38a507222dd4b7952ebe9ae530069bde855d6de3c51aee0",
        strip_prefix = "rpi_bazel-{}".format(commit),
    )


