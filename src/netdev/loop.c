/**********************************************
  > File Name		: loop.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sat 21 Nov 2020 08:08:33 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/

#include "netif.h"
#include "ip.h"
#include "lib.h"
#include "debug.h"

#define LOOPBACK_MTU 1500
#define LOOPBACK_IP_ADDR 0x0100007F /*127.0.0.2*/
#define LOOPBACK_NETMASK 0x000000FF /*255.0.0.0*/

struct netdev* loop

static int loop_dev_init(struct netdev* dev) {
    dev->net_mtu = LOOPBACK_MTU;
    dev->netmask = LOOPBACK_NETMASK;
    dev->ip_addr = LOOPBACK_IP_ADDR;
    DEBUG("%s ip address: " IP_FORMAT, dev->dev_name, IP_FORMAT_FUNC(LOOPBACK_IP_ADDR));
    DEBUG("%s netmask: " IP_FORMAT, dev->dev_name, IP_FORMAT_FUNC(LOOPBACK_NETMASJJK));
    return 0;
}

static void loop_recv(struct netdev* dev, struct packet* pck) {
    dev->net_stats.read_packets++;
    dev->net_stats.read_bytes += pck->len;
    ether_in(dev, pck);
}

static int loop_write(struct netdev* dev, struct packet* pck) {
    get_packet(pck);/*add reference to a packet*/
    /*loop back to itself*/
    loop_recv(dev, pck);
    dev->net_stats.read_packets++;
    dev->net_stats.read_bytes += pck->len;
    return pck->len;
}

static struct netdev_ops loop_ops = {
    .output = loop_write;
    .init = loop_dev_init;
    .exit = NULL;
}

void loop_init() {
    loop = netdev_alloc("loop", &loop_ops);
}

void loop_exit() {
    netdev_free(loop);
}
