#include "RawSocket.hpp"

using namespace std;

#ifdef __unix__
struct sockaddr_ll addr;
int interfaceIndex;
int sockFd;

void getInterfaceIndex(const char* inter) {
    struct ifreq ifr = {0};
    memcpy(ifr.ifr_name, inter, strlen(inter));
    if (ioctl(sockFd, SIOCGIFINDEX, &ifr) != 0) {
        perror("Interface Index Failure:");
        exit(-1);
    }
    interfaceIndex = ifr.ifr_ifindex;
}

void createAddressStruct() {
    addr = {0};
    addr.sll_family = PF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = interfaceIndex;
}

void createSocket() {
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0) {
        perror("Socket failed to create:");
        exit(-1);
    }
}

void setSocketOptions() {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    if(setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0){
        perror("Setting socket options failed:");
        exit(-1);
    }
}

void bindSocket() {
    if(bind(sockFd, (struct sockaddr*) & addr, sizeof(addr)) < 0){
        perror("Binding to socket failed:");
        exit(-1);
    }
}

void findOutwardFacingNIC(const char * destination_address){
    sockaddr_storage addrOut = { 0 };
    unsigned long addrDest = inet_addr( destination_address );
    ( ( struct sockaddr_in * ) &addrOut)->sin_addr.s_addr = addrDest;
    ( ( struct sockaddr_in * ) &addrOut)->sin_family = AF_INET;
    ( ( struct sockaddr_in * ) &addrOut)->sin_port = htons( 9 );

    int handle = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (handle < 0) {
        perror("Socket failed to create:");
        exit(-1);
    }
    socklen_t addrLen = sizeof(addrOut);
    if(connect( handle, (sockaddr*)&addrOut, addrLen) != 0){
        perror("Connecting failed:");
        exit(-1);
    }
    if(getsockname(handle, (sockaddr*)&addrOut, &addrLen) != 0){
        perror("Get socket name failed:");
        exit(-1);
    }
    char* source_address = inet_ntoa(((struct sockaddr_in *)&addrOut)->sin_addr);
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;
    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr){
            if (AF_INET == ifa->ifa_addr->sa_family){
                struct sockaddr_in* inaddr = (struct sockaddr_in*)ifa->ifa_addr;
                if (inaddr->sin_addr.s_addr == ((struct sockaddr_in *)&addrOut)->sin_addr.s_addr){
                    if (ifa->ifa_name){
                        cout << "Using interface " << ifa->ifa_name << " to bind to. Interface uses IP: " << source_address << endl;
                        getInterfaceIndex(ifa->ifa_name);
                    }
                }
            }
        }
    }
    freeifaddrs(ifaddr);
}

RawSocket::~RawSocket() {
    delete packet;
}

RawSocket::RawSocket(const char* intNameOrIP, int debug, bool isIP /* =false */) : debugMode(debug) {
    packet = new Packet();
    packet->data = new unsigned char[PACKET_SIZE];
    createSocket();
    if(isIP){
        findOutwardFacingNIC(intNameOrIP);
    }else{
        getInterfaceIndex(intNameOrIP);
    }
    createAddressStruct();
    setSocketOptions();
    bindSocket();
}

RawSocket::RawSocket() = default;

Packet* RawSocket::getPacket() {
    return packet;
}

void RawSocket::recieve(){
    packet->dataLength = recv(sockFd, packet->data, PACKET_SIZE, 0);
    if(packet->dataLength < 0){
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)){
            perror("Error in reading received packet:");
            exit(-1);
        }
    }else{
        if (debugMode) {
            cout << "From socket: " << sockFd << endl;
            for( int i = 0; i < packet->dataLength; i++ ){
                cout << hex << int(packet->data[i]) << " ";
            }
            cout << endl;
        }
    }
}

void RawSocket::send(Packet* dataframe) {
    struct sockaddr_ll socket_address;
    socket_address.sll_ifindex = interfaceIndex;
    socket_address.sll_halen = ETH_ALEN;

    if(dataframe->dataLength < 14){
        if(debugMode){
            cout << "Packet must be atleast 14 bytes long" << endl;
        }
    }else if (sendto(sockFd, dataframe->data, dataframe->dataLength, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0){
        perror("Send failed\n");
    }
    
}

#elif defined(OS_Windows)
//#define MAXBUF              WINDIVERT_MTU_MAX
//#define INET6_ADDRSTRLEN    45
HANDLE handle, console;
INT16 priority = 0;
WINDIVERT_ADDRESS address;
const char* err_str;
//LARGE_INTEGER base, freq;

void setup() {
    // Get console for pretty colors.
    console = GetStdHandle(STD_OUTPUT_HANDLE);
    // Divert traffic matching the filter: ALL
    handle = WinDivertOpen("true", WINDIVERT_LAYER_NETWORK, priority, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_FRAGMENTS);
    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_INVALID_PARAMETER &&
            !WinDivertHelperCompileFilter("true", WINDIVERT_LAYER_NETWORK,
                NULL, 0, &err_str, NULL))
        {
            fprintf(stderr, "error: invalid filter \"%s\"\n", err_str);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "error: failed to open the WinDivert device (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }
    //Don't know if this is needed but I will leave it here in case it is in the future. Same with stuff commented out above.
    /*
    // Max-out the packet queue:
    if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_LENGTH,
        WINDIVERT_PARAM_QUEUE_LENGTH_MAX))
    {
        fprintf(stderr, "error: failed to set packet queue length (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }
    if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_TIME,
        WINDIVERT_PARAM_QUEUE_TIME_MAX))
    {
        fprintf(stderr, "error: failed to set packet queue time (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }
    if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_SIZE,
        WINDIVERT_PARAM_QUEUE_SIZE_MAX))
    {
        fprintf(stderr, "error: failed to set packet queue size (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }

    // Set up timing:
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&base);*/
}

RawSocket::~RawSocket() {
    delete packet;
}

RawSocket::RawSocket(int debug) : debugMode(debug) {
    packet = new Packet();
    packet->data = new unsigned char[PACKET_SIZE];
    setup();
}

RawSocket::RawSocket() = default;

Packet* RawSocket::getPacket() {
    return packet;
}

void RawSocket::recieve() {
    if (!WinDivertRecv(handle, packet->data, PACKET_SIZE, (UINT *)(&packet->dataLength), &address))
    {
        fprintf(stderr, "warning: failed to read packet (%d)\n",
            GetLastError());
    }
    else {
        if (debugMode) {
            SetConsoleTextAttribute(console,
                FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_BLUE);
            cout << "RAW BYTES" << endl;
            for (int i = 0; i < packet->dataLength; i++) {
                cout << hex << (int)packet->data[i] << " ";
            }
            cout << endl;
        }
    }
}

void RawSocket::send(Packet * dataframe) {
    if (debugMode) {
        cout << "Sending currently not supported for Windows" << endl;
    }
}
#endif

