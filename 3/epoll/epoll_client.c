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

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);

  char sendline[MAXLINE], recvline[MAXLINE];

  if (argc != 3) {
    printf("Usage: %s <ip> <port>", argv[0]);
    exit(EXIT_FAILURE);
  }

  struct epoll_event ev, events[MAX_EVENTS];
  int epfd = epoll_create(256);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));  
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
  connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  
  ev.data.fd = sockfd;
  ev.events = EPOLLIN;
  epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

  int stdinfd = fileno(stdin);
  ev.data.fd = stdinfd;
  epoll_ctl(epfd, EPOLL_CTL_ADD, stdinfd, &ev);

  printf(" >> ");

  while (1) {
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; ++i) {
      int fd = events[i].data.fd;
      if (fd == stdinfd) {
        // input
        if (fgets(sendline, MAXLINE, stdin) != NULL) {
          send(sockfd, sendline, strlen(sendline), 0);
        }
      } else {
        // server echo
        int n;
        if ((n = recv(sockfd, recvline, MAXLINE, 0)) <= 0) {
          fprintf(stderr, "server terminated prematurely");
          close(epfd);
          return 0;
        }
        recvline[n] = '\0';
        fputs(recvline, stdout);
        printf(" >> ");
      }
    }
  }
  close(epfd);
  return 0;
}
