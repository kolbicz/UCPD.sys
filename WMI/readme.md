PoC Code for WMI Attack on UCPD.sys
===================================

This is another method I previously used in SetUserFTA to bypass UCPD.sys.

By leveraging WMI, the actual registry access is performed by WmiPrvSE.exe, not the calling process. 
Since UCPD.sys permits access by Microsoft binaries, and WmiPrvSE.exe was not initially on the deny list, the method was effective.

In earlier UCPD.sys versions, Microsoft briefly added WmiPrvSE.exe to the deny list, then removed it again. Later, the restriction returned as an optional/additional feature, and was eventually folded into the base protection feature set.

As a result, this workaround is no longer effective, and I am therefore releasing the code.

Function declaration:
=====================

int WMI_WriteRegistryValues(const wchar_t* keyPath, const wchar_t* progId, const wchar_t* hash);<br>
int WMI_DeleteRegistryKey(const wchar_t* keyPath);    

