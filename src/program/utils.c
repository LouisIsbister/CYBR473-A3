
#include "utils.h"

KEY_PAIR* initKeyPair(char normal, char onShift, char* unprStr) {
    KEY_PAIR* kp = malloc(sizeof(KEY_PAIR));
    if (kp == NULL) return NULL;
    
    kp->normal = normal;
    kp->onShift = onShift;
    kp->unprintableKeyStr = unprStr;
    return kp;
}

BOOL isKeyUnprintable(KEY_PAIR* kp) {
    if (kp == NULL) return FALSE;
    if (kp->unprintableKeyStr == NULL)
        return FALSE;
    return TRUE;
}

void cleanupKeyPair(KEY_PAIR* kp) {
    if (kp->unprintableKeyStr != NULL) 
        free(kp->unprintableKeyStr);
    free(kp);
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

void swapBOOL(BOOL* value) {
    if (*value == FALSE) *value = TRUE;
    else *value = FALSE;
}

/**
 * get appropriate error message based on error code
 */
const char* getErrMessage(ERR_CODE code) {
    switch (code) {
        case ECODE_SUCCESS: return "Success";
        case ECODE_SAFE_RET: return "Safely returned.";
        case ECODE_GET: return "GET error";
        case ECODE_POST: return "POST error";
        case ECODE_EMPTY_BUFFER: return "Empty buffer";
        case ECODE_UNKNOWN_COMMAND: return "Unknown command given.";
        case ECODE_FULL_BUFF: return "Full buffer.";
        case ECODE_NULL: return "Null pointer";
        default: return "Unknown error"; // should never be hit
    }
}

