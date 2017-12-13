#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVENTS 20
#define MAXLINE 4096
#define LISTENQ 1024

char buf[MAXLINE];

typedef struct {
  int fd;
  char *data;
  size_t data_size;
} client_data;

int setnonblocking(int sockfd) {
  if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
    return -1;
  }
  return 0;
}

client_data *create_client_data(int fd) {
  client_data *data = malloc(sizeof(client_data));
  setnonblocking(fd);
  data->fd = fd;
  data->data = NULL;
  data->data_size = 0;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>", argv[0]);
    exit(EXIT_FAILURE);
  }
  struct epoll_event ev, events[MAX_EVENTS];
  int epfd = epoll_create(256);
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  ev.data.ptr = create_client_data(listenfd);
  ev.events = EPOLLIN;
  struct sockaddr_in servaddr;
  epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));
  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  listen(listenfd, LISTENQ);
  while (1) {
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; ++i) {
      client_data *data = events[i].data.ptr;
      int fd = data->fd;
      if (fd == listenfd) {
        struct sockaddr in_addr;
        socklen_t in_len = sizeof(in_addr);
        int connfd = accept(listenfd, &in_addr, &in_len);
        ev.data.ptr = create_client_data(connfd);
        ev.events = EPOLLIN | EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
      } else if (events[i].events & EPOLLIN) {
        int n;
        if ((n = read(fd, buf, MAXLINE)) == 0) {
          if (data->data != NULL) {
            free(data->data);
          }
          free(data);
          close(fd);
          epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        } else {
          int wrote = write(fd, buf, n);
          if (wrote <= 0) {
            if (data->data_size > 0) {
              // already has unsent data, create a bigger buf
              int new_size = n + data->data_size;
              char *newbuf = malloc(new_size);
              strncpy(newbuf, data->data, data->data_size);
              strncpy(newbuf + data->data_size, buf, n);
              free(data->data);
              data->data = newbuf;
              data->data_size = new_size;
            } else {
              data->data = malloc(n);
              strncpy(data->data, buf, n);
              data->data_size = n;
              ev.data.ptr = data;
              ev.events = EPOLLOUT | EPOLLET;
              epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
            }
          }
        }
      } else if (events[i].events & EPOLLOUT) {
        client_data *data = events[i].data.ptr;
        if (data->data != NULL) {
          write(fd, data->data, data->data_size);
          free(data->data);
          data->data = NULL;
          data->data_size = 0;
        }
        ev.data.ptr = data;
        ev.events = EPOLLIN;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
      }
    }
  }
  close(epfd);
  return 0;
}
