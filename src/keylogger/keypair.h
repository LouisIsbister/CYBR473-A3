
#include "../program/utils.h"

#ifndef KEYPAIR_H
#define KEYPAIR_H


/**
 * most keys have two values based upon whether caps lock
 * or shift is is on/being held down. 'Normal' char is regular 
 * key press, 'onShift' accounts for when shift is held down, and 
 * for capital letters when capslock is on. The last value is
 * assigned a string representation ONLY for unprintable keys! 
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


