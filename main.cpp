#include "botSocket.hpp"
#include <csignal>
#include <vector>
using namespace std;

vector<botSocket *> socks;

void cleanup(int i){
    cout << "Safely shutting down..." << endl;
    for(auto s: socks){
        s->~botSocket();
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
        socks.push_back(new botSocket(input.c_str(), true));
    }
    for(;;){
        for(auto sock: socks){
            sock->recieve();
            Packet * meep = new Packet;
            string d = "HERE IS SOME TOTALLY REAL TRAFFIC";
            meep->data= (unsigned char*)d.c_str();
            meep->dataLength = d.size();
            sock->send(meep);
        }
    }
    #elif defined(OS_Windows)
    socks.push_back(new botSocket(true));
    for (;;) {
        for (auto sock : socks) {
            sock->recieve();
            Packet* meep = new Packet;
            string d = "HERE IS SOME TOTALLY REAL TRAFFIC";
            meep->data = (unsigned char*)d.c_str();
            meep->dataLength = d.size();
            sock->send(meep);
        }
    }
    #endif
}