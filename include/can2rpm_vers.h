/*
V1.00 - base file
V1.01 - added error LED for no CAN messages
V1.02 - added WiFi Setup pages
V1.03 - bug: WiFi had 5 s delay on boot; removed for instantaneous start
V1.04 - PlatformIO conversion; native ESP TWAI (replaces ESP32_CAN);
        ESPUI replaced with lightweight API + LittleFS web UI
V1.05 - added Aftermarket ECU support (configurable CAN ID, byte indices, scaling);
        added EEPROM persistence for settings; added self-test mode;
        various bug fixes and optimisations

Forbes-Automotive, 2025
*/
#pragma once

#define FW_VERSION "1.05"
