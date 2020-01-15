#include "botSocket.hpp"

using namespace std;
#ifdef __unix__
void botSocket::getInterfaceIndex(const char* inter) {
    struct ifreq ifr = {0};
    memcpy(ifr.ifr_name, inter, strlen(inter));
    if (ioctl(sockFd, SIOCGIFINDEX, &ifr) != 0) {
        perror("Interface Index Failure:");
        exit(-1);
    }
    interfaceIndex = ifr.ifr_ifindex;
}

void botSocket::createAddressStruct() {
    addr = {0};
    addr.sll_family = PF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = interfaceIndex;
}

void botSocket::createSocket() {
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0) {
        perror("Socket failed to create: ");
        exit(-1);
    }
}

void botSocket::setSocketOptions() {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

void botSocket::bindSocket() {
    bind(sockFd, (struct sockaddr*) & addr, sizeof(addr));
}

botSocket::~botSocket() {
    delete packet;
}

botSocket::botSocket(const char* intName, int debugMode) : DebugMode(debugMode) {
    packet = new Packet();
    packet->data = new unsigned char[PACKET_SIZE];
    createSocket();
    getInterfaceIndex(intName);
    createAddressStruct();
    setSocketOptions();
    bindSocket();
}

botSocket::botSocket() = default;

Packet* botSocket::getPacket() {
    return packet;
}

void botSocket::recieve(){
    packet->dataLength = recv(sockFd, packet->data, PACKET_SIZE, 0);
    if(packet->dataLength < 0){
        //LINUX
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)){
            perror("Error in reading received packet: ");
            exit(-1);
        }
    }

    if (DebugMode) {
        cout << "From socket: " << sockFd << endl;
        for( int i = 0; i < packet->dataLength; i++ ){
            cout << hex << int(packet->data[i]) << " ";
        }
        cout << endl;
    }
}

void botSocket::send(void* dataframe) {

}

#elif defined(OS_Windows)
WSADATA wsaData;
int iResult;
struct sockaddr_in dest;
char hostname[100];
struct hostent* local;
struct in_addr adr;
int in;

void botSocket::getInterfaceIndex(const char* inter) {
    gethostname(hostname, sizeof(hostname));
    local = gethostbyname(hostname);
    for (int i = 0; local->h_addr_list[i] != 0; ++i)
    {
        memcpy(&adr, local->h_addr_list[i], sizeof(struct in_addr));
        cout << "Interface Number : " << i << " Address : " << inet_ntoa(adr) << endl;
    }
}

void botSocket::createSocket() {
    sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    if (sockFd == INVALID_SOCKET) {
        perror("socket failed with error: "+ WSAGetLastError());
        WSACleanup();
        exit(-1);
    }
}

void botSocket::setSocketOptions() {
    DWORD timeout = 10;
    setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
    /*DWORD inn = RCVALL_ON;
    DWORD out = 0;
    WSAIoctl(sockFd, SIO_RCVALL, &inn, sizeof(inn), NULL, 0, &out, NULL, NULL);
    int v = 0;
    v = 0xffffffff & ~(1 << 4) & ~(1 << 5);
    setsockopt(sockFd, 263, 18, (const char*)&v, sizeof(v));*/

    //Enable this socket with the power to sniff : SIO_RCVALL is the key Receive ALL ;)
    int j = 1;
    int meep = 1;
    DWORD optval = 1;
    printf("\nSetting socket to sniff...");
    if (WSAIoctl(sockFd, SIO_RCVALL, &j, sizeof(j), 0, 0, (LPDWORD)&meep, 0, 0) == SOCKET_ERROR)
    {
        printf("WSAIoctl() failed.\n");
        wprintf(L"IOCTL failed with error %d\n", WSAGetLastError());
        if (WSAIoctl(sockFd, SIO_RCVALL, &j, sizeof(j), 0, 0, (LPDWORD)&meep, 0, 0) == SOCKET_ERROR) {
            printf("Failed again\n");
            wprintf(L"IOCTL failed again with error %d\n", WSAGetLastError());
            exit(-1);
        }
    }
    printf("Socket set.");
    /*printf("\nSetting the socket in RAW mode...");
    if (setsockopt(sockFd, IPPROTO_IP, IP_HDRINCL, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
    {
        printf("failed to set socket in raw mode.");
        exit(-1);
    }*/
}

void botSocket::bindSocket() {
    memset(&dest, 0, sizeof(dest));
    cout << "Enter the interface number you would like to sniff : " << endl;
    int notused = scanf("%d", &in);
    memcpy(&dest.sin_addr.s_addr, local->h_addr_list[in], sizeof(dest.sin_addr.s_addr));
    dest.sin_family = AF_INET;
    dest.sin_port = 0;
    bind(sockFd, (struct sockaddr*) &dest, sizeof(dest));
}

botSocket::~botSocket() {
    delete packet;
}

void setupWSA() {
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        perror("WSAStartup failed with error: " + iResult);
        exit(-1);
    }
}

botSocket::botSocket(const char* intName, int debugMode) : DebugMode(debugMode) {
    packet = new Packet();
    packet->data = new unsigned char[PACKET_SIZE];
    setupWSA();
    getInterfaceIndex(intName);
    createSocket();
    bindSocket();
    setSocketOptions();
}

botSocket::botSocket() = default;

Packet* botSocket::getPacket() {
    return packet;
}

void botSocket::recieve() {
    packet->dataLength = recv(sockFd, (char *)packet->data, PACKET_SIZE, 0);
    if (packet->dataLength < 0) {
        if (WSAGetLastError() != WSAETIMEDOUT){
            perror("Error in reading received packet: ");
            exit(-1);
        }
    }

    if (DebugMode) {
        cout << "From socket: " << sockFd << endl;
        for (int i = 0; i < packet->dataLength; i++) {
            cout << hex << (int)packet->data[i] << " ";
        }
        cout << endl;

    }
}

void botSocket::send(void* dataframe) {

}
#endif

