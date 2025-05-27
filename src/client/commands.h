
#include "client.h"

#ifndef COMMANDS_H
#define COMMANDS_H

#define MAX_SLEEP_DIGITS 10

ERR_CODE processCommands(CLIENT_HANDLER* client);  // extracts all the commands
ERR_CODE executeCommand(char* cmdStr);    // matches a given command and generates COMMAND structure

void doSleep(int n); // handles slp
void doPawn();        // handles pwn

int extractN(char* cmdStr);  // extracts number from a command, only used for slp currently

#endif // COMMANDS_H