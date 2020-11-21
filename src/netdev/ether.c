/**********************************************
  > File Name		: ether.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sat 21 Nov 2020 08:30:31 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/

/*ether net level, level 2*/
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/if_tun.h>

#include "netif.h"
#include "ip.h"
#include "ether.h"
#include "arp.h"
#include "lib.h"
#include "netcfg.h"

static struct ether* ether_init(struct netdev* dev, struct packet* pck) {
    struct ether* ether_header = (struct ether*)pck->data;
    if (pck->len < ETHER_HEADER_SIZE) {
        free_packet(pck);
        DEBUG("received packet is tool small: %d bytes", pck->len);
        return NULL;
    }
    /*mac address type*/
    if (is_ether_multicast(ether_header->dst)) {
        if (is_ether_broadcast(ether_header->dst)) {
            pck->type = PACKET_BROADCAST;
        } else {
            pck->type = PACKET_MULTICAST;
        }
    } else if (!memcmp(ether_header->eth_dst, dev->mac_addr, ETHER_ADDR_LEN)) {
        pck->type = PACKET_LOCALHOST;
    } else {
        pck->type = PACKET_OTHERHOST;
    }
    /*packet protocol*/
    pck->protocol = _ntohs(ether_header->ether_protocol);
    return ether_header;
}

void ether_in(struct netdev* dev, struct packet* pck) {
    struct ether* ether_header = ether_init(dev, pck);
    if (!ether_header) {
        return ;
    }
    DEBUG("[ETHERNET]: parsing ethernet packet");
    DEBUG(MAC_FORMAT " -> " MAC_FORMAT "(%s)", MAC_FORMAT_FUNC(ether_header->ether_src), MAC_FORMAT_FUNC(ether_header->ether_dst), ethpro(pck->protocol));
    pck->in_dev = dev;
    switch (pck->protocol) {
        case ETH_PROTOCOL_ARP:
            arp_in(dev, pck);
            break;
        case ETH_PROTOCOL_IP:
            ip_in(dev, pck);
            break;
        case ETH_PROTOCOL_RARP:
            //rarp_in(dev, pck);
            break;
        default:
            DEBUG("drop unsupported type packet");
            free_packet(pck);
            break;
    }
}

void ether_timer() {
    while (1) {
        sleep(1);
        arp_timer(1);
        ip_timer(1);
    }
}
