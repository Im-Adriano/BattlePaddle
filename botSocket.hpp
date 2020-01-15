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

class botSocket
{
    int PACKET_SIZE = 65535;

    private:
        Packet * packet;
        bool DebugMode;
  
    public:
        #ifdef __unix__
        botSocket(const char* intName, int debugMode);
        #elif defined(OS_Windows)
        botSocket(int debugMode);
        #endif

        botSocket();
        
        ~botSocket();

        Packet * getPacket();

        void recieve();

        void send(Packet * dataframe);
};