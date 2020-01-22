#include "RawSocketHelper.hpp"

using namespace std;

#ifdef __unix__
int RawSocketHelper::getInterfaceIndex(const char* inter) {
    struct ifreq ifr = {0};
    memcpy(ifr.ifr_name, inter, strlen(inter));
    if (ioctl(sockFd, SIOCGIFINDEX, &ifr) != 0) {
        perror("Interface Index Failure:");
        return -1;
    }
    interfaceIndex = ifr.ifr_ifindex;
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
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    if(setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0){
        perror("Setting socket options failed:");
        return -1;
    }
    return 0;

}

int RawSocketHelper::bindSocket() {
    if(bind(sockFd, (struct sockaddr*) &addr, sizeof(addr)) < 0){
        perror("Binding to socket failed:");
        return -1;
    }
    return 0;
}

int RawSocketHelper::findOutwardFacingNIC(const char * destination_address){
    sockaddr_storage addrOut = { 0 };
    unsigned long addrDest = inet_addr( destination_address );
    ( ( struct sockaddr_in * ) &addrOut)->sin_addr.s_addr = addrDest;
    ( ( struct sockaddr_in * ) &addrOut)->sin_family = AF_INET;
    ( ( struct sockaddr_in * ) &addrOut)->sin_port = htons( 9 );

    int handle = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (handle < 0) {
        perror("Socket failed to create:");
        return -1;
    }
    socklen_t addrLen = sizeof(addrOut);
    if(connect( handle, (sockaddr*)&addrOut, addrLen) != 0){
        perror("Connecting failed:");
        return -1;
    }
    if(getsockname(handle, (sockaddr*)&addrOut, &addrLen) != 0){
        perror("Get socket name failed:");
        return -1;
    }
    char* source_address = inet_ntoa(((struct sockaddr_in *)&addrOut)->sin_addr);
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;
    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr && AF_INET == ifa->ifa_addr->sa_family){
            struct sockaddr_in* inaddr = (struct sockaddr_in*)ifa->ifa_addr;
            if (inaddr->sin_addr.s_addr == ((struct sockaddr_in *)&addrOut)->sin_addr.s_addr && ifa->ifa_name){
                cout << "Using interface " << ifa->ifa_name << " to bind to. Interface uses IP: " << source_address << endl;
                getInterfaceIndex(ifa->ifa_name);
            }
        }
    }
    freeifaddrs(ifaddr);
    return 0;
}

#elif defined(OS_Windows)

int RawSocketHelper::setup() {
    handle = WinDivertOpen("true", WINDIVERT_LAYER_NETWORK, priority, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_FRAGMENTS);
    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_INVALID_PARAMETER &&
            !WinDivertHelperCompileFilter("true", WINDIVERT_LAYER_NETWORK,
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
#endif