# BattlePaddle

## What is it?

BattlePaddle is a C2 bot with some extra functionality


# Timeline
- [X] Setup Raw sockets on linux
- [ ] Investigate Raw sockets on windows
  - [X] ~~winsock~~
  - [ ] WinDivert
    - [X] Receiving Packets
    - [ ] Sending Packets
- [X] Design BP protocol
- [ ] Create Linux relay bot
  - [ ] Setup structs to parse raw socket packets
    - [ ] Help me 
      - | 0x01 | Port #
    - [ ] I can help 
      - | 0x02 | Port #
    - [ ] Send  
      - | 0x03 | Frag # | Curr # | Port | Length | Data Frag
    - [ ] C2 response 
      - | 0x04 | Frag # | Curr # | Port | Length | Data Frag
    - [ ] C2 command for Windows?
      - | 0x05 | Frag # | Curr # | Length | Data Frag
  - [ ] Create functions for performing actions for each type of packet
    - [ ] Help me (0x01)
    - [ ] I can help (0x02)
    - [ ] Send (0x03)
    - [ ] C2 response (0x04)
    - [ ] C2 command for windows? (0x05)
- [ ] Determine how Windows will interact with Linux bot
  - [ ] Recieve C2 commands via Linux bots
  - [X] ~~Don't send anything to avoid pcap dependency if raw sockets work~~
  - [ ] Get WinDivert injects to work
