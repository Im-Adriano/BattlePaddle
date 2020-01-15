#include "RawSocket.hpp"

using namespace std;
#ifdef __unix__
struct sockaddr_ll addr;
int interfaceIndex;
int sockFd;

void RawSocket::getInterfaceIndex(const char* inter) {
    struct ifreq ifr = {0};
    memcpy(ifr.ifr_name, inter, strlen(inter));
    if (ioctl(sockFd, SIOCGIFINDEX, &ifr) != 0) {
        perror("Interface Index Failure:");
        exit(-1);
    }
    interfaceIndex = ifr.ifr_ifindex;
}

void RawSocket::createAddressStruct() {
    addr = {0};
    addr.sll_family = PF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = interfaceIndex;
}

void RawSocket::createSocket() {
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0) {
        perror("Socket failed to create: ");
        exit(-1);
    }
}

void RawSocket::setSocketOptions() {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

void RawSocket::bindSocket() {
    bind(sockFd, (struct sockaddr*) & addr, sizeof(addr));
}

RawSocket::~RawSocket() {
    delete packet;
}

RawSocket::RawSocket(const char* intName, int debugMode) : DebugMode(debugMode) {
    packet = new Packet();
    packet->data = new unsigned char[PACKET_SIZE];
    createSocket();
    getInterfaceIndex(intName);
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
            perror("Error in reading received packet: ");
            exit(-1);
        }
    }else{
        if (DebugMode) {
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
        if(DebugMode){
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

RawSocket::RawSocket(int debugMode) : DebugMode(debugMode) {
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
        if (DebugMode) {
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
    cout << "Sending currently not supported for Windows" << endl;
}
#endif

