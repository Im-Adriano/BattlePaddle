// #include <iostream>
// #include <thread>
// #include <vector>
// #include <mutex>
// #include <sys/socket.h>
// #include <netinet/in.h> 
// #include <sys/types.h> 
// #include <arpa/inet.h> 
// #include <string.h>
// #include <net/ethernet.h>
// #include <netpacket/packet.h>	
// #include <net/if.h>
// #include <sys/ioctl.h>

// using namespace std;

// mutex meep;

// void sayHi(char * interface){
//     int PACKET_SIZE = 65535;
//     unsigned char *buffer = (unsigned char *)malloc(PACKET_SIZE);
//     int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
//     if( sock < 0 ){
//         perror("Socket failed to create");
//         exit(-1);
//     }

//     struct ifreq ifr;
//     memcpy(ifr.ifr_name, interface, strlen(interface));
//     ioctl(sock, SIOCGIFINDEX, &ifr);
//     int index = ifr.ifr_ifindex;

    
//     struct sockaddr_ll addr={0};
//     addr.sll_family=PF_PACKET;
//     addr.sll_protocol=htons(ETH_P_ALL);
//     addr.sll_ifindex=index;
//     bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    
//     for(int i = 0; i < 100; i++){
//         int buffer_len = recv(sock, buffer, PACKET_SIZE, 0);
//         // int buffer_len = recvfrom(sock, &buffer, 65535, 0, NULL, NULL);
//         if(buffer_len < 0){
//             perror("Error in reading received packet");
//             exit(-1);
//         }
//         for( int i = 0; i < buffer_len; i++ ){
//             cout << hex << int(buffer[i]) << " ";
//         }
//         cout << endl;
//     }
// }

// int main(){
//     vector<thread> threads;
//     threads.reserve(2);
//     char * meep = "lo";
//     char * meep2 = "wlo1";
//     threads.push_back(thread(sayHi, meep));
//     threads.push_back(thread(sayHi, meep2));

//     for(auto &thr: threads){
//         thr.join();
//     }

//     cout<<"Hello World!"<<endl;
//     return 0;
// }