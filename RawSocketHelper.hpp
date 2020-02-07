#ifdef __unix__
#include <net/ethernet.h>
#include <arpa/inet.h> 
#include <iostream>
#include <net/if.h>
#include <cstring>
#include <sys/ioctl.h>
#include <netpacket/packet.h>	
#include <ifaddrs.h>
#elif defined(_WIN32) || defined(WIN32)
#define OS_Windows
#include <iostream>
#include <string.h>
#include <windows.h>
#include "WinDivert\windivert.h"
#endif

#if __unix__
class RawSocketHelper{
    public: 
        int interfaceIndex;
        int sockFd;
        struct sockaddr_ll addr;
        uint8_t macAddress[6];
        uint32_t ipAddress;

        int getInterfaceIndexAndInfo(const char* inter);

        int createAddressStruct();

        int createSocket();

        int setSocketOptions();

        int bindSocket();

        int findOutwardFacingNIC(const char * destination_address);
};

#elif defined(OS_Windows)
class RawSocketHelper{
    public: 
        HANDLE handle;
        INT16 priority = 0;
        WINDIVERT_ADDRESS address;
        const char* err_str;
        int setup();
};
#endif