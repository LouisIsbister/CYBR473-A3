

#include "keypair.h"

/**
 * initialise a KEY_PAIR structure
 * 
 * @param normal charcter associated with regular key press 
 * @param onShift charcter associated with key press when shift is also selected
 * @param unprStr the string representation of key presses that are not printable 
 * @return the KEY_PAIR struct
 */
KEY_PAIR* initKeyPair(char normal, char onShift, char* unprStr) {
    KEY_PAIR* kp = malloc(sizeof(KEY_PAIR));
    if (kp == NULL) { return NULL; }

    kp->normal = normal;
    kp->onShift = onShift;
    kp->unprintableKeyStr = unprStr;
    return kp;
}

/**
 * if the value of a key pairs unprintable string is not null
 * then the key must have been an unprintable one, hence
 * it must be true
 * 
 * @param kp the key pair struct
 * @return whether the key press is an unprintable string
 */
BOOL isKeyUnprintable(KEY_PAIR* kp) {
    if (kp == NULL) { return FALSE; }
    else if (kp->unprintableKeyStr == NULL) { 
        return FALSE; 
    }
    return TRUE;
}

/**
 * For a given key pair, simply free all allocated strings then free 
 * structure itself
 * 
 * @param kp the key pair to free
 */
void cleanupKeyPair(KEY_PAIR* kp) {
    free(kp);
}