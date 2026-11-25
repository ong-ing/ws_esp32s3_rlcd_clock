#!/bin/sh
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
c++ -std=c++17 -Wall -Wextra -pedantic -I"$project_dir/main" "$project_dir/tests/model_test.cpp" "$project_dir/main/ClockModel.cpp" "$project_dir/main/FileBrowser.cpp" "$project_dir/tests/fakes/CardStorage.cpp" "$project_dir/main/ClockTools.cpp" "$project_dir/main/ClockPreferences.cpp" -o "$test_dir/model_test"
"$test_dir/model_test"
