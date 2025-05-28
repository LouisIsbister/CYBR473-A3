
#include <stdio.h>

#include "utils.h"


/**
 * Given a string pointer, iterate through each character and XOR it with 
 * the encoding key. If the current character is the key, or the key is the 
 * null terminator then simply leave the char as is. Then rotate the
 * key right and increment the string
 */
void encode(char* str, unsigned char* encKey) {
    char* s  = str;
    while (*s != '\0') {
        if (*s != *encKey && *encKey != 0x0) {
            *s ^= *encKey;
        }
        rotateRight(encKey);
        s++;
    }
}

/**
 * Shift the original char (key) to the right 1 bit, then | it 
 * with the same char shifted left by 7 bits. It is an unsigned char 
 * because when testing regular and signed char it didn't work haha
 * e.g ch = 1 2 3 4 5 6 7 8
 * 
 * ch >> 1 == 0 1 2 3 4 5 6 7
 * ch << 7 == 8 0 0 0 0 0 0 0
 * Then take the bitwise OR and we can see how it rotates right!
 * https://stackoverflow.com/questions/13289397/circular-shift-in-c
 */
void rotateRight(unsigned char* ch) {
    *ch = (*ch >> 1) | (*ch << (8 - 1));
}

/**
 * simply retrieevs the current time in seconds. The server handles 
 * conversion for better user viewing
 */
time_t getCurrentTime() {
    time_t seconds;
    time(&seconds);
    return seconds;
}

/**
 * Simply swap a bool value
 */
void swapBOOL(BOOL* value) {
    if (*value == FALSE) { *value = TRUE; }
    else { *value = FALSE; }
}

/**
 * testing function to report err codes
 */
void printErr(ERR_CODE err) {
    printf("ERR: %s\n", getErrMessage(err));
}

/**
 * get appropriate error message based on error code
 */
const char* getErrMessage(ERR_CODE code) {
    switch (code) {
        case ECODE_SUCCESS: return "Success.\n";
        case ECODE_SAFE_RET: return "Safely returned.\n";
        case ECODE_INCORRECT_ENC: return "Incorrect encoding.\n";
        case ECODE_DO_SHUTDOWN: return "Remote shutdown...\n";
        case ECODE_GET: return "GET error.\n";
        case ECODE_POST: return "POST error.\n";
        case ECODE_EMPTY_BUFFER: return "Empty buffer.\n";
        case ECODE_FULL_BUFF: return "Full buffer.\n";
        case ECODE_NULL: return "Null pointer.\n";
        default: return "Unknown error"; // should never be hit
    }
}
