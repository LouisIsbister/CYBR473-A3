
#include "env_detector.h"
#include <psapi.h>


/**
 * This file looks for the prescence of:
 *  - VMs       (starts line X)
 *  - Sandboxes (starts line Y)
 *  - Debuggers (starts line Z)
 */

static RET_CODE detectVM();
static RET_CODE detectVMWare(char* mac);
static RET_CODE detectVirtualBox(char* mac);
static RET_CODE detectSandbox();
static RET_CODE detectDebugger();


RET_CODE detectAnalysisTools() {
    // debugger detection method 1
    LARGE_INTEGER perfCounterStart;
    LARGE_INTEGER perfCounterEnd;
    LARGE_INTEGER perfCounterFrequency;
    QueryPerformanceFrequency(&perfCounterFrequency);
    QueryPerformanceCounter(&perfCounterStart);
    

    // try detect malware analysis tools
    RET_CODE ret = detectVM();
    if (ret != R_SAFE_RET) { return ret; }
    
    ret = detectDebugger();
    if (ret != R_SAFE_RET) { return ret; }
    
    ret = detectSandbox();
    if (ret != R_SAFE_RET) { return ret; }


    // compute the time that has elapsed since the last instruction 
    QueryPerformanceCounter(&perfCounterEnd);
    LONG elapsedTime = (perfCounterEnd.QuadPart - perfCounterStart.QuadPart) / perfCounterFrequency.QuadPart;
    if (elapsedTime > 2.5) {   // if it longer than 2.5 seconds then exit
        printf("Debugger detected by performance!\n");
        return R_DETECT;
    }

    printf("No analysis tools detected!\n");
    return R_SAFE_RET;
}



// --------------------------
// ---
// ---       Anti-VM      ---
// ---
// --------------------------


/**
 * 
 */
static RET_CODE detectVM() {
    char mac[18];
    RET_CODE ret = retrieveMAC(mac);    
    if (ret != R_SUCCESS) { return ret; }

    if (detectVMWare(mac) == R_DETECT)     { return R_DETECT; } 
    if (detectVirtualBox(mac) == R_DETECT) { return R_DETECT; }
    return R_SAFE_RET;
}

/**
 * simply checks whether the prefix of the mac addr is "00:0C:29",
 * if it is then we are in a VMware environment!
 */
static RET_CODE detectVMWare(char* mac) {
    if (strncmp(mac, VMWARE_MAC_PREFIX, MAC_PREFIX_LEN) == 0) {
        printf("VMware detected!\n");
        return R_DETECT;
    }
    return R_SAFE_RET;
}

/**
 * similarly to the pervious function, checks whether the prefix of the mac addr
 * is "0A:00:27" or "08:00:27" we are in vbox this time!
 */
static RET_CODE detectVirtualBox(char* mac) {
    if (strncmp(mac, VBOX_HOST_ONLY_MAC_PREFIX, MAC_PREFIX_LEN) == 0) {
        printf("VBOX A detected!\n");
        return R_DETECT;
    } else if (strncmp(mac, VBOX_OTHER_MAC_PREFIX, MAC_PREFIX_LEN) == 0) {
        printf("VBOX B detected!\n");
        return R_DETECT;
    }
    return R_SAFE_RET;
}



// --------------------------
// ---
// ---    Anti-Sandbox    ---
// ---
// --------------------------


static RET_CODE detectSandbox() {
    DWORD processes[20];
    DWORD bytesOut;
    EnumProcesses(processes, sizeof(processes), &bytesOut);
    
    DWORD numPr = bytesOut / sizeof(DWORD);
    if (numPr < 15) {
        printf("Sandbox detected by < 15!\n");
        return R_DETECT;
    }
    return R_SAFE_RET;
}



// ---------------------------
// ---
// ---   Anti-Debugging   ----
// ---
// ---------------------------


static RET_CODE detectDebugger() {
    // debugger method 2: simply call windows api function to detect debugger
    BOOL res = IsDebuggerPresent();
    if (res) {
        printf("Debugger detected by IsDebuggerPresent!\n");
        return R_DETECT;
    }

    // debugger method 3: detect debugger using set last error!
    DWORD errorValue = 4334;
    SetLastError(errorValue);
    OutputDebugString("_");
    if(GetLastError() == errorValue) {
        printf("Debugger detected by SetLastError!\n");
        return R_DETECT;
    }


    return R_SAFE_RET;
}