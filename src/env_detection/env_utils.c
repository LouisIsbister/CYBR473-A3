
#include "env_utils.h"
#include <tlhelp32.h>


/**
 * go through all running processes and attempt to match them against 
 * a target process name! i.e. "OLLYDBG" of "ida"
 */
BOOL enumProcessesForTargets(char** targets, UINT8 tarCount) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        printf("Snapshot failed.\n");
        return FALSE;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnap, &pe)) {
        CloseHandle(hSnap);
        return FALSE;
    }

    do {
        for (UINT8 i = 0; i < tarCount; i++) {
            char* procName = pe.szExeFile;
            if (strncmp(targets[i], procName, strlen(targets[i])) == 0) {
                CloseHandle(hSnap);
                return TRUE;
            }
        }
    } while (Process32Next(hSnap, &pe));

    CloseHandle(hSnap);
    return FALSE;
}
