#!/bin/sh
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
c++ -std=c++17 -Wall -Wextra -pedantic -I"$project_dir/main" "$project_dir/tests/tools_test.cpp" "$project_dir/main/HaloTools.cpp" "$project_dir/main/HaloPreferences.cpp" "$project_dir/main/HaloModel.cpp" -o "$test_dir/tools_test"
"$test_dir/tools_test"
