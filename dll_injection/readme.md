This is a simple proof of concept (PoC) demonstrating DLL injection into explorer.exe to bypass the registry protection of UCPD.sys.

UCPD.sys v3.1 addresses this method by adding explorer.exe to its deny list, thereby preventing this approach from working effectively.

Note: This PoC is not a complete solution. It requires additional handling, such as checking for already injected DLLs. 
Additionally, this technique is easily detected by modern antivirus systems, including Windows Defender, which blocks it immediately. 
As a result, this method is not suitable for most environments.

compile with following commands: 

x86_64-w64-mingw32-gcc -shared -o setuserfta.dll setuserfta.c -Wl,--subsystem,windows

x86_64-w64-mingw32-gcc -o injector.exe injector.c -municode
