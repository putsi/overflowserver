/* 
Copyright 2014 Jarmo Puttonen <jarmo.puttonen@gmail.com>. 
All rights reserved.
Use of this source code is governed by a MIT-style licence that can be found in the LICENCE file.

Overflow-server
-----------------
This is a simple server that simulates a multi-user chat server.
Server asks user for a username and after that user can send chat-messages to server.
Server can be exploited using EIP-based stack buffer overflow attack in chat-message.
*/

#define _WIN32_WINNT 0x501
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>

// Default listening port of server. 
// Different port can be supplied as a command-line argument.
// eg. chatserver.exe 48879
#define SERVER_PORT "666"
// Size for receive buffer used in winsock.
#define RECEIVE_BUFFER_SIZE 4096
// Overflow-function buffer size. Must be smaller than RECEIVE_BUFFER_SIZE.
#define OVERFLOW_BUFFER_SIZE 2000

// This function does a simple stack overflow using given data.
void Overflow(char *Data) {
	char buffer[OVERFLOW_BUFFER_SIZE];
	strcpy(buffer, Data);
}

// Print description of given winsock error code.
void PrintWinsockError(int errorCode) {
	LPSTR errorString;
	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorString, 0, NULL); 
		printf("\nEncountered following winsock error while starting server:\n%d: %s\n", errorCode, errorString);
		LocalFree(errorString);
}

DWORD WINAPI ConnectionHandler(LPVOID CSocket);
struct sockaddr_in ClientIP;

int main(int argc, char *argv[]) {
	char port[6];
	// Allow server port to be given as argument.
	// If it is not given, user default port.
	if (argc == 2) {
		strncpy(port, argv[1], 6);
	} else	strncpy(port, SERVER_PORT, 6);;
	printf("Starting overflow chat-server.\n");
	printf("Clients may connect to this server using port %s\n\n", port);
	printf("NOTE: Server port can be changed by passing new port as argument.\nEg: chatserver.exe 48879\n");
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL
	struct hints;
	int res;

	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		PrintWinsockError(res);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	res = getaddrinfo(NULL, port, &hints, &result);
	if (res != 0) {
		PrintWinsockError(WSAGetLastError());
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		PrintWinsockError(WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	res = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (res == SOCKET_ERROR) {
		PrintWinsockError(WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	res = listen(ListenSocket, SOMAXCONN);
	if (res == SOCKET_ERROR) {
		PrintWinsockError(WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server started, waiting for connections.\n");
	while (ListenSocket) {
		int ClientIPlen = sizeof(ClientIP);
		ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientIP, &ClientIPlen);
		if (ClientSocket == INVALID_SOCKET) {
			PrintWinsockError(WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		printf("Received a client connection from %s:%u\n", inet_ntoa(ClientIP.sin_addr), htons(ClientIP.sin_port));
		CreateThread(0, 0, ConnectionHandler, (LPVOID)ClientSocket, 0, 0);
	}
	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}

DWORD WINAPI ConnectionHandler(LPVOID Clientsock) {
	int ReceiveBufferLen = RECEIVE_BUFFER_SIZE;
	char *ReceiveBuffer = malloc(RECEIVE_BUFFER_SIZE);
	memset(ReceiveBuffer, 0, RECEIVE_BUFFER_SIZE);
	int res, Sendres;
	SOCKET Socket = (SOCKET)Clientsock;
	Sendres = send(Socket, "Welcome to overflow chat-server.\n", 33, 0);
	Sendres = send(Socket, "Please enter your username (max 20 characters): \n", 49, 0);
	if (Sendres == SOCKET_ERROR) {
		PrintWinsockError(WSAGetLastError());
		closesocket(Socket);
		return 1;
	}

	int usernameSet = 0;
	char username[21];
	memset(username, 0, 21);
	while (Clientsock) {
		res = recv(Socket, ReceiveBuffer, ReceiveBufferLen, 0);
		if (res > 0) {
			// If username has not been set, save string sent by user to username-variable.
			if (usernameSet != 1) {
				strncat(username, ReceiveBuffer, 21);
				// Replace newline-character of string with null-character.
				username[strlen(username) - 1] = '\x0';
				usernameSet = 1;
				memset(ReceiveBuffer, 0, RECEIVE_BUFFER_SIZE);
				printf("Client %s:%u selected username: %s\n", inet_ntoa(ClientIP.sin_addr), htons(ClientIP.sin_port), username);
				Sendres = send(Socket, "Write a message: ", 17, 0);
			}
			// Receive chat-message, run it through overflow-function and send it as chat-styled output to user.
			else {
				Overflow(ReceiveBuffer);
				// Create a string containing current date and time, username and message.
				time_t rawtime;
				struct tm * timeinfo;
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				char msgbuf[RECEIVE_BUFFER_SIZE + 100];
				sprintf(msgbuf, "\n\n%s%s said: %s\n\nWrite a message:  ", asctime(timeinfo), username, ReceiveBuffer);
				// Send created string to client.
				Sendres = send(Socket, (const char *)&msgbuf, strlen((const char *)&msgbuf), 0);
				// Clear receivebuffer.
				memset(ReceiveBuffer, 0, RECEIVE_BUFFER_SIZE);
			}
		}
		else if (res == 0) {
			printf("Client %s:%u closed connection.\n", inet_ntoa(ClientIP.sin_addr), htons(ClientIP.sin_port));
			closesocket(Socket);
			return 0;
		}
		else  {
			PrintWinsockError(WSAGetLastError());
			closesocket(Socket);
			return 1;
		}
	}
}

