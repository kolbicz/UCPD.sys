Simple PoC demonstrating an mshta.exe bypass of UCPD.sys
========================================================

This was the first public UCPD.sys bypass I released, and surprisingly, it remained effective for a long time.

I hadn't expected that. Since the approach isn't particularly elegant and isn't very AV/EDR-friendly, I ultimately limited it to the Personal edition of SetUserFTA.

Recently, Microsoft enabled UCPD.sys dynamic rules and expanded the deny list to include mshta.exe, which makes this workaround obsolete.

In any case, it was a relatively slow method, it briefly showed a window on the taskbar and momentarily stole focus.

Details:
========

mmshta.exe is the Microsoft HTML Application Host. It can launch .hta (HTML Application) files, which can include embedded script (for example VBScript or JScript). 
Those scripts can then use Windows scripting interfaces (such as WScript / Windows Script Host or ActiveX/COM objects) to perform actions like reading/writing the registry.

https://learn.microsoft.com/en-us/previous-versions/ms536495(v=vs.85)

Since UCPD.sys permits Microsoft-signed binaries, it was possible to write to (or delete) protected registry keys by leveraging mshta.exe - simply because it wasn't on the deny list at the time.

This PoC writes a .hta file to %TEMP% and executes it silently using mshta.exe.

Alternatively, it's also possible to run mshta.exe without creating a temporary file s follows:

wchar_t commandTemplate[] =<br>
    L"mshta.exe \"javascript:try{var s=new ActiveXObject('WScript.Shell');"<br>
    L"s.RegWrite('HKCU\\\\%s\\\\Hash','%s','REG_SZ');"<br>
    L"s.RegWrite('HKCU\\\\%s\\\\ProgId','%s','REG_SZ');"<br>
    L"close();}catch(e){alert('Error: '+e.message);close();}\"";
