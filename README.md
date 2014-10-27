overflowserver
==============

Example of stack overflow vulnerable server and exploit which can be used to gain unauthorized access to machine running this server.
Tested with Windows XP and Windows 7.
Server uses winsock2 for networking. DEP and ASLR must be disabled for included exploit to work.

Included batch-files require gcc-compiler and included exploit requires python-interpreter.

For instructions, see comments in source-files.

Use of this source code is governed by a MIT-style licence that can be found in the [LICENCE](https://raw.githubusercontent.com/putsi/overflowserver/master/LICENSE)-file.
