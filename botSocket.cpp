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

void botSocket::bindSocket(){
    bind(sockFd, (struct sockaddr *) &addr, sizeof(addr));
}

botSocket::~botSocket(){
    delete packet;
}

botSocket::botSocket(char * intName, int debugMode): DebugMode(debugMode){
    packet =  new unsigned char [PACKET_SIZE];
    createSocket();
    getInterfaceIndex(intName);
    createAddressStruct();
    bindSocket();
}

botSocket::botSocket() = default;

void botSocket::recieve(){
    packetLen = recv(sockFd, packet, PACKET_SIZE, 0);
    if(packetLen < 0){
        perror("Error in reading received packet: ");
        exit(-1);
    }
    if(DebugMode){
        for( int i = 0; i < packetLen; i++ ){
            cout << hex << int(packet[i]) << " ";
        }
        cout << endl;
    }
}

void botSocket::send(void * dataframe){

}
