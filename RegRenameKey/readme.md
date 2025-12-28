Simple PoC for RegRenameKey Attack on UCPD.sys
==============================================

The code to block this attack has been present in UCPD.sys for several months, but it was not active until recently.

With the release of KB5055627, Microsoft activated the UCPD_RENAME_ATTACK protection feature on Windows 11, causing this attack to fail.

Details:
========

Before this change, it was possible to bypass UCPD.sys by renaming the parent key of a UserChoice key. This allowed unrestricted access without triggering any protection.

The method involved renaming, for example, the https key to https1, and then writing the Hash value without interference.
This worked because UCPD.sys validates only static registry paths, making it blind to renamed keys.

Attack flow in detail:

-> Rename: HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\Shell\Associations\UrlAssociations\https<br>
-> To: HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\Shell\Associations\UrlAssociations\https1<br>
-> Write UserChoice, Hash, ProgId<br>
-> Rename back to: HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\Shell\Associations\UrlAssociations\https<br>
-> Profit!

API definition:

LSTATUS RegRenameKey(<br>
  HKEY    hKey,<br>
  LPCWSTR lpSubKeyName,<br>
  LPCWSTR lpNewKeyName<br>
);

https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regrenamekey

The idea for this approach was originally suggested by @GHaslinger (thanks!), but I later found the same technique implemented in Mozilla’s source code:

https://searchfox.org/mozilla-central/source/toolkit/mozapps/defaultagent/SetDefaultBrowser.cpp

This might have contributed to Microsoft’s decision to finally block the method.