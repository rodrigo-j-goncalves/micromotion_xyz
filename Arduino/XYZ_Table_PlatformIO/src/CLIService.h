/*
 * CLIService.h
 *
 *  Created on: 03/04/2020
 *      Author: Ignacio Mart�nez Navajas
 *      E-mail: imnavajas@coit.es
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
	void Begin(void);
	void Init(void);
	void Loop(void);
	void PrintPrompt(void);
private:
	Cmd aCmdLine;
	/* List of commands: */
	/* System Commands */
	static void Version(int arg_cnt, char **args);
	static void Reboot(int arg_cnt, char **args);
	static void Ram(int arg_cnt, char **args);
	/* Stepper Commands */
	static void Axe(int arg_cnt, char **args);
	/* Control Commands */
	static void MoveSingle(int arg_cnt, char **args);
	static void Run(int arg_cnt, char **args);
	static void Stop(int arg_cnt, char **args);

};

#endif /* CLISERVICE_H_ */
