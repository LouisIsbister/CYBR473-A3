
#include <stdio.h>

#include "program/program.h"

int main(int argc, char const** argv) {
    int ret = setup();
    if (ret != 0) {
        printf("ECODE: %s\n", getErrMessage(ret));
        programCleanup();
        return ret;
    }
    
    ret = startThreads();
    printf("Cleaning up.\nProgram exited with msg: %s", getErrMessage(ret));

    programCleanup();
    return 0;
}
