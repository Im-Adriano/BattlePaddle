#include "RawSocket.hpp"

using namespace std;

#ifdef __unix__

RawSocket::~RawSocket() = default;

RawSocket::RawSocket(const string &intNameOrIP, bool debug, bool isIP /* =false */) : debugMode(debug) {
    rawSocketHelper = RawSocketHelper();
    rawSocketHelper.createSocket();
    if (isIP) {
        rawSocketHelper.findOutwardFacingNIC(intNameOrIP.c_str());
    } else {
        rawSocketHelper.getInterfaceIndexAndInfo(intNameOrIP.c_str());
    }
    rawSocketHelper.createAddressStruct();
    rawSocketHelper.setSocketOptions();
    rawSocketHelper.bindSocket();
}

RawSocket::RawSocket() = default;

Packet RawSocket::getPacket() {
    return packet;
}

vector<uint8_t> RawSocket::getMacOfIP(uint32_t targetIP) {
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
        cout << "From socket: " << rawSocketHelper.sockFd << endl;
        for (int i = 0; i < packet.size(); i++) {
            cout << HEX(packet.at(i)) << " ";
        }
        cout << endl;
    }

    return 0;
}

int RawSocket::send(Packet dataframe) {
    struct sockaddr_ll socket_address{};
    socket_address.sll_ifindex = rawSocketHelper.interfaceIndex;
    socket_address.sll_halen = ETH_ALEN;

    if (dataframe.size() < 14) {
        if (debugMode) {
            cout << "Packet must be atleast 14 bytes long" << endl;
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

uint8_t *RawSocket::getMac() {
    return rawSocketHelper.macAddress;
}

#elif defined(OS_Windows)

RawSocket::~RawSocket() {}

RawSocket::RawSocket(bool debug) : debugMode(debug) {
    rawSocketHelper = RawSocketHelper();
    rawSocketHelper.setup();
}

Packet RawSocket::getPacket() {
    return packet;
}

int RawSocket::receive() {
    packet.clear();
    unsigned char buf[PACKET_SIZE];
    int packetLen = 0;
    if (!WinDivertRecv(rawSocketHelper.handle, buf, PACKET_SIZE, (UINT *)(&packetLen), &rawSocketHelper.address))
    {
        fprintf(stderr, "warning: failed to read packet (%d)\n",
            GetLastError());
        return -1;
    }
    packet.insert(packet.begin(), buf, buf + packetLen);
    if (debugMode) {
        cout << "RAW BYTES" << endl;
        for (int i = 0; i < packet.size(); i++) {
            cout << hex << (int)packet.at(i) << " ";
        }
        cout << endl;
    }
    return 0;
}

int RawSocket::send(Packet dataframe) {
    unique_ptr<WINDIVERT_ADDRESS> sendAddr = make_unique<WINDIVERT_ADDRESS>();
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
#endif

