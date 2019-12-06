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

using namespace std;

class botSocket
{
    int PACKET_SIZE = 65535;

    private:
        int sockFd;
        bool DebugMode;
        struct sockaddr_ll addr;
        int interfaceIndex;

        void getInterfaceIndex(char * interface);

        void createAddressStruct();

        void createSocket();

        void bindSocket();

    public:
        unsigned char * packet;
        int packetLen;

        botSocket(char * intName, int debugMode);

        botSocket();
        
        ~botSocket();

        void recieve();

        void send(void * dataframe);
};