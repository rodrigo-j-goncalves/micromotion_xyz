/*******************************************************************
 Copyright (C) 2009 FreakLabs — originally written by Christopher Wang (Akiba).
 Modified for micromotion_xyz v1.0.0:
   - char* msg_ptr (was uint8_t*)
   - buffer overflow guard in cmd_handler default case
   - argv off-by-one fix in cmd_parse
   - removed '.' recall-last-command feature (conflicted with decimal numbers)
 *******************************************************************/
#include <avr/pgmspace.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include "Cmd.h"

const char cmd_prompt[]  PROGMEM = ">";
const char cmd_unrecog[] PROGMEM = "Command not recognized.";

Cmd::Cmd()
{
    msg_ptr      = msg;
    cmd_tbl_list = NULL;
    cmd_tbl      = NULL;
}

Cmd::~Cmd() {}

void Cmd::Begin(uint32_t baudRate)
{
    CMD_SERIAL.begin(baudRate);
}

void Cmd::PrintPrompt()
{
    cmd_display();
}

void Cmd::cmd_display()
{
    char buf[50];
    CMD_SERIAL.print("\n");
    strcpy_P(buf, cmd_prompt);
    CMD_SERIAL.print(buf);
}

void Cmd::cmd_parse(char *cmd)
{
    uint8_t argc;
    uint8_t i = 0;
    char   *argv[30];
    char    buf[50];
    _cmd_t *cmd_entry;

    fflush(stdout);

    if (cmd[0] == '\0')
        goto unrecognized;

    strcpy(last_cmd, cmd);

    // Tokenize — write argv[0..28] at most (argv has 30 slots, indices 0-29).
    // Check the bound BEFORE writing to avoid the off-by-one overrun.
    argv[0] = strtok(cmd, " ");
    i = 0;
    while (argv[i] != NULL && i < 29) {
        argv[i + 1] = strtok(NULL, " ");
        i++;
    }
    argc = i;  // number of valid tokens (argv[0..argc-1] are non-NULL)

    for (cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry = cmd_entry->next) {
        if (!strcmp(argv[0], cmd_entry->cmd)) {
            cmd_entry->func(argc, argv);
            cmd_display();
            return;
        }
    }

unrecognized:
    strcpy_P(buf, cmd_unrecog);
    CMD_SERIAL.print(buf);
    CMD_SERIAL.print("\n");
    cmd_display();
}

void Cmd::cmd_handler()
{
    char c = CMD_SERIAL.read();

    switch (c)
    {
    case '\r':
        *msg_ptr = '\0';
        CMD_SERIAL.print("\r\n");
        cmd_parse((char *)msg);
        msg_ptr = msg;
        break;

    case '\b':
        CMD_SERIAL.print(c);
        if (msg_ptr > msg)
            msg_ptr--;
        break;

    default:
        // Guard against buffer overflow — leave room for the null terminator.
        // '.' is treated as a regular character (decimal numbers in commands).
        if (msg_ptr < msg + MAX_MSG_SIZE - 1) {
            CMD_SERIAL.print(c);
            *msg_ptr++ = c;
        }
        break;
    }
}

void Cmd::CmdInit()
{
    msg_ptr = msg;
}

void Cmd::CmdPoll()
{
    while (CMD_SERIAL.available())
        cmd_handler();
}

void Cmd::CmdAdd(const char *name, void (*func)(int argc, char **argv))
{
    cmd_tbl = (_cmd_t *)malloc(sizeof(_cmd_t));

    char *cmd_name = (char *)malloc(strlen(name) + 1);
    strcpy(cmd_name, name);
    cmd_name[strlen(name)] = '\0';

    cmd_tbl->cmd  = cmd_name;
    cmd_tbl->func = func;
    cmd_tbl->next = cmd_tbl_list;
    cmd_tbl_list  = cmd_tbl;
}

uint32_t Cmd::CmdStr2Long(char *str, uint8_t base)
{
    return strtol(str, NULL, base);
}
