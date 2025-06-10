
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef UTILS_H
#define UTILS_H


#define MAX_MSG_LEN     1028
#define MAX_BUFF_LEN    1028

typedef enum {
    ECODE_SUCCESS,
    ECODE_FAILURE,
    ECODE_SAFE_RET,
    ECODE_INCORRECT_ENC,
    
    ECODE_DO_SHUTDOWN,

    ECODE_GET,
    ECODE_POST,

    ECODE_EMPTY_BUFFER,
    ECODE_FULL_BUFF,
    ECODE_NULL,

    ECODE_VMWARE_DETECTED,
    ECODE_VBOX_DETECTED,
} ERR_CODE;

ERR_CODE retrieveMAC(char* mac);

unsigned char* freshEncodingKeyPtr(unsigned char key);

void encode(char* str, unsigned char* encKey);  // symetrically encode a string
void rotateRight(unsigned char* ch) ; // circular char rotation

time_t getCurrentTime(); // get the current time in seconds, this will be handled by the server

void swapBOOL(BOOL* value);

void printErr(ERR_CODE err);
const char* getErrMessage(ERR_CODE code);

#endif