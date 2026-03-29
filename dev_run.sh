#!/bin/sh
#
# Development script to run your program locally using build-agent folder.
# This is for local development - the official your_program.sh uses build folder.
#

set -e # Exit early if any commands fail

# Build using build-agent folder
(
  cd "$(dirname "$0")"
  cmake -B build-agent -S . \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake 2>/dev/null || true
  cmake --build ./build-agent
)

# Run the interpreter
exec $(dirname "$0")/build-agent/interpreter "$@"
