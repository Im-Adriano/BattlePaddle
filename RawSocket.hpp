#ifdef __unix__

#include <iostream>
#include "RawSocketHelper.hpp"
#include <vector>
#include <iomanip>
#include <string>

#elif defined(_WIN32) || defined(WIN32)
#define OS_Windows
#include <iostream>
#include <vector>
#include <iomanip>
#include <windows.h>
#include "WinDivert\windivert.h"
#include "RawSocketHelper.hpp"
#define ntohs(x) WinDivertHelperNtohs(x)
#define ntohl(x) WinDivertHelperNtohl(x)
#define htons(x) WinDivertHelperHtons(x)
#define htonl(x) WinDivertHelperHtonl(x)
#endif
using namespace std;

typedef vector<uint8_t> Packet;
#define HEX(x) setw(2) << setfill('0') << hex << (int)( x )


class RawSocket {
    static const int PACKET_SIZE = 65535;

private:
    Packet packet;
    bool debugMode{};
    RawSocketHelper rawSocketHelper{};

public:
#ifdef __unix__

    explicit RawSocket(const string &intName, bool debug = false);

    explicit RawSocket(uint32_t IP, bool debug = false);

    RawSocket();

#elif defined(OS_Windows)
    RawSocket(uint32_t ipAddress, bool debug = false);
    
    RawSocket();
#endif

    ~RawSocket();

    Packet getPacket();

    uint32_t getIP();

    vector<uint8_t> getMac();

    vector<uint8_t> getMacOfIP(uint32_t targetIP);

    int receive();

    int send(Packet dataframe);
};