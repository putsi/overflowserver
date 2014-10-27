overflowserver
==============

Example of stack overflow vulnerable server and exploit which can be used to gain unauthorized access to machine running this server.

Server simulates a multi-user chat server and asks user for a username and after that user can send chat-messages to server.
Server can be exploited using EIP-based stack buffer overflow attack in chat-message.
Server uses winsock2 for networking and should work atleast with Windows XP and Windows 7. 

For included exploit to work, DEP&ASLR must be disabled and few variables inside exploit-script must be modified.

Included batch-files require gcc-compiler and included exploit requires python-interpreter.

For instructions, see comments in source-files.

Use of this source code is governed by a MIT-style licence that can be found in the [LICENCE](https://raw.githubusercontent.com/putsi/overflowserver/master/LICENSE)-file.
