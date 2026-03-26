#include "can2rpm_defs.h"
#include "can2rpm_tasks.h"

static void taskCheckError(void*) {
    while (1) {
        if ((millis() - lastCAN) > 500) {
            canHealthy = false;
            hasError   = true;
            vehicleRPM = 0;
            rpmTruth   = false;
        } else {
            canHealthy = true;
            hasError   = false;
        }
        checkError();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ─────────────────────────────────────────────────────────────────
// taskEEP  (every eepRefresh ms)
// Persists current settings to NVS.
// ─────────────────────────────────────────────────────────────────
static void taskEEP(void*) {
    while (1) {
        writeEEP();
        vTaskDelay(pdMS_TO_TICKS(eepRefresh));
    }
}

// ─────────────────────────────────────────────────────────────────
// taskCheckConnections  (every 2 s)
// Drives the WiFi-client-present indicator via checkConnections().
// ─────────────────────────────────────────────────────────────────
static void taskCheckConnections(void*) {
    while (1) {
        checkConnections();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void taskUpdateRPM(void*) {
    while (1) {
        if (clusterRPMLimit > 0) {
            if (tempDiagTest) {
                frequencyRPM = map((long)tempRPM, 0, (long)clusterRPMLimit, 0, (long)maxRPM);
            } else {
                frequencyRPM = map((long)(uint16_t)vehicleRPM, 0, (long)clusterRPMLimit, 0, (long)maxRPM);
            }
        } else {
            frequencyRPM = 0;
        }

        setFrequencyRPM(frequencyRPM);
        vTaskDelay(pdMS_TO_TICKS(rpmDelay));
    }
}

void suspendOutputTasks() {
    if (updateRPMHandle != NULL) {
        vTaskSuspend(updateRPMHandle);
    }
}

void resumeOutputTasks() {
    if (updateRPMHandle != NULL) {
        vTaskResume(updateRPMHandle);
    }
}

void startTasks() {
    // Give WiFi/Preferences tasks larger stacks to avoid canary trips.
    xTaskCreate(taskCheckError,       "chkErr", 2048, NULL, 1, NULL);
    xTaskCreate(taskEEP,              "eep",    3072, NULL, 2, NULL);
    xTaskCreate(taskCheckConnections, "wifiCk", 4096, NULL, 3, NULL);
    startCanReceiveTask();
    xTaskCreate(taskUpdateRPM,        "rpmUpd", 2048, NULL, 5, &updateRPMHandle);
}
