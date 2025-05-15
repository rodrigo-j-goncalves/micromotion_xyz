/**
 * ===============================================================
 *  StepperMotors.cpp
 *  XYZ Camera Positioning System - Stepper Motor Control Module
 * ===============================================================
 *  Description:
 *  - Implements motion and configuration logic for each stepper axis.
 *  - Manages limit switches via interrupts and ENABLE control.
 *
 *  Created on: 24/04/2025
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#include "StepperMotors.h"

// Pin definitions (Arduino Mega)
#define STEP_PIN_X 7
#define DIR_PIN_X 6
#define ENABLE_PIN_X 5
#define LIMIT_MIN_X 2 // interrupción válida
#define LIMIT_MAX_X 3 // interrupción válida

#define STEP_PIN_Y 25
#define DIR_PIN_Y 26
#define ENABLE_PIN_Y 27
#define LIMIT_MIN_Y 18 // interrupción válida
#define LIMIT_MAX_Y 19 // interrupción válida

#define STEP_PIN_Z 28
#define DIR_PIN_Z 29
#define ENABLE_PIN_Z 30
#define LIMIT_MIN_Z 20 // interrupción válida
#define LIMIT_MAX_Z 21 // interrupción válida

StepperMotors *StepperMotors::instance = nullptr;

StepperMotors::StepperMotors()
{
	instance = this;

	motors[X] = {800.0, 800.0, 100, true, true};	// !Invertir direccion del eje X
	motors[Y] = {600.0, 600.0, 40, false, true};
	motors[Z] = {600.0, 600.0, 40, true, true};

	enablePins[X] = ENABLE_PIN_X;
	enablePins[Y] = ENABLE_PIN_Y;
	enablePins[Z] = ENABLE_PIN_Z;

	for (int i = 0; i < 3; ++i)
	{
		pinMode(enablePins[i], OUTPUT);
		digitalWrite(enablePins[i], HIGH); // Disabled by default
	}

	initializeStepper(X, STEP_PIN_X, DIR_PIN_X);
	initializeStepper(Y, STEP_PIN_Y, DIR_PIN_Y);
	initializeStepper(Z, STEP_PIN_Z, DIR_PIN_Z);

	attachLimitSwitches(X, LIMIT_MIN_X, LIMIT_MAX_X);
	attachLimitSwitches(Y, LIMIT_MIN_Y, LIMIT_MAX_Y);
	attachLimitSwitches(Z, LIMIT_MIN_Z, LIMIT_MAX_Z);
}

StepperMotors::~StepperMotors()
{
	for (int i = 0; i < 3; i++)
	{
		delete steppers[i];
	}
}

void StepperMotors::initializeStepper(Axis axis, uint8_t stepPin, uint8_t dirPin)
{
	steppers[axis] = new AccelStepper(AccelStepper::DRIVER, stepPin, dirPin);
	steppers[axis]->setMaxSpeed(motors[axis].maxSpeed);
	steppers[axis]->setAcceleration(motors[axis].acceleration);
	if (motors[axis].invertDirection)
		steppers[axis]->setPinsInverted(true, false, false);
}

void StepperMotors::attachLimitSwitches(Axis axis, uint8_t minPin, uint8_t maxPin)
{
	limitSwitches[axis].minPin = minPin;
	limitSwitches[axis].maxPin = maxPin;
	pinMode(minPin, INPUT_PULLUP);
	pinMode(maxPin, INPUT_PULLUP);

	switch (axis)
	{
	case X:
		attachInterrupt(digitalPinToInterrupt(minPin), handleInterruptXMin, FALLING);
		attachInterrupt(digitalPinToInterrupt(maxPin), handleInterruptXMax, FALLING);
		break;
	case Y:
		attachInterrupt(digitalPinToInterrupt(minPin), handleInterruptYMin, FALLING);
		attachInterrupt(digitalPinToInterrupt(maxPin), handleInterruptYMax, FALLING);
		break;
	case Z:
		attachInterrupt(digitalPinToInterrupt(minPin), handleInterruptZMin, FALLING);
		attachInterrupt(digitalPinToInterrupt(maxPin), handleInterruptZMax, FALLING);
		break;
	}
}

void StepperMotors::onLimitHit(Axis axis, bool isMin)
{
	limitSwitches[axis].limitHit = true;
	limitSwitches[axis].isRetracting = true;

	steppers[axis]->stop();

	long direction = isMin ? -1L : 1L;
	long retractSteps = direction * static_cast<long>(motors[axis].stepsPerUnit) * 25L;

	setEnabled(axis, true);
	steppers[axis]->move(retractSteps);

	const char *axisName = (axis == X) ? "X" : (axis == Y) ? "Y" : "Z";
	const char *limitType = isMin ? "MIN" : "MAX";

	Serial.print("[^");
	Serial.print(axisName);
	Serial.print(limitType);
	Serial.println("]: RETRACT");
}

bool StepperMotors::isLimitReached(Axis axis, bool minLimit) const
{
	return minLimit ? limitSwitches[axis].minTriggered : limitSwitches[axis].maxTriggered;
}

void StepperMotors::handleInterruptXMin() { instance->onLimitHit(X, true); }
void StepperMotors::handleInterruptXMax() { instance->onLimitHit(X, false); }
void StepperMotors::handleInterruptYMin() { instance->onLimitHit(Y, true); }
void StepperMotors::handleInterruptYMax() { instance->onLimitHit(Y, false); }
void StepperMotors::handleInterruptZMin() { instance->onLimitHit(Z, true); }
void StepperMotors::handleInterruptZMax() { instance->onLimitHit(Z, false); }

void StepperMotors::moveTo(Axis axis, long units)
{
	long target = units * motors[axis].stepsPerUnit;
	steppers[axis]->moveTo(target);
}

void StepperMotors::moveRelative(Axis axis, long units)
{
	long steps = units * motors[axis].stepsPerUnit;
	steppers[axis]->move(steps);
}

void StepperMotors::setCurrentPosition(Axis axis, long units)
{
	steppers[axis]->setCurrentPosition(units * motors[axis].stepsPerUnit);
}

void StepperMotors::stop(Axis axis)
{
	steppers[axis]->stop();
}

void StepperMotors::runAll()
{
	for (int i = 0; i < 3; i++)
	{
		// Ejecutar cualquier movimiento pendiente (normal o retracción)
		steppers[i]->run();

		// Finaliza retracción si estaba activa
		if (limitSwitches[i].isRetracting && steppers[i]->distanceToGo() == 0)
		{
			limitSwitches[i].isRetracting = false;
			limitSwitches[i].limitHit = false;

			setEnabled(static_cast<Axis>(i), false);

			Serial.print("[SECURITY] Axis ");
			Serial.print(i == X ? "X" : i == Y ? "Y" : "Z");
			Serial.println(": retract complete, motor disabled");
		}
	}
}

bool StepperMotors::isRunning(Axis axis) const
{
	return steppers[axis]->distanceToGo() != 0;
}

MotorSettings StepperMotors::getMotorSettings(Axis axis) const
{
	return motors[axis];
}

void StepperMotors::setMotorSettings(Axis axis, const MotorSettings &settings)
{
	motors[axis] = settings;
	steppers[axis]->setMaxSpeed(settings.maxSpeed);
	steppers[axis]->setAcceleration(settings.acceleration);
	steppers[axis]->setPinsInverted(settings.invertDirection, false, false);
}

void StepperMotors::setMaxSpeed(Axis axis, float maxSpeed)
{
	motors[axis].maxSpeed = maxSpeed;
	steppers[axis]->setMaxSpeed(maxSpeed);
}

void StepperMotors::setAcceleration(Axis axis, float acceleration)
{
	motors[axis].acceleration = acceleration;
	steppers[axis]->setAcceleration(acceleration);
}

void StepperMotors::setStepsPerUnit(Axis axis, uint16_t steps)
{
	motors[axis].stepsPerUnit = steps;
}

void StepperMotors::setInverted(Axis axis, bool inverted)
{
	motors[axis].invertDirection = inverted;
	steppers[axis]->setPinsInverted(inverted, false, false);
}

void StepperMotors::setEnabled(Axis axis, bool enabled)
{
	motors[axis].enable = enabled;
	digitalWrite(enablePins[axis], enabled ? LOW : HIGH); // Active LOW
}

String StepperMotors::toJson(Axis axis)
{
	const MotorSettings &m = instance->motors[axis];
	const LimitSwitches &sw = instance->limitSwitches[axis];
	uint8_t stepPin = 0, dirPin = 0, enablePin = instance->enablePins[axis];

	switch (axis)
	{
	case X:
		stepPin = STEP_PIN_X;
		dirPin = DIR_PIN_X;
		break;
	case Y:
		stepPin = STEP_PIN_Y;
		dirPin = DIR_PIN_Y;
		break;
	case Z:
		stepPin = STEP_PIN_Z;
		dirPin = DIR_PIN_Z;
		break;
	}

	String json = "{\n";
	json += "  \"axis\": \"" + String(axis == X ? "X" : axis == Y ? "Y"
																  : "Z") +
			"\",\n";
	json += "  \"motor\": {\n";
	json += "    \"maxSpeed\": " + String(m.maxSpeed) + ",\n";
	json += "    \"acceleration\": " + String(m.acceleration) + ",\n";
	json += "    \"stepsPerUnit\": " + String(m.stepsPerUnit) + ",\n";
	json += "    \"inverted\": " + String(m.invertDirection ? "true" : "false") + ",\n";
	json += "    \"enabled\": " + String(m.enable ? "true" : "false") + ",\n";
	json += "    \"stepPin\": " + String(stepPin) + ",\n";
	json += "    \"dirPin\": " + String(dirPin) + ",\n";
	json += "    \"enablePin\": " + String(enablePin) + "\n";
	json += "  },\n";

	json += "  \"limitSwitches\": {\n";
	json += "    \"minPin\": " + String(sw.minPin) + ",\n";
	json += "    \"maxPin\": " + String(sw.maxPin) + ",\n";
	json += "    \"minTriggered\": " + String(sw.minTriggered ? "true" : "false") + ",\n";
	json += "    \"maxTriggered\": " + String(sw.maxTriggered ? "true" : "false") + "\n";
	json += "  }\n";
	json += "}\n";

	return json;
}

void StepperMotors::axisCallback(int arg_cnt, char **args)
{
	if (arg_cnt < 2)
	{
		MegaBoard::Println("Usage: axe <X|Y|Z> [param=value ...]");
		return;
	}

	char axisChar = toupper(args[1][0]);
	Axis axis;

	switch (axisChar)
	{
	case 'X':
		axis = X;
		break;
	case 'Y':
		axis = Y;
		break;
	case 'Z':
		axis = Z;
		break;
	default:
		MegaBoard::Println("Invalid axis. Use X, Y, or Z.");
		return;
	}

	MotorSettings current = instance->motors[axis];

	for (int i = 2; i < arg_cnt; i++)
	{
		String arg = String(args[i]);
		int sep = arg.indexOf('=');
		if (sep == -1)
			continue;

		String key = arg.substring(0, sep);
		String val = arg.substring(sep + 1);

		if (key == "maxSpeed")
			current.maxSpeed = val.toFloat();
		else if (key == "acceleration")
			current.acceleration = val.toFloat();
		else if (key == "stepsPerUnit")
			current.stepsPerUnit = val.toFloat();
		else if (key == "inverted")
			current.invertDirection = (val == "true");
		else if (key == "enabled")
			current.enable = (val == "true");
	}

	// Solo setea si hubo algo para cambiar
	if (arg_cnt > 2)
	{
		instance->setMotorSettings(axis, current);
		MegaBoard::Println("[OK] Updated axis settings.");
	}

	MegaBoard::Println(toJson(axis));
}

bool StepperMotors::limitTriggered() const
{
	for (int i = 0; i < 3; ++i)
	{
		if (digitalRead(limitSwitches[i].minPin) == LOW || digitalRead(limitSwitches[i].maxPin) == LOW)
			return true;
	}
	return false;
}

bool StepperMotors::isRetracting(Axis axis) const
{
	return limitSwitches[axis].isRetracting;
}