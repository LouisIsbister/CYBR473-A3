
#include "../program/utils.h"

#ifndef REGISTRY_H
#define REGISTRY_H

#define TARGET_REG_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"

RET_CODE generateRegKey(char* path, char* arg, LONG access);

#endif // REGISTRY_H
