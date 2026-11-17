#!/usr/bin/env bash
set -e

FQBN="esp32:esp32:esp32"
PORT="/dev/ttyUSB0"
SKETCH_DIR="$(dirname "$0")"

echo "==> Compiling sketch for $FQBN..."
arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"

echo "==> Uploading sketch to $PORT..."
arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"

echo "==> Done!"
