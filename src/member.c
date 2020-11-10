/**********************************************
  > File Name		: member.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Tue 10 Nov 2020 08:37:54 PM CST
  > Location        : Shanghai
 **********************************************/
#include <stdio.h>
#include <stddef.h>

struct member {
    int a;
    double sd;
};

#define memberof(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))

int main() {
    struct member m{1,2.0};
    struct member* ptr = memberof(&m, struct member, sd);
    printf("%p\n", &m);
    printf("%p\n", ptr);
    printf("%zd\n", offsetof(struct member, a));
    printf("%zd\n", offsetof(struct member, sd));
    return 0;
}
