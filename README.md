# CAN2RPM

ESP32-based converter that reads engine RPM from a CAN bus and outputs a traditional coil-type signal for use with MK1/MK2 analog tachometers.

Built around the ESP32 DevKit V1, it captures VAG/ME7.x CAN messages and translates them into a high-voltage square-wave RPM output suitable for clusters that expect a coil ignition pulse.

**Current firmware version: 1.06** — adopts Forbes Automotive's shared Wi‑Fi AP/mDNS manager, two-step OTA updater and Ignitron dark theme, common across the ESP32 product line.


![CAN2RPM Web UI — dashboard, configuration, advanced, status, aftermarket ECU and OTA](/Images/can2rpmUI.png)

---

## What it does

- Reads **engine RPM** from the chassis CAN bus (500 kbit/s TWAI)
- Outputs a **coil-type RPM signal** on a dedicated pin (high-voltage square wave)
- Supports **aftermarket CAN IDs** with configurable byte mapping and scaling
- Optional **needle sweep** on power-up
- Hosts a **WiFi AP + web UI** for configuration and OTA updates — no laptop or serial cable needed

---

## Hardware

| Function         | Detail                                      |
|------------------|---------------------------------------------|
| Board            | ESP32 DevKit V1 (`esp32doit-devkit-v1`)     |
| CAN transceiver  | SN65HVD230 (3.3 V compatible)               |
| RPM output       | Coil-type signal on GPIO 26                  |
| CAN RX / TX      | GPIO 17 / 16                                |

---

## Setup

> Connect the module as per the wiring diagram

> Connect to 'CAN2RPM' on WiFi and browse to **192.168.1.1**

> Set the RPM CAN ID and byte positions to match your ECU

> Enable needle sweep if desired

> Save — settings are stored in NVS (EEPROM)

---

## WiFi & Web UI

WiFi is enabled at all times while the unit is powered and a client is connected. Once the last client disconnects the radio turns off automatically (see Power Management below). A power-cycle restores it.

Connect to the **CAN2RPM** access point (open network, `192.168.1.1`) and browse to **192.168.1.1** or **can2rpm.local**.


| Tab | Cards |
|---|---|
| **Dashboard** | **Live Data** — engine RPM and coil output frequency as dial gauges; **System Status** — CAN bus and Test Mode pills; **Display Options** — gauge or numeric per readout |
| **Configuration** | **Needle Sweep** (enable on start-up, ms per step, test button); **Cluster RPM Limit** — the vehicle RPM that maps to full output; **Output Frequency Scaling** — the maximum coil output frequency at full RPM |
| **Advanced** | **ECU Source** — Bosch ME7.x (VAG) or Aftermarket ECU; **Aftermarket ECU Specifics** (shown when Aftermarket is selected) — CAN ID, low / high byte, multiplier and addition; **RPM Output Test** — override the CAN input with a fixed RPM for calibration; **Live Diagnostics** — CAN RPM, output Hz, CAN status and RPM validity |
| **OTA** | Device information and the two-step updater — see below |

<p align="center">
  <img src="/Images/ui-dashboard.png" alt="Dashboard — engine RPM and output frequency gauges" width="300">
  &nbsp;&nbsp;
  <img src="/Images/ui-system-status.png" alt="Dashboard — system status pills (CAN bus, test mode) and display options" width="300">
</p>
<p align="center">
  <img src="/Images/ui-configuration.png" alt="Configuration — needle sweep, cluster RPM limit and output frequency scaling" width="300">
  &nbsp;&nbsp;
  <img src="/Images/ui-advanced.png" alt="Advanced — ECU source, RPM output test and live diagnostics" width="300">
</p>
<p align="center">
  <img src="/Images/ui-aftermarket.png" alt="Advanced — ECU source set to Aftermarket ECU, revealing the CAN ID, byte positions, multiplier and addition fields" width="300">
</p>

*Setting the ECU type to **Aftermarket ECU** reveals the byte-mapping card: pick the CAN ID (hex), which two bytes carry RPM, and the multiplier / addition that turn the raw value into RPM.*

### OTA Updates (Two-Step)

CAN2RPM uses Forbes Automotive's shared OTA module. Firmware and web UI are on separate flash partitions, so an update is done in **two steps** from the OTA tab:

1. **Filesystem** — upload `littlefs.bin` (`POST /api/ota/fs`). Updates the web UI (`index.html`, `app.js`, `style.css`). The device does **not** reboot after this step.
2. **Firmware** — upload `firmware.bin` (`POST /api/ota`). Updates the application code and reboots automatically once complete.

The OTA tab shows a "1 Filesystem / 2 Firmware" step indicator so it's clear which upload is in progress. `GET /api/ota/info` reports the running version, board and hardware.

> Download the most recent release `.bin` file(s) from GitHub, connect to CAN2RPM on WiFi, open the OTA tab at 192.168.1.1 (or can2rpm.local), and upload the filesystem image followed by the firmware image.


<p align="center">
  <img src="/Images/ui-ota.png" alt="OTA tab — device information and two-step update" width="300">
</p>

---

## Status Indicators

The web UI uses Forbes Automotive's shared dark theme (the same look used across OpenHaldex, Can2Cluster, SpeedPulser, SpeedPulserPro, the MQB Steering Wheel Controller and AirLift Controller), with a consistent colour convention for status pills and badges:

| Colour | Meaning in CAN2RPM |
|---|---|
| 🟢 Green | CAN bus healthy and the displayed RPM/Hz values are live, real data |
| 🔴 Red | CAN bus unhealthy / no signal, or the decoded RPM value isn't yet verified |
| 🟠 Orange | **Test mode is active** — the RPM/Hz readouts are synthetic values generated by the built-in diagnostic test, not real CAN data |

Orange here specifically means "you're looking at test data," so always confirm the Test Mode pill is off before trusting a live reading.

---

## Power Management

The firmware includes a universal reduced-power module that automatically saves current once no clients are connected to the WiFi AP. This directly reduces heat generated by the on-board linear voltage regulator — a linear regulator dissipates heat proportional to `(Vin − Vout) × I_load`, so every milliamp saved is less heat.

**What happens 1 minute after the last client disconnects:**
> WiFi radio off — the single biggest saving (~80–120 mA average)

> CPU frequency scaled: 240 MHz → 80 MHz

> Bluetooth controller disabled at boot (frees ~60 KB RAM)

> Onboard LED off at boot

Everything restores automatically the moment a client reconnects. A power-cycle (ignition off/on) will also bring WiFi back if required.

---

## Build & Flash

PlatformIO project (`platformio.ini`):

```bash
# Build firmware
pio run

# Upload firmware (USB)
pio run -t upload

# Upload web UI (LittleFS)
pio run -t uploadfs
```

---

## Disclaimer

This drives a tachometer but should always be assumed to have some inaccuracy. Forbes Automotive accepts no responsibility for speed or RPM related incidents.
