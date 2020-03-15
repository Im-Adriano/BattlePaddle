#include "RawSocketHelper.hpp"

#ifdef __unix__

int RawSocketHelper::getInterfaceIndexAndInfo(const char *inter) {
    struct ifreq ifr = {0};
    struct ifreq macIfr = {0};
    memcpy(ifr.ifr_name, inter, strlen(inter));
    if (ioctl(sockFd, SIOCGIFINDEX, &ifr) != 0) {
        perror("Interface Index Failure:");
        return -1;
    }
    memcpy(&macIfr, &ifr, sizeof(ifr));
    if (ioctl(sockFd, SIOCGIFHWADDR, &macIfr) != 0) {
        perror("Interface HW Failure:");
        return -1;
    }
    interfaceIndex = ifr.ifr_ifindex;
    macAddress = std::vector<uint8_t>(macIfr.ifr_hwaddr.sa_data, macIfr.ifr_hwaddr.sa_data + 6);
    printf("Interface uses MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    return 0;
}

int RawSocketHelper::createAddressStruct() {
    addr = {0};
    addr.sll_family = PF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = interfaceIndex;
    return 0;

}

int RawSocketHelper::createSocket() {
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0) {
        perror("Socket failed to create:");
        return -1;
    }
    return 0;

}

int RawSocketHelper::setSocketOptions() {
    struct timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv) < 0) {
        perror("Setting socket options failed:");
        return -1;
    }
    return 0;

}

int RawSocketHelper::bindSocket() {
    if (bind(sockFd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Binding to socket failed:");
        return -1;
    }
    return 0;
}

int RawSocketHelper::findOutwardFacingNIC(uint32_t destination_address) {
    sockaddr_storage addrOut = {0};
    ((struct sockaddr_in *) &addrOut)->sin_addr.s_addr = destination_address;
    ((struct sockaddr_in *) &addrOut)->sin_family = AF_INET;
    ((struct sockaddr_in *) &addrOut)->sin_port = htons(9);

    int handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (handle < 0) {
        perror("Socket failed to create:");
        return -1;
    }
    socklen_t addrLen = sizeof(addrOut);
    if (connect(handle, reinterpret_cast<sockaddr *>(&addrOut), addrLen) != 0) {
        perror("Connecting failed:");
        return -1;
    }
    if (getsockname(handle, reinterpret_cast<sockaddr *>(&addrOut), &addrLen) != 0) {
        perror("Get socket name failed:");
        return -1;
    }
    char *source_address = inet_ntoa(((struct sockaddr_in *) &addrOut)->sin_addr);
    struct ifaddrs *ifaddr;
    struct ifaddrs *ifa;
    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && AF_INET == ifa->ifa_addr->sa_family) {
            auto *inaddr = (struct sockaddr_in *) ifa->ifa_addr;
            if (inaddr->sin_addr.s_addr == ((struct sockaddr_in *) &addrOut)->sin_addr.s_addr && ifa->ifa_name) {
                std::cout << "Using interface " << ifa->ifa_name << " to bind to." << std::endl;
                std::cout << "Interface uses IP: " << source_address << std::endl;
                ipAddress = ((struct sockaddr_in *) &addrOut)->sin_addr.s_addr;
                getInterfaceIndexAndInfo(ifa->ifa_name);
            }
        }
    }
    freeifaddrs(ifaddr);
    return 0;
}

std::vector<uint8_t> RawSocketHelper::getMacOfIP(uint32_t targetIP) {
    arp arpReq;
    uint32_t networkTargetIP = htonl(targetIP);
    memcpy(arpReq.sender_mac, macAddress.data(), 6);
    memcpy(arpReq.src_mac, macAddress.data(), 6);
    memcpy(arpReq.sender_ip, &ipAddress, 4);
    memcpy(arpReq.target_ip, &networkTargetIP, 4);

    auto arpReq_ptr = reinterpret_cast<uint8_t *>(&arpReq);
    std::vector buf(arpReq_ptr, arpReq_ptr + sizeof(arpReq));

    struct sockaddr_ll socket_address{};
    memcpy(socket_address.sll_addr, macAddress.data(), 6);
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ARP);
    socket_address.sll_ifindex = interfaceIndex;
    socket_address.sll_hatype = htons(0x0001);
    socket_address.sll_pkttype = (PACKET_BROADCAST);
    socket_address.sll_halen = 6;
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;
    if (sendto(sockFd, buf.data(), buf.size(), 0, (struct sockaddr *) &socket_address, sizeof(socket_address)) < 0) {
        perror("Send arp failed: ");
        return std::vector<uint8_t>();
    }

    int attempts = 0;
    while (true) {
        unsigned char recvBuf[60];
        arp arpResp{};
        if (attempts > 5) {
            if (sendto(sockFd, buf.data(), buf.size(), 0, (struct sockaddr *) &socket_address, sizeof(socket_address)) < 0) {
                perror("Send arp failed: ");
                return std::vector<uint8_t>();
            }
        }
        int length = recv(sockFd, recvBuf, 60, 0);
        if (length < 0) {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                perror("Receive arp failed:");
                return std::vector<uint8_t>();
            }
        }
        memcpy(&arpResp, recvBuf, sizeof(arpResp));
        if (arpResp.type == htons(0x0806)) {
            if (arpResp.opcode == htons(0x0002)) {
                if (memcmp(&arpResp.target_ip, &networkTargetIP, 4) != 0) {
                    return std::vector<uint8_t>(arpResp.sender_mac, arpResp.sender_mac + 6);
                }
            }
        }
        attempts++;
    }
}

#elif defined(OS_Windows)

int RawSocketHelper::setup() {
    handle = WinDivertOpen("inbound", WINDIVERT_LAYER_NETWORK, priority, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_FRAGMENTS);
    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_INVALID_PARAMETER &&
            !WinDivertHelperCompileFilter("inbound", WINDIVERT_LAYER_NETWORK,
                NULL, 0, &err_str, NULL))
        {
            fprintf(stderr, "error: invalid filter \"%s\"\n", err_str);
            return -1;
        }
        fprintf(stderr, "error: failed to open the WinDivert device (%d)\n",
            GetLastError());
        return -1;
    }
    return 0;
}


int RawSocketHelper::findOutwardFacingNIC(uint32_t destination_address) {
    DWORD bestInterface{};
    GetBestInterface(destination_address, &bestInterface);

    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    struct tm newtime;
    UINT i;
    char buffer[32];
    errno_t error;

    pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(sizeof(IP_ADAPTER_INFO)));
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(ulOutBufLen));
        if (pAdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return 1;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            if (pAdapter->ComboIndex == bestInterface) {
                ipAddress = pAdapter->IpAddressList.Context;
                std::cout << "Using interface " << pAdapter->AdapterName << " to bind to." << std::endl;
                std::cout << "Interface uses IP: " << pAdapter->IpAddressList.IpAddress.String << std::endl;
                break;
            }
            pAdapter = pAdapter->Next;
        }
    } else {
        printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
    }

    if (pAdapterInfo)
        free(pAdapterInfo);
    return 0;
}

#endif