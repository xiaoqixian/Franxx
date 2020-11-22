/**********************************************
  > File Name		: arp.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sun Nov 22 21:27:47 2020
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/
#ifndef _ARP_H
#define _ARP_H

#include "ether.h"
#include "ip.h"
#include "lish.h"

/*arp format*/
#define ARP_HRD_ETHER 1

/*arp cache*/
#define ARP_CACHE_SIZE 20
#define ARP_TIMEOUT 600 /*10 minutes*/
#define ARP_WAITTIME 1

/*arp entry state*/
#define ARP_FREE 1
#define ARP_WAITING 2
#define ARP_RESOLVED 3
#define ARP_REQ_ENTRY 4

struct arp_entry {
    struct list_node* list; /*packets pending for mac address*/
    struct netdev* dev; /*associated net interface*/
    int retry; /*arp request retrying times*/
    int ttl; /*entry timeout*/
    unsigned int state;
    unsigned int protocol; /*L3 protocol supported by arp protocol*/
    unsigned int ip_addr; 
    unsigned char mac_addr[ETHER_ADDR_LEN];
    
};

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2

#define ARP_HEADER_SIZE sizeof(struct arp)

struct arp {
    //unsigned short mac_addr_type;/*hardware address type*/
    unsigned short arp_hrd;
    unsigned short protocol; /*protocol address type*/
    unsigned char arp_hrd_len; /*hardware address length*/
    unsigned char protocol_len; /*protocol address length*/
    unsigned short arp_op;/*arp op code*/
#if defined(ARP_ETHERNET) && defined(ARP_IP)
    unsigned char src_mac[ETHER_ADDR_LEN];
    unsigned int src_ip;
    unsigned char tgt_mac[ETHER_ADDR_LEN];/*target mac address*/
    unsigned int tgt_ip;
#else
    unsigned char data[0];
#endif
} __attribute__((packed));

static inline void arp_hton(struct arp* arp_header) {
    arp_header->mac_addr_type = _htons(arp_header->mac_addr_type);
    arp_header->protocol = _htons(arp_header->protocol);
    arp_header->arp_op = _htons(arp_header->arp_op);
}

#define arp_ntoh(arp_header) arp_hton(arp_header)

#endif /* _ARP_H*/
