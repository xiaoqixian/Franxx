/**********************************************
  > File Name		: arp.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sun 22 Nov 2020 09:27:37 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/

#include "netif.h"
#include "ether.h"
#include "arp.h"
#include "lib.h"

#define BRAOADCAST_MAC_ADDR ((unsigned char*)"\xff\xff\xff\xff\xff\xff")

void arp_request(struct arp_entry* ae) {
    struct packet* pck;
    struct ether* ehdr;
    struct arp* ahdr;

    pck = alloc_packet(ETHER_HEADER_SIZE + ARP_HEADER_SIZE);
    ehdr = (struct ether*)pck->data;
    ahdr = (struct arp*)ehdr->data;

    ahdr->arp_hrd = _htons(ARP_HRD_ETHER);
    ahdr->protocol = _htons(ETHER_PROTOCOL_IP);
    ahdr->arp_hrd_len = ETHER_ADDR_LEN;
    ahdr->protocol_len = IP_ADDR_LEN;
    ahdr->arp_op = _htons(ARP_OP_REQUEST);
    ahdr->src_ip = ae->dev->ip_addr;
    memcpy(ahdr->src_mac, ae->dev->mac_addr, MAC_ADDR_LEN);
    ahdr->tgt_ip = ae->ip_addr;
    memcpy(ahdr->tgt_mac, BRAOADCAST_MAC_ADDR, MAC_ADDR_LEN);
    DEBUG("[ARP] " IP_FORMAT "(" MAC_FORMAT ")->" IP_FORMAT "(" MAC_FORMAT "): request", ahdr->src_ip, ahdr->src_mac, ahdr->tgt_ip, ahdr->tgt_mac);
}
