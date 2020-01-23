#include <stdint.h>
#include <istream>
#include <ostream>
#include <cstring>
#include <iostream>
#include <iomanip>

using namespace std;
#define HEX( x ) setw(2) << setfill('0') << hex << (int)( x )
#define PRINTHEX( x, y , z) y << setw(15) << setfill(' ') << z; \
    for(int i = 0; i < sizeof(x); i++){ \
        y << HEX(x[i]) << " "; \
    } \
    y << endl;

namespace PacketParse {
    struct ether_header_t {
        uint8_t dst_mac[6];
        uint8_t src_mac[6];
        uint8_t llc_len[2];
    };

    struct ip_header_t {
        uint8_t ver_ihl[1];  // 4 bits version and 4 bits internet header length
        uint8_t tos[1];
        uint8_t total_length[2];
        uint8_t id[2];
        uint8_t flags_fo[2]; // 3 bits flags and 13 bits fragment-offset
        uint8_t ttl[1];
        uint8_t protocol[1];
        uint8_t checksum[2];
        uint8_t src_addr[4];
        uint8_t dst_addr[4];

        uint8_t ihl() const;
        size_t size() const;
    };

    struct udp_header_t {
        uint8_t src_port[2];
        uint8_t dst_port[2];
        uint8_t length[2];
        uint8_t checksum[2];
    };

    struct bp_command_header_t {
        uint8_t magic_bytes[3]; // 'BP '
        uint8_t header_type[1]; // 0x01
        uint8_t target_OS[1]; // 0x01 Linux | 0x02 Windows | 0x03 Both
        uint8_t command[1];  
        uint8_t flags[2]; 
        uint8_t extra[499];
    };

    struct bp_command_request_header_t {
        uint8_t magic_bytes[3]; // 'BP '
        uint8_t header_type[1]; // 0x02
    };

    struct bp_response_header_t{
        uint8_t magic_bytes[3]; // 'BP '
        uint8_t header_type[1]; // 0x03
        uint8_t host_ip[4];
        uint8_t data[];
    };

    template< typename T >
    T load( std::istream& stream, bool ntoh = true );
    template<>
    ether_header_t load( std::istream& stream, bool ntoh );
    template<>
    ip_header_t  load( std::istream& stream, bool ntoh );
    template<>
    udp_header_t load( std::istream& stream, bool ntoh );

    std::ostream& operator << (std::ostream& o, const ether_header_t& a);
    std::ostream& operator << (std::ostream& o, const ip_header_t& a);
    std::ostream& operator << (std::ostream& o, const udp_header_t& a);
}