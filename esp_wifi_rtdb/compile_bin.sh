#!/bin/bash

# Script to compile ESP8266 or ESP32 firmware into a .bin file for OTA update

SKETCH="esp_wifi_rtdb.ino"
ESP8266_FQBN="esp8266:esp8266:nodemcuv2"
ESP32_FQBN="esp32:esp32:esp32"
BUILD_DIR="./build"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

usage() {
    echo "Usage: $0 [esp8266|esp32]"
    exit 1
}

if [ "$1" == "" ]; then
    usage
fi

BOARD_TYPE=$1

if [ "$BOARD_TYPE" == "esp8266" ]; then
    FQBN=$ESP8266_FQBN
elif [ "$BOARD_TYPE" == "esp32" ]; then
    FQBN=$ESP32_FQBN
else
    echo -e "${RED}Error: Invalid board type. Use 'esp8266' or 'esp32'.${NC}"
    usage
fi

echo -e "${GREEN}Compiling for $BOARD_TYPE ($FQBN)...${NC}"
mkdir -p $BUILD_DIR

arduino-cli compile --fqbn $FQBN --output-dir $BUILD_DIR $SKETCH

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Compilation successful!${NC}"
    echo -e "Binary file(s) saved in ${GREEN}$BUILD_DIR/${NC}:"
    ls -lh $BUILD_DIR/*.bin
else
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi
