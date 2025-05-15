/*
 FancyLED.h - FancyLED for Wiring/Arduino
 (cc) 2011 Carlyn Maw, Attribute, Share Alike

 Created 06 July 2011
 Version 0.1
 */

// include core Wiring API
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include "HardwareSerial.h"
#include "Cmd.h"

// include this library's description file
#include "FancyLED.h"

#define LED_FSM_READY 	0
#define LED_FSM_FIRE	1
#define LED_FSM_REST	2

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

/* Default constructor, use built in led */
FancyLED::FancyLED() {
	// Initialize the class attributes
	aLedCurrentTime = 0;

	aLed.pinNumber = 13;

	aLed.defaultOnState = HIGH;
	aLed.state = HIGH;
	aLed.pulseDelayedStart = 0;

	aLed.pulseMode = false;
	aLed.pulseForever = false;
	aLed.pulseFSM = LED_FSM_READY;
	aLed.pulseDutyCycle = 50;
	aLed.pulsePeriod = 1000;
	aLed.pulseOnTime = (aLed.pulsePeriod * aLed.pulseDutyCycle) / 100;
	aLed.pulseOffTime = (aLed.pulsePeriod - aLed.pulseOnTime);
	aLed.pulseTimer = 0;
	aLed.pulseCnt = 0;
	aLed.pulseCntMax = 0;
}

FancyLED::FancyLED(uint8_t rLedPin, bool rDefaultOnState) {

	// Initialize the class attributes
	aLedCurrentTime = 0;

	aLed.pinNumber = rLedPin;

	aLed.defaultOnState = rDefaultOnState;
	aLed.state = rDefaultOnState;
	aLed.pulseDelayedStart = 0;

	aLed.pulseMode = false;
	aLed.pulseForever = false;
	aLed.pulseFSM = LED_FSM_READY;
	aLed.pulseDutyCycle = 50;
	aLed.pulsePeriod = 1000;
	aLed.pulseOnTime = (aLed.pulsePeriod * aLed.pulseDutyCycle) / 100;
	aLed.pulseOffTime = (aLed.pulsePeriod - aLed.pulseOnTime);
	aLed.pulseTimer = 0;
	aLed.pulseCnt = 0;
	aLed.pulseCntMax = 0;

}

void FancyLED::Begin() {
	// Set the LED pin number as an output
	pinMode(this->aLed.pinNumber, OUTPUT);
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

//---------////////////////////MAIN LOOP / LISTENER ///////////--------------//

void FancyLED::Loop(void) {
	Update(millis());
}

void FancyLED::Update(uint32_t rCurrentTime) {
	aLedCurrentTime = rCurrentTime;

	if (aLed.pulseCnt < aLed.pulseCntMax || aLed.pulseForever) {
		aLed.pulseMode = true;
	} else if (aLed.pulseCnt >= aLed.pulseCntMax) {
		// Maybe not needed
	}

	// Led Pulse Logic
	if (aLed.pulseMode) {

		// Check delayed start
		if (aLed.pulseDelayedStart > aLedCurrentTime) {
			return;
		}

		// Led Pulse State Machine
		switch (aLed.pulseFSM) {

		case LED_FSM_READY:
			//get the time
			aLed.pulseTimer = aLedCurrentTime;
			//turn on the pin
			aLed.state = aLed.defaultOnState;
			UpdateLedState(aLed.state);
			//set my state to firing
			aLed.pulseFSM = LED_FSM_FIRE;
			break;

		case LED_FSM_FIRE:
			if ((aLed.pulseOnTime) < (aLedCurrentTime - aLed.pulseTimer)) {
				//if it's time, turn me off
				aLed.pulseTimer = aLedCurrentTime;
				//turn off the pin
				aLed.state = !aLed.defaultOnState;
				UpdateLedState(aLed.state);
				aLed.pulseFSM = LED_FSM_REST;
			}
			break;

		case LED_FSM_REST:
			//keep me off
			aLed.state = !aLed.defaultOnState;
			UpdateLedState(aLed.state);
			//check the time and make me ready
			if ((aLed.pulseOffTime) < (aLedCurrentTime - aLed.pulseTimer)) {
				aLed.pulseTimer = aLedCurrentTime;
				aLed.pulseCnt++;
				aLed.pulseMode = false;
				aLed.pulseFSM = LED_FSM_READY;
			}
			break;

		default:
			break;

		}
	}

}

void FancyLED::SetCurrentTime(uint32_t rCurrentTime) {
	aLedCurrentTime = rCurrentTime;
}

int FancyLED::GetState(void) {
	return aLed.pulseFSM;
}

void FancyLED::TurnOn(void) {
	aLed.state = aLed.defaultOnState;
	UpdateLedState(aLed.state);
	aLed.pulseTimer = aLedCurrentTime;
	aLed.pulseFSM = LED_FSM_FIRE; //firing

}

void FancyLED::TurnOff(void) {
	aLed.state = !aLed.defaultOnState;
	UpdateLedState(aLed.state);
	aLed.pulseFSM = LED_FSM_REST; //waiting
}

void FancyLED::Toggle(void) {
	if (aLed.pulseFSM == LED_FSM_FIRE) {
		TurnOff();
	} else {
		TurnOn();
	}
}

void FancyLED::PulseOneTime(void) {
	Pulse(1);
}

void FancyLED::PulseNTimes(uint8_t rPulseRepetitions) {
	Pulse(rPulseRepetitions);
}

void FancyLED::PulseForever(void) {
	aLed.pulseForever = true;
}

void FancyLED::StopPulses(void) {
	aLed.pulseForever = false;
	Pulse(0);
}

void FancyLED::DelayedPulseNTimes(uint32_t rInitialDelay,
		uint8_t rPulseRepetitions) {
	aLed.pulseDelayedStart = rInitialDelay + aLedCurrentTime;
	Pulse(rPulseRepetitions);
}

uint8_t FancyLED::GetDutyCycle(void) {
	return aLed.pulseDutyCycle;
}

void FancyLED::SetLedPulseDutyCycle(uint8_t rDutyCycle) {
	aLed.pulseDutyCycle = rDutyCycle;
	aLed.pulseOnTime = (aLed.pulsePeriod * aLed.pulseDutyCycle) / 100;
	aLed.pulseOffTime = (aLed.pulsePeriod - aLed.pulseOnTime);
}

long FancyLED::GetLedPulsePeriod(void) {
	return aLed.pulsePeriod;
}

void FancyLED::SetLedPulsePeriod(uint32_t rPulsePeriod) {
	aLed.pulsePeriod = rPulsePeriod;
	aLed.pulseOnTime = (aLed.pulsePeriod * aLed.pulseDutyCycle) / 100;
	aLed.pulseOffTime = (aLed.pulsePeriod - aLed.pulseOnTime);
}

// Private Methods //////////////////////////////////////////////////////////////
// Functions available to the library only.

void FancyLED::UpdateLedState(bool rLedStatus) {
	digitalWrite(aLed.pinNumber, rLedStatus);
}

void FancyLED::Pulse(uint8_t rPulseRepetitions) {
	aLed.pulseMode = true;
	aLed.pulseCntMax = rPulseRepetitions;
	aLed.pulseCnt = 0;
}
