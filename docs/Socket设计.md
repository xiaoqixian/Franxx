# Socket 设计

先来看下面两个结构体：

```c
/* PF_INET family sock structure */
struct sock {
	unsigned char protocol;	/* l4 protocol: tcp, udp */
	struct sock_addr sk_addr;
	struct socket *sock;
	struct sock_ops *ops;
	struct rtentry *sk_dst;
	/*
	 * FIXME: lock for recv_queue, or
	 *        Should we add recv_queue into recv_wait,
	 *        just using one mutex?
	 */
	struct list_head recv_queue;
	struct tapip_wait *recv_wait;
	unsigned int hash;	/* hash num for sock hash table lookup */
	struct hlist_node hash_list;
	int refcnt;
} __attribute__((packed));
```

```c
struct socket {
	unsigned int state;
	unsigned int family;	/* socket family: always AF_INET */
	unsigned int type;	/* l4 protocol type: stream, dgram, raw */
	struct tapip_wait sleep;
	struct socket_ops *ops;
	struct sock *sk;
	int refcnt;		/* refer to linux file::f_count */
};
```

这就是最主要的两个socket数据结构，`struct sock` 位于更核心的位置，在`ops`指针中，存储了一个指向包含了所有 socket 可进行的操作的函数指针的结构体的指针。

```c
struct sock_ops {
	void (*recv_notify)(struct sock *);
	void (*send_notify)(struct sock *);
	int (*send_pkb)(struct sock *, struct pkbuf *);
	int (*send_buf)(struct sock *, void *, int, struct sock_addr *);
	struct pkbuf *(*recv)(struct sock *);
	int (*recv_buf)(struct sock *, char *, int);
	int (*hash)(struct sock *);
	void (*unhash)(struct sock *);
	int (*bind)(struct sock *, struct sock_addr *);
	int (*connect)(struct sock *, struct sock_addr *);
	int (*set_port)(struct sock *, unsigned short);
	int (*close)(struct sock *);
	int (*listen)(struct sock *, int);
	struct sock *(*accept)(struct sock *);
};
```

而 `struct socket` 则像是对 `struct sock` 的一个简单封装，包含了 socket 的状态，协议簇，协议类型等成员。

在 `struct sock_ops` 结构体中定义了一系列函数指针，这些指针可以指向任何同类型的函数；而我们又知道，在创建socket时可以通过type参数指定不同的网络层协议：包括TCP和UDP。因此，我们的计策是为TCP和UDP分别根据各自的特性定义一套 `socket_ops` ，然后在创建socket时根据type参数决定赋予哪套函数指针。



### TCP three-way handshake

(1) 第一次握手：建立连接时，客户端A发送SYN包(SYN=j)到服务器B，并进入SYN_SEND状态，等待服务器B确认。

 (2) 第二次握手：服务器B收到SYN包，必须确认客户A的SYN(ACK=j+1)，同时自己也发送一个SYN包(SYN=k)，即SYN+ACK包，此时服务器B进入SYN_RECV状态。

 (3) 第三次握手：客户端A收到服务器B的SYN＋ACK包，向服务器B发送确认包ACK(ACK=k+1)，此包发送完毕，客户端A和服务器B进入ESTABLISHED状态，完成三次握手。