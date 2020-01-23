#include "PacketParse.hpp"

namespace PacketParse {

    uint8_t ip_header_t::ihl() const {
        return (ver_ihl[0] & 0x0F);
    }

    size_t ip_header_t::size() const {
        return ihl() * sizeof(uint32_t);
    }

    std::ostream& operator << (std::ostream& o, const ether_header_t& a)
    {
        o << "Ethernet Header" << endl;
        PRINTHEX(a.dst_mac, o, "dst_mac: ");
        PRINTHEX(a.src_mac, o, "src_mac: ");
        PRINTHEX(a.llc_len, o, "length: ");
        return o;
    }

    std::ostream& operator << (std::ostream& o, const ip_header_t& a){
        o << "IP Header" << endl;
        PRINTHEX(a.ver_ihl, o, "ver_ihl: ");
        PRINTHEX(a.tos, o, "tos: ");
        PRINTHEX(a.total_length, o, "total_length: ");
        PRINTHEX(a.id, o, "id: ");
        PRINTHEX(a.flags_fo, o, "flags_fo: ");
        PRINTHEX(a.ttl, o, "ttl: ");
        PRINTHEX(a.protocol, o, "protocol: ");
        PRINTHEX(a.checksum, o, "checksum: ");
        PRINTHEX(a.src_addr, o, "src_addr: ");
        PRINTHEX(a.dst_addr, o, "dst_addr: ");
        return o;
    }

    std::ostream& operator << (std::ostream& o, const udp_header_t& a){
        o << "UDP Header" << endl;
        PRINTHEX(a.src_port, o, "src_port: ");
        PRINTHEX(a.dst_port, o, "dst_port: ");
        PRINTHEX(a.length, o, "length: ");
        PRINTHEX(a.checksum, o, "checksum: ");
        return o;
    }

    template<>
    ether_header_t load( std::istream& stream, bool ntoh ) {
        ether_header_t header;
        stream.read((char*)&header.dst_mac, sizeof(header.dst_mac));
        stream.read((char*)&header.src_mac, sizeof(header.src_mac));
        stream.read((char*)&header.llc_len, sizeof(header.llc_len));
        return header;
    }

    template<>
    ip_header_t load( std::istream& stream, bool ntoh ) {
        ip_header_t header;
        stream.read((char*)&header.ver_ihl,      sizeof(header.ver_ihl));
        stream.read((char*)&header.tos,          sizeof(header.tos));
        stream.read((char*)&header.total_length, sizeof(header.total_length));
        stream.read((char*)&header.id,           sizeof(header.id));
        stream.read((char*)&header.flags_fo,     sizeof(header.flags_fo));
        stream.read((char*)&header.ttl,          sizeof(header.ttl));
        stream.read((char*)&header.protocol,     sizeof(header.protocol));
        stream.read((char*)&header.checksum,     sizeof(header.checksum));
        stream.read((char*)&header.src_addr,     sizeof(header.src_addr));
        stream.read((char*)&header.dst_addr,     sizeof(header.dst_addr));
        // if( ntoh ) {
        // header.total_length = ntohs(header.total_length);
        // header.id =           ntohs(header.id);
        // header.flags_fo =     ntohs(header.flags_fo);
        // header.checksum =     ntohs(header.checksum);
        // header.src_addr =     ntohl(header.src_addr);
        // header.dst_addr =     ntohl(header.dst_addr);
        // }
        return header;
    }

    template<>
    udp_header_t load( std::istream& stream, bool ntoh ) {
        udp_header_t header;
        stream.read((char*)&header.src_port, sizeof(header.src_port));
        stream.read((char*)&header.dst_port, sizeof(header.dst_port));
        stream.read((char*)&header.length,   sizeof(header.length));
        stream.read((char*)&header.checksum, sizeof(header.checksum));
        // if( ntoh ) {
        // header.src_port = ntohs(header.src_port);
        // header.dst_port = ntohs(header.dst_port);
        // header.length   = ntohs(header.length);
        // header.checksum = ntohs(header.checksum);
        // }
        return header;
    }
}