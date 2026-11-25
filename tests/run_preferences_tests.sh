#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
c++ -std=c++17 -Wall -Wextra -pedantic -I"$root/main" "$root/tests/preferences_test.cpp" "$root/main/ClockPreferences.cpp" "$root/main/ClockTools.cpp" "$root/main/ClockModel.cpp" "$root/main/FileBrowser.cpp" "$root/tests/fakes/CardStorage.cpp" -o "$test_dir/preferences"
"$test_dir/preferences"
c++ -std=c++17 -Wall -Wextra -pedantic -I"$root/main" "$root/tests/weather_schedule_test.cpp" -o "$test_dir/schedule"
"$test_dir/schedule"
c++ -std=c++17 -Wall -Wextra -pedantic -I"$root/tests/fakes" -I"$root/main" "$root/tests/settings_storage_test.cpp" -o "$test_dir/storage"
"$test_dir/storage"
