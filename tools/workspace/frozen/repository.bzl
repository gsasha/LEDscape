load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cesanta_frozen_repository(name):
    commit = "ea88e40dd45015cc77f99835282d2b53112e900c"
    url = "https://github.com/cesanta/frozen/archive/{}.zip".format(commit)
    # Try the following empty sha256 hash first, then replace with whatever
    # bazel says it is looking for once it complains.
    sha256 = "44b0a18c4ceb1d3be4b2c589dcbbbbfb367afdc6ce7eee5e0b17be8abbfa05aa"
    http_archive(
        name = "cesanta_frozen",
        url = url,
        sha256 = sha256,
        strip_prefix = "frozen-{}".format(commit),
    )
