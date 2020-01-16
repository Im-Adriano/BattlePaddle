#include "RawSocket.hpp"
#include <csignal>
#include <vector>
using namespace std;

vector<RawSocket *> socks;

void cleanup(int i){
    cout << "Safely shutting down..." << endl;
    for(auto s: socks){
        s->~RawSocket();
    }
    exit(i);
}

int main(){
    signal(SIGINT, cleanup);
    #ifdef __unix__
    int in = 0;
    cout << "Enter the number of sockets to make: " ;
    cin >> in;
    cin.ignore();
    for (in; in > 0; in--) {
        string input;
        cout << "Enter interface name: ";
        getline(cin, input);
        // socks.push_back(new RawSocket(input.c_str(), true));
        socks.push_back(new RawSocket("127.0.0.1", true, true));

    }
    #elif defined(OS_Windows)
    socks.push_back(new RawSocket(true));
    #endif
    for(;;){
        for(auto sock: socks){
            sock->recieve();
            Packet * meep = new Packet;
            string d = "HERE IS SOME TOTALLY REAL TRAFFIC";
            meep->data= (unsigned char*)d.c_str();
            meep->dataLength = d.size();
            sock->send(meep);
            delete meep;
        }
    }
}