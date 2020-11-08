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

如果返回结果为0说明没有发生位翻转，数据没有问题。

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

