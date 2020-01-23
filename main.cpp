#include "RawSocket.hpp"
#include "PacketParse.hpp"
#include <sstream>
#include <csignal>
#include <vector>

using namespace std;
using namespace PacketParse;

/*
**************************************************************************
THIS IS FOR TESING OF THE RAWSOCKET LIBRARY NOT AN ACTUAL USEFUL PROGRAM.
**************************************************************************
*/


vector<RawSocket> socks;

void cleanup(int i){
    cout << "Safely shutting down..." << endl;
    // for(auto s: socks){
    //     s.~RawSocket();
    // }
    exit(i);
}

int main(){
    signal(SIGINT, cleanup);

    #ifdef __unix__
    // int in = 0;
    // cout << "Enter the number of sockets to make: " ;
    // cin >> in;
    // cin.ignore();
    // for (in; in > 0; in--) {
        // string input;
        // cout << "Enter interface name: ";
        // getline(cin, input);
        //socks.push_back(new RawSocket(input.c_str(), false));
        socks.push_back(RawSocket("8.8.8.8", false, true));
    // }
    #elif defined(OS_Windows)
    socks.push_back(RawSocket(false));
    #endif
    for (;;) {
        for(auto sock: socks){
            sock.recieve();
            Packet pack = sock.getPacket();
            if(pack.size() > 0){
                std::istringstream stream(std::string((char*)pack.data(), pack.size()));
                #ifdef __unix__
                auto ether_header = load<ether_header_t>(stream);
                #endif
                auto ip_header = load<ip_header_t>(stream);
                if(ip_header.protocol[0] == 0x11){
                    if( ip_header.size() > 20 ) { 
                        stream.seekg(ip_header.size() + sizeof(ether_header_t), std::ios_base::beg);
                    }
                    auto udp_header = load<udp_header_t>(stream);
                    #ifdef __unix__
                    cout << ether_header << endl;
                    #endif
                    cout << ip_header << endl;
                    cout << udp_header << endl;
                }
            }
            // Packet meep;
            // string d = "HERE IS SOME TOTALLY REAL TRAFFIC";
            // meep.insert(meep.begin(), d.begin(), d.end());
            // sock.send(meep);
        }
    }
}


/*
**************************************************************************
THIS IS FOR TESING OF THE RAWSOCKET LIBRARY NOT AN ACTUAL USEFUL PROGRAM.
**************************************************************************
*/
