 /*
 * Scheduler.cpp
 *
 *  Created on: 03/04/2020
 *      Author: Ignacio Martínez Navajas
 *      E-mail: imnavajas@coit.es
 */

/*Includes*/
#include "Scheduler.h"       /* include the declaration for this class */

Scheduler::Scheduler() {

	aStatusLed = FancyLED(STATUS_LED_PIN, LOW);
	aCLIService = CLIService();
	aMotorControl = ControlService();

}
//<<destructor>>
Scheduler::~Scheduler() {
}

/* Method TO BE CALLED IN THE SKETCH SETUP() */
void Scheduler::Begin() {

	MegaBoard::Begin();

	/* Init the Status LED */
	aStatusLed.Begin();
	aStatusLed.SetLedPulsePeriod(2000);
	aStatusLed.SetLedPulseDutyCycle(3);
	aStatusLed.PulseForever();
	aStatusLed.TurnOn();	// LED on meanwhile system initialization

	/* Init Command Line Interface */
	aCLIService.Begin();
	delay(1500);
	MegaBoard::Println("\n\n^SYSTART\n");	

	/* Init the stepper motor control */
	aMotorControl.Begin();

	/* LED off once initialization has ended */
	aStatusLed.TurnOff();

	while (Serial.available() > 0) {
		Serial.read();
	}

	// System prompt
	aCLIService.PrintPrompt();
}

void Scheduler::Loop() {
	/* Scheduled task and priorities */
	aStatusLed.Loop();
	aCLIService.Loop();
	aMotorControl.Loop();
}

