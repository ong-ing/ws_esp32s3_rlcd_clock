#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
idf_root=${IDF_PATH:?请先加载 ESP-IDF 环境}
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
cc -c "$idf_root/components/json/cJSON/cJSON.c" -o "$test_dir/cJSON.o"
python3 "$root/tools/embed_config.py" "$root/device.config.json" "$test_dir/DefaultConfig.h"
for target in boot_config boot_storage; do
  extra=""
  if [ "$target" = boot_storage ]; then extra="$root/main/BootConfigStore.cpp"; fi
  c++ -std=c++17 -Wall -Wextra -Werror -I"$root/main" -I"$test_dir" -I"$idf_root/components/json/cJSON" "$root/tests/${target}_test.cpp" "$root/main/DeviceConfig.cpp" ${extra:+"$extra"} "$test_dir/cJSON.o" -o "$test_dir/$target"
  "$test_dir/$target"
done
c++ -std=c++17 -Wall -Wextra -Werror -I"$root/main" "$root/tests/file_browser_test.cpp" "$root/main/FileBrowser.cpp" -o "$test_dir/files"
"$test_dir/files"
