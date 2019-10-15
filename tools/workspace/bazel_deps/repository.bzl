#load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def bazel_deps_repository(name):
    commit = "89a34bf761e48883abb65922443e036ee26bf696"
    url = "https://github.com/mjbots/bazel_deps/archive/{}.zip".format(commit)
    sha256 = "7f220121e919319b35dccfe592256d68ea682dceb9d3739044d1355af659b452"
    print("---sss--- Loading repository from url", url)
    archive = http_archive(
        name = "com_github_mjbots_bazel_deps",
        url = url,
        # Try the following empty sha256 hash first, then replace with whatever
        # bazel says it is looking for once it complains.
        sha256 = sha256,
        strip_prefix = "bazel_deps-{}".format(commit),
    )
    print ("---sss--- result of loading", archive)
#def bazel_deps_repository(name):
#  native.local_repository(
#    name="com_github_mjbots_bazel_deps",
#    path="/home/gsasha/work/beaglebone/third_party/bazel_deps"
#  )
