/**
 * ===============================================================
 *  main.ino
 *  XYZ Camera Positioning System - Main Application Entry Point
 * ===============================================================
 *  Description:
 *  - Entry point for the Arduino firmware.
 *  - Initializes the system and continuously runs scheduled tasks.
 *  - Delegates startup and runtime operations to the Scheduler.
 *
 *  Created on: [Insert current date]
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#include "Arduino.h"
#include "Scheduler.h"

// Create an instance of the main scheduler
Scheduler taskControl;

void setup() {
    // Initialize all subsystems (CLI, motors, status LED, etc.)
    taskControl.Begin();
}

void loop() {
    // Continuously run the main task loop
    taskControl.Loop();
}
