#include "ae.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <strings.h>

#define SOCKET_READ_BUFFER_LENGTH 4096

const int server_port = 9999;
const unsigned int LISTEN_QUEUE_LENGTH = 32;
int server_fd;
aeEventLoop* el;

int set_nonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    if (flag < 0) {
        printf("fcntl get flag error!\n");
        return -1;
    }
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) < 0) {
        printf("set flag error!\n");
        return -1;
    }
    return 0;
}

int set_closexec(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    if (flag < 0) {
        printf("fcntl get flag error!\n");
        return -1;
    }
    flag |= FD_CLOEXEC;
    if (fcntl(fd, F_SETFL, flag) < 0) {
        printf("set flag error!\n");
        return -1;
    }
    return 0;
}

void init_server()
{
    int reuseaddr = 1;
    struct sockaddr_in server_address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERROR: create socket error!\n");
        exit(1);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuseaddr, sizeof(int));
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("ERROR: failed to bind port %d\n",server_port);
        exit(1);
    }
    if (listen(server_fd, LISTEN_QUEUE_LENGTH) < 0) {
        printf("ERROR: failed to call listen\n");
        exit(1);
    }
    if (set_nonblocking(server_fd) == -1) {
        printf("ERROR: failed to set noblock socket\n");
        exit(1);
    }
}

int non_block_send(int fd,const char* buffer,unsigned int total_size)
{
    unsigned int ss = total_size;
    unsigned int cp = 0;
    while (ss > 0) {
        int rv=send(fd,buffer+cp,ss,0);
        if (rv > 0) {
            ss -= rv;
            cp += rv;
        }
        else {
            if (errno == EAGAIN || errno == EINTR || errno == ENOBUFS) {
                continue;
            }
            else {
                printf("nonblock send failed!\n");
                break;
            }
        }
    } // end while
    if (ss == 0)
        return 0;
    return -1;
}

void drive_machine(aeEventLoop *el, int fd, void *privdata, int mask)
{
    static char buffer[SOCKET_READ_BUFFER_LENGTH];
    char say[256];
    int rl = read(fd, buffer, SOCKET_READ_BUFFER_LENGTH);
    if (rl == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        else {
            aeDeleteFileEvent(el, fd, AE_READABLE);
            close(fd);
            return;
        }
    }
    else if (rl == 0) {
        aeDeleteFileEvent(el, fd, AE_READABLE);
        close(fd);
        return;
    }
    //printf("recv: [%s]\n", buffer);
    say[0] = 'o';
    say[1] = 'k';
    say[2] = '\0';
    non_block_send(fd, say, 2);
}

void accept_handler(aeEventLoop *el, int fd, void *privdata, int mask)
{
    struct sockaddr_in client_address;
    socklen_t length = sizeof(client_address);
    int client_fd = accept(fd, (struct sockaddr*)&client_address, &length);

    if (client_fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        printf("ERROR: errno %d comes when call accept!\n", errno);
        return;
    }
    if (set_nonblocking(client_fd) == -1) {
        printf("ERROR: failed to set noblock client socket\n");
        close(client_fd);
        return;
    }
    if (set_closexec(client_fd) == -1) {
        printf("ERROR: failed to set cloexec client socket\n");
        close(client_fd);
        return;
    }
    fflush(stdout);
    aeCreateFileEvent(el, client_fd, AE_READABLE, drive_machine, NULL);
}

int main(int argc, char** argv)
{
    init_server();

    el = aeCreateEventLoop();
    aeCreateFileEvent(el, server_fd, AE_READABLE, accept_handler, NULL);

    //aeSetBeforeSleepProc(el, beforeSleep);
    aeMain(el);
    aeDeleteEventLoop(el);

    return 0;
}

