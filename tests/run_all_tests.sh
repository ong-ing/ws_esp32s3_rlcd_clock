#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ "$(uname)" = Darwin ]; then
  sdk=$(xcrun --show-sdk-path)
  export CPLUS_INCLUDE_PATH="$sdk/usr/include/c++/v1${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
fi
for name in model tools preferences weather storage; do
  sh "$root/tests/run_${name}_tests.sh"
done
python3 "$root/tests/config_test.py"
