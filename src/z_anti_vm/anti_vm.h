
#include "../program/utils.h"

#ifndef ANTI_VM_H
#define ANTI_VM_H

#define ADAPTER_PREFIX_LEN 8
#define VMWARE_ADAPTER_PREFIX "00:0C:29"
#define VBOX_HOST_ONLY_ADAPTER_PREFIX "0A:00:27"
#define VBOX_OTHER_ADAPTER_PREFIX "08:00:27"


ERR_CODE detectVM();


#endif // ANTI_VM_H