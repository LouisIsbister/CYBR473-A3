
#include "utils.h"

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
 * Simply swap a 
 */
void swapBOOL(BOOL* value) {
    if (*value == FALSE) *value = TRUE;
    else *value = FALSE;
}

/**
 * get appropriate error message based on error code
 */
const char* getErrMessage(ERR_CODE code) {
    switch (code) {
        case ECODE_SUCCESS: return "Success.\n";
        case ECODE_SAFE_RET: return "Safely returned.\n";
        case ECODE_DO_SHUTDOWN: return "Remote shutdown...\n";
        case ECODE_UNKNOWN_COMMAND: return "Unknown command given.\n";
        case ECODE_GET: return "GET error.\n";
        case ECODE_POST: return "POST error.\n";
        case ECODE_EMPTY_BUFFER: return "Empty buffer.\n";
        case ECODE_FULL_BUFF: return "Full buffer.\n";
        case ECODE_NULL: return "Null pointer.\n";
        default: return "Unknown error"; // should never be hit
    }
}
