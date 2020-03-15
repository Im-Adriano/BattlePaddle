#include "RawSocket.hpp"

#ifdef __unix__

RawSocket::~RawSocket() = default;

RawSocket::RawSocket(const std::string &intName, bool debug) : debugMode(debug) {
    rawSocketHelper = RawSocketHelper();
    rawSocketHelper.createSocket();
    rawSocketHelper.getInterfaceIndexAndInfo(intName.c_str());
    rawSocketHelper.createAddressStruct();
    rawSocketHelper.setSocketOptions();
    rawSocketHelper.bindSocket();
}

RawSocket::RawSocket(const uint32_t IP, bool debug) : debugMode(debug) {
    rawSocketHelper = RawSocketHelper();
    rawSocketHelper.createSocket();
    rawSocketHelper.findOutwardFacingNIC(IP);
    rawSocketHelper.createAddressStruct();
    rawSocketHelper.setSocketOptions();
    rawSocketHelper.bindSocket();
}

RawSocket::RawSocket() = default;

Packet RawSocket::getPacket() {
    return packet;
}

std::vector<uint8_t> RawSocket::getMacOfIP(uint32_t targetIP) {
    return rawSocketHelper.getMacOfIP(targetIP);
}

int RawSocket::receive() {
    packet.clear();
    unsigned char buf[PACKET_SIZE];
    int packetLen = recv(rawSocketHelper.sockFd, buf, PACKET_SIZE, 0);
    if (packetLen < 0) {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
            perror("Error in reading received packet:");
            return -1;
        }
        return -2;
    }
    packet.insert(packet.begin(), buf, buf + packetLen);
    if (debugMode) {
        std::cout << "From socket: " << rawSocketHelper.sockFd << std::endl;
        for (int i = 0; i < packet.size(); i++) {
            std::cout << std::hex << static_cast<int>(packet.at(i)) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

int RawSocket::send(Packet dataframe) {
    struct sockaddr_ll socket_address{};
    socket_address.sll_ifindex = rawSocketHelper.interfaceIndex;
    socket_address.sll_halen = ETH_ALEN;

    if (dataframe.size() < 14) {
        if (debugMode) {
            std::cout << "Packet must be atleast 14 bytes long" << std::endl;
        }
    } else if (
            sendto(rawSocketHelper.sockFd, dataframe.data(), dataframe.size(), 0, (struct sockaddr *) &socket_address,
                   sizeof(struct sockaddr_ll)) < 0) {
        perror("Send failed: ");
        return -1;
    }
    return 0;
}

uint32_t RawSocket::getIP() {
    return rawSocketHelper.ipAddress;
}

std::vector<uint8_t> RawSocket::getMac() {
    return rawSocketHelper.macAddress;
}

#elif defined(OS_Windows)

RawSocket::~RawSocket() {}

RawSocket::RawSocket() = default;

RawSocket::RawSocket(uint32_t ipAddress, bool debug) : debugMode(debug) {
    rawSocketHelper = RawSocketHelper();
    rawSocketHelper.setup();
    rawSocketHelper.findOutwardFacingNIC(ipAddress);
}

Packet RawSocket::getPacket() {
    return packet;
}

int RawSocket::receive() {
    packet.clear();
    unsigned char buf[PACKET_SIZE];
    int packetLen = 0;
    if (!WinDivertRecv(rawSocketHelper.handle, buf, PACKET_SIZE, reinterpret_cast<UINT *>(&packetLen), &rawSocketHelper.address))
    {
        fprintf(stderr, "warning: failed to read packet (%d)\n",
            GetLastError());
        return -1;
    }
    packet.insert(packet.begin(), buf, buf + packetLen);
    if (debugMode) {
        std::cout << "RAW BYTES" << std::endl;
        for (int i = 0; i < packet.size(); i++) {
            std::cout << std::hex << static_cast<int>(packet.at(i)) << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}

int RawSocket::send(Packet dataframe) {
    std::unique_ptr<WINDIVERT_ADDRESS> sendAddr = std::make_unique<WINDIVERT_ADDRESS>();
    if (sendAddr != nullptr) {
        sendAddr->Outbound = 1;
        if (!WinDivertSend(rawSocketHelper.handle, dataframe.data(), dataframe.size(), NULL, sendAddr.get())) {
            fprintf(stderr, "warning: failed to send packet (%d)\n",
                GetLastError());
            return -1;
        }
        return 0;
    }
    return 1;
}

uint32_t RawSocket::getIP() {
    return rawSocketHelper.ipAddress;
}
#endif

