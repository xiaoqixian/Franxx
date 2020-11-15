/**********************************************
  > File Name		: packet.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sun 15 Nov 2020 11:44:26 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/

#include "netif.h"
#include "debug.h"
#include "lib.h"

#define MAX_PACKETS 200
int free_packets = 0;
int alloc_packets = 0;

#define packets_safe()\
do {\
    if ((alloc_packets - free_packets) > MAX_PACKETS) {\
        DEBUG("Too many packets");\
    }\
} while (0)

void packet_trim(struct packet* pck, int len) {
    if (realloc(pck, sizeof(*pck)+len) == NULL) {
        perrx("realloc failed");
    }
    pck->len = len;
}

struct packet* alloc_packet(int size) {
    struct packet* pck;
    pck = xcalloc(sizeof(*pkb) + size);
    pck->len = size;
    pck->ref_count = 1;
    pck->protocol = 0xffff;
    pck->type = 0;
    pck->in_dev = NULL;
    pck->route_dst = NULL;
    list_init(pck->list_node);
    alloc_packets++;
    packets_safe();
    return pck;
}


