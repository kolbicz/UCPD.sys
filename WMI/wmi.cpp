#include "wmi.h"
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <stdio.h>
#include <wchar.h>

#pragma comment(lib, "wbemuuid.lib")

// Helper function to initialize COM and WMI
static HRESULT InitializeWMI(IWbemServices** pSvc) {
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        wprintf(L"Failed to initialize COM library. Error code = 0x%x\n", hr);
        return hr;
    }

    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hr)) {
        wprintf(L"Failed to initialize security. Error code = 0x%x\n", hr);
        CoUninitialize();
        return hr;
    }

    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );

    if (FAILED(hr)) {
        wprintf(L"Failed to create IWbemLocator object. Error code = 0x%x\n", hr);
        CoUninitialize();
        return hr;
    }

    hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        pSvc
    );

    pLoc->Release();

    if (FAILED(hr)) {
        wprintf(L"Failed to connect to WMI. Error code = 0x%x\n", hr);
        CoUninitialize();
        return hr;
    }

    hr = CoSetProxyBlanket(
        *pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(hr)) {
        wprintf(L"Failed to set proxy blanket. Error code = 0x%x\n", hr);
        (*pSvc)->Release();
        CoUninitialize();
        return hr;
    }

    return S_OK;
}

extern "C" {

int WMI_WriteRegistryValues(
    const wchar_t* keyPath,
    const wchar_t* progId,
    const wchar_t* hash) {
    
    if (!keyPath || !progId || !hash) {
        wprintf(L"Invalid parameters provided\n");
        return WMI_E_INVALID_PARAMS;
    }

    IWbemServices* pSvc = NULL;
    HRESULT hr = InitializeWMI(&pSvc);
    if (FAILED(hr)) {
        return WMI_E_INIT_FAILED;
    }

    IWbemClassObject* pClass = NULL;
    hr = pSvc->GetObject(_bstr_t(L"StdRegProv"), 0, NULL, &pClass, NULL);
    
    if (FAILED(hr)) {
        wprintf(L"Failed to get StdRegProv object. Error code = 0x%x\n", hr);
        pSvc->Release();
        CoUninitialize();
        return WMI_E_CREATE_FAILED;
    }

    // First, create the key if it doesn't exist
    IWbemClassObject* pInParamsDefinition = NULL;
    hr = pClass->GetMethod(L"CreateKey", 0, &pInParamsDefinition, NULL);
    
    if (FAILED(hr)) {
        wprintf(L"Failed to get method definition. Error code = 0x%x\n", hr);
        pClass->Release();
        pSvc->Release();
        CoUninitialize();
        return WMI_E_CREATE_FAILED;
    }

    IWbemClassObject* pInParams = NULL;
    hr = pInParamsDefinition->SpawnInstance(0, &pInParams);
    
    // Set the registry key parameters
    VARIANT varCommand;
    VariantInit(&varCommand);
    
    // Set hive to HKEY_CURRENT_USER
    V_VT(&varCommand) = VT_I4;
    V_I4(&varCommand) = 0x80000001; // HKEY_CURRENT_USER
    hr = pInParams->Put(L"hDefKey", 0, &varCommand, 0);
    
    // Set key path
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(keyPath);
    hr = pInParams->Put(L"sSubKeyName", 0, &varCommand, 0);
    VariantClear(&varCommand);

    // Create the key
    hr = pSvc->ExecMethod(
        _bstr_t(L"StdRegProv"),
        _bstr_t(L"CreateKey"),
        0,
        NULL,
        pInParams,
        NULL,
        NULL
    );

    if (FAILED(hr)) {
        wprintf(L"Failed to create registry key. Error code = 0x%x\n", hr);
        pInParams->Release();
        pInParamsDefinition->Release();
        pClass->Release();
        pSvc->Release();
        CoUninitialize();
        return WMI_E_EXEC_FAILED;
    }

    // Now set up to write the ProgId value
    pInParams->Release();
    pInParamsDefinition->Release();

    hr = pClass->GetMethod(L"SetStringValue", 0, &pInParamsDefinition, NULL);
    hr = pInParamsDefinition->SpawnInstance(0, &pInParams);

    // Set parameters for writing the ProgId value
    VariantInit(&varCommand);
    
    // Set hive to HKEY_CURRENT_USER
    V_VT(&varCommand) = VT_I4;
    V_I4(&varCommand) = 0x80000001; // HKEY_CURRENT_USER
    hr = pInParams->Put(L"hDefKey", 0, &varCommand, 0);
    
    // Set key path
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(keyPath);
    hr = pInParams->Put(L"sSubKeyName", 0, &varCommand, 0);

    // Set value name to "ProgId"
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(L"ProgId");
    hr = pInParams->Put(L"sValueName", 0, &varCommand, 0);

    // Set the ProgId value
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(progId);
    hr = pInParams->Put(L"sValue", 0, &varCommand, 0);
    VariantClear(&varCommand);

    // Execute the method to write ProgId
    hr = pSvc->ExecMethod(
        _bstr_t(L"StdRegProv"),
        _bstr_t(L"SetStringValue"),
        0,
        NULL,
        pInParams,
        NULL,
        NULL
    );

    if (FAILED(hr)) {
        wprintf(L"Failed to write ProgId value. Error code = 0x%x\n", hr);
        pInParams->Release();
        pInParamsDefinition->Release();
        pClass->Release();
        pSvc->Release();
        CoUninitialize();
        return WMI_E_EXEC_FAILED;
    }

    // Now set up to write the Hash value
    pInParams->Release();
    pInParamsDefinition->Release();

    hr = pClass->GetMethod(L"SetStringValue", 0, &pInParamsDefinition, NULL);
    hr = pInParamsDefinition->SpawnInstance(0, &pInParams);

    // Set parameters for writing the Hash value
    VariantInit(&varCommand);
    
    // Set hive to HKEY_CURRENT_USER
    V_VT(&varCommand) = VT_I4;
    V_I4(&varCommand) = 0x80000001; // HKEY_CURRENT_USER
    hr = pInParams->Put(L"hDefKey", 0, &varCommand, 0);
    
    // Set key path
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(keyPath);
    hr = pInParams->Put(L"sSubKeyName", 0, &varCommand, 0);

    // Set value name to "Hash"
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(L"Hash");
    hr = pInParams->Put(L"sValueName", 0, &varCommand, 0);

    // Set the hash value
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(hash);
    hr = pInParams->Put(L"sValue", 0, &varCommand, 0);
    VariantClear(&varCommand);
    VariantClear(&varCommand);

    // Execute the method
    IWbemClassObject* pOutParams = NULL;
    hr = pSvc->ExecMethod(
        _bstr_t(L"StdRegProv"),
        _bstr_t(L"SetStringValue"),
        0,
        NULL,
        pInParams,
        &pOutParams,
        NULL
    );

    if (FAILED(hr)) {
        wprintf(L"Failed to execute WMI method. Error code = 0x%x\n", hr);
        pInParams->Release();
        pInParamsDefinition->Release();
        pClass->Release();
        pSvc->Release();
        CoUninitialize();
        return WMI_E_EXEC_FAILED;
    }

    // Cleanup
    if (pOutParams) pOutParams->Release();
    pInParams->Release();
    pInParamsDefinition->Release();
    pClass->Release();
    pSvc->Release();
    CoUninitialize();

    return S_OK;
}

int WMI_DeleteRegistryKey(const wchar_t* keyPath) {
    if (!keyPath) {
        wprintf(L"Invalid key path provided\n");
        return WMI_E_INVALID_PARAMS;
    }

    IWbemServices* pSvc = NULL;
    HRESULT hr = InitializeWMI(&pSvc);
    if (FAILED(hr)) {
        return WMI_E_INIT_FAILED;
    }

    // Create WMI query for deleting registry key
    IWbemClassObject* pClass = NULL;
    hr = pSvc->GetObject(_bstr_t(L"StdRegProv"), 0, NULL, &pClass, NULL);
    
    if (FAILED(hr)) {
        wprintf(L"Failed to get StdRegProv object. Error code = 0x%x\n", hr);
        pSvc->Release();
        CoUninitialize();
        return WMI_E_CREATE_FAILED;
    }

    // Set up the input parameters
    IWbemClassObject* pInParamsDefinition = NULL;
    hr = pClass->GetMethod(L"DeleteKey", 0, &pInParamsDefinition, NULL);
    
    if (FAILED(hr)) {
        wprintf(L"Failed to get method definition. Error code = 0x%x\n", hr);
        pClass->Release();
        pSvc->Release();
        CoUninitialize();
        return WMI_E_CREATE_FAILED;
    }

    IWbemClassObject* pInParams = NULL;
    hr = pInParamsDefinition->SpawnInstance(0, &pInParams);
    
    // Set parameters for deleting the key
    VARIANT varCommand;
    VariantInit(&varCommand);
    
    // Set hive to HKEY_CURRENT_USER
    V_VT(&varCommand) = VT_I4;
    V_I4(&varCommand) = 0x80000001; // HKEY_CURRENT_USER
    hr = pInParams->Put(L"hDefKey", 0, &varCommand, 0);
    
    // Set key path
    V_VT(&varCommand) = VT_BSTR;
    V_BSTR(&varCommand) = SysAllocString(keyPath);
    hr = pInParams->Put(L"sSubKeyName", 0, &varCommand, 0);
    VariantClear(&varCommand);

    // Execute the method
    IWbemClassObject* pOutParams = NULL;
    hr = pSvc->ExecMethod(
        _bstr_t(L"StdRegProv"),
        _bstr_t(L"DeleteKey"),
        0,
        NULL,
        pInParams,
        &pOutParams,
        NULL
    );

    if (FAILED(hr)) {
        wprintf(L"Failed to execute WMI method. Error code = 0x%x\n", hr);
        pInParams->Release();
        pInParamsDefinition->Release();
        pClass->Release();
        pSvc->Release();
        CoUninitialize();
        return WMI_E_EXEC_FAILED;
    }

    // Cleanup
    if (pOutParams) pOutParams->Release();
    pInParams->Release();
    pInParamsDefinition->Release();
    pClass->Release();
    pSvc->Release();
    CoUninitialize();

    return S_OK;
}

} // extern "C"