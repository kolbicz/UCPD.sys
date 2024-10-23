This is a simple proof of concept (PoC) demonstrating DLL injection into explorer.exe to bypass the registry protection of UCPD.sys.

I initially assumed, incorrectly, that this approach had been blocked in UCPD.sys v3.1. However, this was a misinterpretation, and this approach is still working.

Note: This PoC is not a complete solution. It requires additional handling, such as checking for already injected DLLs. 
Additionally, this technique is easily detected by modern antivirus systems, including Windows Defender, which blocks it immediately. 
As a result, this method is not suitable for most environments.

Why does this work? The DLL code is executed in the context of explorer.exe, a Microsoft-signed binary, which is not blocked by the driver.

Compile with following commands: 

x86_64-w64-mingw32-gcc -o injector.exe injector.c -municode  
x86_64-w64-mingw32-gcc -shared -o setuserfta.dll setuserfta.c -Wl,--subsystem,windows

Next, run injector.exe, and the code from the DLL will be executed. To unload the DLL, simply restart explorer.exe.
