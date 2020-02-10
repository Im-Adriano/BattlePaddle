#include "PacketParse.hpp"
#include "Config.hpp"
#include "RawSocket.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <mutex>

using namespace PacketParse;
using namespace std;

class BPHelper {
private:
    RawSocket rawSocket;
    mutex socketMutex;
    vector<uint8_t> nextHopMac;
public:
    BPHelper();

    int actionResponse(unique_ptr<info_t> eventInfo);

    void Receive();

    void requestAction();

    void requestActionLoop(int interval);

    void requestActionThread();
};