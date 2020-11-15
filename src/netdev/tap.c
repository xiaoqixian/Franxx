/**********************************************
  > File Name		: tap.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Fri 13 Nov 2020 11:12:05 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>

#include <net/if.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/if_tun.h>

#include "debug.h"
#include "lib.h"
#include "tap.h"
/*
 * Open a tun device file and return a file descriptor
 * dev_name: when a tun device is opend, the kernel will allocate a name to it, we make the argument dev_name points to the name so we can know the allocated name.
 * flags: IFF_TUN(create a TUN device), IFF_TAP(create a TAP device), IFF_NO_PI(don't include any header information, by default, every data packet passed to user space will include a header to save packet information)
 */
int alloc_tap(char* dev_name) {
    assert(dev_name != NULL);
    
    struct ifreq ifr;
    int fd, err;

    if ((fd = open(TUNTAPDEV, O_RDWR)) < 0) {
        return fd;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    
    if (*dev_name != '\0') {
        strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void*)&ifr)) < 0) {
        close(fd);
        return err;
    }

    strcpy(dev_name, ifr.ifr_name);
    return fd;
}

/*
 * set TUN device socket, as a TUN device is just a tunnel, it need a socket to send a packet back to the host network stack and the host network stack will send the packet out.
 */
static int skfd;/*socket file descriptor*/
void set_tap_socket() {
    skfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);/*this socket function is a system call*/
    if (skfd < 0) {
        perror("create socket error");
    }
}

void unset_tap_socket() {
    close(skfd);
}

void delete_tap(int tap_fd) {
    if (ioctl(tap_fd, TUNSETPERSIST, 0) < 0) {
        return ;
    }
    close(tap_fd);
}

void unset_tap() {
    close(skfd);
}

/*
 * flag TUNSETPERSIST: Set the corresponding network device to the continuous mode, the default virtual network device, when its associated file character is closed, will also be associated with the routing and other information will disappear. If set to persistent mode, it will be reserved for later use. Bool value of type int.
 */
int set_persist_tap(int tap_fd) {
    if (!errno && ioctl(tap_fd, TUNSETPERSIST, 1) < 0) {
        perror("ioctl TUNSETPERSIST");
        return -1;
    }
    return 0;
}

/*
 * set TAP socket flags
 * SIOCGIFFLAGS: get original flags
 * SIOCSIFFLAGS: set new flags
 */
void setflags_tap(unsigned char* name, unsigned short flags, int set) {
    struct ifreq ifr = {};

    strcpy(ifr.ifr_name, (char*)name);
    /*get original flags*/
    if (ioctl(skfd, SIOCGIFFLAGS, (void*)&ift) < 0) {
        close(skfd);
        perrx("socket SIOCGIFFLAGS");
    }

    /*set new flags*/
    if (set) {
        ifr.ifr_flags |= flags;
    } else {
        ifr.ifr_flags &= ~flags & 0xffff;
    }
    if (ioctl(skfd, SIOCSIFFLAGS, (void*)&ifr) < 0) {
        close(skfd);
        perrx("socket SIOCSIFFLAGS");
    }
}

/*
 * SIOCSIFNETMASK: set netmask
 */
void set_tap_netmask(unsigned char* name, unsigned int netmask) {
    struct ifreq ifr = {};
    struct sockaddr_in* sockaddr;/*defined in <linux/socket.h>*/

    strcpy(ifr.ifr_name, (char*)name);
    sockaddr = (struct sockaddr_in*)&ift.ifr_netmask;
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = netmask;
    if (ioctl(skfd, SIOCSIFNETMASK, (void*)&ifr) < 0) {
        close(skfd);
        perrx("socket SIOCSIFNETMASK");
    }
    DEBUG("set netmask: ", ipfmt(netmask));
}

void setdown_tap(unsigned char* name) {
    setflags_tap(name, IFF_UP | IFF_RUNNING, 0);
    DEBUG("ifdown %s", name);
}

void setup_tap(unsigned char* name) {
    setflags_tap(name, IFF_UP | IFF_RUNNING, 1);
    DEBUG("ifup %s", name);
}

/*
 * mtu: Maximum Transmission Unit of data link layer
 */
void get_mtu_tap(unsigned char* name, int* mtu) {
    struct ifreq ifr = {};
    strcpy(ifr.ifr_name, (char*)name);

    if (ioctl(skfd, SIOCGIFMTU, (void*)&ifr) < 0) {
        close(skfd);
        perrx("ioctl SIOCGIFMTU");
    }
    *mtu = ifr.ifr_mtu;
    DEBUG("mtu: %d", ifr.ifr_mtu);
}

void set_ipaddr_tap(unsigned char* name, unsigned int ipaddr) {
    struct ifreq ifr = {};
    struct sockaddr_in* sockaddr;
    strcpy(ifr.ifr_name, (char*)name);
    
    sockaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    sockaddr->sin_addr.s_addr = ipaddr;
    sockaddr->sin_family = AF_INET;
    
    if (ioctl(skfd, SIOCSIFADDR, (void*)&ifr) < 0) {
        close(skfd);
        perrx("socket SIOCSIFADDR");
    }
    DEBUG("set tap ipaddr: %d", ipfmt(ipaddr));
}

void get_ipaddr_tap(unsigned char* name, unsigned int* ipaddr) {
    struct ifreq ifr = {};
    strcpy(ifr.ifr_name, (char*)name);

    if (ioctl(skfd, SIOCGIFADDR, (void*)&ifr) < 0) {
        close(skfd);
        perrx("socket SIOCGIFADDR");
        return ;
    }
    struct sockaddr_in* sockaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    *ipaddr = sockaddr->sin_addr.s_addr;
    DEBUG("get tap ipaddr: %d", ipfmt(*ipaddr));
}

void get_name_tap(int tap_fd, unsigned char* name) {
    struct ifreq ifr = {};
    if (ioctl(tap_fd, TUNGETIFF, (void*)&ifr)) {
        perrx("ioctl TUNGETIFF");
    }
    strcpy((char*)name, ifr.ifr_name);
    DEBUG("get tap name: %s", name);
}

/*
 * get hardware address of the tap device, which is known as MAC address.
 */
void get_hwadddr_tap(int tap_fd, unsigned char* hw_addr) {
    struct ifreq ifr;
    memset(&ifr, 0x0, sizeof(ifr));
    if (ioctl(tap_fd, SIOCGIFHWADDR, (void*)&ifr) < 0) {
        perrx("ioctl SIOCGIFHWADDR");
    }
    strcpy(hw_addr, ifr.ifr_hwaddr.sa_data);
    DEBUG("get tap hw_addr: %02x:%02x:%02x:%02x:%02x:%02x", hw_addr[0], hw_addr[1], hw_addr[2], hw_addr[3], hw_addr[4], hw_addr[5]);
}
