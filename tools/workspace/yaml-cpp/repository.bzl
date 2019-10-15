load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def yaml_cpp_repository(name):
    commit = "c9a00770241b6dfd5e68df4574cf488822cfef14"
    sha256 = "00345e722107cfc03739bd944b56d3c0592fc0f0ce84ecb2da8a6c9056964d94"
    http_archive(
        name = name,
        urls = [
          "https://github.com/jbeder/yaml-cpp/archive/{}.zip".format(commit),
        ],
        sha256 = sha256,
        strip_prefix = "yaml-cpp-{}".format(commit),
    )
