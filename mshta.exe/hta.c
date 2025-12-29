#include <windows.h>
#include <stdio.h>

void create_temp_hta_write(const char* path, const char* key, const char* hash, const char* progid) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating HTA file.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html>\n");
    fprintf(file, "<head>\n");
    fprintf(file, "<title>SetUserFTA</title>\n");
    fprintf(file, "<HTA:APPLICATION ID=\"oHTA\" \n");
    fprintf(file, "APPLICATIONNAME=\"WriteToRegistry\" \n");
    fprintf(file, "BORDER=\"none\" \n");
    fprintf(file, "CAPTION=\"no\" \n");
    fprintf(file, "SHOWINTASKBAR=\"no\" \n");
    fprintf(file, "SINGLEINSTANCE=\"yes\" \n");
    fprintf(file, "SYSMENU=\"no\" \n");
    fprintf(file, "WINDOWSTATE=\"minimize\">\n");
    fprintf(file, "<script language=\"VBScript\">\n");
    fprintf(file, "Sub Window_OnLoad\n");
    fprintf(file, "Set WshShell = CreateObject(\"WScript.Shell\")\n");
    fprintf(file, "On Error Resume Next\n");
    fprintf(file, "WshShell.RegWrite \"HKCU\\%s\\Hash\", \"%s\", \"REG_SZ\"\n", key, hash);
    fprintf(file, "WshShell.RegWrite \"HKCU\\%s\\ProgId\", \"%s\", \"REG_SZ\"\n", key, progid);
    fprintf(file, "Set WshShell = Nothing\n");
    fprintf(file, "self.close\n");
    fprintf(file, "End Sub\n");
    fprintf(file, "</script>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");

    fclose(file);
}

void create_temp_hta_delete(const char* path, const char* key) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating HTA file.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html>\n");
    fprintf(file, "<head>\n");
    fprintf(file, "<title>SetUserFTA</title>\n");
    fprintf(file, "<HTA:APPLICATION ID=\"oHTA\" \n");
    fprintf(file, "APPLICATIONNAME=\"DeleteRegistryKey\" \n");
    fprintf(file, "BORDER=\"none\" \n");
    fprintf(file, "CAPTION=\"no\" \n");
    fprintf(file, "SHOWINTASKBAR=\"no\" \n");
    fprintf(file, "SINGLEINSTANCE=\"yes\" \n");
    fprintf(file, "SYSMENU=\"no\" \n");
    fprintf(file, "WINDOWSTATE=\"minimize\">\n");
    fprintf(file, "<script language=\"VBScript\">\n");
    fprintf(file, "Sub Window_OnLoad\n");
    fprintf(file, "Set WshShell = CreateObject(\"WScript.Shell\")\n");
    fprintf(file, "On Error Resume Next\n");
    fprintf(file, "WshShell.RegDelete \"HKCU\\%s\\\"\n", key);
    fprintf(file, "Set WshShell = Nothing\n");
    fprintf(file, "self.close\n");
    fprintf(file, "End Sub\n");
    fprintf(file, "</script>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");

    fclose(file);
}

void run_silent_hta(const char* path) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Ensure the window is hidden

    ZeroMemory(&pi, sizeof(pi));

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mshta.exe \"%s\"", path);

    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void WriteToRegistryHTA(const char* registryKey, const char* hashValue, const char* progidValue) {
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    char htaPath[MAX_PATH];
    snprintf(htaPath, sizeof(htaPath), "%s\\SetUserFTA_write.hta", tempPath);

    create_temp_hta_write(htaPath, registryKey, hashValue, progidValue);
    run_silent_hta(htaPath);

    DeleteFile(htaPath);
}

void DeleteRegistryKey(const char* registryKey) {
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    char htaPath[MAX_PATH];
    snprintf(htaPath, sizeof(htaPath), "%s\\SetUserFTA_delete.hta", tempPath);

    create_temp_hta_delete(htaPath, registryKey);
    run_silent_hta(htaPath);

    DeleteFile(htaPath);
}