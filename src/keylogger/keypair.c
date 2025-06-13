

#include "keypair.h"

/**
 * initialise a KEY_PAIR structure
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
 */
void cleanupKeyPair(KEY_PAIR* kp) {
    free(kp);
}