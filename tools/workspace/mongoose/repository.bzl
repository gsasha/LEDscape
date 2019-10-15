load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cesanta_mongoose_repository(name):
    commit = "c41a22195ceabc02ffd0379f0e71d6c3575337aa"
    url = "https://github.com/cesanta/mongoose/archive/{}.zip".format(commit)
    # Try the following empty sha256 hash first, then replace with whatever
    # bazel says it is looking for once it complains.
    sha256 = "868f214f8e7f80ac6a20963a56c1fde13e2733abd4d6a8bb52ca69e055ad5721"
    http_archive(
        name = "cesanta_mongoose",
        url = url,
        sha256 = sha256,
        strip_prefix = "mongoose-{}".format(commit),
        build_file = Label("//tools/workspace/mongoose:package.BUILD"),
    )
