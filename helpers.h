#pragma once
#include <iostream>
#include <string.h>
#include <algorithm>

#define BUFFER_LENGTH 4096

#ifdef _WIN32
#define timeSleep 10000
#endif

// Some important macros that really needed to be defined in linux idk im newb ;-;
#ifdef __linux__
#define SOCKET int // This is used so that we don't have to change the return type of getSocket() xDDDD
#define _popen popen
#define _pclose pclose
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#define INT_MAX 2147483647
#define Sleep sleep
#define timeSleep 10
#endif

class Helpers {
public:
	static void divider(int size = 50) {
		for (int i = 0; i < size; i++) {
			std::cout << "=";
		}
		std::cout << std::endl;
	}
	static std::string getDivider(int size = 50) {
		std::string divider;
		for (int i = 0; i < size; i++)
			divider += "=";
		divider += "\n";
		return divider;
	}
	static std::string executeCommand(const char command[]) {
		FILE* fp = NULL;
		int status;
		const int PATH_MAX = 4096;
		char path[PATH_MAX] = { 0 };
		std::string out; // This will contain the value to be returned.

		fp = _popen(command, "r");
		if (fp == NULL) {
			std::cout << "Unable to execute command!\n";
			return "";
		}

		while (fgets(path, PATH_MAX, fp) != NULL)
			out += path;

		if (fp == NULL)
			return "";
		status = _pclose(fp);
		if (status == -1) {
			std::cout << "Unable to close\n";
		}
		return out;
	}
	static void removeDisallowed(std::string& in) {
		static const std::string allowed = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:;\\.,/!@#$%^&*()-=+~`";
		in.erase(
			std::remove_if(in.begin(), in.end(), [&](const char c) {
				return allowed.find(c) == std::string::npos;
				}),
			in.end());
	}
	static char* toCharArray(std::string str) {
		char* retCharArr = new char[str.length() + 1];
		for (size_t i = 0; i <= str.length(); i++)
			retCharArr[i] = NULL;
		for (size_t i = 0; i < str.length(); i++)
			retCharArr[i] = str[i];
		return retCharArr;
	}
	static std::string getMacAddress() {
		#ifdef __linux__
			char cmd[] = "cat /sys/class/net/eth0/address";
		#endif
		#ifdef _WIN32
			char cmd[] ="powershell.exe -c \"Get-NetAdapter | Select-Object MacAddress -ExpandProperty MacAddress | Select-Object -first 1\"";
		#endif
		return removeNewline(executeCommand(cmd).c_str());
	}
	static std::string getBanner(const char* ipAddress = "0.0.0.0") {
		std::string banner = "[";
		std::string sysinfocmd;
#ifdef __linux__
		sysinfocmd = "cat /etc/os-release  | grep PRETTY_NAME | cut -d \"=\" -f 2 | tr -d '\"'";
		sysinfocmd = executeCommand(sysinfocmd.c_str());
#endif
#ifdef _WIN32
		sysinfocmd = "powershell.exe -c \"Get-ComputerInfo | Select-Object OSname -ExpandProperty OSname\"";
		sysinfocmd = executeCommand(sysinfocmd.c_str());
#endif
		banner += ipAddress;
		banner += "]=[" + getMacAddress() + "]=[" + removeNewline(sysinfocmd.c_str()) + "]\n";
		return banner;
	}
	static std::string getShell() {
		std::string shell = "";
#ifdef __linux__
		shell = "┌──[";
		shell += executeCommand("whoami");
		shell += "@" + executeCommand("hostname") + "]─[";
		shell += executeCommand("pwd") + "]";
		shell = removeNewline(shell.c_str()) + "\n└──╼ $ ";
#endif
#ifdef _WIN32
		shell = "[" + removeNewline(executeCommand("echo %username%").c_str()) + "]=[" + removeNewline(executeCommand("cd").c_str()) + "] > ";
#endif
		return shell;
	}
	static std::string removeNewline(const char* ar) {
		size_t len = strlen(ar);
		char* arr = new char[len];
		for (size_t i = 0; i < len; i++)
			arr[i] = 0;
		for (size_t i = 0; i < len; i++)
			arr[i] = ar[i];
		arr[std::remove_if(arr, arr + len, [](char c) { return c == '\n'; }) - arr] = 0;
		return arr;
	}
};
