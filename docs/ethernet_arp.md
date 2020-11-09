---
author: lunar
date: Wed 04 Nov 2020 11:19:56 PM CST
location: Shanghai
---

# 1. Ethernet & ARP

### Contents

[TOC]

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

#### TAP设备的注册

我们需要先在设备中注册一个TAP设备才能使用它拦截流量。

源代码中的做法为：

```c
char *tapaddr = "10.0.0.5";
char *taproute = "10.0.0.0/24";

static int set_if_route(char *dev, char *cidr)
{
    return run_cmd("ip route add dev %s %s", dev, cidr);
}

static int set_if_address(char *dev, char *cidr)
{
    return run_cmd("ip address add dev %s local %s", dev, cidr);
}

static int set_if_up(char *dev)
{
    return run_cmd("ip link set dev %s up", dev);
}


```

基本就是运行几个shell命令来进行注册。

但说实话，这部分我还没有完全看懂，我知道每个命令代表什么，但是不知道为什么要这么做（如果有懂的大佬，麻烦评论去留言）。`ip`命令是linux中一个很强大的用于配置网络设备的命令，不太懂的同学可以翻到最后查看。

#### TAP设备的初始化

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

`struct ifreq`是一个用于设置网络设备的结构体。

其定义为

```c
struct ifreq {
    char ifr_name[IFNAMSIZ]; /* Interface name */
    union {
        struct sockaddr ifr_addr;
        struct sockaddr ifr_dstaddr;
        struct sockaddr ifr_broadaddr;
        struct sockaddr ifr_netmask;
        struct sockaddr ifr_hwaddr;
        short           ifr_flags;
        int             ifr_ifindex;
        int             ifr_metric;
        int             ifr_mtu;
        struct ifmap    ifr_map;
        char            ifr_slave[IFNAMSIZ];
        char            ifr_newname[IFNAMSIZ];
        char           *ifr_data;
    };
};
```

#### `ioctl`函数

`ioctl`函数是一个linux内核提供的用于用户对流设备进行设置的系统调用，你可以将其看作 io control 的缩写。该函数的第一个参数是打开该设备文件的文件描述符，第二个参数通常是要对该设备进行的操作的flag，再后面的参数可以是要传入的一些数据结构什么的。

在上面的`tun_alloc`函数（明明是用TAP设备我也不知道为什么要命名为`tun_alloc`）中可以看到，`ifr_flags`设置了TAP设备和不要提供包信息两种flag。然后就直接打开了TAP设备的文件描述符与`ifreq`绑定。这样就完成了TAP设备的初始化。

#### TAP设备的读写

TAP设备读写的代码非常简洁，就和普通的文件读写没什么两样。

```c
int tun_read(char *buf, int len)
{
    return read(tun_fd, buf, len);
}

int tun_write(char *buf, int len)
{
    return write(tun_fd, buf, len);
}

```

Linux的“一切皆文件”的思想在这里就体现得很好。

#### TAP读写的封装

尽管TAP设备的读写函数已经很好用了，但是我们还要对其进行进一步的封装。

在源代码的`netdev.*`文件中包含了负责底层信息传输的代码，其中`netdev`结构体定义为：

```c
struct netdev {
    uint32_t addr;//netdevice的ip地址，二进制的
    uint8_t addr_len; //MAC地址，6
    uint8_t hwaddr[6]; //MAC地址
    uint32_t mtu; //只在结构体定义和netdev_alloc用到，目前还不清楚用途。
};
```

网络栈中用该结构体表示一个基本的网络设备，`netdev_alloc`函数传入一个ip地址和一个MAC地址可生成一个`netdev`。

在`netdev_init`函数生成了两个具有特定IP和MAC地址的网络设备

```c
void netdev_init(char *addr, char *hwaddr)
{
    loop = netdev_alloc("127.0.0.1", "00:00:00:00:00:00", 1500);
    netdev = netdev_alloc("10.0.0.4", "00:0c:29:6d:50:25", 1500);
}
```

目前还不知道做什么用。

接下来重点看一下传输数据的代码

```c
int netdev_transmit(struct sk_buff *skb, uint8_t *dst_hw, uint16_t ethertype)
{
    struct netdev *dev;
    struct eth_hdr *hdr;
    int ret = 0;

    dev = skb->dev;

    skb_push(skb, ETH_HDR_LEN);

    hdr = (struct eth_hdr *)skb->data;

    memcpy(hdr->dmac, dst_hw, dev->addr_len);
    memcpy(hdr->smac, dev->hwaddr, dev->addr_len);

    hdr->ethertype = htons(ethertype);
    eth_dbg("out", hdr);

    ret = tun_write((char *)skb->data, skb->len);

    return ret;
}

```

该函数接受三个参数：发送的数据`skb`（现在暂时只要知道`sk_buff`是一个通用的用于传输数据的数据结构就行了）、接收端的MAC地址以及传输的数据类型，从接收数据的代码来看，目前接受的`ethertype`只有ARP和IPv4，IPv6或其它类型的暂时还不支持。

在这个函数里面，携带数据的`skb`的`data`指针实际上是一个以太网帧的结构体（什么是以太网帧下面就会讲）。将源MAC和目MAC复制到以太网帧里面后就可以通过`tun_write`函数写入了。

### Ethernet Frame

以太网帧是用于在数据链路层传输信息的一个基本单元。其结构体如下

```c
#include <linux/if_ether.h>
struct eth_hdr
{
    unsigned char dmac[6];
    unsigned char smac[6];
    uint16_t ethertype; //指示有效负载的长度或类型，本项目只用指示ARP还是IPv4
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

`data`字段用于指示arp协议的实际有效负载，这里用于映射MAC地址和IPv4地址。结构体为

```c
struct arp_ipv4 {
    unsigned char smac[6];
    uint32_t sip;
    unsigned char dmac[6];
    uint32_t dip;
} __attribute__((packed));
```

在[源代码](https://github.com/saminiir/level-ip.git)中，在src文件夹下，`arp.h`和`arp.c`文件定义了ARP协议的一些操作。比如可以往映射表中添加表项，可以更新某个IP对应的MAC地址，也可以直接接收一个`arp_hdr`结构体进行更新。甚至，还可以通过`arp_request`方法向外传输一个`arp_hdr`。

### `ip` command

由于在网络栈中使用了较多的`ip`命令来设置底层网络配置，所以了解一下linux下的 `ip` 还是很有必要的。

`ip` 命令的一般用法为：

```shell
ip [OPTIONS] OBJECT {COMMAND | HELP}
```

OBJECT是进行操作的对象，常用的对象有：

-   link: 网络设备
-   address: 设备上的协议地址（IPv4或IPv6）
-   route: 路由表条目
-   rule: 路由策略数据库中的规则



示例：

```shell
ip link show                     # 显示网络接口信息
ip link set eth0 up             # 开启网卡
ip link set eth0 down            # 关闭网卡
ip link set eth0 promisc on      # 开启网卡的混合模式
ip link set eth0 promisc offi    # 关闭网卡的混个模式
ip link set eth0 txqueuelen 1200 # 设置网卡队列长度
ip link set eth0 mtu 1400        # 设置网卡最大传输单元
ip addr show     # 显示网卡IP信息
ip addr add 192.168.0.1/24 dev eth0 # 设置eth0网卡IP地址192.168.0.1，后缀24表示子网掩码为255.255.255.0
ip addr del 192.168.0.1/24 dev eth0 # 删除eth0网卡IP地址

ip route show # 显示系统路由
ip route add default via 192.168.1.254   # 设置系统默认路由
ip route list                 # 查看路由信息
ip route add 192.168.4.0/24  via  192.168.0.254 dev eth0 # 设置192.168.4.0网段的网关为192.168.0.254,数据走eth0接口
ip route add default via  192.168.0.254  dev eth0        # 设置默认网关为192.168.0.254
ip route del 192.168.4.0/24   # 删除192.168.4.0网段的网关
ip route del default          # 删除默认路由
ip route delete 192.168.1.0/24 dev eth0 # 删除路由

```

综上来说，这一篇文章讲的是数据链路层，涉及到的数据交换自然是与物理层（TAP设备）的和与IP层的（ARP数据传输和IPv4数据传输，IPv4下章讲）。

从最底层开始讲起的话我们就知道我们最多能够仿真到就是数据链路层，数据链路层之间的传输是靠MAC地址，而MAC地址之间是如何寻址的是物理层之间的事，这个我们只能交给内核来解决。我们将源MAC和目MAC写入TAP文件就万事大吉了。

### 参考文章

[1] http://www.saminiir.com/lets-code-tcp-ip-stack-1-ethernet-arp/
[2] https://www.jianshu.com/p/09f9375b7fa7

[3] https://opengers.github.io/openstack/openstack-base-virtual-network-devices-tuntap-veth/

[4] https://www.runoob.com/linux/linux-comm-ip.html