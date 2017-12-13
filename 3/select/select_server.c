#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/select.h>
#include <unistd.h>

#define MAXLINE 4096
#define LISTENQ 1024

char buf[MAXLINE];

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>", argv[0]);
    exit(EXIT_FAILURE);
  }

  int i, maxi, maxfd, listenfd, connfd, sockfd;
  int nready, client[FD_SETSIZE];
  ssize_t n;
  fd_set rset, allset;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    error("can't open socket\n");
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));

  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

  listen(listenfd, LISTENQ);

  maxfd = listenfd;
  maxi = -1;
  for (i = 0; i < FD_SETSIZE; i++)
    client[i] = -1;
  FD_ZERO(&allset);
  FD_SET(listenfd, &allset);

  for (;;) {
    rset = allset;
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
    if (nready < 0) {
      error("select error");
    }

    if (FD_ISSET(listenfd, &rset)) { /* new client connection */
      clilen = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
      for (i = 0; i < FD_SETSIZE; i++)
        if (client[i] < 0) {
          client[i] = connfd;
          break;
        }
      if (i == FD_SETSIZE) {
        fprintf(stderr, "Error: too many clients\n");
        exit(EXIT_FAILURE);
      }

      FD_SET(connfd, &allset);
      if (connfd > maxfd)
        maxfd = connfd;
      if (i > maxi)
        maxi = i;

      if (--nready <= 0)
        continue;
    }

    for (i = 0; i <= maxi; i++) {
      if ((sockfd = client[i]) < 0)
        continue;
      if (FD_ISSET(sockfd, &rset)) {
        if ((n = read(sockfd, buf, MAXLINE)) == 0) {
          close(sockfd);
          FD_CLR(sockfd, &allset);
          client[i] = -1;
        } else
          write(sockfd, buf, n);

        if (--nready <= 0)
          break;
      }
    }
  }
  return EXIT_SUCCESS;
}