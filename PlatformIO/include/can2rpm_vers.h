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
V1.06 - standardised Forbes Automotive UI theme (shared style.css);
        adopted common wifi_manager (mDNS: can2rpm.local) and ota_manager
        (firmware + filesystem OTA via /api/ota, /api/ota/fs); per-product
        cache-busting on web assets
V1.07 - OTA overhaul (shared ota_manager / wifi_manager v2 + data/ota.js, ported
        from OpenHaldex 9.00): upload callbacks no longer answer mid-body (the
        old per-chunk "200 OK" made the browser drop the connection after the
        first 1.4 kB - a crash in AsyncTCP and a half-written partition, so no
        OTA through the UI had ever completed); filesystem updates unmount
        first, check the announced size, verify the mount and wipe on failure;
        boot only mounts a sane superblock and the web server always starts -
        with no usable UI "/" is a recovery page with the two uploads.
        "Update from GitHub" on the OTA tab (Releases/releases.json via
        tools/make_release.py) plus a Home WiFi (bridge mode) card; power_manager
        holds WiFi up while any browser is active. Assets served no-cache (ETag)
        instead of the hand-bumped ?v=.

Forbes-Automotive, 2025
*/
#pragma once

#define FW_VERSION "1.07"
