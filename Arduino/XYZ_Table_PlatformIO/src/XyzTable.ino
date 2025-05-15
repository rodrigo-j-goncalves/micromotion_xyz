#include "Arduino.h"
#include "Scheduler.h"

Scheduler taskControl;

void setup() {
	taskControl.Begin();
}

void loop() {
	taskControl.Loop();
}
