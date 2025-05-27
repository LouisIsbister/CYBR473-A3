
#include "windows.h"
#include <time.h>

#ifndef UTILS_H
#define UTILS_H

#define MAX_MSG_LEN 1028
#define MAX_BUFF_LEN 1028

#define ENC_KEY 0x2e

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

void encode(char* str, unsigned char* encKey);  // symetrically encode a string
void rotateRight(unsigned char* ch) ; // circular char rotation

time_t getCurrentTime(); // get the current time in seconds, this will be handled by the server

void swapBOOL(BOOL* value);

void printErr(ERR_CODE err);
const char* getErrMessage(ERR_CODE code);

#endif