#include <windows.h>
#include <stdio.h>
#include <wchar.h>

int wmain() {
    HKEY hKey = NULL;
    HKEY hNewKey = NULL;
    LONG result;
    WCHAR originalPath[] = L"SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https";
    WCHAR tempKeyName[] = L"https1";
    
    // 1. Open the original key
    result = RegOpenKeyExW(HKEY_CURRENT_USER, originalPath, 0, KEY_ALL_ACCESS, &hKey);
    if (result != ERROR_SUCCESS) {
        wprintf(L"Failed to open original key. Error code: %ld\n", result);
        return 1;
    }
    wprintf(L"Successfully opened original key.\n");
    
    // 2. Rename the key using RegRenameKey
    result = RegRenameKey(hKey, NULL, tempKeyName);
    if (result != ERROR_SUCCESS) {
        wprintf(L"Failed to rename key. Error code: %ld\n", result);
        RegCloseKey(hKey);
        return 1;
    }
    RegCloseKey(hKey);
    wprintf(L"Successfully renamed key to temporary name.\n");
    
    // 3. Open the renamed key with its new path
    WCHAR temporaryPath[] = L"SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https1";
    result = RegOpenKeyExW(HKEY_CURRENT_USER, temporaryPath, 0, KEY_ALL_ACCESS, &hNewKey);
    if (result != ERROR_SUCCESS) {
        wprintf(L"Failed to open renamed key. Error code: %ld\n", result);
        return 1;
    }
    wprintf(L"Successfully opened renamed key.\n");
    
    // 4. Open existing UserChoice subkey
    HKEY hUserChoiceKey = NULL;
    result = RegOpenKeyExW(hNewKey, L"UserChoice", 0, KEY_ALL_ACCESS, &hUserChoiceKey);
    if (result != ERROR_SUCCESS) {
        wprintf(L"Failed to open UserChoice subkey. Error code: %ld\n", result);
        RegCloseKey(hNewKey);
        return 1;
    }
    
    // 5. Set dummy values in UserChoice
    WCHAR progIdValue[] = L"MyBrowser";
    result = RegSetValueExW(hUserChoiceKey, L"ProgId", 0, REG_SZ, 
                           (BYTE*)progIdValue, (wcslen(progIdValue) + 1) * sizeof(WCHAR));
    if (result != ERROR_SUCCESS) {
        wprintf(L"Failed to set ProgId value. Error code: %ld\n", result);
    } else {
        wprintf(L"Successfully set ProgId value.\n");
    }
    
    WCHAR hashValue[] = L"TXlCcm93c2Vy";
    result = RegSetValueExW(hUserChoiceKey, L"Hash", 0, REG_SZ, 
                           (BYTE*)hashValue, (wcslen(hashValue) + 1) * sizeof(WCHAR));
    if (result != ERROR_SUCCESS) {
        wprintf(L"Failed to set Hash value. Error code: %ld\n", result);
    } else {
        wprintf(L"Successfully set Hash value.\n");
    }
    
    // Close UserChoice key
    RegCloseKey(hUserChoiceKey);
    
    // 6. Rename the key back to its original name
    result = RegRenameKey(hNewKey, NULL, L"https");
    if (result != ERROR_SUCCESS) {
        wprintf(L"Failed to rename key back. Error code: %ld\n", result);
    } else {
        wprintf(L"Successfully renamed key back to original name.\n");
    }
    
    // 7. Clean up
    RegCloseKey(hNewKey);
    
    wprintf(L"Operation completed.\n");
    return 0;
}
