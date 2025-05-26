
#include "windows.h"
#include <time.h>

#ifndef UTILS_H
#define UTILS_H

#define MAX_MSG_LEN 1028
#define MAX_BUFF_LEN 1028

typedef enum {
    ECODE_SUCCESS,
    ECODE_SAFE_RET,
    ECODE_DO_SHUTDOWN,
    ECODE_UNKNOWN_COMMAND,
    ECODE_GET,
    ECODE_POST,
    ECODE_EMPTY_BUFFER,
    ECODE_FULL_BUFF,
    ECODE_NULL
} ERR_CODE;

time_t getCurrentTime(); // get the current time in seconds, this will be handled by the server

void swapBOOL(BOOL* value);

const char* getErrMessage(ERR_CODE code);

#endif