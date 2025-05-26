
#include "../program/utils.h"

#ifndef KEYPAIR_H
#define KEYPAIR_H


/**
 * most keys have two values based upon whether caps lock
 * or shift is help down
 */
typedef struct {
    char normal;
    char onShift;
    char* unprintableKeyStr; 
} KEY_PAIR;

// functions for handling KEY_PAIR's
KEY_PAIR* initKeyPair(char normal, char onShift, char* unprStr);
BOOL isKeyUnprintable(KEY_PAIR* kp);
void cleanupKeyPair(KEY_PAIR* kp);


#endif // KEYPAIR_H


