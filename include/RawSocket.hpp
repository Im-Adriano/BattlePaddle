/*
 * RawSocket
 *
 * Main component of the RawSocket lib.
 * Abstracts the use of Raw Sockets away from the user as much as possible,
 * while still giving them full control of packet manipulation.
 *
 * Aids the user in setting up RawSockets as easily as possible.
 */

#ifndef RAWSOCKET_H
#define RAWSOCKET_H
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
#include "windivert.h"
#include "RawSocketHelper.hpp"
#define ntohs(x) WinDivertHelperNtohs(x)
#define ntohl(x) WinDivertHelperNtohl(x)
#define htons(x) WinDivertHelperHtons(x)
#define htonl(x) WinDivertHelperHtonl(x)
#endif

typedef std::vector<uint8_t> Packet;

class RawSocket {
    static const int PACKET_SIZE = 65535;

private:
    Packet packet;
    bool debugMode{};
    RawSocketHelper rawSocketHelper{};

public:
#ifdef __unix__

    explicit RawSocket(const std::string &intName, bool debug = false);

    std::vector<uint8_t> getMac();

    std::vector<uint8_t> getMacOfIP(uint32_t targetIP);

#endif
    RawSocket(uint32_t ipAddress, bool debug = false);
    
    RawSocket();

    ~RawSocket();

    Packet getPacket();

    uint32_t getIP();

    int receive();

    int send(Packet dataframe);
};

#endif