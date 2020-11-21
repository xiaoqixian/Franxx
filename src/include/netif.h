/**********************************************
  > File Name		: netif.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sun 15 Nov 2020 12:02:34 AM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/
#ifndef _NETIF_H
#define _NETIF_H

/*
 * network interface
 */

#define HW_ADDR_LEN 6 /*MAC address length*/
#define TAP_DEV_NAME_LEN 16 /*IFNAMSIZ*/

#define IP_FORMAT "%d.%d.%d.%d"
#define IP_FORMAT_FUNC(ip) (ip) & 0xff, ((ip) >> 8) & 0xff, ((ip) >> 16) & 0xff, ((ip) >> 24) & 0xff

#define MAC_FORMAT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_FORMAT_FUNC(hw_addr) (hw_addr)[0],(hw_addr)[1],(hw_addr)[2],(hw_addr)[3],(hw_addr)[4],(hw_addr)[5]

struct netstats;

struct netdev_ops;

struct pkbuf;

struct netdev {
    int net_mtu; /*net minimum transmission unit*/
    int fd;
    unsigned int ip_addr;
    unsigned int netmask;
    unsigned char* mac_addr[HW_ADDR_LEN];
    unsigned char* dev_name[TAP_DEV_NAME_LEN];
    struct netdev_ops* ops;
    struct netstats stats;
    struct list_node* netdev_list; 
};

#define GET_LOCAL_NET(dev) ((dev)->ipaddr & (dev)->netmask)

struct netstats {
    unsigned int read_packets;
    unsigned int write_packets;
    unsigned int read_errors;
    unsigned int write_errors;
    unsigned int read_bytes;
    unsigned int write_bytes;
};

/*net device operations*/
struct netdev_ops {
    int (*output)(struct netdev*, struct pkbuf*);
    int (*init)(sturct netdev*);
    void (*exit)(struct netdev*);
};

/* packet hardware address type*/
struct packet {
    struct list_node* pk_list;
    unsigned short protocol; /*ethernet packet type ID*/
    unsigned short type; /*packet hardware address ID: MULTICASE,BROADCASE,LOCALHOST,OTHERHOST*/
    int len; /*length of data*/
    int ref_count;
    struct netdev* in_dev;
    struct route_entry* route_dst; /*defined in route.h*/
    struct sock* sock;
    unsigned char data[0];
} __attribute__((packed));

/* packet hardware address type*/
#define PKT_NONE 0
#define PKT_LOCALHOST 1
#define PKT_OTHERHOST 2
#define PKT_MULTICAST 3
#define PKT_BROADCAST 4

#define HOST_LITTLE_ENDIAN /* default: little endian host*/
#ifdef HOST_LITTLE_ENDIAN

static _inline unsigned short _htons(unsigned short host) {
    return (host >> 8) | ((host << 8) & 0xff00);
}

#define _ntohs(net) _htons(net)

static _inline unsigned int _htonl(unsigned int host) {
    return ((host & 0x000000ff) << 24) | ((host & 0x0000ff00) << 8) | ((host & 0x00ff0000) >> 8) | ((host & 0xff000000) >> 24);
}

#define _nthhl(net) _htonl(net)

#endif /*HOST_LITTLE_ENDIAN*/

extern struct netdev* tap;
extern struct netdev* loop;

extern void netdev_init(void);
extern struct netdev* netdev_alloc(char*, struct netdev_ops*);
extern void netdev_free(struct netdev*);
extern void netdev_interrupt(void);
extern void netdev_exit(void);

extern void ether_in(struct netdev*, struct packet*);
extern void ether_timer(void);

extern struct packet* alloc_pakcet(int size);
extern struct packet* alloc_netdev_packet(struct netdev*);

#endif
