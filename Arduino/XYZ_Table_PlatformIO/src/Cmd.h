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
 \file
 \ingroup


 */
/**************************************************************************/
#ifndef CMD_H
#define CMD_H

#ifndef CMD_SERIAL
	#define CMD_SERIAL Serial
#endif

#define MAX_MSG_SIZE    180
#include <stdint.h>

// command line structure
struct _cmd_t {
	char *cmd;
	void (*func)(int argc, char **argv);
	struct _cmd_t *next;
};

class Cmd {
public:
	Cmd();
	~Cmd();
	void CmdInit();
	void CmdPoll();
	void CmdAdd(const char *name, void (*func)(int argc, char **argv));
	/* Static objects and methods */
	static void Begin(uint32_t baudRate = 115200);
	static uint32_t CmdStr2Long(char *str, uint8_t base);
	static void Print(String rText);
	static void PrintPrompt(void);
private:
	char msg[MAX_MSG_SIZE];
	char last_cmd[MAX_MSG_SIZE] = {0};
	uint8_t *msg_ptr;
	_cmd_t *cmd_tbl_list, *cmd_tbl;
	void cmd_parse(char *cmd);
	void cmd_handler();
	/* Static objects and methods */
	static void cmd_display();
};
#endif //CMD_H
