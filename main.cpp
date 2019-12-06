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
    socks.push_back(new botSocket("wlo1", true));
    socks.push_back(new botSocket("lo", true));
    for(;;){
        for(auto sock: socks){
            sock->recieve();
        }
    }
}