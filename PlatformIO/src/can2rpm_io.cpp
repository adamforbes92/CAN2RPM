#include "can2rpm_defs.h"
#include "can2rpm_tasks.h"

void setupTimer() {
    ledc_timer_config_t timerCfg = {};
    timerCfg.speed_mode      = rpmLedcMode;
    timerCfg.timer_num       = rpmLedcTimer;
    timerCfg.duty_resolution = rpmLedcResolution;
    timerCfg.freq_hz         = rpmLedcLatchFreqHz; // duty is 0 here; real rate set later
    timerCfg.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&timerCfg);

    ledc_channel_config_t chCfg = {};
    chCfg.gpio_num   = pinCoil;
    chCfg.speed_mode = rpmLedcMode;
    chCfg.channel    = rpmLedcChannel;
    chCfg.intr_type  = LEDC_INTR_DISABLE;
    chCfg.timer_sel  = rpmLedcTimer;
    chCfg.duty       = rpmLedcDutyOff;
    chCfg.hpoint     = 0;
    ledc_channel_config(&chCfg);

    ledc_stop(rpmLedcMode, rpmLedcChannel, 0); // idle low, applied immediately
}

// LEDC latches a duty change on the timer's NEXT period, and IDF 5.5.2's
// ledc_ll_set_duty_start() spins (interrupts disabled) on the previous latch:
//   while (hw->channel_group[mode].channel[ch].conf1.duty_start);
// At a few Hz that wait is hundreds of ms - past the 300 ms interrupt watchdog -
// and the board panics with "Interrupt wdt timeout". It applies to BOTH high-
// and low-speed mode on the ESP32. So: make every duty change while the timer
// is parked fast, then set the real rate (a frequency change needs no latch),
// and turn the output off with ledc_stop(), which applies immediately.
void setFrequencyRPM(long frequencyHz) {
    if (frequencyHz < 0) {
        frequencyHz = 0;
    }

    static bool outputOn = false;

    if (frequencyHz > 0) {
        uint32_t targetFreq = (uint32_t)frequencyHz;
        if (targetFreq < rpmLedcMinFreqHz) {
            targetFreq = rpmLedcMinFreqHz;
        }

        if (!outputOn) {
            // Latch 50% duty while the timer is fast, THEN drop to the real rate.
            ledc_set_freq(rpmLedcMode, rpmLedcTimer, rpmLedcLatchFreqHz);
            ledc_set_duty(rpmLedcMode, rpmLedcChannel, rpmLedcDuty50);
            ledc_update_duty(rpmLedcMode, rpmLedcChannel);
            outputOn = true;
        }
        ledc_set_freq(rpmLedcMode, rpmLedcTimer, targetFreq);
    } else if (outputOn) {
        ledc_stop(rpmLedcMode, rpmLedcChannel, 0); // idle low, applied immediately
        outputOn = false;
    }
}

void basicInit() {
#if serialDebug
    Serial.begin(baudSerial);
    delay(200);
    DEBUG_PRINTLN("CAN-BUS to RPM: initialising...");
#endif

    DEBUG_PRINTLN("Setting up pins...");
    setupPins();
    DEBUG_PRINTLN("Pins ready");

    DEBUG_PRINTLN("CAN: initialising...");
    canInit();
    DEBUG_PRINTLN("CAN: ready");
}

void setupPins() {
    pinMode(onboardLED, OUTPUT);
    pinMode(pinCoil,    OUTPUT);
}

void needleSweep() {
    if (needleSweepInProgress) {
        return; // prevent overlapping sweeps if triggered multiple times in quick succession
    }
    needleSweepInProgress = true;

    DEBUG_PRINTLN("Needle sweep: starting...");

    // Suspend periodic output writes so sweep owns the output path.
    suspendOutputTasks();

    const uint16_t effectiveSweepSpeed = sweepSpeed > 0 ? sweepSpeed : 1;
    const uint16_t settleDelayMs = effectiveSweepSpeed * 5;

    long currentRPM = 0;

    frequencyRPM = 0;
    setFrequencyRPM(0);
    delay(settleDelayMs);
    delay(effectiveSweepSpeed);

    while (currentRPM < (long)maxRPM) {
        currentRPM++;
        frequencyRPM = currentRPM;
        setFrequencyRPM(currentRPM);
        delay(effectiveSweepSpeed);
    }

    delay((int)effectiveSweepSpeed * 50);   // pause at top of sweep

    while (currentRPM > 0) {
        currentRPM--;
        frequencyRPM = currentRPM;
        setFrequencyRPM(currentRPM);
        delay(effectiveSweepSpeed);
    }

    frequencyRPM = 0;
    setFrequencyRPM(0);
    delay(effectiveSweepSpeed);

    resumeOutputTasks();
    needleSweepInProgress = false;

    DEBUG_PRINTLN("Needle sweep: complete");
}

void checkError() {
    if (hasError) {
        triggerLED = !triggerLED;
    } else {
        triggerLED = false;
    }
    digitalWrite(onboardLED, triggerLED ? HIGH : LOW);
}
