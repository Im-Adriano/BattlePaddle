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
#include <malloc.h>
#include <iostream>
#include <string.h>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <time.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#define OS_Windows
#endif



using namespace std;

typedef struct Packet{
    unsigned char* data;
    int dataLength;
};

class botSocket
{
    int PACKET_SIZE = 65535;

    private:
        int sockFd;
        bool DebugMode;
        struct sockaddr_ll* addr;
        int interfaceIndex;
        Packet * packet;

        void getInterfaceIndex(const char * inter);

        void createAddressStruct();

        void createSocket();

        void setSocketOptions();

        void bindSocket();

    public:
        botSocket(const char * intName, int debugMode);

        botSocket();
        
        ~botSocket();

        Packet * getPacket();

        void recieve();

        void send(void * dataframe);
};