/**
 * ===============================================================
 *  ControlService.h
 *  XYZ Camera Positioning System - CLI Command Interface with FSM
 * ===============================================================
 *  Description:
 *  - Receives CLI commands via serial to control stepper motors.
 *  - FSM handles step motion, and continuous motion.
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
 
 class ControlService {
 public:
	 ControlService();
	 void Begin();
	 void Loop();
 
	 // CLI command handlers
	 static void RunCallback(int arg_cnt, char **args);
	 static void StopCallback(int arg_cnt, char **args);
	 static void MoveCallback(int arg_cnt, char **args);	 
 
 private:
	 enum class FSMState {
		 IDLE,
		 MOVING_CONTINUOUS,
		 MOVING_STEPS
	 };
 
	 static StepperMotors motors;
	 static FSMState aState;
 
	 static void enableMotors();
	 static void disableMotors();
	 static bool limitTriggered();
 };
 
 #endif /* CONTROLSERVICE_H_ */
 