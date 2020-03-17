# BattlePaddle
Still in alpha, it works but still needs a lot of cleaning up and documentation.
This is a work in progress.

## What is it?

BattlePaddle is a Raw Socket C2 bot for both Linux and Windows.


## Dependencies?

The dependencies are as minimal as possible:
- Linux
  - `pthread` on the device compiling the binary. 
    - I have chosen to include all of `pthread` into the binary to make deployment as simple as possible. Just drop the binary and run.
- Windows
  - Just the things found in this repo :)

## Configure and Compiling
### Configuration
Edit the config file found in [source/bpLib/config/Config.cpp](./source/bpLib/config/Config.cpp)

Each configuration option in that file is as followed:
- `c2IpEdit` is the IP of your C2.
  - If your C2 IP is 10.1.1.100 the line would look like:
    ```cpp
    uint8_t c2IpEdit[4] = {10, 1, 1, 100};
    ```
- `gatewayipEdit` is the IP of your this bot's gateway. Only linux uses this configuration. I am looking into alternatives to find this automatically in a way that keeps it distro agnostic.  
  - If your Gateway IP is 10.1.1.1 the line would look like:
    ```cpp
    uint8_t gatewayipEdit[4] = {10, 1, 1, 1};
    ```
- `useGateway`, once again only used for linux, tells the bot whether or not there is a hop between it and the C2. Used so the bot knows what IP to arp for.
- `requestActionInterval` is the rate at which the bot will request a command from the C2. This is in milliseconds.
  
### Compilation
- Linux
  - The build systems I use include:
    - cmake
    - g++
    - make
  
  An example of commands to compile:
  ```bash
  mkdir build
  cd build
  cmake ..
  make
  ```
- Windows
  - The build systems used include:
    - ninja

  I build the project on windows either with `Visual Studio` with it's cmake build tools or `VsCode` using the [Cmake Tools Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)

## RawSocket Library

Go to [source/bpLib/rawsocket](/source/bpLib/rawsocket) to read more.

## Alpha Timeline
- [X] Setup Raw sockets on linux
- [X] Investigate Raw sockets on windows
  - [X] ~~winsock~~
  - [X] WinDivert
    - [X] Receiving Packets
    - [X] Sending Packets
- [X] Design BP protocol
- [X] Determine how Windows will interact with Linux bot
  - [ ] ~~Recieve C2 commands via Linux bots~~
  - [X] ~~Don't send anything to avoid pcap dependency if raw sockets work~~
  - [X] Get WinDivert injects to work
- [X] Implement ability for bots to run commands. 
- [ ] Refactor Code
- [ ] Documentation
