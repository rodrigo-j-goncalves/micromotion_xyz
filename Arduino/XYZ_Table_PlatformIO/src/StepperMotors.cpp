/**
 * ===============================================================
 *  StepperMotors.cpp
 *  XYZ Camera Positioning System - Stepper Motor Control Module
 * ===============================================================
 *  Description:
 *  - Implements axis-specific motion logic for the X, Y, and Z steppers.
 *  - Applies settings such as speed, acceleration, and direction.
 *  - Detects and responds to limit switch events via interrupts.
 *  - Performs automatic retraction upon limit switch triggers.
 *
 *  Created on: 24/04/2025
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

 #include "StepperMotors.h"

// Define pin mappings for step/dir/enable and limit switches
#define STEP_PIN_X 7
#define DIR_PIN_X 6
#define ENABLE_PIN_X 5
#define LIMIT_MIN_X 2
#define LIMIT_MAX_X 3

#define STEP_PIN_Y 25
#define DIR_PIN_Y 26
#define ENABLE_PIN_Y 27
#define LIMIT_MIN_Y 18
#define LIMIT_MAX_Y 19

#define STEP_PIN_Z 28
#define DIR_PIN_Z 29
#define ENABLE_PIN_Z 30
#define LIMIT_MIN_Z 20
#define LIMIT_MAX_Z 21

// Initialize static instance pointer
StepperMotors *StepperMotors::instance = nullptr;

StepperMotors::StepperMotors() {
    instance = this; // Store singleton pointer for ISRs

    // Default motor configurations
    motors[X] = {800.0, 800.0, 100, true, true};   // Inverted direction for X
    motors[Y] = {300.0, 300.0, 8, false, true};
    motors[Z] = {300.0, 300.0, 8, true, true};

    // Enable pin assignments
    enablePins[X] = ENABLE_PIN_X;
    enablePins[Y] = ENABLE_PIN_Y;
    enablePins[Z] = ENABLE_PIN_Z;

    // Set all enable pins to OUTPUT and disable motors initially
    for (int i = 0; i < 3; ++i) {
        pinMode(enablePins[i], OUTPUT);
        digitalWrite(enablePins[i], HIGH); // HIGH = Disabled (active low)
    }

    // Initialize stepper drivers
    initializeStepper(X, STEP_PIN_X, DIR_PIN_X);
    initializeStepper(Y, STEP_PIN_Y, DIR_PIN_Y);
    initializeStepper(Z, STEP_PIN_Z, DIR_PIN_Z);

    // Attach hardware limit switches
    attachLimitSwitches(X, LIMIT_MIN_X, LIMIT_MAX_X);
    attachLimitSwitches(Y, LIMIT_MIN_Y, LIMIT_MAX_Y);
    attachLimitSwitches(Z, LIMIT_MIN_Z, LIMIT_MAX_Z);
}

StepperMotors::~StepperMotors() {
    for (int i = 0; i < 3; i++) {
        delete steppers[i];
    }
}

// Set up the AccelStepper object with initial config
void StepperMotors::initializeStepper(Axis axis, uint8_t stepPin, uint8_t dirPin) {
    steppers[axis] = new AccelStepper(AccelStepper::DRIVER, stepPin, dirPin);
    steppers[axis]->setMaxSpeed(motors[axis].maxSpeed);
    steppers[axis]->setAcceleration(motors[axis].acceleration);
    if (motors[axis].invertDirection)
        steppers[axis]->setPinsInverted(true, false, false);
}

// Configure and attach interrupts to limit switch pins
void StepperMotors::attachLimitSwitches(Axis axis, uint8_t minPin, uint8_t maxPin) {
    limitSwitches[axis].minPin = minPin;
    limitSwitches[axis].maxPin = maxPin;
    pinMode(minPin, INPUT_PULLUP);
    pinMode(maxPin, INPUT_PULLUP);

    switch (axis) {
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

// ISR callback when a limit switch is triggered
void StepperMotors::onLimitHit(Axis axis, bool isMin) {
    limitSwitches[axis].limitHit = true;
    limitSwitches[axis].isRetracting = true;

    steppers[axis]->stop(); // Immediately stop motion

    // Move in opposite direction to retract away from the limit
    long direction = isMin ? -1L : 1L;
    long retractSteps = direction * motors[axis].stepsPerUnit * 25L;

    setEnabled(axis, true);              // Enable motor
    steppers[axis]->move(retractSteps); // Begin retracting

    // Debug output to serial
    MegaBoard::Print("^");
    MegaBoard::Print((axis == X) ? "X" : (axis == Y) ? "Y" : "Z");
    MegaBoard::Print(isMin ? "MIN" : "MAX");
    MegaBoard::Println(": [RETRACT]");
}

// Return true if limit switch is triggered
bool StepperMotors::isLimitReached(Axis axis, bool minLimit) const {
    return minLimit ? limitSwitches[axis].minTriggered : limitSwitches[axis].maxTriggered;
}

// Static ISR handlers — delegate to instance
void StepperMotors::handleInterruptXMin() { instance->onLimitHit(X, true); }
void StepperMotors::handleInterruptXMax() { instance->onLimitHit(X, false); }
void StepperMotors::handleInterruptYMin() { instance->onLimitHit(Y, true); }
void StepperMotors::handleInterruptYMax() { instance->onLimitHit(Y, false); }
void StepperMotors::handleInterruptZMin() { instance->onLimitHit(Z, true); }
void StepperMotors::handleInterruptZMax() { instance->onLimitHit(Z, false); }

void StepperMotors::moveTo(Axis axis, long units) {
    long target = units * motors[axis].stepsPerUnit;
    steppers[axis]->moveTo(target);
}

void StepperMotors::moveRelative(Axis axis, long units) {
    long steps = units * motors[axis].stepsPerUnit;
    steppers[axis]->move(steps);
}

void StepperMotors::setCurrentPosition(Axis axis, long units) {
    steppers[axis]->setCurrentPosition(units * motors[axis].stepsPerUnit);
}

void StepperMotors::stop(Axis axis) {
    steppers[axis]->stop();
}

// Non-blocking run call for all motors; also manages retract completion
void StepperMotors::runAll() {
    for (int i = 0; i < 3; i++) {
        steppers[i]->run(); // Continue stepper movement

        // If retract is complete, disable motor
        if (limitSwitches[i].isRetracting && steppers[i]->distanceToGo() == 0) {
            limitSwitches[i].isRetracting = false;
            limitSwitches[i].limitHit = false;

            setEnabled(static_cast<Axis>(i), false);

            MegaBoard::Print("^SECURITY [Axis ");
            MegaBoard::Print(i == X ? "X" : i == Y ? "Y" : "Z");
            MegaBoard::Println(": retract complete, motor disabled]");
        }
    }
}

bool StepperMotors::isRunning(Axis axis) const {
    return steppers[axis]->distanceToGo() != 0;
}

MotorSettings StepperMotors::getMotorSettings(Axis axis) const {
    return motors[axis];
}

void StepperMotors::setMotorSettings(Axis axis, const MotorSettings &settings) {
    motors[axis] = settings;
    steppers[axis]->setMaxSpeed(settings.maxSpeed);
    steppers[axis]->setAcceleration(settings.acceleration);
    steppers[axis]->setPinsInverted(settings.invertDirection, false, false);
}

void StepperMotors::setMaxSpeed(Axis axis, float maxSpeed) {
    motors[axis].maxSpeed = maxSpeed;
    steppers[axis]->setMaxSpeed(maxSpeed);
}

void StepperMotors::setAcceleration(Axis axis, float acceleration) {
    motors[axis].acceleration = acceleration;
    steppers[axis]->setAcceleration(acceleration);
}

void StepperMotors::setStepsPerUnit(Axis axis, uint16_t steps) {
    motors[axis].stepsPerUnit = steps;
}

void StepperMotors::setInverted(Axis axis, bool inverted) {
    motors[axis].invertDirection = inverted;
    steppers[axis]->setPinsInverted(inverted, false, false);
}

void StepperMotors::setEnabled(Axis axis, bool enabled) {
    motors[axis].enable = enabled;
    digitalWrite(enablePins[axis], enabled ? LOW : HIGH); // Active LOW logic
}

// Convert motor configuration to JSON (for CLI/debug)
String StepperMotors::toJson(Axis axis) {
    const MotorSettings &m = instance->motors[axis];
    const LimitSwitches &sw = instance->limitSwitches[axis];
    uint8_t stepPin = 0, dirPin = 0, enablePin = instance->enablePins[axis];

    switch (axis) {
    case X: stepPin = STEP_PIN_X; dirPin = DIR_PIN_X; break;
    case Y: stepPin = STEP_PIN_Y; dirPin = DIR_PIN_Y; break;
    case Z: stepPin = STEP_PIN_Z; dirPin = DIR_PIN_Z; break;
    }

    String json = "{\n";
    json += "  \"axis\": \"" + String(axis == X ? "X" : axis == Y ? "Y" : "Z") + "\",\n";
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

// CLI command handler: axe X maxSpeed=500 acceleration=300
void StepperMotors::axisCallback(int arg_cnt, char **args) {
    if (arg_cnt < 2) {
        MegaBoard::Println("Usage: axe <X|Y|Z> [param=value ...]");
        return;
    }

    char axisChar = toupper(args[1][0]);
    Axis axis;

    switch (axisChar) {
    case 'X': axis = X; break;
    case 'Y': axis = Y; break;
    case 'Z': axis = Z; break;
    default:
        MegaBoard::Println("Invalid axis. Use X, Y, or Z.");
        return;
    }

    MotorSettings current = instance->motors[axis];

    // Parse arguments
    for (int i = 2; i < arg_cnt; i++) {
        String arg = String(args[i]);
        int sep = arg.indexOf('=');
        if (sep == -1) continue;

        String key = arg.substring(0, sep);
        String val = arg.substring(sep + 1);

        if (key == "maxSpeed") current.maxSpeed = val.toFloat();
        else if (key == "acceleration") current.acceleration = val.toFloat();
        else if (key == "stepsPerUnit") current.stepsPerUnit = val.toFloat();
        else if (key == "inverted") current.invertDirection = (val == "true");
        else if (key == "enabled") current.enable = (val == "true");
    }

    // Apply settings if any parameter was modified
    if (arg_cnt > 2) {
        instance->setMotorSettings(axis, current);
        MegaBoard::Println("[AXE] Updated axis settings.");
    } else {
        MegaBoard::Println(toJson(axis));
    }
}

bool StepperMotors::limitTriggered() const {
    for (int i = 0; i < 3; ++i) {
        if (digitalRead(limitSwitches[i].minPin) == LOW || digitalRead(limitSwitches[i].maxPin) == LOW)
            return true;
    }
    return false;
}

bool StepperMotors::isRetracting(Axis axis) const {
    return limitSwitches[axis].isRetracting;
}
