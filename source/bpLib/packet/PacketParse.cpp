#include "PacketParse.hpp"

namespace PacketParse {
    template<typename T>
    std::string hex_out_s(T val, bool mac = false) {
        const int NUMBER_BIT_PER_CHAR = 8;
        const int NUMBER_BIT_PER_HEX = 4;
        std::stringstream sformatter;
        sformatter << std::hex
                   << std::internal
                   << "0x"
                   << std::setfill('0');
        if (mac) {
            for (int i = 0; i < 6; i++)
                sformatter << std::setw(2) << 0u + ((uint8_t *) val)[i];
        }else {
            sformatter << std::setw(sizeof(T) * NUMBER_BIT_PER_CHAR / NUMBER_BIT_PER_HEX) << 0u + val;
        }
        return sformatter.str();
    }

    uint8_t ip_header_t::ihl() const {
        return ver_ihl & 0x0F;
    }

    size_t ip_header_t::size() const {
        return ihl() * sizeof(uint32_t);
    }

    std::ostream &operator<<(std::ostream &o, const ether_header_t &a) {
        o << "Ethernet Header" << std::endl;
        o << "dst_mac: " << hex_out_s(a.dst_mac, true) << std::endl;
        o << "src_mac: " << hex_out_s(a.src_mac, true) << std::endl;
        o << "type: " << hex_out_s(a.type) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, const ip_header_t &a) {
        o << "IP Header" << std::endl;
        o << "ver_ihl: " << hex_out_s(a.ver_ihl) << std::endl;
        o << "tos: " << hex_out_s(a.tos) << std::endl;
        o << "total_length: " << hex_out_s(a.total_length) << std::endl;
        o << "id: " << hex_out_s(a.id) << std::endl;
        o << "flags_fo: " << hex_out_s(a.flags_fo) << std::endl;
        o << "ttl: " << hex_out_s(a.ttl) << std::endl;
        o << "protocol: " << hex_out_s(a.protocol) << std::endl;
        o << "checksum: " << hex_out_s(a.checksum) << std::endl;
        o << "src_addr: " << hex_out_s(a.src_addr) << std::endl;
        o << "dst_addr: " << hex_out_s(a.dst_addr) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, const udp_header_t &a) {
        o << "UDP Header" << std::endl;
        o << "src_port: " << hex_out_s(a.src_port) << std::endl;
        o << "dst_port: " << hex_out_s(a.dst_port) << std::endl;
        o << "length: " << hex_out_s(a.length) << std::endl;
        o << "checksum: " << hex_out_s(a.checksum) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, const bp_header_t &a) {
        o << "BP Header" << std::endl;
        o << "magic_bytes: " << hex_out_s(a.magic_bytes) << std::endl;
        o << "header_type: " << hex_out_s(a.header_type) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, const bp_response_t &a) {
        o << "BP Response" << std::endl;
        o << "command_num: " << hex_out_s(a.command_num) << std::endl;
        o << "host_ip: " << hex_out_s(a.host_ip) << std::endl;
        o << "data_len: " << hex_out_s(a.data_len) << std::endl;
        o << "data: " << hex_out_s(a.data) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, const bp_command_request_t &a) {
        o << "BP Command Request" << std::endl;
        o << "target_OS: " << hex_out_s(a.target_OS) << std::endl;
        o << "command_num: " << hex_out_s(a.command_num) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, const bp_raw_command_t &a) {
        o << "BP Raw Commmand" << std::endl;
        o << "command_num: " << hex_out_s(a.command_num) << std::endl;
        o << "target_OS: " << hex_out_s(a.target_OS) << std::endl;
        o << "host_ip: " << hex_out_s(a.host_ip) << std::endl;
        o << "cmd_len: " << hex_out_s(a.cmd_len) << std::endl;
        o << "raw_command: " << hex_out_s(a.raw_command) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, const bp_keep_alive_t &a) {
        o << "BP Keep Alive" << std::endl;
        o << "command_num: " << hex_out_s(a.command_num) << std::endl;
        o << "target_OS: " << hex_out_s(a.target_OS) << std::endl;
        o << "host_ip: " << hex_out_s(a.host_ip) << std::endl;
        return o;
    }

    std::ostream &operator<<(std::ostream &o, std::unique_ptr<info_t> a) {
        if (a == nullptr) {
            return o;
        }
        o << a->etherHeader << std::endl;
        o << a->ipHeader << std::endl;
        o << a->udpHeader << std::endl;
        o << a->bpHeader << std::endl;
        o << a->bpCommandRequest << std::endl;
        o << a->bpRawCommand << std::endl;
        o << a->bpResponse << std::endl;
        o << a->bpKeepAlive << std::endl;
        return o;
    }

    template<>
    ether_header_t load(std::istream &stream, bool) {
        ether_header_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        return header;
    }

    template<>
    ip_header_t load(std::istream &stream, bool ntoh) {
        ip_header_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
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
    udp_header_t load(std::istream &stream, bool ntoh) {
        udp_header_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (ntoh) {
            header.src_port = ntohs(header.src_port);
            header.dst_port = ntohs(header.dst_port);
            header.length = ntohs(header.length);
            header.checksum = ntohs(header.checksum);
        }
        return header;
    }

    template<>
    bp_header_t load(std::istream &stream, bool ntoh) {
        bp_header_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (ntoh) {
            header.magic_bytes = ntohl(header.magic_bytes);
        }
        return header;
    }

    template<>
    bp_command_request_t load(std::istream &stream, bool ntoh) {
        bp_command_request_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (ntoh) {
            header.command_num = ntohl(header.command_num);
        }
        return header;
    }

    template<>
    bp_raw_command_t load(std::istream &stream, bool ntoh) {
        bp_raw_command_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (ntoh) {
            header.cmd_len = ntohs(header.cmd_len);
            header.command_num = ntohl(header.command_num);
            header.host_ip = ntohl(header.host_ip);
        }
        return header;
    }

    template<>
    bp_response_t load(std::istream &stream, bool ntoh) {
        bp_response_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (ntoh) {
            header.data_len = ntohs(header.data_len);
            header.command_num = ntohl(header.command_num);
            header.host_ip = ntohl(header.host_ip);
        }
        return header;
    }

    template<>
    bp_keep_alive_t load(std::istream &stream, bool ntoh) {
        bp_keep_alive_t header{};
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (ntoh) {
            header.command_num = ntohl(header.command_num);
            header.host_ip = ntohl(header.host_ip);
        }
        return header;
    }

    std::unique_ptr<info_t> parsePacket(Packet packet) {
        std::unique_ptr<info_t> info = std::make_unique<info_t>();

        if (packet.empty() || packet.size() < 20) {
            return nullptr;
        }

        std::istringstream stream(std::string((char *)packet.data(), packet.size()));
#ifdef __unix__
        info->etherHeader = load<ether_header_t>(stream);
#endif
        info->ipHeader = load<ip_header_t>(stream);
        if (info->ipHeader.protocol == 0x11) {
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
                    case 0x04:
                        info->bpKeepAlive = load<bp_keep_alive_t>(stream);
                        break;
                }
            }
        }
        return info;
    }
}