
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
    int n;  // 0 for SHD and PWN
} COMMAND;

int executeCommands(CLIENT_HANDLER* client);  // extracts all the commands
int executeCommand(char* cmdStr);    // matches a given command and generates COMMAND structure
void dispatchCommand(COMMAND* cmd);  // handles the command generated in `executeCommand`

void doSleep(COMMAND* cmd);    // handles slp
void doShutdown(COMMAND* cmd); // handles shd
void doPawn(COMMAND* cmd);     // handles pwn

int extractN(char* cmdStr);  // extracts number from a command, only used for slp currently

#endif // COMMANDS_H