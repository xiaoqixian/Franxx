/**********************************************
  > File Name		: lib/lib.c
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sat 14 Nov 2020 07:58:17 PM CST
  > Location        : Shanghai
  > Copyright @ https://github.com/xiaoqixian
 **********************************************/

#include <stdio.h>
#include "lib.h"

void perrx(char* str) {
    if (errno) {
        perror(str);
    } else {
        fprintf(stderr, "ERROR:%s\n", str);
    }
    exit(EXIT_FAILURE);
}

void* xmalloc(int size) {
    void* p = malloc((size_t)size);
    if (!p) {
        perrx("xmalloc failed");
    }
    return p;
}

void* xcalloc(int size) {
    void* p = cmalloc((size_t)size);
    if (!p) {
        perrx("xcmalloc failed");
    }
    return p;
}
