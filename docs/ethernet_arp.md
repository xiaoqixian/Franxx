---
author: lunar
date: Wed 04 Nov 2020 11:19:56 PM CST
location: Shanghai
---

# 1. Ethernet & ARP

### TUN/TAP 设备

#### TUN 设备

TUN 设备是一种虚拟网络设备，通过它可以方便地模拟网络行为。

![](https://upload-images.jianshu.io/upload_images/1918847-92c47952b6f7fdd6.png?imageMogr2/auto-orient/strip|imageView2/2/w/840/format/webp)

如图，在网络栈上，TUN 设备与网卡同层，也就是说TUN 设备可以被虚拟为一个虚拟网卡。

虚拟网卡通过设备文件/dev/tun与应用程序相连，应用程序可以通过write之类的系统调用向这个文件写数据，这些数据会以网络数据包的形式经由虚拟网卡传递给网络协议栈。

也就是说，从真实网卡进来的数据包先通过网络协议栈后到达某个应用程序（比如一个全局代理的VPN），然后该应用程序再将数据写会到底层的TUN设备文件，再经过一次网络协议栈到达其它目标应用程序。

所以，通过TUN 设备可以对计算机上的流量进行监管和控制。

#### TAP 设备

TAP 设备与TUN 设备的工作方式完全相同，区别在于：

1. TUN 设备是一个第3层设备，只模拟到了IP层，无法与物理网卡做bridge。但是可以通过三层交换与物理网卡连通；
2. TAP 则是一个第2层设备，可以通过tap文件收发数据链路层数据包

TAP设备常用于模拟虚拟网卡，比如在虚拟机中就使用它来模拟网卡；TUN设备更多的用于模拟网络层设备，openvpn的底层就是使用这个。

我们将使用 Linux TAP 设备来拦截来自Linux内核的低级网络流量。

如果我们要想从网络栈的第2层起来构建（第1层是物理层肯定没办法软件模拟），我们就需要如下初始化一个 TAP 设备：

```c
/*
 * Taken from Kernel Documentation/networking/tuntap.txt
 */
int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tap", O_RDWR)) < 0 ) {
        print_error("Cannot open TUN/TAP dev");
        exit(1);
    }

    CLEAR(ifr);

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if( *dev ) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
        print_error("ERR: Could not ioctl tun: %s\n", strerror(errno));
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}
```

### Ethernet Frame

以太网帧是用于在数据链路层传输信息的一个基本单元。其结构体如下

```c
#include <linux/if_ether.h>

struct eth_hdr
{
    unsigned char dmac[6];
    unsigned char smac[6];
    uint16_t ethertype; //指示有效负载的长度或类型
    unsigned char payload[]; //包含一个指向以太网帧有效负载的指针
} __attribute__((packed));
```



### ARP Protocol

ARP协议全称为 Address Resolution Protocol. 主要用于数据链路层的寻址，ARP协议能够将IP层地址与MAC地址互相映射

#### Packet Structure

![图来自维基百科](https://i.loli.net/2020/11/07/CQgKkwRiXjIcaZS.png)

最重要的部分包括了发送者的MAC地址和IP地址，接收者的MAC地址和IP地址。

其结构体为

```c
struct arp_hdr
{
    uint16_t hwtype;
    uint16_t protype;
    unsigned char hwsize;
    unsigned char prosize;
    uint16_t opcode;
    unsigned char data[];
} __attribute__((packed));
```

### 参考文章

[1] http://www.saminiir.com/lets-code-tcp-ip-stack-1-ethernet-arp/
[2] https://www.jianshu.com/p/09f9375b7fa7

[3] https://opengers.github.io/openstack/openstack-base-virtual-network-devices-tuntap-veth/