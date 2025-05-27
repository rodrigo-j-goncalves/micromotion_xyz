/**
 * ===============================================================
 *  StepperMotors.h
 *  XYZ Camera Positioning System - Stepper Motor Control Module
 * ===============================================================
 *  Description:
 *  - Handles configuration and control of stepper motors for XYZ motion.
 *  - Integrates the AccelStepper library for motion profiling.
 *  - Manages limit switches using hardware interrupts.
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
#include "MegaBoard.h" // Assumed to handle serial and other utilities

// Structure for storing configuration settings of each motor
struct MotorSettings {
    float maxSpeed;            // Maximum speed of the motor
    float acceleration;        // Acceleration rate
    uint16_t stepsPerUnit;     // Steps per physical unit (e.g., mm)
    bool invertDirection;      // Whether to invert the motor direction
    bool enable;               // Whether the motor is enabled
};

// Structure for handling limit switches on both ends of the axis
struct LimitSwitches {
    uint8_t minPin;            // Pin number for minimum limit switch
    uint8_t maxPin;            // Pin number for maximum limit switch
    volatile bool minTriggered; // Flag for minimum limit triggered
    volatile bool maxTriggered; // Flag for maximum limit triggered
    bool limitHit = false;     // Whether a limit was hit
    bool isRetracting = false; // Whether the motor is retracting after hitting a limit
};

// Main class for managing all stepper motors (X, Y, Z)
class StepperMotors {
public:
    // Enum to identify motor axes
    enum Axis {
        X = 0,
        Y = 1,
        Z = 2
    };

    StepperMotors();
    virtual ~StepperMotors();

    // Motor configuration methods
    MotorSettings getMotorSettings(Axis axis) const;
    void setMotorSettings(Axis axis, const MotorSettings &settings);
    void setMaxSpeed(Axis axis, float maxSpeed);
    void setAcceleration(Axis axis, float acceleration);
    void setStepsPerUnit(Axis axis, uint16_t steps);
    void setInverted(Axis axis, bool inverted);
    void setEnabled(Axis axis, bool enabled);

    // Command-line interface callback
    static void axisCallback(int arg_cnt, char **args);

    // Motion control methods
    void moveTo(Axis axis, long units);         // Move to absolute position
    void moveRelative(Axis axis, long units);   // Move relative to current position
    void setCurrentPosition(Axis axis, long units); // Reset current position
    void stop(Axis axis);                       // Stop motion
    void runAll();                              // Run all steppers (non-blocking)
    bool isRunning(Axis axis) const;            // Check if motor is still moving

    // Limit switch handling
    void attachLimitSwitches(Axis axis, uint8_t minPin, uint8_t maxPin);
    bool isLimitReached(Axis axis, bool minLimit) const;
    bool limitTriggered() const;
    bool isRetracting(Axis axis) const;

private:
    MotorSettings motors[3];          // Motor configuration for each axis
    AccelStepper *steppers[3];        // AccelStepper instances
    LimitSwitches limitSwitches[3];   // Limit switch info per axis
    uint8_t enablePins[3];            // Enable pin numbers

    void initializeStepper(Axis axis, uint8_t stepPin, uint8_t dirPin); // Init helper

    // Interrupt handlers for each limit switch
    static void handleInterruptXMin();
    static void handleInterruptXMax();
    static void handleInterruptYMin();
    static void handleInterruptYMax();
    static void handleInterruptZMin();
    static void handleInterruptZMax();

    static String toJson(Axis axis);           // Export motor config as JSON
    void onLimitHit(Axis axis, bool isMin);    // Called when a limit is hit

    static StepperMotors *instance;            // Singleton instance for ISR access
};

#endif /* STEPPER_MOTORS_H_ */
