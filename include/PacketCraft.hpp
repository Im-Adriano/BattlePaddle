/*
 * PacketCraft
 *
 * Contains functions to help aid the user in creation of packets.
 * Takes the headache out of using Raw Sockets.
 */

#ifndef PACKETCRAFT_H
#define PACKETCRAFT_H

#include <vector>
#include <string>

#include "PacketParse.hpp"

#ifdef __unix__

#include <arpa/inet.h>

#elif defined(_WIN32) || defined(WIN32)
#include "windivert.h"
#define ntohs(x) WinDivertHelperNtohs(x)
#define ntohl(x) WinDivertHelperNtohl(x)
#define htons(x) WinDivertHelperHtons(x)
#define htonl(x) WinDivertHelperHtonl(x)
#endif

uint16_t CalculateIPChecksum(std::vector<uint8_t> buff);

uint16_t CalculateUDPChecksum(std::vector<uint8_t> buff, uint32_t srcAddr, uint32_t dstAddr);

std::vector<uint8_t> CraftUDPPacket(uint32_t srcAddr,
                                    uint32_t dstAddr,
                                    uint16_t srcPort,
                                    uint16_t dstPort,
                                    std::vector<uint8_t> payload,
                                    std::vector<uint8_t> srcMac = std::vector<uint8_t>(),
                                    std::vector<uint8_t> dstMac = std::vector<uint8_t>());

#endif
