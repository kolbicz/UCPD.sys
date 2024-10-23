#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>

// Get the process ID of explorer.exe
DWORD GetExplorerPID() {
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"explorer.exe") == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
    return pid;
}

// Inject the DLL into explorer.exe
BOOL InjectDLL(DWORD dwProcessId, const wchar_t *dllPath) {
    wprintf(L"Attempting to open process with PID: %lu\n", dwProcessId);
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (!hProcess) {
        wprintf(L"Failed to open process. Error: %lu\n", GetLastError());
        return FALSE;
    }

    wprintf(L"Allocating memory in target process...\n");
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, (wcslen(dllPath) + 1) * sizeof(wchar_t), MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteMemory) {
        wprintf(L"Failed to allocate memory in target process. Error: %lu\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }

    wprintf(L"Writing DLL path to allocated memory...\n");
    if (!WriteProcessMemory(hProcess, pRemoteMemory, (LPVOID)dllPath, (wcslen(dllPath) + 1) * sizeof(wchar_t), NULL)) {
        wprintf(L"Failed to write DLL path to target process. Error: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    HMODULE hKernel32 = GetModuleHandleW(L"Kernel32");
    if (!hKernel32) {
        wprintf(L"Failed to get handle to Kernel32. Error: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    LPVOID pLoadLibraryW = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibraryW) {
        wprintf(L"Failed to get address of LoadLibraryW. Error: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    wprintf(L"Creating remote thread in target process...\n");
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteMemory, 0, NULL);
    if (!hRemoteThread) {
        wprintf(L"Failed to create remote thread. Error: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    wprintf(L"Waiting for remote thread to finish (with timeout)...\n");
    DWORD waitResult = WaitForSingleObject(hRemoteThread, 5000); // 5-second timeout
    if (waitResult == WAIT_TIMEOUT) {
        wprintf(L"Timeout while waiting for the remote thread. Possible issue with LoadLibraryW.\n");
    } else if (waitResult == WAIT_OBJECT_0) {
        wprintf(L"Remote thread completed.\n");

        DWORD exitCode;
        GetExitCodeThread(hRemoteThread, &exitCode);
        if (exitCode == 0) {
            wprintf(L"LoadLibraryW failed in the target process. Check DLL path and permissions.\n");
        }
    } else {
        wprintf(L"Unexpected result from WaitForSingleObject. Error: %lu\n", GetLastError());
    }

    // Cleanup
    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return (waitResult == WAIT_OBJECT_0);
}

int wmain() {
    DWORD pid = GetExplorerPID();
    if (pid == 0) {
        wprintf(L"Explorer.exe process not found.\n");
        return 1;
    }

    wchar_t dllPath[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, dllPath) == 0) {
        wprintf(L"Failed to get current directory. Error: %lu\n", GetLastError());
        return 1;
    }

    // Append the DLL filename to the current directory
    wcscat(dllPath, L"\\setuserfta.dll");
    wprintf(L"Using DLL path: %s\n", dllPath);

    if (InjectDLL(pid, dllPath)) {
        wprintf(L"DLL injected successfully.\n");
    } else {
        wprintf(L"DLL injection failed.\n");
    }

    return 0;
}
