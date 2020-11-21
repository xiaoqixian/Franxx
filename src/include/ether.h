/**********************************************
  > File Name		: ether.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sat 21 Nov 2020 08:45:35 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/
#ifndef _ETHER_H
#define _ETHER_H

#include <string.h>

#define ETH_HEADER_SIZE sizeof(struct ether)
#define ETHER_ADDR_LEN 6

#define ETH_PROTOCOL_IP 0x0800
#define ETH_PROTOCOL_ARP 0x0806
#define ETH_PROTOCOL_RARP 0x8035

#define PACKET_MULTICAST 1
#define PACKET_BROADCAST 2
#define PACKET_LOCALHOST 3
#define PACKET_OTHERHOST 4

struct ether {
    unsigned char eth_dst[ETHER_ADDR_LEN];
    unsigned char eth_src[ETHER_ADDR_LEN];
    unsigned short ether_protocol;
    unsigned char ether_data[0];
} __attribute__((packed));

static inline int is_ether_multicast(unsigned char* mac_addr) {
    /*The eigth bit of the first byte of a mac address represents it's cast type.
     * When it's 0, it means send packet from source to a single destination. And when it's 1, it represents multicast.*/
    return (mac_addr[0] & 0x01);
}

static inline int is_ether_broadcast(unsigned char* mac_addr) {
    /*When the destination mac address equal to 0xffffffffffff, it means broadcast*/
    return (mac_addr[0] & mac_addr[1] & mac_addr[2] & mac_addr[3] & mac_addr[4] & mac_addr[5]) == 0xff;
}

static inline char* ethpro(unsigned short protocol) {
    if (protocol == ETH_PROTOCOL_IP) {
        return "IP";
    } else if (protocol == ETH_PROTOCOL_ARP) {
        return "ARP";
    } else if (protocol == ETH_PROTOCOL_RARP) {
        return "RARP";
    } else {
        return "Unknown";
    }
}

#endif /* _ETHER_H*/
