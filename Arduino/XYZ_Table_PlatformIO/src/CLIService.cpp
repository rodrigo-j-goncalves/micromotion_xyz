/**
 * ===============================================================
 *  CLIService.cpp
 *  XYZ Camera Positioning System - Command Line Interface Service
 * ===============================================================
 *  Description:
 *  - Registers and routes CLI commands to appropriate services.
 *  - Enables real-time control of system and motion via serial terminal.
 *
 *  Created on: 03/04/2020
 *  Author: Ignacio Martínez Navajas
 *  E-mail: imnavajas@coit.es
 * ===============================================================
 */

#include "CLIService.h"

#define FS(x) (__FlashStringHelper*)(x) // Flash string helper macro

// Constructor
CLIService::CLIService() {}

// Destructor
CLIService::~CLIService() {}

// Start CLI service and initialize available commands
void CLIService::Begin() {
	aCmdLine.Begin();  // Start the command parser
	Init();            // Register command callbacks
}

// Register supported commands
void CLIService::Init() {
	/* System Commands */
	aCmdLine.CmdAdd("version", Version); // Prints firmware version
	aCmdLine.CmdAdd("reboot", Reboot);   // Restarts the system
	aCmdLine.CmdAdd("ram", Ram);         // Displays free RAM in bytes

	/* Stepper Configuration Command */
	aCmdLine.CmdAdd("axe", Axe);         // Modify axis settings (speed, accel, etc.)

	/* Motion Commands */
	aCmdLine.CmdAdd("move", MoveSingle); // Relative move
	aCmdLine.CmdAdd("run", Run);         // Continuous move
	aCmdLine.CmdAdd("stop", Stop);       // Stop axes

	aCmdLine.CmdInit(); // Finalize registration
}

// Poll serial input for commands
void CLIService::Loop() {
	aCmdLine.CmdPoll(); // Process any new command from serial
}

// Print command-line prompt (e.g., '>')
void CLIService::PrintPrompt() {
	aCmdLine.PrintPrompt();
}

/* ========== Command Callbacks ========== */

// System command: prints firmware name and version
void CLIService::Version(int arg_cnt, char **args) {
	MegaBoard::Version();
}

// System command: perform a software reboot
void CLIService::Reboot(int arg_cnt, char **args) {
	MegaBoard::Reboot();
}

// System command: report free RAM
void CLIService::Ram(int arg_cnt, char **args) {
	MegaBoard::Println(String(MegaBoard::FreeRam()));
}

/* Stepper motor configuration command */
void CLIService::Axe(int arg_cnt, char **args) {
	StepperMotors::axisCallback(arg_cnt, args);
}

/* Motion command: move stepper(s) to a relative position */
void CLIService::MoveSingle(int arg_cnt, char **args) {
	ControlService::MoveCallback(arg_cnt, args);
}

/* Motion command: run one or more axes continuously */
void CLIService::Run(int arg_cnt, char **args) {
	ControlService::RunCallback(arg_cnt, args);
}

/* Motion command: stop axis or all axes */
void CLIService::Stop(int arg_cnt, char **args) {
	ControlService::StopCallback(arg_cnt, args);
}
