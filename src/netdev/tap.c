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

/*
 * Open a tun device file and return a file descriptor
 * dev_name: when a tun device is opend, the kernel will allocate a name to it, we make the argument dev_name points to the name so we can know the allocated name.
 * flags: IFF_TUN(create a TUN device), IFF_TAP(create a TAP device), IFF_NO_PI(don't include any header information, by default, every data packet passed to user space will include a header to save packet information)
 */
int tun_alloc(char* dev_name, int flags) {
    assert(dev_name != NULL);
    
    struct ifreq ifr;
    int fd, err;
    
    char* tun_dev = "/dev/net/tun";

    if ((fd = open(tun_dev, O_RDWR)) < 0) {
        return fd;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_flags = flags;
    
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
 * flag TUNSETPERSIST: Set the corresponding network device to the continuous mode, the default virtual network device, when its associated file character is closed, will also be associated with the routing and other information will disappear. If set to persistent mode, it will be reserved for later use. Bool value of type int.
 */
int set_tap(int fd) {
    if (!errno && ioctl(fd, TUNSETPERSIST, 1) < 0) {
        perror("ioctl TUNSETPERSIST");
        return -1;
    }
    return 0;
}

int main() {
    int tun, res;
    char dev_name[IFNAMSIZ];
    unsigned char buf[4096];

    dev_name[0] = '\0';
    tun = tun_alloc(dev_name, IFF_TUN | IFF_NO_PI);
    if (tun < 0) {
        perror("tun_alloc error");
        return -1;
    }
    printf("TUN name: %s\n", dev_name);

    while (1) {
        unsigned char ip[4];
        res = read(tun, buf, sizeof(buf));
        if (res < 0) {
            break;
        }
        printf("receive: %s\n", buf);
        memcpy(ip, &buf[12], 4);
        memcpy(&buf[12], &buf[16], 4);
        memcpy(&buf[16], ip, 4);
        buf[20] = 0;
        *((unsigned char*)&buf[22]) += 8;
        printf("read %d bytes\n", res);
        res = write(tun, buf, res);
        printf("write %d bytes\n", res);
        printf("write %s\n", buf);
    }
    return 0;
}
