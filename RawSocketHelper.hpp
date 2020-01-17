#ifdef __unix__
#include <malloc.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h> 
#include <iostream>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>	
#include <mutex>
#include <ifaddrs.h>

class RawSocketHelper{
    public: 
        int interfaceIndex;
        int sockFd;
        struct sockaddr_ll addr;
        int getInterfaceIndex(const char* inter);

        int createAddressStruct();

        int createSocket();

        int setSocketOptions();

        int bindSocket();

        int findOutwardFacingNIC(const char * destination_address);
};
#elif Defined(OS_Window)
#include <iostream>
#include <string.h>
#include <windows.h>
#include "WinDivert\windivert.h"
class RawSocketHelper{
    public: 
        HANDLE handle, console;
        INT16 priority = 0;
        WINDIVERT_ADDRESS address;
        const char* err_str;
        void setup();
};
#endif