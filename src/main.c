
#include <stdio.h>

#include "program/program.h"

int main(int argc, char const** argv) {
    int ret = setup();
    if (ret != 0) {
        printf("Err: %s\n", getErrMessage(ret));
        programCleanup();
        return ret;
    }
    
    ret = startThreads();
    printf("Cleaning up.\nProgram finished with: %s", getErrMessage(ret));

    programCleanup();
    return 0;
}
