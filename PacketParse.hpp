#include <iomanip>
#include <cstring>
#include <arpa/inet.h>
#include <string>     
#include <sstream>    
#include <ios>                              
#include <climits>     
#include <type_traits> 

using namespace std;
namespace PacketParse {
    const uint32_t MAGIC_BYTES = 0x43503c33; // BP<3

    struct ether_header_t {
        uint8_t dst_mac[6];
        uint8_t src_mac[6];
        uint16_t type;
    };
    struct ip_header_t {
        uint8_t ver_ihl;  // 4 bits version and 4 bits internet header length
        uint8_t tos;
        uint16_t total_length;
        uint16_t id;
        uint16_t flags_fo; // 3 bits flags and 13 bits fragment-offset
        uint8_t ttl;
        uint8_t protocol;
        uint16_t checksum;
        uint32_t src_addr;
        uint32_t dst_addr;

        uint8_t ihl() const;
        size_t size() const;
    };
    struct udp_header_t {
        uint16_t src_port;
        uint16_t dst_port;
        uint16_t length;
        uint16_t checksum;
    };
    struct bp_command_request_t {
        uint32_t magic_bytes = MAGIC_BYTES;
        uint8_t header_type = 0x01;
        uint8_t target_OS{}; // 0x01 Linux | 0x02 Windows
    };
    struct bp_raw_command_t {
        uint32_t magic_bytes = MAGIC_BYTES;
        uint8_t header_type = 0x03;
        uint8_t target_OS{}; // 0x01 Linux | 0x02 Windows
        uint32_t command_num{}; // For metrics
        uint32_t host_ip{};
        uint16_t cmd_len{};
        uint8_t raw_command[500]{}; // The custom command
    };
    struct bp_response_t{
        uint32_t magic_bytes = MAGIC_BYTES; 
        uint8_t header_type = 0x04;
        uint32_t command_num{}; // For metrics
        uint32_t host_ip{}; // When relaying becomes a thing
        uint16_t data_len{};
        uint8_t data[500]{}; // Other metrics about command ran or health of host
    };

    template<typename T> T load(istream& stream, bool ntoh = true);
    template<> ether_header_t load(istream& stream, bool ntoh);
    template<> ip_header_t  load(istream& stream, bool ntoh);
    template<> udp_header_t load(istream& stream, bool ntoh);
    template<> bp_command_request_t load(istream& stream, bool ntoh);
    template<> bp_raw_command_t load(istream& stream, bool ntoh);
    template<> bp_response_t load(istream& stream, bool ntoh);

    ostream& operator << (ostream& o, const ether_header_t& a);
    ostream& operator << (ostream& o, const ip_header_t& a);
    ostream& operator << (ostream& o, const udp_header_t& a);
    ostream& operator << (ostream& o, const bp_command_request_t& a);
    ostream& operator << (ostream& o, const bp_raw_command_t& a);
    ostream& operator << (ostream& o, const bp_response_t& a);

}