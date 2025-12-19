#!/bin/bash

SCRIPT_DIR="$(dirname "$0")"

# Parse command line arguments
DEBUG=false
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            DEBUG=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--debug|-d]"
            exit 1
            ;;
    esac
done

if [ "$DEBUG" = true ]; then
    "$SCRIPT_DIR/build/Omnify_artefacts/Debug/Standalone/Omnify.app/Contents/MacOS/Omnify"
else
    "$SCRIPT_DIR/build/Omnify_artefacts/Standalone/Omnify.app/Contents/MacOS/Omnify"
fi
