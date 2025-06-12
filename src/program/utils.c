

#include "utils.h"

#include <stdio.h>
#include <iphlpapi.h>



static void circularShiftRight(unsigned char* ch);

/**
 * Given a string declared as 'char mac[18]', find the mac address for this device and store it in 'mac'
 * https://stackoverflow.com/questions/13646621/how-to-get-mac-address-in-windows-with-c
 */
RET_CODE retrieveMAC(char* mac) {
    DWORD bufferLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO adapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));

    if (adapterInfo == NULL) { return R_NULL; }

    if (GetAdaptersInfo(adapterInfo, &bufferLen) != NO_ERROR) { return R_FAILURE; }

    BYTE* addr = adapterInfo->Address;
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    free(adapterInfo);
    return R_SUCCESS;
}

/**
 * Given a string pointer, iterate through each character and XOR it with 
 * the encoding key. If the current character is the key, or the key is the 
 * null terminator then simply leave the char as is. Then rotate the
 * key right and increment the string
 */
void encode(char* str, unsigned char* encKey) {
    char* s  = str;
    while (*s != '\0') {
        // don't encode chars that match the key or if key is 0
        if (*s != *encKey && *encKey != 0x0) {
            *s ^= *encKey;
        }
        circularShiftRight(encKey);
        s++;
    }
}

/**
 * Shift the original char (key) to the right 1 bit, then | it 
 * with the same char shifted left by 7 bits. It is an unsigned char 
 * because when testing regular and signed char it didn't work haha
 * e.g ch = 0 0 0 0 1 1 1 1
 * 
 * ch >> 1 == 0 0 0 0 0 1 1 1
 * ch << 7 == 1 0 0 0 0 0 0 0
 * Then take the bitwise OR and we can see how it rotates right!
 * https://stackoverflow.com/questions/13289397/circular-shift-in-c
 */
static void circularShiftRight(unsigned char* ch) {
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
void printErr(RET_CODE err) {
    printf("ERR: %s\n", getRetMessage(err));
}

/**
 * get appropriate error message based on error code
 */
const char* getRetMessage(RET_CODE code) {
    switch (code) {
        case R_SUCCESS: return "Success.\n";
        case R_FAILURE: return "Failure.\n";
        case R_SAFE_RET: return "Safely returned.\n";
        case R_INCORRECT_ENC: return "Incorrect encoding.\n";
        case R_DO_SHUTDOWN: return "Remote shutdown...\n";
        case R_GET: return "GET error.\n";
        case R_POST: return "POST error.\n";
        case R_EMPTY_BUFFER: return "Empty buffer.\n";
        case R_FULL_BUFF: return "Full buffer.\n";
        case R_NULL: return "Null pointer.\n";
        case R_DETECT: return "Analysis tool detected!\n";
    }
    return "Unknown error"; // should never be hit
}
