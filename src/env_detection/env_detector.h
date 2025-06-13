
#include "../program/utils.h"

#ifndef ENV_DETECTOR_H
#define ENV_DETECTOR_H

#define MAC_PREFIX_LEN   8
#define VMWARE_MAC_A     "00:50:56"
#define VMWARE_MAC_B     "00:0C:29"
#define VBOX_3_3_MAC     "00:21:F6" // vbox v3.3
#define VBOX_5_2_MAC     "08:00:27" // vbox v5.2


RET_CODE detectAnalysisTools();


#endif // ENV_DETECTOR_H
