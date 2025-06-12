
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef UTILS_H
#define UTILS_H


#define MAX_MSG_LEN     1028
#define MAX_BUFF_LEN    1028

/**
 * Different return codes to be utilised throughout the code
 * 
 */
typedef enum {
    R_SUCCESS, 
    R_FAILURE,
    R_SAFE_RET,
    R_INCORRECT_ENC,
    R_DO_SHUTDOWN,
    R_GET, 
    R_POST,
    R_EMPTY_BUFFER, 
    R_FULL_BUFF,
    R_NULL,
    R_DETECT
} RET_CODE;

RET_CODE retrieveMAC(char* mac);

// unsigned char* freshEncodingKeyPtr(unsigned char key);

void encode(char* str, unsigned char* encKey);  // symetrically encode a string

time_t getCurrentTime(); // get the current time in seconds, this will be handled by the server

void swapBOOL(BOOL* value);

void printErr(RET_CODE err);
const char* getRetMessage(RET_CODE code);

#endif