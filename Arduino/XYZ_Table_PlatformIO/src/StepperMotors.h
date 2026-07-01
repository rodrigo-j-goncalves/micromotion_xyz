#ifndef STEPPER_MOTORS_H_
#define STEPPER_MOTORS_H_

#include <Arduino.h>
#include <AccelStepper.h>
#include "MegaBoard.h"

// Minimum time between two limit-switch triggers on the same axis (ms).
// Filters electrical noise spikes without delaying real events.
#define LIMIT_DEBOUNCE_MS 5

struct MotorSettings {
    float    maxSpeed;
    float    acceleration;
    uint16_t stepsPerUnit;
    bool     invertDirection;
    bool     enable;
};

struct LimitSwitches {
    uint8_t minPin;
    uint8_t maxPin;

    // Written by ISR → must be volatile so the compiler never caches them.
    volatile bool     limitHit;
    volatile bool     isRetracting;
    volatile bool     minTriggered;
    volatile bool     maxTriggered;
    // Set in ISR, cleared in runAll() after the message is printed safely.
    volatile bool     needsPrint;
    volatile uint32_t lastTriggerMs;   // debounce timestamp

    // Read only in main-loop context (safe without volatile).
    bool isMinHit;   // which end triggered (for the print message)
};

class StepperMotors {
public:
    enum Axis { X = 0, Y = 1, Z = 2 };

    StepperMotors();
    virtual ~StepperMotors();

    void setMotorSettings(Axis axis, const MotorSettings &settings);
    MotorSettings getMotorSettings(Axis axis) const;
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
    bool limitTriggered() const;          // true if ANY axis pin is LOW
    bool limitTriggered(Axis axis) const; // true if this axis pin is LOW
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
