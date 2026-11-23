#!/bin/sh
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
idf_root=${IDF_PATH:?请先运行 get_idf}
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
cc -c "$idf_root/components/json/cJSON/cJSON.c" -o "$test_dir/cJSON.o"
c++ -std=c++17 -Wall -Wextra -Werror -I"$project_dir/main" -I"$idf_root/components/json/cJSON" \
 "$project_dir/tests/weather_test.cpp" "$project_dir/main/HaloWeatherData.cpp" "$test_dir/cJSON.o" -o "$test_dir/weather_test"
"$test_dir/weather_test"
