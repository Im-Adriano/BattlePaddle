#include <vector>
#include <string>

#include "PacketParse.hpp"

#ifdef __unix__

#include <arpa/inet.h>

#elif defined(_WIN32) || defined(WIN32)
#include "WinDivert\windivert.h"
#define ntohs(x) WinDivertHelperNtohs(x)
#define ntohl(x) WinDivertHelperNtohl(x)
#define htons(x) WinDivertHelperHtons(x)
#define htonl(x) WinDivertHelperHtonl(x)
#endif

using namespace std;
using namespace PacketParse;


uint16_t CalculateIPChecksum(vector<uint8_t> buff);

uint16_t CalculateUDPChecksum(vector<uint8_t> buff, uint32_t srcAddr, uint32_t dstAddr);

vector<uint8_t> CraftUDPPacket(uint32_t srcAddr,
                               uint32_t dstAddr,
                               uint16_t srcPort,
                               uint16_t dstPort,
                               vector<uint8_t> payload,
                               vector<uint8_t> srcMac = vector<uint8_t>(),
                               vector<uint8_t> dstMac = vector<uint8_t>());