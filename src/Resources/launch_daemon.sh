#!/bin/bash
# Launcher script for daemomnify daemon
# Redirects stdout/stderr to a log file to avoid pipe buffer blocking
# Usage: launch_daemon.sh <temp_dir> <port> <command> [args...]

TEMP_DIR="$1"
PORT="$2"
shift 2
LOG_FILE="${TEMP_DIR}/daemomnify-${PORT}.log"
export PYTHONUNBUFFERED=1
exec "$@" >> "$LOG_FILE" 2>&1
