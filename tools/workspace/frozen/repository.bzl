load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cesanta_frozen_repository(name):
    commit = "ea88e40dd45015cc77f99835282d2b53112e900c"
    url = "https://github.com/cesanta/frozen/archive/{}.zip".format(commit)
    # Try the following empty sha256 hash first, then replace with whatever
    # bazel says it is looking for once it complains.
    sha256 = "a2a959a2144f0482b9bd61e67a9897df02234fff6edf82294579a4276f2f4b97"
    http_archive(
        name = "cesanta_frozen",
        url = url,
        sha256 = sha256,
        strip_prefix = "frozen-{}".format(commit),
    )
