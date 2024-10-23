#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define REGISTRY_PATH L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice"
#define PROGID_VALUE L"Progid"
#define HASH_VALUE L"Hash"

// Function to write values to the registry (executed in a new thread)
DWORD WINAPI WriteRegistryValues(LPVOID lpParam) {
    HKEY hKey;
    LONG result;

    // Open the registry key
    result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS) {
        wchar_t errorMsg[256];
        swprintf(errorMsg, sizeof(errorMsg) / sizeof(wchar_t), L"Failed to open registry key. Error code: %ld", result);
        MessageBoxW(NULL, errorMsg, L"Registry Write Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Set ProgID
    const wchar_t progID[] = L"MyFakeBrowser";
    result = RegSetValueExW(hKey, PROGID_VALUE, 0, REG_SZ, (const BYTE*)progID, (wcslen(progID) + 1) * sizeof(wchar_t));
    if (result != ERROR_SUCCESS) {
        wchar_t errorMsg[256];
        swprintf(errorMsg, sizeof(errorMsg) / sizeof(wchar_t), L"Failed to write ProgID. Error code: %ld", result);
        MessageBoxW(NULL, errorMsg, L"Registry Write Error", MB_OK | MB_ICONERROR);
        RegCloseKey(hKey);
        return 1;
    }

    // Set Hash (this is just an example)
    const wchar_t hash[] = L"MyFakeHash";
    result = RegSetValueExW(hKey, HASH_VALUE, 0, REG_SZ, (const BYTE*)hash, (wcslen(hash) + 1) * sizeof(wchar_t));
    if (result != ERROR_SUCCESS) {
        wchar_t errorMsg[256];
        swprintf(errorMsg, sizeof(errorMsg) / sizeof(wchar_t), L"Failed to write Hash. Error code: %ld", result);
        MessageBoxW(NULL, errorMsg, L"Registry Write Error", MB_OK | MB_ICONERROR);
        RegCloseKey(hKey);
        return 1;
    }

    // Close the registry key
    RegCloseKey(hKey);
    MessageBoxW(NULL, L"Registry values successfully written.", L"Success", MB_OK | MB_ICONINFORMATION);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Create a new thread to write the registry values
        CloseHandle(CreateThread(NULL, 0, WriteRegistryValues, NULL, 0, NULL));
        break;
    }
    return TRUE;
}
