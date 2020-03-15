# BattlePaddle
Still in alpha, it works but still needs a lot of cleaning up and documentation.
This is a work in progress.

## What is it?

BattlePaddle is a C2 bot with some extra functionality


# Alpha Timeline
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
