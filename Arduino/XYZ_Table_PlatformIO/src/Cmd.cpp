/*******************************************************************
 Copyright (C) 2009 FreakLabs
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. Neither the name of the the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

 Originally written by Christopher Wang aka Akiba.
 Please post support questions to the FreakLabs forum.

 *******************************************************************/
/*!
 \file Cmd.c

 This implements a simple command line interface for the Arduino so that
 its possible to execute individual functions within the sketch.
 */
/**************************************************************************/
#include <avr/pgmspace.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include "Cmd.h"

// text strings for command prompt (stored in flash)
const char cmd_prompt[] PROGMEM = ">";
const char cmd_unrecog[] PROGMEM = "Command not recognized.";

Cmd::Cmd()
{

	// init the msg ptr
	msg_ptr = msg;

	// init the command table list
	cmd_tbl_list = NULL;

	// init the command table
	cmd_tbl = NULL;
}

//<<destructor>>
Cmd::~Cmd()
{
	/*nothing to destruct*/
}

void Cmd::Begin(uint32_t baudRate)
{

	CMD_SERIAL.begin(baudRate); // Baud rate stated by encoder
}

/**************************************************************************/
/*!
 Generate the main command prompt, static public version
 */
/**************************************************************************/
void Cmd::PrintPrompt()
{
	cmd_display();
}

/**************************************************************************/
/*!
 Generate the main command prompt
 */
/**************************************************************************/
void Cmd::cmd_display()
{
	char buf[50];

	// IMN CMD_SERIAL.println();
	CMD_SERIAL.print("\n");

	strcpy_P(buf, cmd_prompt);
	CMD_SERIAL.print(buf);
}

/**************************************************************************/
/*!
 Parse the command line. This function tokenizes the command input, then
 searches for the command table entry associated with the commmand. Once found,
 it will jump to the corresponding function.
 */
/**************************************************************************/
void Cmd::cmd_parse(char *cmd)
{
	uint8_t argc, i = 0;
	char *argv[30];
	char buf[50];
	_cmd_t *cmd_entry;

	fflush(stdout);

	// IMN: avoid empty buffers (it crash!)
	if (cmd[0] != '\0')
	{
		strcpy(last_cmd, cmd);
		// parse the command line statement and break it up into space-delimited
		// strings. the array of strings will be saved in the argv array.
		argv[i] = strtok(cmd, " ");
		do
		{
			argv[++i] = strtok(NULL, " ");
		} while ((i < 30) && (argv[i] != NULL));

		// save off the number of arguments for the particular command.
		argc = i;

		// parse the command table for valid command. used argv[0] which is the
		// actual command name typed in at the prompt
		for (cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry =
														 cmd_entry->next)
		{
			if (!strcmp(argv[0], cmd_entry->cmd))
			{
				cmd_entry->func(argc, argv);
				cmd_display();
				return;
			}
		}
	}

	// command not recognized. print message and re-generate prompt.
	strcpy_P(buf, cmd_unrecog);
	CMD_SERIAL.print(buf);
	CMD_SERIAL.print("\n");

	cmd_display();
}

/**************************************************************************/
/*!
 This function processes the individual characters typed into the command
 prompt. It saves them off into the message buffer unless its a "backspace"
 or "enter" key.
 */
/**************************************************************************/
void Cmd::cmd_handler()
{
	char c = CMD_SERIAL.read();
	
	switch (c)
	{
	// Last command sequence
	case '.':
		strcpy(msg, last_cmd);
		msg_ptr = msg + strlen(last_cmd);
		CMD_SERIAL.print("\r\n");
		CMD_SERIAL.print(msg);
		break;
	
	case '\r':
		// terminate the msg and reset the msg ptr. then send
		// it to the handler for processing.
		*msg_ptr = '\0';
		CMD_SERIAL.print("\r\n");
		cmd_parse((char *)msg);
		msg_ptr = msg;
		break;

	case '\b':
		// backspace
		CMD_SERIAL.print(c);
		if (msg_ptr > msg)
		{
			msg_ptr--;
		}
		break;

	default:
		// normal character entered. add it to the buffer
		CMD_SERIAL.print(c); // Echo the character: removed IMN
		*msg_ptr++ = c;
		break;
	}
}

/**************************************************************************/
/*!
 This function should be called in order to assign real address space
 to msg_pointer. Should be studied if use all static variables IMN!
 */
/**************************************************************************/
void Cmd::CmdInit()
{
	msg_ptr = msg;
}

/**************************************************************************/
/*!
 This function should be set inside the main loop or serial interrupt.
 It needs to be called constantly to check if there is any available input
 at the command prompt.
 */
/**************************************************************************/
void Cmd::CmdPoll()
{
	while (CMD_SERIAL.available())
	{
		cmd_handler();
	}
}

/**************************************************************************/
/*!
 Add a command to the command table. The commands should be added in
 at the setup() portion of the sketch.
 */
/**************************************************************************/
void Cmd::CmdAdd(const char *name, void (*func)(int argc, char **argv))
{
	// alloc memory for command struct
	cmd_tbl = (_cmd_t *)malloc(sizeof(_cmd_t));

	// alloc memory for command name
	char *cmd_name = (char *)malloc(strlen(name) + 1);

	// copy command name
	strcpy(cmd_name, name);

	// terminate the command name
	cmd_name[strlen(name)] = '\0';

	// fill out structure
	cmd_tbl->cmd = cmd_name;
	cmd_tbl->func = func;
	cmd_tbl->next = cmd_tbl_list;
	cmd_tbl_list = cmd_tbl;
}

/**************************************************************************/
/*!
 Convert a string to a number. The base must be specified, ie: "32" is a
 different value in base 10 (decimal) and base 16 (hexadecimal).
 */
/**************************************************************************/
uint32_t Cmd::CmdStr2Long(char *str, uint8_t base)
{
	return strtol(str, NULL, base);
}
