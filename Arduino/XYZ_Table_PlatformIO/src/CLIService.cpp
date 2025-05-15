/*
 * CLIService.cpp
 *
 *  Created on: 03/04/2020
 *      Author: Ignacio Mart�nez Navajas
 *      E-mail: imnavajas@coit.es
 */

#include "CLIService.h"

#define FS(x) (__FlashStringHelper*)(x)

CLIService::CLIService() {
}

CLIService::~CLIService() {
}

void CLIService::Begin() {

	aCmdLine.Begin();
	/* Init this class */
	Init();

}

void CLIService::Init() {
	/* System Commands */
	aCmdLine.CmdAdd("version", Version);
	aCmdLine.CmdAdd("reboot", Reboot);
	aCmdLine.CmdAdd("ram", Ram);
	/* Stepper Commands */
	aCmdLine.CmdAdd("axe", Axe);
	/* Control Commands */
	aCmdLine.CmdAdd("move", MoveSingle);
	aCmdLine.CmdAdd("run", Run);
	aCmdLine.CmdAdd("stop", Stop);

	aCmdLine.CmdInit();

}

void CLIService::Loop() {
	// Check for incoming commands
	aCmdLine.CmdPoll();
}

void CLIService::PrintPrompt() {
	aCmdLine.PrintPrompt();
}

/* private methods */

/* System Commands */
void CLIService::Version(int arg_cnt, char **args) {
	MegaBoard::Version();
}

void CLIService::Reboot(int arg_cnt, char **args) {
	MegaBoard::Reboot();
}

void CLIService::Ram(int arg_cnt, char **args) {
	MegaBoard::Println(String(MegaBoard::FreeRam()));
}

/**** Stepper Commands ****/
void CLIService::Axe(int arg_cnt, char **args) {
	StepperMotors::axisCallback(arg_cnt, args);
}

/**** Control Commands ****/
void CLIService::MoveSingle(int arg_cnt, char **args) {
	ControlService::MoveCallback(arg_cnt, args);
}

void CLIService::Run(int arg_cnt, char **args) {
	ControlService::RunCallback(arg_cnt, args);
}

void CLIService::Stop(int arg_cnt, char **args) {
	ControlService::StopCallback(arg_cnt, args);
}
