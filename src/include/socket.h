/**********************************************
  > File Name		: socket.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Fri 13 Nov 2020 09:37:36 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/
#ifndef _SOCKET_H
#define _SOCKET_H

#define SOCK_STREAM 0x01
#define SOCK_DGRAM 0x02

struct sock_ops;

struct socket {
    uint8_t state;
    uint8_t family; /*alaways AF_INET*/
    uint8_t type; /*SOCK_STREAM, SOCK_DGRAM*/
    struct socket_ops* ops; /*a struct of function pointers*/
    int ref_count;
};

struct sock_addr {
    unsigned int saddr; /*source ip address*/
    unsigned int daddr; /*destination ip address*/
    unsigned short sport; /*source port*/
    unsigned short dport; /*destination port*/
} __attribute__((packed)); /*tell the compiler not to do byte alignment*/

struct socket_ops {
    int (*socket)(struct socket*, int);
    int (*close)(struct socket*);
    int (*accept)(struct socket*, struct socket*, struct sock_addr*);
    int (*listen)(struct socket*, int);
    int (*bind)(struct socket*, struct sock_addr*);
    int (*connect)(struct socket*, struct sock_addr*);
    int (*read)(struct socket*, void*, int);
    int (*write)(struct socket*, void*, int);
    int (*send)(struct socket*, void*, int);
    void* (*recv)(struct socket*);
};



#endif
