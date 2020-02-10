#ifdef __unix__

#include <net/ethernet.h>
#include <arpa/inet.h>
#include <iostream>
#include <net/if.h>
#include <cstring>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <ifaddrs.h>
#include <vector>

#elif defined(_WIN32) || defined(WIN32)
#define OS_Windows
#include <iostream>
#include <string.h>
#include <windows.h>
#include "WinDivert\windivert.h"
#endif

using namespace std;

#if __unix__
struct arp {
    uint8_t dst_mac[6]{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t src_mac[6]{};
    uint16_t type = htons(0x0806);
    uint16_t hardware_type = htons(0x0001);
    uint16_t protocol_type = htons(0x0800);
    uint8_t hardware_len = 0x06;
    uint8_t protocol_len = 0x04;
    uint16_t opcode = htons(0x0001);
    uint8_t sender_mac[6]{};
    uint8_t sender_ip[4]{};
    uint8_t target_mac[6]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t target_ip[4]{};
};

class RawSocketHelper {
public:
    int interfaceIndex;
    int sockFd;
    struct sockaddr_ll addr;
    uint8_t macAddress[6];
    uint32_t ipAddress;

    int getInterfaceIndexAndInfo(const char *inter);

    int createAddressStruct();

    int createSocket();

    int setSocketOptions();

    int bindSocket();

    int findOutwardFacingNIC(const char *destination_address);

    vector<uint8_t> getMacOfIP(uint32_t targetIP);
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