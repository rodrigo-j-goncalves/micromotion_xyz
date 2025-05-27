/**
 * ===============================================================
 *  CLIService.h
 *  XYZ Camera Positioning System - Command Line Interface Service
 * ===============================================================
 *  Description:
 *  - Provides a serial-based command-line interface (CLI).
 *  - Registers system, motion, and stepper motor commands.
 *  - Acts as the main user entry point to control the system via serial.
 *
 *  Created on: 03/04/2020
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#ifndef CLISERVICE_H_
#define CLISERVICE_H_

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "Cmd.h"
#include "MegaBoard.h"
#include "ControlService.h"
#include "StepperMotors.h"

class CLIService {
public:
	CLIService();
	virtual ~CLIService();

	void Begin();        // Starts the CLI system
	void Init();         // Registers commands
	void Loop();         // Polls serial input for commands
	void PrintPrompt();  // Prints a command prompt (e.g., ">")

private:
	Cmd aCmdLine;  // Command line parser instance

	// System-level commands
	static void Version(int arg_cnt, char **args); // Print firmware version
	static void Reboot(int arg_cnt, char **args);  // Reboot the device
	static void Ram(int arg_cnt, char **args);     // Report free RAM

	// Stepper configuration command
	static void Axe(int arg_cnt, char **args);     // Configure axis settings

	// Motion control commands
	static void MoveSingle(int arg_cnt, char **args); // Move command
	static void Run(int arg_cnt, char **args);        // Continuous movement
	static void Stop(int arg_cnt, char **args);       // Stop motion
};

#endif /* CLISERVICE_H_ */
