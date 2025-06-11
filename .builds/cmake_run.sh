#!/bin/bash

PROFILE=$1
EXEC_ARGS=$2

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

CURRENT_ONE=""
META_FILE=""
if [ -z "$PROFILE" ]; then
    META_FILE="$PARENT_DIR/.clangd"
    CURRENT_ONE="$PARENT_DIR/cmake-build"
    bash "$SCRIPT_DIR/cmake_compile.sh"
else
    META_FILE="$SCRIPT_DIR/profile_$PROFILE"
    CURRENT_ONE="$PARENT_DIR/cmake-build-$PROFILE"
    bash "$SCRIPT_DIR/cmake_compile.sh" "$PROFILE"
fi


declare -A meta
while IFS= read -r line; do
    if [[ "$line" =~ ^#([a-zA-Z0-9_]+):[[:space:]]*(.*) ]]; then
        key="${BASH_REMATCH[1]}"
        value="${BASH_REMATCH[2]}"
        meta["$key"]="$value"
    fi
done < "$META_FILE"

# Read build metadata
BUILD_EXEC="${meta[build_exec]}"
unset meta

EXEC_PATH="$CURRENT_ONE/$BUILD_EXEC"

echo "Starting executable: $EXEC_PATH"
env -C "$CURRENT_ONE" $EXEC_PATH
