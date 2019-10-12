#load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def abseil_repository(name):
    commit = "aa844899c937bde5d2b24f276b59997e5b668bde"
    url = "https://github.com/abseil/abseil-cpp/archive/{}.zip".format(commit)
    # Try the following empty sha256 hash first, then replace with whatever
    # bazel says it is looking for once it complains.
    sha256 = "f1a959a2144f0482b9bd61e67a9897df02234fff6edf82294579a4276f2f4b97"
    http_archive(
        name = "com_google_absl",
        url = url,
        sha256 = sha256,
        strip_prefix = "abseil-cpp-{}".format(commit),
    )
