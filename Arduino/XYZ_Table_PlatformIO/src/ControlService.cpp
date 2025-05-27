/**
 * ===============================================================
 *  ControlService.cpp
 *  XYZ Camera Positioning System - CLI Command Interface with FSM
 * ===============================================================
 *  Description:
 *  - Implements a finite state machine (FSM) to manage motion control.
 *  - Integrates with the CLI to parse and respond to commands.
 *
 *  Created on: 24/04/2025
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#include "ControlService.h"

// Static member initialization
StepperMotors ControlService::motors;
ControlService::FSMState ControlService::aState = ControlService::FSMState::IDLE;

ControlService::ControlService() {}

void ControlService::Begin() {
	// Motors are implicitly initialized. Disable them by default.
	disableMotors();
}

void ControlService::Loop() {
	// Run pending stepper movements
	motors.runAll();

	switch (aState) {
	case FSMState::IDLE:
		// Do nothing in idle state
		break;

	case FSMState::MOVING_CONTINUOUS:
		// Check if any axis is in retract mode (wait until done)
		for (int i = 0; i < 3; i++) {
			if (motors.isRetracting(static_cast<StepperMotors::Axis>(i))) {
				return; // Wait for retraction to finish
			}
		}

		// If a physical limit was triggered, stop all motion
		if (motors.limitTriggered()) {
			motors.stop(StepperMotors::X);
			motors.stop(StepperMotors::Y);
			motors.stop(StepperMotors::Z);
			disableMotors();
			aState = FSMState::IDLE;
			MegaBoard::Println("^FSM [Limit triggered - stopping]");
		}
		break;

	case FSMState::MOVING_STEPS:
		// Transition to idle once all movement is complete
		if (!motors.isRunning(StepperMotors::X) &&
			!motors.isRunning(StepperMotors::Y) &&
			!motors.isRunning(StepperMotors::Z)) {
			disableMotors();
			aState = FSMState::IDLE;
			MegaBoard::Println("^FSM [Move complete]");
		}
		break;
	}
}

void ControlService::enableMotors() {
	motors.setEnabled(StepperMotors::X, true);
	motors.setEnabled(StepperMotors::Y, true);
	motors.setEnabled(StepperMotors::Z, true);
}

void ControlService::disableMotors() {
	motors.setEnabled(StepperMotors::X, false);
	motors.setEnabled(StepperMotors::Y, false);
	motors.setEnabled(StepperMotors::Z, false);
}

// Handle 'run' CLI command for continuous movement
void ControlService::RunCallback(int arg_cnt, char **args) {
	if (arg_cnt < 2) {
		MegaBoard::Println("[Run] Usage: run [x|y|z|all|-x|-y|-z|-all]");
		disableMotors();
		return;
	}

	String rawArg = String(args[1]);
	rawArg.toLowerCase();

	bool reverse = false;
	String axis = rawArg;

	if (rawArg.startsWith("-")) {
		reverse = true;
		axis = rawArg.substring(1); // Remove negative sign
	}

	if (axis != "x" && axis != "y" && axis != "z" && axis != "all") {
		MegaBoard::Println("[Run] Invalid argument. Usage: run [x|y|z|all|-x|-y|-z|-all]");
		disableMotors();
		return;
	}

	int dirSign = reverse ? -1 : 1;
	long steps = 100000L * dirSign;

	// Start continuous relative movement in the selected axes
	if (axis == "x" || axis == "all") {
		motors.setEnabled(StepperMotors::X, true);
		motors.moveRelative(StepperMotors::X, steps);
	}
	if (axis == "y" || axis == "all") {
		motors.setEnabled(StepperMotors::Y, true);
		motors.moveRelative(StepperMotors::Y, steps);
	}
	if (axis == "z" || axis == "all") {
		motors.setEnabled(StepperMotors::Z, true);
		motors.moveRelative(StepperMotors::Z, steps);
	}

	aState = FSMState::MOVING_CONTINUOUS;

	MegaBoard::Print("[Run] Continuous motion ");
	MegaBoard::Print(reverse ? "reverse " : "forward ");
	MegaBoard::Println(axis);
}

// Handle 'stop' CLI command
void ControlService::StopCallback(int arg_cnt, char **args) {
	String target = "all";
	if (arg_cnt > 1) {
		target = String(args[1]);
		target.toLowerCase();

		if (target != "x" && target != "y" && target != "z" && target != "all") {
			MegaBoard::Println("[Stop] Invalid argument. Usage: stop [x|y|z|all]");
			return;
		}
	}

	// Stop selected axes
	if (target == "x" || target == "all") motors.stop(StepperMotors::X);
	if (target == "y" || target == "all") motors.stop(StepperMotors::Y);
	if (target == "z" || target == "all") motors.stop(StepperMotors::Z);

	// Disable motors if all are stopped
	if (target == "all" ||
		(!motors.isRunning(StepperMotors::X) &&
		 !motors.isRunning(StepperMotors::Y) &&
		 !motors.isRunning(StepperMotors::Z))) {
		disableMotors();
	}

	aState = FSMState::IDLE;

	MegaBoard::Print("^STOP [Motors stopped for ");
	MegaBoard::Print(target);
	MegaBoard::Println("]");
}

// Handle 'move' CLI command for relative movement
void ControlService::MoveCallback(int arg_cnt, char **args) {
	bool shouldMoveX = false, shouldMoveY = false, shouldMoveZ = false;
	float x = 0, y = 0, z = 0;
	bool usedAll = false;

	// Parse arguments
	for (int i = 1; i < arg_cnt - 1; i++) {
		String arg = String(args[i]);
		arg.toLowerCase();

		if (arg == "all") {
			float val = atof(args[++i]);
			x = y = z = val;
			shouldMoveX = shouldMoveY = shouldMoveZ = true;
			usedAll = true;
			break;
		} else if (arg == "x") {
			x = atof(args[++i]);
			shouldMoveX = true;
		} else if (arg == "y") {
			y = atof(args[++i]);
			shouldMoveY = true;
		} else if (arg == "z") {
			z = atof(args[++i]);
			shouldMoveZ = true;
		}
	}

	// If no valid axes were specified
	if (!shouldMoveX && !shouldMoveY && !shouldMoveZ) {
		MegaBoard::Println("[Move] No valid axes specified. Usage: move X <val> Y <val> Z <val> | move all <val>");
		return;
	}

	enableMotors();

	// Perform movement
	if (shouldMoveX) motors.moveRelative(StepperMotors::X, x);
	if (shouldMoveY) motors.moveRelative(StepperMotors::Y, y);
	if (shouldMoveZ) motors.moveRelative(StepperMotors::Z, z);

	aState = FSMState::MOVING_STEPS;

	// Output movement summary
	MegaBoard::Print("[Move] Moving to: ");
	if (usedAll) {
		MegaBoard::Print("ALL=");
		MegaBoard::Println(x);
	} else {
		if (shouldMoveX) {
			MegaBoard::Print("X=");
			MegaBoard::Print(x);
			MegaBoard::Print(" ");
		}
		if (shouldMoveY) {
			MegaBoard::Print("Y=");
			MegaBoard::Print(y);
			MegaBoard::Print(" ");
		}
		if (shouldMoveZ) {
			MegaBoard::Print("Z=");
			MegaBoard::Print(z);
			MegaBoard::Print(" ");
		}
		MegaBoard::Println();
	}
}

// Check if any axis has triggered a limit switch
bool ControlService::limitTriggered() {
	return motors.isLimitReached(StepperMotors::X, false) ||
		   motors.isLimitReached(StepperMotors::Y, false) ||
		   motors.isLimitReached(StepperMotors::Z, false);
}
