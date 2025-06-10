
#include "../program/utils.h"

#ifndef REGISTRY_H
#define REGISTRY_H

#define RUN_DIR "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"

ERR_CODE generateRegKey(char* path, char* arg, LONG access);

#endif // REGISTRY_H
