# TCP协议

终于到了最麻烦的TCP协议，TCP拥有着复杂的同步和重传机制，并且本次项目的最终目的是要在TCP协议之上进行socket编程接口的编写。

为了缩减篇幅，就不对TCP协议进行介绍了，网络上关于这方面的文章挺多的，我们直接对源码入手。

### TCP连接的建立

建立TCP连接的函数定义在 `tcp_output.c` 文件中，

```c
int tcp_connect(struct sock *sk)
{
    struct tcp_sock *tsk = tcp_sk(sk);
    struct tcb *tcb = &tsk->tcb;
    int rc = 0;
    
    tsk->tcp_header_len = sizeof(struct tcphdr);
    tcb->iss = generate_iss();
    tcb->snd_wnd = 0;
    tcb->snd_wl1 = 0;

    tcb->snd_una = tcb->iss;
    tcb->snd_up = tcb->iss;
    tcb->snd_nxt = tcb->iss;
    tcb->rcv_nxt = 0;

    tcp_select_initial_window(&tsk->tcb.rcv_wnd);

    rc = tcp_send_syn(sk);
    tcb->snd_nxt++;
    
    return rc;
}
```

基本操作就是初始化一个TCP报文后，调用 `tcp_send_syn` 函数开始建立连接。

于是我们转到 `tcp_send_syn` 函数

```c
static int tcp_send_syn(struct sock *sk)
{
    if (sk->state != TCP_SYN_SENT && sk->state != TCP_CLOSE && sk->state != TCP_LISTEN) {
        print_err("Socket was not in correct state (closed or listen)\n");
        return 1;
    }

    struct sk_buff *skb;
    struct tcphdr *th;
    struct tcp_options opts = { 0 };
    int tcp_options_len = 0;

    tcp_options_len = tcp_syn_options(sk, &opts);
    skb = tcp_alloc_skb(tcp_options_len, 0);
    th = tcp_hdr(skb);

    tcp_write_syn_options(th, &opts, tcp_options_len);
    sk->state = TCP_SYN_SENT;
    th->syn = 1;

    return tcp_queue_transmit_skb(sk, skb);
}
```

我们真正需要关注的是最后一个 return 语句调用的 `tcp_queue_transmit` 函数。

