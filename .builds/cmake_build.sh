#!/bin/bash

# This build script expects 'profile_$PROFILE$' files located in a same directory.
#
# Each profile file is effectivelly '.clangd' file that should have specified:
# ---
#   CompileFlags:
#     CompilationDatabase: cmake-build-$PROFILE$
# ---
#
# Also each file should have provided build metadata in a form of Key-Value comments:
# ---
#   #build_exec: $exec_name$
#   #build_type: Debug|Release|etc...
#   #build_args: -DArg1=Val1 -DArg2=Val2 ...
# ---
PROFILE=$1

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

# shopt -s nullglob
for file in "$SCRIPT_DIR"/profile_*; do
    if [ -f "$file" ]; then
        basename=$(basename "$file")
        profile="${basename#profile_*}"

        # ================================================================
        declare -A meta

        while IFS= read -r line; do
            if [[ "$line" =~ ^#([a-zA-Z0-9_]+):[[:space:]]*(.*) ]]; then
                key="${BASH_REMATCH[1]}"
                value="${BASH_REMATCH[2]}"
                meta["$key"]="$value"
            fi
        done < "$file"

        # Read build metadata
        BUILD_ARGS="${meta[build_args]}"
        BUILD_TYPE="${meta[build_type]}"

        unset meta
        # ================================================================

        echo "Update cmake profile: $profile"
        echo "Build type: $BUILD_TYPE"
        echo "Build args: $BUILD_ARGS"

        cmake -S "$PARENT_DIR" -B "$PARENT_DIR/cmake-build-$profile" "$BUILD_ARGS" \
              -G Ninja \
              -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
            || exit 3

        echo ""
        echo "================================================================"
        echo ""
    fi
done
# shopt -u nullglob

if [ -z "$PROFILE" ]; then
    # Use current profile
    if [ -L "$PARENT_DIR/cmake-build" ]; then
        echo "Symlink to a profile exists, skip"
        exit 0
    fi

    # Find default profile
    for f in "$SCRIPT_DIR"/profile_*; do
        if [ -f "$f" ]; then
            profb=$(basename "$f")
            PROFILE="${BASENAME#profile_*}"
            break
        fi
    done
fi

# Set active profile
if [ -n "$PROFILE" ]; then
    echo "Default cmake profile: $PROFILE"
    ln -sfn "$SCRIPT_DIR/profile_$PROFILE" "$PARENT_DIR/.clangd"
    ln -sfn "$PARENT_DIR/cmake-build-$PROFILE" "$PARENT_DIR/cmake-build"
else
    echo "No cmake profiles found"
fi
