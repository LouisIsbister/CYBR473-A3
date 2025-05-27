
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>

#include "commands.h"
#include "../program/program.h"

/**
 * split the cmds based upon newline characters. Then execute each one sequentially.
 */
ERR_CODE processCommands(CLIENT_HANDLER* client) {
    if (strlen(client->cmdBuffer) == 0) { return ECODE_EMPTY_BUFFER; } // no commands

    char* saveState;
    char* line = strtok_r(client->cmdBuffer, "\n", &saveState);
    
    // we check for shutdown here in case theere are commands after the shd
    while (line != NULL) {
        if (executeCommand(line) == ECODE_DO_SHUTDOWN) { // if we receive the shutdown command then exit!
            return ECODE_DO_SHUTDOWN;
        }
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
ERR_CODE executeCommand(char* cmdStr) {
    if (strncmp(cmdStr, "slp", 3) == 0) {
        progContext->sleeping = TRUE;
        doSleep(extractN(cmdStr + 3));
        progContext->sleeping = FALSE;

        return ECODE_SUCCESS;
    } else if (strncmp(cmdStr, "shd", 3) == 0) {
        progContext->shutdown = TRUE;
        return ECODE_DO_SHUTDOWN;
    } else if (strncmp(cmdStr, "pwn", 3) == 0) {
        doPawn();
        return ECODE_SUCCESS;
    } else { 
        return ECODE_UNKNOWN_COMMAND; 
    }
}

/**
 * simply sleep for n seconds, this halts all threads because the mutex was aquired
 * before the remote commands could be executed!
 */
void doSleep(int n) {
    printf("Sleeping for %ds\n", n);
    Sleep(n);
    printf("Sleep finsished!\n");
}

/**
 * print out a you've been pwned message
 */
void doPawn() {
    printf("\nUnfortunately you been pwn'ed hehehe (educationally speaking :))!\n");
    printf("You may potentially want to change you password (or not, simply live in the fast lane man).\n\n");
}

/**
 * This method will only only return a +ve int when the slp command is given, otherwise it returns 0. 
 * It will simple skip an whitespaces and then return the number if it exists
 */
int extractN(char* cmdStr) {
    while (*cmdStr != '\0' && !isdigit(*cmdStr)) { cmdStr++; }  // skip non-digit chars

    if (isdigit(*cmdStr)) { // return the number if it exists
        return atoi(cmdStr) * 1000;  // convert to seconds!
    }
    return 0;
}
