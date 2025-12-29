#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <sddl.h>

// Function to check if regini.exe exists
int ReginiExists() {
    char reginiPath[MAX_PATH];
    if (SearchPathA(NULL, "regini.exe", NULL, MAX_PATH, reginiPath, NULL) == 0) {
        return 0;  // regini.exe not found
    }
    return 1;  // regini.exe found
}

// Function to get the current user's SID
int GetCurrentUserSid(char* sidString, size_t size) {
    HANDLE tokenHandle = NULL;
    DWORD tokenSize = 0;
    PTOKEN_USER tokenUser = NULL;
    char* sid = NULL;
    int result = 0;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
        return 0;
    }

    GetTokenInformation(tokenHandle, TokenUser, NULL, 0, &tokenSize);
    tokenUser = (PTOKEN_USER)malloc(tokenSize);
    if (tokenUser == NULL) {
        CloseHandle(tokenHandle);
        return 0;
    }

    if (!GetTokenInformation(tokenHandle, TokenUser, tokenUser, tokenSize, &tokenSize)) {
        free(tokenUser);
        CloseHandle(tokenHandle);
        return 0;
    }

    if (!ConvertSidToStringSidA(tokenUser->User.Sid, &sid)) {
        free(tokenUser);
        CloseHandle(tokenHandle);
        return 0;
    }

    strncpy(sidString, sid, size);
    sidString[size - 1] = '\0';
    result = 1;

    LocalFree(sid);
    free(tokenUser);
    CloseHandle(tokenHandle);
    return result;
}

// Function to write to the registry using regini.exe
void WriteToRegistry(const char* subkey, const char* progId, const char* hash) {
    FILE* iniFile;
    char iniContent[1024];
    char command[1024];
    char tempFilePath[MAX_PATH];
    char userSid[256];

    // Check if regini.exe exists
    if (!ReginiExists()) {
        fprintf(stderr, "regini.exe not found in PATH.\n");
        return;
    }

    // Get the current user's SID
    if (!GetCurrentUserSid(userSid, sizeof(userSid))) {
        fprintf(stderr, "Error getting current user SID.\n");
        return;
    }

    // Get a temporary file path
    if (GetTempPathA(MAX_PATH, tempFilePath) == 0) {
        fprintf(stderr, "Error getting temporary path.\n");
        return;
    }

    // Append a temporary file name to the path
    strcat(tempFilePath, "temp.ini");

    // Create the INI file content for HKCU
    snprintf(iniContent, sizeof(iniContent),
             "\\Registry\\User\\%s\\%s\nProgId = \"%s\"\nHash = \"%s\"\n", userSid, subkey, progId, hash);

    // Create and write to the temporary INI file
    iniFile = fopen(tempFilePath, "w");
    if (iniFile == NULL) {
        fprintf(stderr, "Error creating temporary INI file.\n");
        return;
    }

    fprintf(iniFile, "%s", iniContent);
    fclose(iniFile);

    // Execute regini.exe with the temporary INI file and suppress output
    snprintf(command, sizeof(command), "regini.exe %s >NUL 2>&1", tempFilePath);
    if (system(command) != 0) {
        fprintf(stderr, "Error executing regini.exe.\n");
        remove(tempFilePath);
        return;
    }

    // Clean up the temporary INI file
    remove(tempFilePath);
}

// Function to delete a registry key using regini.exe
void DeleteRegistryKey(const char* subkey) {
    FILE* iniFile;
    char iniContent[1024];
    char command[1024];
    char tempFilePath[MAX_PATH];
    char userSid[256];

    // Check if regini.exe exists
    if (!ReginiExists()) {
        fprintf(stderr, "regini.exe not found in PATH.\n");
        return;
    }

    // Get the current user's SID
    if (!GetCurrentUserSid(userSid, sizeof(userSid))) {
        fprintf(stderr, "Error getting current user SID.\n");
        return;
    }

    // Get a temporary file path
    if (GetTempPathA(MAX_PATH, tempFilePath) == 0) {
        fprintf(stderr, "Error getting temporary path.\n");
        return;
    }

    // Append a temporary file name to the path
    strcat(tempFilePath, "temp.ini");

    // Create the INI file content to delete the registry key
    snprintf(iniContent, sizeof(iniContent),
             "\\Registry\\User\\%s\\%s [DELETE]\n", userSid, subkey);

    // Create and write to the temporary INI file
    iniFile = fopen(tempFilePath, "w");
    if (iniFile == NULL) {
        fprintf(stderr, "Error creating temporary INI file.\n");
        return;
    }

    fprintf(iniFile, "%s", iniContent);
    fclose(iniFile);

    // Execute regini.exe with the temporary INI file and suppress output
    snprintf(command, sizeof(command), "regini.exe %s >NUL 2>&1", tempFilePath);
    if (system(command) != 0) {
        fprintf(stderr, "Error executing regini.exe.\n");
        remove(tempFilePath);
        return;
    }

    // Clean up the temporary INI file
    remove(tempFilePath);
}


