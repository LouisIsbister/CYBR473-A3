
#include "client.h"

#ifndef COMMANDS_H
#define COMMANDS_H

#define MAX_SLEEP_DIGITS 10

typedef enum {
    CMD_SLP,
    CMD_SHD,
    CMD_PWN
} COMMAND_ID;

typedef struct {
    COMMAND_ID cmdId;
    int n; // 0 for SHD and PWN
} COMMAND;

ERR_CODE executeCommands(CLIENT_HANDLER* client);  // extracts all the commands
ERR_CODE processCommand(char* cmdStr);    // matches a given command and generates COMMAND structure
ERR_CODE dispatchCommandExec(COMMAND* cmd);  // handles the command generated in `executeCommand`

void doSleep(COMMAND* cmd);    // handles slp
void doPawn(COMMAND* cmd);     // handles pwn

int extractN(char* cmdStr);  // extracts number from a command, only used for slp currently

#endif // COMMANDS_H