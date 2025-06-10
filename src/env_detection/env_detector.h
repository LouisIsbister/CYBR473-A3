
#include "../program/utils.h"

#ifndef ENV_DETECTOR_H
#define ENV_DETECTOR_H

#define MAC_PREFIX_LEN              8
#define VMWARE_MAC_PREFIX           "00:0C:29"
#define VBOX_HOST_ONLY_MAC_PREFIX   "0A:00:27"
#define VBOX_OTHER_MAC_PREFIX       "08:00:27"


RET_CODE detectAnalysisTools();


#endif // ENV_DETECTOR_H
