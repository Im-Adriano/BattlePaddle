#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testing.hpp"

#include "WinDivert\windivert.h"
#include <iostream>
using namespace std;

#define ntohs(x)            WinDivertHelperNtohs(x)
#define ntohl(x)            WinDivertHelperNtohl(x)

#define MAXBUF              WINDIVERT_MTU_MAX
#define INET6_ADDRSTRLEN    45

void test()
{
    HANDLE handle, console;
    UINT i;
    INT16 priority = 0;
    unsigned char packet[MAXBUF];
    UINT packet_len;
    WINDIVERT_ADDRESS addr;
    PWINDIVERT_IPHDR ip_header;
    PWINDIVERT_IPV6HDR ipv6_header;
    PWINDIVERT_ICMPHDR icmp_header;
    PWINDIVERT_ICMPV6HDR icmpv6_header;
    PWINDIVERT_TCPHDR tcp_header;
    PWINDIVERT_UDPHDR udp_header;
    UINT32 src_addr[4], dst_addr[4];
    UINT64 hash;
    char src_str[INET6_ADDRSTRLEN + 1], dst_str[INET6_ADDRSTRLEN + 1];
    const char* err_str;
    LARGE_INTEGER base, freq;
    double time_passed;

    // Get console for pretty colors.
    console = GetStdHandle(STD_OUTPUT_HANDLE);

    // Divert traffic matching the filter:
    handle = WinDivertOpen("true", WINDIVERT_LAYER_NETWORK, priority, 0);
    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_INVALID_PARAMETER &&
            !WinDivertHelperCompileFilter("true", WINDIVERT_LAYER_NETWORK,
                NULL, 0, &err_str, NULL))
        {
            fprintf(stderr, "error: invalid filter \"%s\"\n", err_str);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "error: failed to open the WinDivert device (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }

    // Max-out the packet queue:
    if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_LENGTH,
        WINDIVERT_PARAM_QUEUE_LENGTH_MAX))
    {
        fprintf(stderr, "error: failed to set packet queue length (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }
    if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_TIME,
        WINDIVERT_PARAM_QUEUE_TIME_MAX))
    {
        fprintf(stderr, "error: failed to set packet queue time (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }
    if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_SIZE,
        WINDIVERT_PARAM_QUEUE_SIZE_MAX))
    {
        fprintf(stderr, "error: failed to set packet queue size (%d)\n",
            GetLastError());
        exit(EXIT_FAILURE);
    }

    // Set up timing:
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&base);

    // Main loop:
    while (TRUE)
    {
        // Read a matching packet.
        if (!WinDivertRecv(handle, packet, sizeof(packet), &packet_len, &addr))
        {
            fprintf(stderr, "warning: failed to read packet (%d)\n",
                GetLastError());
            continue;
        }
        
        // Print info about the matching packet.
        WinDivertHelperParsePacket(packet, packet_len, &ip_header, &ipv6_header,
            NULL, &icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL,
            NULL, NULL, NULL);
        if (ip_header == NULL && ipv6_header == NULL)
        {
            fprintf(stderr, "warning: junk packet\n");
        }
        
        // Dump packet info: 
        putchar('\n');
        SetConsoleTextAttribute(console, FOREGROUND_RED);
        time_passed = (double)(addr.Timestamp - base.QuadPart) /
            (double)freq.QuadPart;
        hash = WinDivertHelperHashPacket(packet, packet_len, 0);
        printf("Packet [Timestamp=%.8g, Direction=%s IfIdx=%u SubIfIdx=%u "
            "Loopback=%u Hash=0x%.16llX]\n",
            time_passed, (addr.Outbound ? "outbound" : "inbound"),
            addr.Network.IfIdx, addr.Network.SubIfIdx, addr.Loopback, hash);
      
        
        if (ip_header != NULL)
        {
            WinDivertHelperFormatIPv4Address(ntohl(ip_header->SrcAddr),
                src_str, sizeof(src_str));
            WinDivertHelperFormatIPv4Address(ntohl(ip_header->DstAddr),
                dst_str, sizeof(dst_str));
            SetConsoleTextAttribute(console,
                FOREGROUND_GREEN | FOREGROUND_RED);
            printf("IPv4 [Version=%u HdrLength=%u TOS=%u Length=%u Id=0x%.4X "
                "Reserved=%u DF=%u MF=%u FragOff=%u TTL=%u Protocol=%u "
                "Checksum=0x%.4X SrcAddr=%s DstAddr=%s]\n",
                ip_header->Version, ip_header->HdrLength,
                ntohs(ip_header->TOS), ntohs(ip_header->Length),
                ntohs(ip_header->Id), WINDIVERT_IPHDR_GET_RESERVED(ip_header),
                WINDIVERT_IPHDR_GET_DF(ip_header),
                WINDIVERT_IPHDR_GET_MF(ip_header),
                ntohs(WINDIVERT_IPHDR_GET_FRAGOFF(ip_header)), ip_header->TTL,
                ip_header->Protocol, ntohs(ip_header->Checksum), src_str,
                dst_str);

        }
        if (ipv6_header != NULL)
        {
            WinDivertHelperNtohIPv6Address(ipv6_header->SrcAddr, src_addr);
            WinDivertHelperNtohIPv6Address(ipv6_header->DstAddr, dst_addr);
            WinDivertHelperFormatIPv6Address(src_addr, src_str,
                sizeof(src_str));
            WinDivertHelperFormatIPv6Address(dst_addr, dst_str,
                sizeof(dst_str));
            SetConsoleTextAttribute(console,
                FOREGROUND_GREEN | FOREGROUND_RED);
            printf("IPv6 [Version=%u TrafficClass=%u FlowLabel=%u Length=%u "
                "NextHdr=%u HopLimit=%u SrcAddr=%s DstAddr=%s]\n",
                ipv6_header->Version,
                WINDIVERT_IPV6HDR_GET_TRAFFICCLASS(ipv6_header),
                ntohl(WINDIVERT_IPV6HDR_GET_FLOWLABEL(ipv6_header)),
                ntohs(ipv6_header->Length), ipv6_header->NextHdr,
                ipv6_header->HopLimit, src_str, dst_str);
        }

        SetConsoleTextAttribute(console,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_BLUE);
        cout << "RAW BYTES" << endl;
        for (int i = 0; i < packet_len; i++) {
            cout << hex << (int)packet[i] << " ";
        }
        cout << endl;
        /*
        if (icmp_header != NULL)
        {
            SetConsoleTextAttribute(console, FOREGROUND_RED);
            printf("ICMP [Type=%u Code=%u Checksum=0x%.4X Body=0x%.8X]\n",
                icmp_header->Type, icmp_header->Code,
                ntohs(icmp_header->Checksum), ntohl(icmp_header->Body));
        }
        if (icmpv6_header != NULL)
        {
            SetConsoleTextAttribute(console, FOREGROUND_RED);
            printf("ICMPV6 [Type=%u Code=%u Checksum=0x%.4X Body=0x%.8X]\n",
                icmpv6_header->Type, icmpv6_header->Code,
                ntohs(icmpv6_header->Checksum), ntohl(icmpv6_header->Body));
        }
        if (tcp_header != NULL)
        {
            SetConsoleTextAttribute(console, FOREGROUND_GREEN);
            printf("TCP [SrcPort=%u DstPort=%u SeqNum=%u AckNum=%u "
                "HdrLength=%u Reserved1=%u Reserved2=%u Urg=%u Ack=%u "
                "Psh=%u Rst=%u Syn=%u Fin=%u Window=%u Checksum=0x%.4X "
                "UrgPtr=%u]\n",
                ntohs(tcp_header->SrcPort), ntohs(tcp_header->DstPort),
                ntohl(tcp_header->SeqNum), ntohl(tcp_header->AckNum),
                tcp_header->HdrLength, tcp_header->Reserved1,
                tcp_header->Reserved2, tcp_header->Urg, tcp_header->Ack,
                tcp_header->Psh, tcp_header->Rst, tcp_header->Syn,
                tcp_header->Fin, ntohs(tcp_header->Window),
                ntohs(tcp_header->Checksum), ntohs(tcp_header->UrgPtr));
        }
        if (udp_header != NULL)
        {
            SetConsoleTextAttribute(console, FOREGROUND_GREEN);
            printf("UDP [SrcPort=%u DstPort=%u Length=%u "
                "Checksum=0x%.4X]\n",
                ntohs(udp_header->SrcPort), ntohs(udp_header->DstPort),
                ntohs(udp_header->Length), ntohs(udp_header->Checksum));
        }
        SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE);
        for (i = 0; i < packet_len; i++)
        {
            if (i % 80 == 0)
            {
                printf("\n\t");
            }
            printf("%.2X", (UINT8)packet[i]);
        }
        putchar('\n');
        SetConsoleTextAttribute(console,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);*/
    }
}