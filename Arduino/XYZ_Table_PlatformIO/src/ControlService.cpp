/**
 * ===============================================================
 *  ControlService.cpp
 *  XYZ Camera Positioning System - CLI Command Interface with FSM
 * ===============================================================
 *  Description:
 *  - Implements a FSM for motion control and CLI command integration.
 *
 *  Created on: 24/04/2025
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#include "ControlService.h"

StepperMotors ControlService::motors;
ControlService::FSMState ControlService::aState = ControlService::FSMState::IDLE;

ControlService::ControlService() {}

void ControlService::Begin()
{
	// Instancia implícita ya hecha al ser static
	disableMotors();
}

void ControlService::Loop()
{
	motors.runAll();

	switch (aState)
	{
	case FSMState::IDLE:
		break;

	case FSMState::MOVING_CONTINUOUS:
		// Verificar si algún eje está en retracción: no hacer nada
		for (int i = 0; i < 3; i++)
		{
			if (motors.isRetracting(static_cast<StepperMotors::Axis>(i)))
			{
				// Esperar a que finalice la retracción
				return;
			}
		}

		// Si se ha activado un limit switch físico, detener FSM
		if (motors.limitTriggered())
		{
			motors.stop(StepperMotors::X);
			motors.stop(StepperMotors::Y);
			motors.stop(StepperMotors::Z);
			disableMotors();
			aState = FSMState::IDLE;
			MegaBoard::Println("[FSM] Limit triggered - stopping");
		}
		break;

	case FSMState::MOVING_STEPS:
		if (!motors.isRunning(StepperMotors::X) &&
			!motors.isRunning(StepperMotors::Y) &&
			!motors.isRunning(StepperMotors::Z))
		{
			disableMotors();
			aState = FSMState::IDLE;
			MegaBoard::Println("[FSM] Move complete");
		}
		break;
	}
}

void ControlService::enableMotors()
{
	motors.setEnabled(StepperMotors::X, true);
	motors.setEnabled(StepperMotors::Y, true);
	motors.setEnabled(StepperMotors::Z, true);
}

void ControlService::disableMotors()
{
	motors.setEnabled(StepperMotors::X, false);
	motors.setEnabled(StepperMotors::Y, false);
	motors.setEnabled(StepperMotors::Z, false);
}


void ControlService::RunCallback(int arg_cnt, char **args)
{
	if (arg_cnt < 2)
	{
		MegaBoard::Println("[Run] Usage: run [x|y|z|all|-x|-y|-z|-all]");
		disableMotors();
		return;
	}

	String rawArg = String(args[1]);
	rawArg.toLowerCase();

	bool reverse = false;
	String axis = rawArg;

	if (rawArg.startsWith("-"))
	{
		reverse = true;
		axis = rawArg.substring(1);  // remove the '-' prefix
	}

	if (axis != "x" && axis != "y" && axis != "z" && axis != "all")
	{
		MegaBoard::Println("[Run] Invalid argument. Usage: run [x|y|z|all|-x|-y|-z|-all]");
		disableMotors();
		return;
	}

	int dirSign = reverse ? -1 : 1;
	long steps = 100000L * dirSign;

	if (axis == "x" || axis == "all")
	{
		motors.setEnabled(StepperMotors::X, true);
		motors.moveRelative(StepperMotors::X, steps);
	}
	if (axis == "y" || axis == "all")
	{
		motors.setEnabled(StepperMotors::Y, true);
		motors.moveRelative(StepperMotors::Y, steps);
	}
	if (axis == "z" || axis == "all")
	{
		motors.setEnabled(StepperMotors::Z, true);
		motors.moveRelative(StepperMotors::Z, steps);
	}

	aState = FSMState::MOVING_CONTINUOUS;

	MegaBoard::Print("[Run] Continuous motion ");
	MegaBoard::Print(reverse ? "reverse " : "forward ");
	MegaBoard::Println(axis);
}



void ControlService::StopCallback(int arg_cnt, char **args)
{
	String target = "all";
	if (arg_cnt > 1)
	{
		target = String(args[1]);
		target.toLowerCase();

		if (target != "x" && target != "y" && target != "z" && target != "all")
		{
			MegaBoard::Println("[Stop] Invalid argument. Usage: stop [x|y|z|all]");
			return;
		}
	}

	if (target == "x" || target == "all")
	{
		motors.stop(StepperMotors::X);
	}
	if (target == "y" || target == "all")
	{
		motors.stop(StepperMotors::Y);
	}
	if (target == "z" || target == "all")
	{
		motors.stop(StepperMotors::Z);
	}

	// Si se ha detenido todo o se ha pedido "all", desactivamos
	if (target == "all" ||
		(!motors.isRunning(StepperMotors::X) &&
		 !motors.isRunning(StepperMotors::Y) &&
		 !motors.isRunning(StepperMotors::Z)))
	{
		disableMotors();
	}

	aState = FSMState::IDLE;

	MegaBoard::Print("[Stop] Motors stopped for ");
	MegaBoard::Println(target);
}

void ControlService::MoveCallback(int arg_cnt, char **args)
{
	bool shouldMoveX = false, shouldMoveY = false, shouldMoveZ = false;
	float x = 0, y = 0, z = 0;

	bool usedAll = false;

	for (int i = 1; i < arg_cnt - 1; i++)
	{
		String arg = String(args[i]);
		arg.toLowerCase();

		if (arg == "all")
		{
			float val = atof(args[++i]);
			x = y = z = val;
			shouldMoveX = shouldMoveY = shouldMoveZ = true;
			usedAll = true;
			break; // opcional: ignoramos otros argumentos si se usa 'all'
		}
		else if (arg == "x")
		{
			x = atof(args[++i]);
			shouldMoveX = true;
		}
		else if (arg == "y")
		{
			y = atof(args[++i]);
			shouldMoveY = true;
		}
		else if (arg == "z")
		{
			z = atof(args[++i]);
			shouldMoveZ = true;
		}
	}

	if (!shouldMoveX && !shouldMoveY && !shouldMoveZ)
	{
		MegaBoard::Println("[Move] No valid axes specified. Usage: move X <val> Y <val> Z <val> | move all <val>");
		return;
	}

	enableMotors();

	if (shouldMoveX)
		motors.moveRelative(StepperMotors::X, x);
	if (shouldMoveY)
		motors.moveRelative(StepperMotors::Y, y);
	if (shouldMoveZ)
		motors.moveRelative(StepperMotors::Z, z);

	aState = FSMState::MOVING_STEPS;

	MegaBoard::Print("[Move] Moving to: ");
	if (usedAll)
	{
		MegaBoard::Print("ALL=");
		MegaBoard::Println(x);
	}
	else
	{
		if (shouldMoveX)
		{
			MegaBoard::Print("X=");
			MegaBoard::Print(x);
			MegaBoard::Print(" ");
		}
		if (shouldMoveY)
		{
			MegaBoard::Print("Y=");
			MegaBoard::Print(y);
			MegaBoard::Print(" ");
		}
		if (shouldMoveZ)
		{
			MegaBoard::Print("Z=");
			MegaBoard::Print(z);
			MegaBoard::Print(" ");
		}
		MegaBoard::Println();
	}
}


bool ControlService::limitTriggered()
{
	return motors.isLimitReached(StepperMotors::X, false) ||
		   motors.isLimitReached(StepperMotors::Y, false) ||
		   motors.isLimitReached(StepperMotors::Z, false);
}
