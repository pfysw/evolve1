/*
 * mmap.c
 *
 *  Created on: Jan 20, 2021
 *      Author: Administrator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
int mmap_demo(int argc, char *argv[])
{
    int fd;
    char *addr;
    char *str = "Hello World";
    fd = open("a.txt", O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1)
    {
        perror("open file fail:");
        exit(1);
    }
    if (ftruncate(fd, 4096) == -1)
    {
        perror("ftruncate fail:");
        close(fd);
        exit(1);
    }
    addr = (char *) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == (char *) MAP_FAILED)
    {
        perror("mmap fail:");
        exit(1);
    }
    memset(addr, ' ', 4096);

    memcpy(addr, str, strlen(str));                       //hello world  1
    close(fd);
    memcpy(addr + strlen(str), str, strlen(str));           //hello world  2
    if (msync(addr, 4096, MS_SYNC) == -1)
    {
        perror("msync fail:");
        exit(1);
    }
    munmap(addr, 4096);
    return 0;
}
