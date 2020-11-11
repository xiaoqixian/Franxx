---
author: lunar
date: Sat 07 Nov 2020 09:48:26 PM CST
location: Shanghai
---

# IPv4 & ICMPv4

### Internet Protocol version 4

IPv4协议属于网络层，其每一次数据的分发都是没有记忆性的。尽管单次发送的IP报文没有记忆性，但是IP报文允许一次的数据分段发送。因为每次发送的IP报文的数据大小是受限的。

#### IPv4头

```c
struct iphdr {
    uint8_t version : 4;
    uint8_t ihl : 4;
    uint8_t tos;
    uint16_t len; //整个IP数据报文的长度
    uint16_t id; //id用于接收端整理受到的报文，因为IP报文不一定是按发送顺序到达的。
    uint16_t flags : 3; //指示是否允许分段发送，是否是最后一个分段，是否还有分段报文到来等等。
    uint16_t frag_offset : 13; //本次报文分段在整个报文中的偏移量
    uint8_t ttl; //报文的生命周期
    uint8_t proto; //用于只是上一层的协议，通常值为16 (UDP) or 6 (TCP)
    uint16_t csum; //校验码，用于检验传输过程是否有位翻转等问题
    uint32_t saddr; //源地址
    uint32_t daddr; //收地址
    uint8_t data[]; //ip报文的正文，这里看来没有对正文的长度做要求
} __attribute__((packed));
```

检测校验码的函数定义为：

```c
uint16_t checksum(void *addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     * Taken from https://tools.ietf.org/html/rfc1071
     */

    register uint32_t sum = 0;
    uint16_t * ptr = addr;

    while( count > 1 )  {
        /*  This is the inner loop */
        sum += * ptr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if( count > 0 )
        sum += * (uint8_t *) ptr;

    /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}
```

如果返回结果为0说明没有发生位翻转，数据没有问题。其实知道有这么个函数就行了，`checksum`的具体实现涉及到信息传输原理方面的知识，没有这方面的基础就不要硬理解了。

### Internet Control Message Protocol version 4

ICMP4协议，用于传输一些网络控制的数据的协议，我们在使用ping程序时就是使用的这个协议。

其头部定义为：

```c
struct icmp_v4 {
    uint8_t type; //指示发送信息的目的，有42中不同的数字用于表示不同的目的
    uint8_t code; //用于进一步解释信息，比如当接收到返回的ICMP数据的type是3（Destination Unreachable）时，code用于解释无法到达的原因。
    uint16_t csum; //checksum,同上
    uint8_t data[];
} __attribute__((packed));
```

### IP层的传输过程

在源码`ip_input.c`中包含了ip层接收数据的函数，如下：

```c
int ip_rcv(struct sk_buff *skb)
{
    struct iphdr *ih = ip_hdr(skb);
    uint16_t csum = -1;

    if (ih->version != IPV4) {
        print_err("Datagram version was not IPv4\n");
        goto drop_pkt;
    }

    if (ih->ihl < 5) {
        print_err("IPv4 header length must be at least 5\n");
        goto drop_pkt;
    }

    if (ih->ttl == 0) {
        //TODO: Send ICMP error
        print_err("Time to live of datagram reached 0\n");
        goto drop_pkt;
    }

    csum = checksum(ih, ih->ihl * 4, 0);

    if (csum != 0) {
        // Invalid checksum, drop packet handling
        goto drop_pkt;
    }

    // TODO: Check fragmentation, possibly reassemble

    ip_init_pkt(ih);

    ip_dbg("in", ih);

    /*IP层支持向上传递的TCP协议数据和同层之间传递的ICMP协议数据*/
    switch (ih->proto) {
    case ICMPV4:
        icmpv4_incoming(skb);
        return 0;
    case IP_TCP:
        tcp_in(skb);
        return 0;
    default:
        print_err("Unknown IP header proto\n");
        goto drop_pkt;
    }

drop_pkt:
    free_skb(skb);
    return 0;
}
```

收到一个IP包后要完成一下事情：

1.  检查IP版本，目前只支持IPv4版本；
2.  检查ip包头部长度，长度小于5说明数据肯定出问题了；
3.  检查生命周期，如果生命周期到头了，就丢包；
4.  验证校验码是否为0；
5.  将ip的各个数据从网络字节序转为主机字节序；
6.  收到的数据包有两个去处，如果是TCP协议的包，则上传给TCP层（目前看来该网络栈不支持UDP协议）；如果是ICMP4协议的包，则交给该协议处理函数；

`tcp_in`留到下章讲TCP协议的讲，TCP协议的复杂度远超数据链路层和IP层，算是最麻烦的一章了。

现在来看一下`icmpv4_incoming`函数

```c
void icmpv4_incoming(struct sk_buff *skb) 
{
    struct iphdr *iphdr = ip_hdr(skb);
    struct icmp_v4 *icmp = (struct icmp_v4 *) iphdr->data;

    //TODO: Check csum

    switch (icmp->type) {
    case ICMP_V4_ECHO:
        icmpv4_reply(skb);
        return;
    case ICMP_V4_DST_UNREACHABLE:
        print_err("ICMPv4 received 'dst unreachable' code %d, "
                  "check your routes and firewall rules\n", icmp->code);
        goto drop_pkt;
    default:
        print_err("ICMPv4 did not match supported types\n");
        goto drop_pkt;
    }

drop_pkt:
    free_skb(skb);
    return;
}
```

从这段代码可以知道，ICMPv4的数据是包裹在IP报文里面的传输的，所以ICMPv4其实可以看作TCP层。

目前icmp仅支持一种类型`ICMP_V4_ECHO`的应答。

再来看ICMPv4的应答过程：

```c
void icmpv4_reply(struct sk_buff *skb)
{
    struct iphdr *iphdr = ip_hdr(skb);
    struct icmp_v4 *icmp;
    struct sock sk;
    memset(&sk, 0, sizeof(struct sock));
    
    uint16_t icmp_len = iphdr->len - (iphdr->ihl * 4);

    skb_reserve(skb, ETH_HDR_LEN + IP_HDR_LEN + icmp_len);
    skb_push(skb, icmp_len);
    
    icmp = (struct icmp_v4 *)skb->data;
        
    icmp->type = ICMP_V4_REPLY;
    icmp->csum = 0;
    icmp->csum = checksum(icmp, icmp_len, 0);

    skb->protocol = ICMPV4;
    sk.daddr = iphdr->saddr;

    ip_output(&sk, skb);
    free_skb(skb);
}
```

应答过程同样非常简单，首先同时初始化一个ip报文和一个icmp报文，但是并不需要重新分配内存，直接利用接收到的报文的数据结构改改数据就可以继续用了。

各项数据的设置都非常简单，不多讲了。

然后，你会发现多出来了一个新的结构体 `struct sock`，看名字就知道是与socket有关的。看到最后，发现 `ip_output` 函数需要用到这个结构体，查看 `ip_output` 函数

```c
int ip_output(struct sock *sk, struct sk_buff *skb)
{
    struct rtentry *rt;
    struct iphdr *ihdr = ip_hdr(skb);

    rt = route_lookup(sk->daddr);

    if (!rt) {
        // TODO: dest_unreachable
        print_err("IP output route lookup fail\n");
        return -1;
    }

    skb->dev = rt->dev;
    skb->rt = rt;

    skb_push(skb, IP_HDR_LEN);

    ihdr->version = IPV4;
    ihdr->ihl = 0x05;
    ihdr->tos = 0;
    ihdr->len = skb->len;
    ihdr->id = ihdr->id;
    ihdr->frag_offset = 0x4000;
    ihdr->ttl = 64;
    ihdr->proto = skb->protocol;
    ihdr->saddr = skb->dev->addr;
    ihdr->daddr = sk->daddr;
    ihdr->csum = 0;

    ip_dbg("out", ihdr);

    ihdr->len = htons(ihdr->len);
    ihdr->id = htons(ihdr->id);
    ihdr->daddr = htonl(ihdr->daddr);
    ihdr->saddr = htonl(ihdr->saddr);
    ihdr->csum = htons(ihdr->csum);
    ihdr->frag_offset = htons(ihdr->frag_offset);

    ip_send_check(ihdr);

    return dst_neigh_output(skb);
}
```

这个函数里面调用了两个比较重要的函数：`route_lookup`和`dst_neigh_output`。

前者我们稍后再看，先看`dst_neigh_output`，`dst_neigh_output`是一个定义在`dst.c`文件中的函数，其实现为

```c
int dst_neigh_output(struct sk_buff *skb)
{
    struct iphdr *iphdr = ip_hdr(skb);
    struct netdev *netdev = skb->dev;
    struct rtentry *rt = skb->rt;
    uint32_t daddr = ntohl(iphdr->daddr);
    uint32_t saddr = ntohl(iphdr->saddr);

    uint8_t *dmac;

    if (rt->flags & RT_GATEWAY) {
        daddr = rt->gateway;
    }
    
    dmac = arp_get_hwaddr(daddr);
    
    if (dmac) {
        return netdev_transmit(skb, dmac, ETH_P_IP);
    } else {
        arp_request(saddr, daddr, netdev);

        /* Inform upper layer that traffic was not sent, retry later */
        return -1;
    }
}
```

这个函数做了两个比较重要的事：第一个是查看是否设置了网关的flag，如果有，则IP地址要改为网关的IP，我们知道两个局域网之间要进行通信是必须要经过网关的，但是作者好像目前为止还没有添加任何局域网之间的通信，因为没有看到任何局域网之间寻址算法的代码（看来只能由我来加上了吗）。

第二件事就是调用`arp_get_hwaddr`函数，利用ARP协议来根据一个IP地址获取相应设备的MAC地址，最后再调用`netdev_transmit`传输数据。

总的来说，这是一个在同一局域网内传输数据的函数，包括传输给网关。

#### 寻址

再来看我们说的 `route_lookup` 函数。

```c
struct rtentry *route_lookup(uint32_t daddr)
{
    struct list_head *item;
    struct rtentry *rt = NULL;

    list_for_each(item, &routes) {
        rt = list_entry(item, struct rtentry, list);
        /*如果daddr和rt->dst的非子网掩码的部分相同，说明位于同一局域网内*/
        if ((daddr & rt->netmask) == (rt->dst & rt->netmask)) break;
        // If no matches, we default to to default gw (last item)
    }
    
    return rt;
}
```

`rtentry` 的定义如下：

```c
struct rtentry {
    struct list_head list;
    uint32_t dst;
    uint32_t gateway;
    uint32_t netmask;
    uint8_t flags;
    uint32_t metric; //metric即度量，决定了ip在路由中的下一跳，在这个项目中根本没用（苦笑）
    struct netdev *dev;
};
```

`routes`是一个全局变量，是一个循环链表，里面存储了所有的局域网（应该可以这么说）。在那个循环里面，会注意比对目的ip地址 `daddr` 与每个局域网的 `dst` 属性在进行子网掩码运算后的值，如果可以匹配，则说明属于该局域网，就返回该局域网的 `entry`。

