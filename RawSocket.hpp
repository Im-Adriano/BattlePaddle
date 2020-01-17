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
#include "RawSocketHelper.hpp"
#include <vector>
#elif defined(_WIN32) || defined(WIN32) 
#include <iostream>
#include <string.h>
#include <windows.h>
#include "WinDivert\windivert.h"
#define OS_Windows
#endif

using namespace std;

typedef vector<uint8_t> Packet;

class RawSocket
{
    int PACKET_SIZE = 65535;

    private:
        Packet packet;
        bool debugMode;
        RawSocketHelper rawSocketHelper;
  
    public:
        #ifdef __unix__
        RawSocket(const char* intNameOrIP, bool debug, bool isIP=false);
        #elif defined(OS_Windows)
        RawSocket(int debug);
        #endif

        RawSocket();
        
        ~RawSocket();

        Packet getPacket();

        int recieve();

        int send(Packet dataframe);
};