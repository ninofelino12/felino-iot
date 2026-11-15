#!/bin/bash

PORT=${1:-/dev/ttyUSB0}
BAUD=${2:-115200}

echo "Opening serial monitor on port $PORT at $BAUD baud..."
echo "Press Ctrl+C or Ctrl+A, Ctrl+X (depending on tool) to exit."

arduino-cli monitor -p "$PORT" -c baudrate="$BAUD"
