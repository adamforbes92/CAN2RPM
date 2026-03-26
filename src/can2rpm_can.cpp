#include "can2rpm_defs.h"

static void twaiReceiveTask(void* arg);

void canInit() {
    DEBUG_PRINTLN("TWAI: configuring...");

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)pinTX_CAN,    // TX → SN65HVD230 CAN_TX (pin 16)
        (gpio_num_t)pinRX_CAN,    // RX → SN65HVD230 CAN_RX (pin 17)
        TWAI_MODE_NORMAL);

    twai_timing_config_t  t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL(); // change this to listen only

    DEBUG_PRINTLN("TWAI: installing driver...");
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        DEBUG_PRINTLN("TWAI: driver install failed");
        return;
    }

    DEBUG_PRINTLN("TWAI: starting driver...");
    if (twai_start() != ESP_OK) {
        DEBUG_PRINTLN("TWAI: start failed");
        return;
    }

    DEBUG_PRINTLN("TWAI: initialised at 500 kbps");
}

void startCanReceiveTask() {
    // Keep CAN RX cooperative so WiFi/API startup and AsyncWebServer tasks are not starved.
    xTaskCreate(twaiReceiveTask, "twai_rx", 2048, NULL, 1, NULL);
}

static void twaiReceiveTask(void* arg) {
    twai_message_t rx_frame;
    while (1) {
        bool receivedFrame = false;

        while (twai_receive(&rx_frame, 0) == ESP_OK) {
            receivedFrame = true;
            onBodyRX(rx_frame);
            taskYIELD();
        }

        if (!receivedFrame) {
            vTaskDelay(pdMS_TO_TICKS(2));
        } else {
            taskYIELD();
        }
    }
    vTaskDelete(NULL);  // should never reach here
}

void onBodyRX(const twai_message_t& rx_frame) {
    CAN_DEBUG_PRINT("CAN ID: 0x");
    CAN_DEBUG_PRINT_HEX(rx_frame.identifier);
    CAN_DEBUG_PRINT("  DLC: ");
    CAN_DEBUG_PRINT(rx_frame.data_length_code);
    CAN_DEBUG_PRINT("  Data:");
    for (uint8_t i = 0; i < rx_frame.data_length_code; i++) {
        CAN_DEBUG_PRINT(' ');
        CAN_DEBUG_PRINT_HEX(rx_frame.data[i]);
    }
    CAN_DEBUG_PRINTLN();

    // Any received CAN frame counts as bus activity.
    lastCAN = millis();

    if (ecuMode == 0) {
        if (rx_frame.identifier == MOTOR1_ID && rx_frame.data_length_code >= 4) {
            // Bosch ME7.x default decode: bytes 2-3 little-endian, 0.25 rpm/bit.
            int32_t calc = (int32_t)(((rx_frame.data[3] << 8) | rx_frame.data[2]) * 0.25f);
            rpmTruth = (calc >= 0 && calc <= 9000);
            vehicleRPM = rpmTruth ? (uint16_t)calc : 0;
            DEBUG_PRINTF("vehicleRPM (OEM): %u\n", (unsigned)vehicleRPM);
        }
    } else {
        if (rx_frame.identifier == aftermarketCanId && rx_frame.data_length_code >= 8) {
            uint8_t lo = aftermarketByteLow;
            uint8_t hi = aftermarketByteHigh;
            if (lo > 7) lo = 2;
            if (hi > 7) hi = 3;

            uint16_t raw = (uint16_t)(((uint16_t)rx_frame.data[hi] << 8) | rx_frame.data[lo]);
            int64_t scaled = ((int64_t)raw * (int64_t)aftermarketMultiplierFx) / aftermarketMultiplierScale;
            int32_t calc = (int32_t)(scaled + (int64_t)aftermarketAddition);
            rpmTruth = (calc >= 0 && calc <= 9000);
            vehicleRPM = rpmTruth ? (uint16_t)calc : 0;
            DEBUG_PRINTF("vehicleRPM (Non-OEM): %u\n", (unsigned)vehicleRPM);
        }
    }
}
