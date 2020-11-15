/**********************************************
  > File Name		: tap_driver.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sun 15 Nov 2020 07:43:26 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/

#include "tap.h"
#include "netif.h"
#include "lib.h"
#include "list.h"
#include "debug.h"
#include "netcfg.h"

#include <stdio.h>
#include <stdlib.h>

struct netdev* tap;

static int tap_init() {
    tap = (struct netdev*)malloc(sizeof(struct netdev));/*xmalloc: if memory allocation failed, exit the program and print error information on stderr*/
    if (tap == NULL) {
        DEBUG("malloc failed");
        exit(1);
    }
    tap->id = alloc_tap("tap0");
    if (tap->id < 0) {
        DEBUG("alloc tap failed");
        goto free_tap;
    }
    if (set_persist_tap(tap->fd) < 0) {
        goto close_tap;
    }
    /*set tap information*/
    set_tap();
    get_name_tap(tap->fd, tap->dev_name);
    get_mtu_tap(tap->fd, &tap->net_mtu);
#ifndef CONFIG_TOP1
    get_hwaddr_tap(tap->fd, tap->hw_addr);
    set_ipaddr_tap(tap->dev_name, FAKE_TAP_ADDR);
    get_ipaddr_tap(tap->dev_name, &(tap->ipaddr));
    set_netmask_tap(tap->dev_name, FAKE_TAP_NETMASK);
    setup_tap(tap->dev_name);
#endif
    unset_tap();
    list_init(tap->list);
    return 0;

free_tap:
    free(tap);
    return -1;
close_tap:
    close(tap->fd);
}

static int tap_write(struct netdev* dev, struct packet* pck) {
    int l = write(dev->fd, pck->data, pck->len);
    if (l != pck->len) {
        DEBUG("tap_write length wrong");
        dev->stats.write_errors++;
    } else {
        dev->stats.write_packets++;
        dev->stats.write_bytes += l;
        DEBUG("tap write %d bytes", l);
    }
    return l;
}

static int tap_read(struct packet* pck) {
    int l = read(tap->fd, pck->data, pck->len);
    if (l <= 0) {
        DEBUG("tap_read read nothing");
        tap->stats.read_errors++;
    } else {
        tap->stats.read_packets++;
        tap->stats.read_bytes += l;
        pck->len = l;
        DEBUG("tap read %d bytes", l);
    }
}

static void driver_read() {
    struct packet* pck = alloc_packet();
    if (tap_read(pck) > 0) {
        ether_in(tap, pck);/*pass to upper ethernet level*/
    } else {
        free_packet(pck);
    }
}

/*
 * use poll to read from tap device
 */
static void tap_poll() {
    struct pollfd pfd = {};
    int res;
    while (1) {
        pfd.fd = tap->fd;
        pfd.events = POLLIN; /*wait for readable events*/
        pfd.revents = 0;
        res = poll(&pfd, 1, -1); /*one event, infinite time*/
        if (res <= 0) {
            perrx("poll /dev/net/tun");
        }
        driver_read();
    }
}
