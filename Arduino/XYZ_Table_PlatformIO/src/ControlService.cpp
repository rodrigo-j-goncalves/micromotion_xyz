/**
 * ControlService.cpp — v1.0.0
 * Finite-state machine that routes CLI commands to the stepper motors.
 *
 * Changes vs original:
 *   - FSM MOVING_CONTINUOUS: removed global stop-all-axes when limit pin reads LOW
 *     after retraction; the ISR in StepperMotors already handles per-axis retraction.
 *   - MoveCallback: added ETX (via Println with a value) on the else branch
 *     so the Python server receives a properly terminated response.
 *   - Removed dead private limitTriggered() method that only checked maxTriggered.
 */

#include "ControlService.h"

StepperMotors ControlService::motors;
ControlService::FSMState ControlService::aState = ControlService::FSMState::IDLE;

ControlService::ControlService() {}

void ControlService::Begin()
{
    disableMotors();
}

void ControlService::Loop()
{
    motors.runAll();

    switch (aState) {
    case FSMState::IDLE:
        break;

    case FSMState::MOVING_CONTINUOUS:
        // Per-axis limit handling is done inside StepperMotors ISR + runAll().
        // If every axis has stopped on its own (retractions complete, no distance
        // left), the operator must issue an explicit stop command.
        break;

    case FSMState::MOVING_STEPS:
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
    if (arg_cnt < 2) {
        MegaBoard::Println("[Run] Usage: run [x|y|z|all|-x|-y|-z|-all]");
        disableMotors();
        return;
    }

    String rawArg = String(args[1]);
    rawArg.toLowerCase();

    bool   reverse = rawArg.startsWith("-");
    String axis    = reverse ? rawArg.substring(1) : rawArg;

    if (axis != "x" && axis != "y" && axis != "z" && axis != "all") {
        MegaBoard::Println("[Run] Invalid argument. Usage: run [x|y|z|all|-x|-y|-z|-all]");
        disableMotors();
        return;
    }

    int  dirSign = reverse ? -1 : 1;
    long steps   = 100000L * dirSign;

    if (axis == "x" || axis == "all") { motors.setEnabled(StepperMotors::X, true); motors.moveRelative(StepperMotors::X, steps); }
    if (axis == "y" || axis == "all") { motors.setEnabled(StepperMotors::Y, true); motors.moveRelative(StepperMotors::Y, steps); }
    if (axis == "z" || axis == "all") { motors.setEnabled(StepperMotors::Z, true); motors.moveRelative(StepperMotors::Z, steps); }

    aState = FSMState::MOVING_CONTINUOUS;

    MegaBoard::Print("[Run] Continuous motion ");
    MegaBoard::Print(reverse ? "reverse " : "forward ");
    MegaBoard::Println(axis);
}

void ControlService::StopCallback(int arg_cnt, char **args)
{
    String target = "all";
    if (arg_cnt > 1) {
        target = String(args[1]);
        target.toLowerCase();
        if (target != "x" && target != "y" && target != "z" && target != "all") {
            MegaBoard::Println("[Stop] Invalid argument. Usage: stop [x|y|z|all]");
            return;
        }
    }

    if (target == "x" || target == "all") motors.stop(StepperMotors::X);
    if (target == "y" || target == "all") motors.stop(StepperMotors::Y);
    if (target == "z" || target == "all") motors.stop(StepperMotors::Z);

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

void ControlService::MoveCallback(int arg_cnt, char **args)
{
    bool  shouldMoveX = false, shouldMoveY = false, shouldMoveZ = false;
    float x = 0, y = 0, z = 0;
    bool  usedAll = false;

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

    if (!shouldMoveX && !shouldMoveY && !shouldMoveZ) {
        MegaBoard::Println("[Move] No valid axes. Usage: move X <val> Y <val> Z <val> | move all <val>");
        return;
    }

    enableMotors();

    if (shouldMoveX) motors.moveRelative(StepperMotors::X, x);
    if (shouldMoveY) motors.moveRelative(StepperMotors::Y, y);
    if (shouldMoveZ) motors.moveRelative(StepperMotors::Z, z);

    aState = FSMState::MOVING_STEPS;

    MegaBoard::Print("[Move] Moving: ");
    if (usedAll) {
        MegaBoard::Println("ALL=" + String(x));  // Println(value) → adds ETX ✓
    } else {
        String summary = "";
        if (shouldMoveX) summary += "X=" + String(x) + " ";
        if (shouldMoveY) summary += "Y=" + String(y) + " ";
        if (shouldMoveZ) summary += "Z=" + String(z);
        MegaBoard::Println(summary);             // Println(value) → adds ETX ✓
    }
}
