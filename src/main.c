
#include <stdio.h>

#include "program/program.h"

int main(int argc, const char** argv) {
    int ret = setup();
    if (ret != ECODE_SUCCESS) {
        printErr(ret);
        programCleanup();
        return ret;
    }
    
    ret = startThreads();
    printf("Cleaning up.\nProgram exited with msg: %s", getErrMessage(ret));

    programCleanup();
    return 0;
}
