#include "botSocket.hpp"

using namespace std;

void botSocket::getInterfaceIndex(char * interface){
    struct ifreq ifr = {0};
    memcpy(ifr.ifr_name, interface, strlen(interface));
    if(ioctl(sockFd, SIOCGIFINDEX, &ifr) != 0){
        perror("Interface Index Failure:");
        exit(-1);
    }
    interfaceIndex = ifr.ifr_ifindex;
}

void botSocket::createAddressStruct(){
    addr={0};
    addr.sll_family=PF_PACKET;
    addr.sll_protocol=htons(ETH_P_ALL);
    addr.sll_ifindex=interfaceIndex;
}

void botSocket::createSocket(){
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if( sockFd < 0 ){
        perror("Socket failed to create: ");
        exit(-1);
    }
}

void botSocket::setSocketOptions(){
    //LINUX
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // WINDOWS
    // DWORD timeout = timeout_in_seconds * 1000;
    // setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
}

void botSocket::bindSocket(){
    bind(sockFd, (struct sockaddr *) &addr, sizeof(addr));
}

botSocket::~botSocket(){
    delete packet;
}

botSocket::botSocket(char * intName, int debugMode): DebugMode(debugMode){
    packet = new Packet();
    packet->data = new unsigned char [PACKET_SIZE];
    createSocket();
    getInterfaceIndex(intName);
    createAddressStruct();
    setSocketOptions();
    bindSocket();
}

botSocket::botSocket() = default;

Packet * botSocket::getPacket(){
    return packet;
}

void botSocket::recieve(){
    packetLen = recv(sockFd, packet->data, PACKET_SIZE, 0);
    if(packetLen < 0){
        //LINUX
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)){
            perror("Error in reading received packet: ");
            exit(-1);
        }

        //WINDOWS
        // if (WSAGetLastError() != WSAETIMEDOUT){
        //     perror("Error in reading received packet: ");
        //     exit(-1);
        // }
    }

    if(DebugMode){
        cout << "From socket: " << sockFd << endl;
        for( int i = 0; i < packetLen; i++ ){
            cout << hex << int(packet->data[i]) << " ";
        }
        cout << endl;
    }
}

void botSocket::send(void * dataframe){

}
