#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  struct epoll_event ev, events[20];
  epfd = epoll_create(256);
  epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
  bzero(&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  listen(listenfd, LISTENQ);
  while (true) {
    int nfds = epoll_wait(epfd, events, 20, 500);
    //处理所发生的所有事件
    for (i = 0; i < nfds; ++i) {
      if (events[i].data.fd == listenfd) {
        epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
      } else if (events[i].events & EPOLLIN) {
        // read
        epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
      } else if (events[i].events & EPOLLOUT) {
        // out
        sockfd = events[i].data.fd;
        //设置用于读操作的文件描述符
        ev.data.fd = sockfd;
        //设置用于注测的读操作事件
        ev.events = EPOLLIN | EPOLLET;
        //修改sockfd上要处理的事件为EPOLIN
        epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
      }
    }
  }
}
