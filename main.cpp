#include "botSocket.hpp"
#include <csignal>
#include <vector>
#include "testing.hpp"
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
    int in = 0;
    #ifdef __unix__
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
        }
    }
    #elif defined(OS_Windows)
    test();
    #endif
}