# RawSocket Library

## What is it?
 Raw Sockets of course! But really it is an abstraction/simplification of raw sockets on both windows and linux using their respective technologies. Each built on top of minimal dependencies to allow for ease of deployment to many systems.

 ## What do I get with this library?

 A simple class named `RawSocket` that makes setting up Raw Sockets a breeze. Just instantiate a `RawSocket` object like so:
 ```c++
 auto socket = new RawSocket(IP);
 ```
 `IP` can be any IP that you want the socket to communicate with. All this does is help the library determine which interface to bind/listen to. And that's it!

Now you can use the other methods of that object which includes:

```c++
Packet getPacket();
```
- This method returns the Packet that was read in by the receive method.

```c++
uint32_t getIP();
```
- This method returns the IP of the interface we are bound to. Useful in packet crafting.

```c++
int receive();
```
- This method receives a single packet and stores it in the packet buffer to be retrieved by the `getPacket` method.

```c++
int send(Packet dataframe);
```
- This method sends your raw packet, which is a type `Packet` or `std::vector<uint8_t>`. It does no error checking on the packet being sent.

There are then some methods on Linux to aid in packet creation:
```c++
std::vector<uint8_t> getMac();
```
- This method returns the Mac address of the interface the socket are bound to. 

```cpp
std::vector<uint8_t> getMacOfIP(uint32_t targetIP);
```
- This method returns the Mac address of the given IP.

## Some caveats

Windows currently is only able to send Raw Packets from layer 3 and up. You are unable to craft your own layer 2/3 header with this current implementation. You can only craft the layer 3 payload and up. The socket is also blocked by the firewall when attempting to send a packet.

### SO why use this on windows?
 Easy, the library is still able to listen and receive packets with a firewall fully blocking traffic and it does not use pcap which is easily found. It also allows you to create cross platform raw socket programs with minimal platform dependent code due to the abstraction layer provided by this library.  

 On Linux you are responsible for the whole packet from the layer 2 header and up. Good luck. I have also developed a library to aid in packet parsing and crafting found here [source/bpLib/packet](../../bpLib/packet/)


