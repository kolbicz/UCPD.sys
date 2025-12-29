Simple PoC demonstrating a regini.exe bypass for UCPD.sys
=========================================================

regini.exe is a command-line tool that modifies the Windows registry from a script or command prompt by applying changes defined in one or more text files. 
It can create, modify, or delete registry keys and values, and it can also update permissions (ACLs) on registry keys.

https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/regini

Details:
========

Microsoft had simply overlooked adding this executable to the deny list, which meant it could be used to modify protected registry keys.

regini.exe takes a text file containing registry directives and can create, modify, or delete keys/values accordingly.

However, if the policy “Prevent access to registry editing tools” is enabled, regini.exe will not work either.

To access a user’s registry hive, we must specify the SID of the current user.

This workaround was also used in SetUserFTA for quite some time, but regini.exe was eventually added to the UCPD.sys deny list.

I also noticed that Foxit PDF Reader had discovered the same method and used it to set itself as the default handler for .pdf files.