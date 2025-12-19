#!/bin/bash

# Shared script to lint, fix, and format C++ files
# Silent on success, prints red error on failure
# By default, fails on errors. Use --no-fail to always exit 0 (for hooks).

FAIL_FAST=true
WARNINGS_AS_ERRORS=false
FILE_PATH=""

for arg in "$@"; do
    case $arg in
        --no-fail)
            FAIL_FAST=false
            ;;
        --warnings-as-errors)
            WARNINGS_AS_ERRORS=true
            ;;
        *)
            FILE_PATH="$arg"
            ;;
    esac
done

# Skip files in submodules
if [[ "$FILE_PATH" == *"/JUCE/"* ]]; then
    exit 0
fi

if [ "$FAIL_FAST" = true ]; then
    set -euo pipefail
fi

# Check if the file is a C++ file
if [[ "$FILE_PATH" =~ \.(cpp|h|hpp)$ ]]; then
    # For header files, run clang-tidy on the corresponding .cpp file instead
    # This gives clang-tidy the compile commands it needs (headers aren't in compile_commands.json)
    # clang-tidy will also check the header since it's included by the .cpp
    TIDY_TARGET="$FILE_PATH"
    if [[ "$FILE_PATH" =~ \.h$ ]]; then
        CPP_FILE="${FILE_PATH%.h}.cpp"
        if [[ -f "$CPP_FILE" ]]; then
            TIDY_TARGET="$CPP_FILE"
        fi
    fi

    # Step 1: Run clang-tidy with auto-fixes (uses .clang-tidy config file and compile_commands.json)
    # Note: We target arm64 specifically since universal builds create multiple compiler jobs
    SDK_PATH=$(xcrun --show-sdk-path)

    # Find the build directory - it's always at src/build regardless of where the source file is
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    REPO_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
    BUILD_DIR="$REPO_ROOT/src/build"

    TIDY_ARGS=(
        "$TIDY_TARGET"
        -p "$BUILD_DIR"
        --fix
        --extra-arg=-target --extra-arg=arm64-apple-macosx
        --extra-arg=-isysroot --extra-arg="$SDK_PATH"
    )
    if [ "$WARNINGS_AS_ERRORS" = true ]; then
        # Don't use --fix-errors here so exit code reflects actual errors
        TIDY_ARGS+=(--warnings-as-errors='*')
    else
        # Use --fix-errors to apply fixes even with compile errors
        TIDY_ARGS+=(--fix-errors)
    fi

    # Temporarily disable exit-on-error to capture clang-tidy output even on failure
    set +e
    TIDY_OUTPUT=$(/opt/homebrew/opt/llvm/bin/clang-tidy "${TIDY_ARGS[@]}" 2>&1)
    TIDY_EXIT=$?
    if [ "$FAIL_FAST" = true ]; then
        set -e
    fi

    # Step 2: Run clang-format on the original file
    clang-format -i "$FILE_PATH"

    # If warnings-as-errors mode and clang-tidy failed, exit with code 2 to block
    if [ "$WARNINGS_AS_ERRORS" = true ] && [ $TIDY_EXIT -ne 0 ]; then
        # Filter to only show errors from user code (not "X warnings generated" noise)
        FILTERED_OUTPUT=$(echo "$TIDY_OUTPUT" | grep -E ": (error|warning|note):" -A 10)
        echo "clang-tidy errors:" >&2
        echo "$FILTERED_OUTPUT" >&2
        exit 2
    fi
fi
