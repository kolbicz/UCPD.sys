#ifndef WMI_H
#define WMI_H

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define WMI_E_INIT_FAILED        0x1001
#define WMI_E_CONNECT_FAILED     0x1002
#define WMI_E_CREATE_FAILED      0x1003
#define WMI_E_EXEC_FAILED        0x1004
#define WMI_E_INVALID_PARAMS     0x1005

// Function declarations
int WMI_WriteRegistryValues(
    const wchar_t* keyPath,
    const wchar_t* progId,
    const wchar_t* hash);

int WMI_DeleteRegistryKey(const wchar_t* keyPath);

#ifdef __cplusplus
}
#endif

#endif // WMI_H