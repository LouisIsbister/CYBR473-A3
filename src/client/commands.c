
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include "commands.h"
#include "../program/program.h"

/**
 * split the cmds based upon newline characters. Then execute each one sequenetially.
 */
int executeCommands(CLIENT_HANDLER* client) {
    if (strlen(client->cmdBuffer) == 0) return ECODE_EMPTY_BUFFER; // no commands

    char* saveState;
    char* line = strtok_r(client->cmdBuffer, "\n", &saveState);
    
    // we check for shutdown here in case theere are commands after the shd
    while (line != NULL && !progContext->shutdown) {
        executeCommand(line);
        line = strtok_r(NULL, "\n", &saveState);
    }

    // clean out the commands
    memset(client->cmdBuffer, '\0', MAX_BUFF_LEN);
    return ECODE_SUCCESS;
}

/**
 * takes a single command, matching the given action and generating a new 
 * COMMAND struct. Then dispatches command execute before freeing the memory again
 */
int executeCommand(char* cmdStr) {
    COMMAND* cmd = malloc(sizeof(COMMAND));
    if (cmd == NULL) return ECODE_NULL;

    if (strncmp(cmdStr, "slp", 3) == 0)      cmd->cmdId = CMD_SLP; 
    else if (strncmp(cmdStr, "shd", 3) == 0) cmd->cmdId = CMD_SHD;
    else if (strncmp(cmdStr, "pwn", 3) == 0) cmd->cmdId = CMD_PWN;
    else { free(cmd); return ECODE_UNKNOWN_COMMAND; }

    cmdStr = cmdStr + 3;
    cmd->n = extractN(cmdStr);

    dispatchCommand(cmd);
    free(cmd);
    return ECODE_SUCCESS;
}

void dispatchCommand(COMMAND* cmd) {
    switch (cmd->cmdId) {
        case CMD_SLP: doSleep(cmd);    break;
        case CMD_SHD: doShutdown(cmd); break;
        case CMD_PWN: doPawn(cmd);     break;
    }
}

/**
 * simply sleep for n seconds, this halts all threads because the mutex was aquired
 * before the remote commands could be executed!
 */
void doSleep(COMMAND* cmd) {
    printf("Sleeping for %ds\n", cmd->n);
    Sleep(cmd->n);
    printf("Sleep finsished!\n");
}

/**
 * simply set the shutdown flag in to true in the program context
 */
void doShutdown(COMMAND* cmd) {
    printf("Shutting down execution...\n");
    progContext->shutdown = TRUE;
}

/**
 * print out a you've been pwned message
 */
void doPawn(COMMAND* cmd) {
    printf("Unfortunately you been pwn'ed hehehe (educationally speaking :))!\n");
    printf("You may potentially want to change you password (or not, simply live in the fast lane man).\n");
}

/**
 * This methid will only only return a +ve int when the slp command is given, otherwise it returns 0. 
 * It will simple skip an whitespaces and then return the number if it exists
 */
int extractN(char* cmdStr) {
    while (*cmdStr != '\0' && !isdigit(*cmdStr)) cmdStr++;  // skip non-digit chars

    if (isdigit(*cmdStr)) // return the number if it exists
        return atoi(cmdStr) * 1000;  // convert to seconds!
    return 0;
}
