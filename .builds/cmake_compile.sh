#!/bin/sh
PROFILE=$1

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

# Update all cmake profiles
bash "$SCRIPT_DIR/cmake_build.sh"

# Compile current profile
if [ -z "$PROFILE" ]; then
    echo "Proile not specified, using current active one"
    if [ ! -d "$PARENT_DIR/cmake-build" ]; then
        echo "Cannot locate file $PARENT_DIR/cmake-build"
        exit 1
    fi

    cmake --build "$PARENT_DIR/cmake-build"
    exit 0
fi

# Compile given profile
CURRENT_ONE="$PARENT_DIR/cmake-build-$PROFILE"
if [ ! -d "$CURRENT_ONE" ]; then
    echo "Cannot locate file $CURRENT_ONE"
    exit 1
fi

cmake --build "$CURRENT_ONE"
