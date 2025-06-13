
#include "env_detector.h"
#include "env_utils.h"
#include <psapi.h>


/**
 * Anti-VM techniques (9)
 *   1. Reserved MAC address range for VMware and VirtualBox
 *   2. Checks running processes for background VM processes
 *   3. Checks VMware and VBox registry artifacts! 
 *  
 * Anti-Debugger techniques (9)
 *   1. Performs time based detection
 *      - Seen in commands.c file, processCommands(...) function
 *   2. Check for debugger using IsDebuggerPresent() command
 *   3. Use SetLastError to check for debugger
 *   4. * Enumerate processes for known debuggers
 * 
 * Anti-Dissassembly (5)
 *   1. Inline assembly found in main.c file
 * 
 * Sandboxes (7)
 *   1. Sleep on launch for 15 minutes
 *   2. Check the number of processes is < 15
 */

// check current window

static RET_CODE detectVM();
static RET_CODE checkVMArtifacts();
static RET_CODE checkVMProcesses();
static RET_CODE checkVMMacs(char* mac);

static RET_CODE detectDebugger();
static RET_CODE detectDissassembler();
static RET_CODE detectSandbox();


RET_CODE detectAnalysisTools() {
    // try detect malware analysis tools
    RET_CODE ret = detectVM();
    if (ret != R_SAFE_RET) { return ret; }
    
    ret = detectDebugger();
    if (ret != R_SAFE_RET) { return ret; }

    ret = detectDissassembler();
    if (ret != R_SAFE_RET) { return ret; }
    
    ret = detectSandbox();
    if (ret != R_SAFE_RET) { return ret; }

    printf("No analysis tools detected!\n");
    return R_SAFE_RET;
}



// --------------------------
// 
// ---       Anti-VM      ---
// 
// --------------------------


/**
 * 
 */
static RET_CODE detectVM() {
    if (checkVMArtifacts() == R_DETECT) { return R_DETECT; }
    if (checkVMProcesses() == R_DETECT) { return R_DETECT; }

    char mac[18];
    RET_CODE ret = retrieveMAC(mac);
    printf("MAC: %s\n", mac);
    if (ret != R_SUCCESS) { return ret; }
    if (checkVMMacs(mac) == R_DETECT) { return R_DETECT; }

    return R_SAFE_RET;
}

/**
 * Simply checks whether the prefix of the mac addr is "00:0C:29" for
 * VMware environments, and then checks for "0A:00:27" and "08:00:27" 
 * we are in VBox environments!
 */
static RET_CODE checkVMMacs(char* mac) {
    // VMware mac detection
    if (strncmp(mac, VMWARE_MAC_A, MAC_PREFIX_LEN) == 0) {
        printf("VMware A detected!\n");
        return R_DETECT;
    }
    if (strncmp(mac, VMWARE_MAC_B, MAC_PREFIX_LEN) == 0) {
        printf("VMware A detected!\n");
        return R_DETECT;
    }

    // VBox mac detection
    if (strncmp(mac, VBOX_3_3_MAC, MAC_PREFIX_LEN) == 0) {
        printf("VBOX v3.3 detected!\n");
        return R_DETECT;
    }
    if (strncmp(mac, VBOX_5_2_MAC, MAC_PREFIX_LEN) == 0) {
        printf("VBOX v5.2 detected!\n");
        return R_DETECT;
    }
    return R_SAFE_RET;
}

static RET_CODE checkVMProcesses() {
    // detect vmaware processes running
    char* vmWareProcesses[3] = { "vmtoolsd", "vmwaretray", "vmwaretool" };
    BOOL result = enumProcessesForTargets(vmWareProcesses, 3);
    if (result) {
        printf("VMware Process detected!\n");
        return R_DETECT;
    }

    // detect vbox processes running
    char* vBoxProcesses[2] = { "VBoxService", "VBoxTray" };
    result = enumProcessesForTargets(vBoxProcesses, 2);
    if (result) {
        printf("VBOX Process detected!\n");
        return R_DETECT;
    }
    return R_SAFE_RET;
}

/**
 * Loop through a range of registry keys that can be found in VMware 
 * and VBox VMs. If one is found then return with the detecion flag!
 */
static RET_CODE checkVMArtifacts() {
    char* regKeys[5] = {
        // VMware registry artifacts
        "SOFTWARE\\VMware, Inc.",
        "SYSTEM\\CurrentControlSet\\Services\\vmtools",
        // VBox registry artifacts
        "SYSTEM\\CurrentControlSet\\Services\\VBoxGuest",
        "SYSTEM\\CurrentControlSet\\Services\\VBoxService",
        "SYSTEM\\CurrentControlSet\\Services\\VBoxVideo"
    };

    // check each key exists
    for (int i = 0; i < 5; i++) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regKeys[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            printf("DETECTED: %s\n", regKeys[i]);
            RegCloseKey(hKey);
            return R_DETECT;
        }
    }
    return R_SAFE_RET;
}



// ---------------------------
// 
// ---   Anti-Debugging   ----
// 
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
    if(GetLastError() != errorValue) {
        printf("Debugger detected by SetLastError!\n");
        return R_DETECT;
    }

    // anti-debugger method 4:
    char* dbgProcesses[2] = { "OLLYDBG", "Dbg" };
    res = enumProcessesForTargets(dbgProcesses, 2);
    if (res) {
        printf("Debugger Process detected!\n");
        return R_DETECT;
    }

    return R_SAFE_RET;
}



// -----------------------------
// 
// ---   Anti-Dissassembly   ---
// 
// -----------------------------


static RET_CODE detectDissassembler() {

    return R_SAFE_RET;
}



// --------------------------
// 
// ---    Anti-Sandbox    ---
// 
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

