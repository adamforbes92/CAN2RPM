#pragma once

// Starts all background RTOS tasks.
// Call once from setup() after all peripherals are initialised.
void startTasks();

// Temporarily pause/resume output update tasks during manual sweeps.
void suspendOutputTasks();
void resumeOutputTasks();
