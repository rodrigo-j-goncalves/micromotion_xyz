/*******************************************************************
 Copyright (C) 2009 FreakLabs — see Cmd.cpp for full license text.
 Modified for micromotion_xyz v1.0.0.
 *******************************************************************/
#ifndef CMD_H
#define CMD_H

#ifndef CMD_SERIAL
    #define CMD_SERIAL Serial
#endif

#define MAX_MSG_SIZE 180

#include <stdint.h>

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

    static void Begin(uint32_t baudRate = 115200);
    static uint32_t CmdStr2Long(char *str, uint8_t base);
    static void Print(String rText);
    static void PrintPrompt(void);

private:
    char  msg[MAX_MSG_SIZE];
    char  last_cmd[MAX_MSG_SIZE] = {0};
    char *msg_ptr;                        // fixed: was uint8_t* (type mismatch)
    _cmd_t *cmd_tbl_list, *cmd_tbl;

    void cmd_parse(char *cmd);
    void cmd_handler();
    static void cmd_display();
};

#endif // CMD_H
