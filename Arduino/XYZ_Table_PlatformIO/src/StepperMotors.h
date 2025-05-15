/**
 * ===============================================================
 *  StepperMotors.h
 *  XYZ Camera Positioning System - Stepper Motor Control Module
 * ===============================================================
 *  Description:
 *  - Controls configuration and motion of stepper motors.
 *  - Integrates AccelStepper and handles limit switches via interrupts.
 *
 *  Created on: 24/04/2025
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#ifndef STEPPER_MOTORS_H_
#define STEPPER_MOTORS_H_

#include <Arduino.h>
#include <AccelStepper.h>
#include "MegaBoard.h"

struct MotorSettings
{
	float maxSpeed;
	float acceleration;
	uint16_t stepsPerUnit;
	bool invertDirection;
	bool enable;
};

struct LimitSwitches
{
	uint8_t minPin;
	uint8_t maxPin;
	volatile bool minTriggered;
	volatile bool maxTriggered;
	bool limitHit = false;		  // hubo colisión
	bool isRetracting = false;	  // está haciendo retiro	
};

class StepperMotors
{
public:
	enum Axis
	{
		X = 0,
		Y = 1,
		Z = 2
	};

	StepperMotors();
	virtual ~StepperMotors();

	MotorSettings getMotorSettings(Axis axis) const;
	void setMotorSettings(Axis axis, const MotorSettings &settings);
	void setMaxSpeed(Axis axis, float maxSpeed);
	void setAcceleration(Axis axis, float acceleration);
	void setStepsPerUnit(Axis axis, uint16_t steps);
	void setInverted(Axis axis, bool inverted);
	void setEnabled(Axis axis, bool enabled);

	static void axisCallback(int arg_cnt, char **args);

	void moveTo(Axis axis, long units);
	void moveRelative(Axis axis, long units);
	void setCurrentPosition(Axis axis, long units);
	void stop(Axis axis);
	void runAll();
	bool isRunning(Axis axis) const;

	void attachLimitSwitches(Axis axis, uint8_t minPin, uint8_t maxPin);
	bool isLimitReached(Axis axis, bool minLimit) const;
	bool limitTriggered() const;
	bool isRetracting(Axis axis) const;

private:
	MotorSettings motors[3];
	AccelStepper *steppers[3];
	LimitSwitches limitSwitches[3];
	uint8_t enablePins[3];

	void initializeStepper(Axis axis, uint8_t stepPin, uint8_t dirPin);
	static void handleInterruptXMin();
	static void handleInterruptXMax();
	static void handleInterruptYMin();
	static void handleInterruptYMax();
	static void handleInterruptZMin();
	static void handleInterruptZMax();
	static String toJson(Axis axis);
	void onLimitHit(Axis axis, bool isMin);

	static StepperMotors *instance;
};

#endif /* STEPPER_MOTORS_H_ */
