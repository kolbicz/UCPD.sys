PoC Code for WMI Attack on UCPD.sys
===================================

This is another method I previously used in SetUserFTA to bypass UCPD.sys.

By leveraging WMI, the actual registry access is performed by WmiPrvSE.exe, not the calling process. 
Since UCPD.sys permits access by Microsoft binaries, and WmiPrvSE.exe was not initially on the deny list, the method was effective.

However, Microsoft later added WmiPrvSE.exe to the deny list in certain UCPD.sys versions, then removed it again, only to reintroduce the restriction as an additional feature —  which now got activated in Windows 10 and 11.

As a result, this workaround is no longer effective, and I am therefore releasing the code.

Function declaration:
=====================

int WMI_WriteRegistryValues(const wchar_t* keyPath, const wchar_t* progId, const wchar_t* hash);<br>
int WMI_DeleteRegistryKey(const wchar_t* keyPath);    
