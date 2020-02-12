#include "PacketCraft.hpp"
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
#ifdef __unix__
    vector<uint8_t> nextHopMac;
#endif
    uint32_t currentCmd;

public:
    BPHelper();

    int actionResponse(unique_ptr<info_t> eventInfo);

    void Receive();

    void requestAction();

    void requestActionLoop(int interval);

    void requestActionThread();
};