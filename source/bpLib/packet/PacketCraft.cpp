#include "PacketCraft.hpp"

uint16_t CalculateIPChecksum(std::vector<uint8_t> buff) {
    uint32_t sum = 0;
    const uint16_t *ip1 = reinterpret_cast<uint16_t *> (buff.data());
    size_t hdr_len = buff.size();
    while (hdr_len > 1) {
        sum += *ip1++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        hdr_len -= 2;
    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (~sum);
}

uint16_t CalculateUDPChecksum(std::vector<uint8_t> buff, uint32_t srcAddr, uint32_t dstAddr) {
    auto *buf = reinterpret_cast<uint16_t *> (buff.data());
    size_t len = buff.size();
    auto *ip_src = reinterpret_cast<uint16_t *> (&srcAddr);
    auto *ip_dst = reinterpret_cast<uint16_t *> (&dstAddr);
    uint32_t sum = 0;

    while (len > 1) {
        sum += *buf++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        len -= 2;
    }

    if (len & 1)
        sum += *((uint8_t *) buf);

    sum += *(ip_src++);
    sum += *ip_src;

    sum += *(ip_dst++);
    sum += *ip_dst;

    sum += htons(IPPROTO_UDP);
    sum += htons(buff.size());

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return ((uint16_t) (~sum));
}

std::vector<uint8_t>
CraftUDPPacket(const uint32_t srcAddr, const uint32_t dstAddr, uint16_t srcPort, uint16_t dstPort,
               std::vector<uint8_t> payload,
               std::vector<uint8_t> srcMac, std::vector<uint8_t> dstMac) {
    PacketParse::ether_header_t ether_header{};
    PacketParse::ip_header_t ip_header{};
    PacketParse::udp_header_t udp_header{};

    uint16_t udp_len = (uint16_t) payload.size() + (uint16_t) sizeof(udp_header);
    udp_header.length = htons(udp_len);
    udp_header.dst_port = htons(srcPort);
    udp_header.src_port = htons(dstPort);
    udp_header.checksum = htons(0xFFFF);

    ip_header.ver_ihl = 0x45;
    ip_header.total_length = htons(udp_len + (uint16_t) sizeof(ip_header));
    ip_header.id = htons(0xda80);
    ip_header.flags_fo = htons(0x4000);
    ip_header.ttl = 0x80;
    ip_header.protocol = 0x11;
    ip_header.checksum = htons(0x0000);
    ip_header.tos = 0x00;
    ip_header.src_addr = srcAddr;
    ip_header.dst_addr = dstAddr;

#ifdef __unix__
    memcpy(ether_header.dst_mac, dstMac.data(), 6);
    memcpy(ether_header.src_mac, srcMac.data(), 6);
    ether_header.type = htons(0x0800);
    auto ether_ptr = reinterpret_cast<unsigned char *>(&ether_header);
#endif

    auto ip_ptr = reinterpret_cast<unsigned char *>(&ip_header);
    auto udp_ptr = reinterpret_cast<unsigned char *>(&udp_header);

    std::vector<uint8_t> tempIP(ip_ptr, ip_ptr + sizeof(ip_header));
    ip_header.checksum = CalculateIPChecksum(tempIP);

    std::vector<uint8_t> tempUDP(udp_ptr, udp_ptr + sizeof(udp_header));
    tempUDP.insert(tempUDP.end(), payload.begin(), payload.end());
    udp_header.checksum = CalculateUDPChecksum(tempUDP, ip_header.src_addr, ip_header.dst_addr);

#ifdef __unix__
    Packet req(ether_ptr, ether_ptr + sizeof(ether_header));
    req.insert(req.end(), ip_ptr, ip_ptr + sizeof(ip_header));
#else
    Packet req(ip_ptr, ip_ptr + sizeof(ip_header));
#endif
    req.insert(req.end(), udp_ptr, udp_ptr + sizeof(udp_header));
    req.insert(req.end(), payload.begin(), payload.end());

    return req;
}
