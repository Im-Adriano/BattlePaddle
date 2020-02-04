#include "PacketParse.hpp"

namespace PacketParse {
    template<typename T>
    string hex_out_s(T val, bool mac = false) {
        const int NUMBER_BIT_PER_CHAR = 8;
        const int NUMBER_BIT_PER_HEX = 4;
        stringstream sformatter;
        sformatter << hex
                   << internal
                   << "0x"
                   << setfill('0');
        if (mac)
            for (int i = 0; i < 6; i++)
                sformatter << setw(2) << 0u + ((uint8_t *) val)[i];
        else
            sformatter << setw(sizeof(T) * NUMBER_BIT_PER_CHAR / NUMBER_BIT_PER_HEX) << 0u + val;
        return sformatter.str();
    }

    uint8_t ip_header_t::ihl() const {
        return ver_ihl & 0x0F;
    }

    size_t ip_header_t::size() const {
        return ihl() * sizeof(uint32_t);
    }

    ostream &operator<<(ostream &o, const ether_header_t &a) {
        o << "Ethernet Header" << endl;
        o << "dst_mac: " << hex_out_s(a.dst_mac, true) << endl;
        o << "src_mac: " << hex_out_s(a.src_mac, true) << endl;
        o << "type: " << hex_out_s(a.type) << endl;
        return o;
    }

    ostream &operator<<(ostream &o, const ip_header_t &a) {
        o << "IP Header" << endl;
        o << "ver_ihl: " << hex_out_s(a.ver_ihl) << endl;
        o << "tos: " << hex_out_s(a.tos) << endl;
        o << "total_length: " << hex_out_s(a.total_length) << endl;
        o << "id: " << hex_out_s(a.id) << endl;
        o << "flags_fo: " << hex_out_s(a.flags_fo) << endl;
        o << "ttl: " << hex_out_s(a.ttl) << endl;
        o << "protocol: " << hex_out_s(a.protocol) << endl;
        o << "checksum: " << hex_out_s(a.checksum) << endl;
        o << "src_addr: " << hex_out_s(a.src_addr) << endl;
        o << "dst_addr: " << hex_out_s(a.dst_addr) << endl;
        return o;
    }

    ostream &operator<<(ostream &o, const udp_header_t &a) {
        o << "UDP Header" << endl;
        o << "src_port: " << hex_out_s(a.src_port) << endl;
        o << "dst_port: " << hex_out_s(a.dst_port) << endl;
        o << "length: " << hex_out_s(a.length) << endl;
        o << "checksum: " << hex_out_s(a.checksum) << endl;
        return o;
    }

    ostream &operator<<(ostream &o, const bp_header_t &a) {
        o << "BP Header" << endl;
        o << "magic_bytes: " << hex_out_s(a.magic_bytes) << endl;
        o << "header_type: " << hex_out_s(a.header_type) << endl;
        return o;
    }

    ostream &operator<<(ostream &o, const bp_response_t &a) {
        o << "BP Response" << endl;
        o << "command_num: " << hex_out_s(a.command_num) << endl;
        o << "host_ip: " << hex_out_s(a.host_ip) << endl;
        o << "data_len: " << hex_out_s(a.data_len) << endl;
        o << "data: " << hex_out_s(a.data) << endl;
        return o;
    }

    ostream &operator<<(ostream &o, const bp_command_request_t &a) {
        o << "BP Command Request" << endl;
        o << "target_OS: " << hex_out_s(a.target_OS) << endl;
        return o;
    }

    ostream &operator<<(ostream &o, const bp_raw_command_t &a) {
        o << "BP Raw Commmand" << endl;
        o << "command_num: " << hex_out_s(a.command_num) << endl;
        o << "target_OS: " << hex_out_s(a.target_OS) << endl;
        o << "host_ip: " << hex_out_s(a.host_ip) << endl;
        o << "cmd_len: " << hex_out_s(a.cmd_len) << endl;
        o << "raw_command: " << hex_out_s(a.raw_command) << endl;
        return o;
    }

    ostream &operator<<(ostream &o, unique_ptr<info_t> a) {
        if(a == nullptr){
            return o;
        }
        o << a->etherHeader << endl;
        o << a->ipHeader << endl;
        o << a->udpHeader << endl;
        o << a->bpHeader << endl;
        o << a->bpCommandRequest << endl;
        o << a->bpRawCommand << endl;
        o << a->bpResponse << endl;
        return o;
    }

    template<>
    ether_header_t load(istream &stream, bool) {
        ether_header_t header{};
        stream.read((char *) &header, sizeof(header));
        return header;
    }

    template<>
    ip_header_t load(istream &stream, bool ntoh) {
        ip_header_t header{};
        stream.read((char *) &header, sizeof(header));
        if (ntoh) {
            header.total_length = ntohs(header.total_length);
            header.id = ntohs(header.id);
            header.flags_fo = ntohs(header.flags_fo);
            header.checksum = ntohs(header.checksum);
            header.src_addr = ntohl(header.src_addr);
            header.dst_addr = ntohl(header.dst_addr);
        }
        return header;
    }

    template<>
    udp_header_t load(istream &stream, bool ntoh) {
        udp_header_t header{};
        stream.read((char *) &header, sizeof(header));
        if (ntoh) {
            header.src_port = ntohs(header.src_port);
            header.dst_port = ntohs(header.dst_port);
            header.length = ntohs(header.length);
            header.checksum = ntohs(header.checksum);
        }
        return header;
    }

    template<>
    bp_header_t load(istream &stream, bool) {
        bp_header_t header{};
        stream.read((char *) &header, sizeof(header));
        return header;
    }

    template<>
    bp_command_request_t load(istream &stream, bool) {
        bp_command_request_t header{};
        stream.read((char *) &header, sizeof(header));
        return header;
    }

    template<>
    bp_raw_command_t load(istream &stream, bool) {
        bp_raw_command_t header{};
        stream.read((char *) &header, sizeof(header));
        return header;
    }

    template<>
    bp_response_t load(istream &stream, bool) {
        bp_response_t header{};
        stream.read((char *) &header, sizeof(header));
        return header;
    }

    unique_ptr<info_t> parsePacket(Packet packet) {
        unique_ptr<info_t> info = make_unique<info_t>();

        if (packet.empty() || packet.size() < 20) {
            return nullptr;
        }

        std::istringstream stream(std::string((char *)packet.data(), packet.size()));
#ifdef __unix__
        info->etherHeader = load<ether_header_t>(stream);
#endif
        info->ipHeader = load<ip_header_t>(stream);
        if (info->ipHeader.protocol == 0x11) {
            if (info->ipHeader.size() > 20) {
                stream.seekg(info->ipHeader.size() + sizeof(ether_header_t), std::ios_base::beg);
            }
            info->udpHeader = load<udp_header_t>(stream);
            info->bpHeader = load<bp_header_t>(stream);
            if (info->bpHeader.magic_bytes == MAGIC_BYTES) {
                switch (info->bpHeader.header_type) {
                    case 0x01:
                        info->bpCommandRequest = load<bp_command_request_t>(stream);
                        break;
                    case 0x02:
                        info->bpRawCommand = load<bp_raw_command_t>(stream);
                        break;
                    case 0x03:
                        info->bpResponse = load<bp_response_t>(stream);
                        break;
                }
            }
        }
        return info;
    }
}