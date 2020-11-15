/**********************************************
  > File Name		: tap.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sat 14 Nov 2020 09:39:47 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/
#ifndef _TAP_H
#define _TAP_H

#define TUNTAPDEV "/dev/net/tun"

void unset_tap(void);

void set_tap(void);

void delete_tap(void);

int alloc_tap(char* dev);

void get_hwaddr_tap(int tap_fd, unsigned char* hw_addr);

void get_name_tap(int tap_fd, unsigned char* name);

void get_ipaddr_tap(unsigned char* name, unsigned int* ipaddr);

void set_ipaddr_tap(unsigned char* name, unsigned int ipaddr);

void get_mtu_tap(unsigned char* name, int* mtu);

void set_up_tap(unsigned char* name);

void set_down_tap(unsigned char* name);

void set_falgs_tap(unsigned char* name, unsigned short flags, int set);

void set_netmask_tap(unsigned char* name, unsigned int netmask);

int set_persist_tap(int fd);

#endif
