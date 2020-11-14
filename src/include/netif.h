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

struct netstats;

struct netdev_ops;

struct pkbuf;

struct netdev {
    int net_mtu;
    int fd;
    unsigned int ipaddr;
    unsigned int netmask;
    unsigned char* hw_addr[HW_ADDR_LEN];
    unsigned char* dev_name[TAP_DEV_NAME_LEN];
    struct netdev_ops* ops;
    struct netstats* stats;
    struct list_node* netdev_list; 
};

#define GET_LOCAL_NET(dev) ((dev)->ipaddr & (dev)->netmask)

struct netstats {
    unsigned int rx_packets;
    unsigned int tx_packets;
    unsigned int rx_errors;
    unsigned int tx_errors;
    unsigned int rx_bytes;
    unsigned int tx_bytes;
};

/*net device operations*/
struct netdev_ops {
    int (*xmit)(struct netdev*, struct pkbuf*);
    int (*init)(sturct netdev*);
    void (*exit)(struct netdev*);
};

/* packet hardware address type*/
struct pkbuf {
    struct list_node* pk_list;
    unsigned short pk_pro;
    unsigned short pk_type;
    int len;
    int ref_count;
    struct netdev* pk_indev;
    
}

#endif
