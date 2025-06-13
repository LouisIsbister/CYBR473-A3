
#include "utils.h"
#include <stdio.h>
#include <iphlpapi.h>


static void circularShiftRight(unsigned char* ch);
static const char* getRetMessage(RET_CODE code);

/**
 * This method simply finds the mac address for this device.
 * Reference: https://stackoverflow.com/questions/13646621/how-to-get-mac-address-in-windows-with-c
 * 
 * @param mac string that recieves the mac address, is declared as 'char mac[18]'
 * @return
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
 * null terminator then simply leave the character as is.
 * 
 * @param str string to be encoded
 * @param encKey initial state of the encoding key
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
 * Cicular shift the encoding char right once.
 * e.g ch = 0 0 0 0 1 1 1 1
 * 
 * ch >> 1 == 0 0 0 0 0 1 1 1
 * ch << 7 == 1 0 0 0 0 0 0 0
 * Then take the bitwise OR and we can see how it rotates right!
 * Resource: https://stackoverflow.com/questions/13289397/circular-shift-in-c
 * 
 * @param ch encoding key to be shifted
 */
static void circularShiftRight(unsigned char* ch) {
    *ch = (*ch >> 1) | (*ch << (8 - 1));
}

/**
 * @return the current time in seconds
 */
time_t getCurrentTime() {
    time_t seconds;
    time(&seconds);
    return seconds;
}

/**
 * @param value a boolean value to be swapped
 */
void swapBOOL(BOOL* value) {
    *value = !(*value);
}

/**
 * Simply print the seult of calling getRetMessage
 * 
 * @param code provided return code 
 */
void printRetCode(RET_CODE code) {
    printf("Ret: %s\n", getRetMessage(code));
}

/**
 * Return the appropriate message based on the return code
 * 
 * @param code the provided return code
 * @return the associated return code message
 */
static const char* getRetMessage(RET_CODE code) {
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
