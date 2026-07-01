/**
 * ===============================================================
 *  ControlService.h
 *  XYZ Camera Positioning System - CLI Command Interface with FSM
 * ===============================================================
 *  Description:
 *  - Receives CLI commands via serial to control stepper motors.
 *  - A finite state machine (FSM) handles step and continuous motion.
 *
 *  Created on: 24/04/2025
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#ifndef CONTROLSERVICE_H_
#define CONTROLSERVICE_H_

#include <Arduino.h>
#include "StepperMotors.h"

// Service that interprets CLI commands to control motors using a finite state machine
class ControlService {
public:
	ControlService();
	void Begin();   // Initializes the service
	void Loop();    // FSM loop to handle states

	// CLI command handlers
	static void RunCallback(int arg_cnt, char **args);   // Handles 'run' command
	static void StopCallback(int arg_cnt, char **args);  // Handles 'stop' command
	static void MoveCallback(int arg_cnt, char **args);  // Handles 'move' command

private:
	// Possible FSM states
	enum class FSMState {
		IDLE,
		MOVING_CONTINUOUS,
		MOVING_STEPS
	};

	static StepperMotors motors;  // Stepper motor controller
	static FSMState aState;       // Current FSM state

	static void enableMotors();   // Enable all motors
	static void disableMotors();  // Disable all motors
	static bool limitTriggered(); // Check if any limit switch was triggered
};

#endif /* CONTROLSERVICE_H_ */
