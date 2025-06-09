
#include <stdio.h>
#include <iphlpapi.h>

#include "anti_vm.h"

static ERR_CODE detectVMWare(char* mac);
static ERR_CODE detectVirtualBox(char* mac);


ERR_CODE detectVM() {
    char mac[18];
    ERR_CODE stat = retrieveMAC(mac);
    if (stat != ECODE_SUCCESS) { return stat; }

    if (detectVMWare(mac) == ECODE_VMWARE_DETECTED) {
        return ECODE_VMWARE_DETECTED;
    }
    if (detectVirtualBox(mac) == ECODE_VBOX_DETECTED) {
        return ECODE_VBOX_DETECTED;
    }
    return ECODE_SAFE_RET;
}

static ERR_CODE detectVMWare(char* mac) {
    if (strncmp(mac, VMWARE_ADAPTER_PREFIX, ADAPTER_PREFIX_LEN) == 0) {
        return ECODE_VMWARE_DETECTED;
    }
    return ECODE_SAFE_RET;
}

static ERR_CODE detectVirtualBox(char* mac) {
    if (strncmp(mac, VBOX_HOST_ONLY_ADAPTER_PREFIX, ADAPTER_PREFIX_LEN) == 0) {
        return ECODE_VBOX_DETECTED;
    }
    if (strncmp(mac, VBOX_OTHER_ADAPTER_PREFIX, ADAPTER_PREFIX_LEN) == 0) {
        return ECODE_VBOX_DETECTED;
    }
    return ECODE_SAFE_RET;
}
