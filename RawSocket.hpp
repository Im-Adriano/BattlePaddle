#ifdef __unix__         
#include <iostream>
#include "RawSocketHelper.hpp"
#include <vector>
#include <iomanip>
#elif defined(_WIN32) || defined(WIN32) 
#define OS_Windows
#include <iostream>
#include <vector>
#include <iomanip>
#include <windows.h>
#include "WinDivert\windivert.h"
#include "RawSocketHelper.hpp"
#endif
using namespace std;

typedef vector<uint8_t> Packet;
#define HEX( x ) setw(2) << setfill('0') << hex << (int)( x )

   
class RawSocket
{
    static const int PACKET_SIZE = 65535;

    private:
        Packet packet;
        bool debugMode{};
        RawSocketHelper rawSocketHelper{};
  
    public:
        #ifdef __unix__
        RawSocket(const char* intNameOrIP, bool debug, bool isIP=false);
        RawSocket();
        #elif defined(OS_Windows)
        RawSocket(bool debug = false);
        #endif

        ~RawSocket();

        Packet getPacket();

        int receive();

        int send(Packet dataframe);
};