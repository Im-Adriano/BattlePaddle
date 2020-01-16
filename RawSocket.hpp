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
#elif defined(_WIN32) || defined(WIN32) 
#include <iostream>
#include <string.h>
#include <windows.h>
#include "WinDivert\windivert.h"
#define OS_Windows
#endif



using namespace std;

typedef struct {
    unsigned char* data;
    int dataLength;
}Packet;

class RawSocket
{
    int PACKET_SIZE = 65535;

    private:
        Packet * packet;
        bool debugMode;
  
    public:
        #ifdef __unix__
        RawSocket(const char* intNameOrIP, int debug, bool isIP=false);
        #elif defined(OS_Windows)
        RawSocket(int debug);
        #endif

        RawSocket();
        
        ~RawSocket();

        Packet * getPacket();

        void recieve();

        void send(Packet * dataframe);
};