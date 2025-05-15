 /*
 * Scheduler.h
 *
 *  Created on: 03/04/2020
 *      Author: Ignacio Martínez Navajas
 *      E-mail: imnavajas@coit.es
 */



#ifndef SCHEDULER_H
#define SCHEDULER_H

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "MegaBoard.h"
#include "FancyLED.h"
#include "CLIService.h"
#include "ControlService.h"

#define STATUS_LED_PIN 13

class Scheduler {
public:
	Scheduler();
	~Scheduler();
	void Begin(void);
	void Loop(void);
private:

	FancyLED aStatusLed;
	CLIService aCLIService;
	ControlService aMotorControl;
};
#endif
