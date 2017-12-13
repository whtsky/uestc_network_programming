#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv) {
  setbuf(stdout, NULL);

  int sockfd;
  struct sockaddr_in servaddr;

  if (argc != 3) {
    printf("Usage: %s <ip> <port>", argv[0]);
    exit(EXIT_FAILURE);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  int stdinfd = fileno(stdin);
  int maxfd = sockfd > stdinfd ? sockfd : stdinfd;
  fd_set rset, allset;
  FD_ZERO(&allset);
  FD_SET(sockfd, &allset);
  FD_SET(stdinfd, &allset);

  char sendline[MAXLINE], recvline[MAXLINE];

  printf(" >> ");

  while (1) {
    rset = allset;
    int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
    if (nready < 0) {
      printf("select failed\n ");
      return -1;
    }
    if (FD_ISSET(sockfd, &rset)) {
      // echo back from server, write into
      int n;
      if ((n = recv(sockfd, recvline, MAXLINE, 0)) <= 0) {
        fprintf(stderr, "server terminated prematurely");
                  close(sockfd);
        return 0;
      }
      recvline[n] = '\0';
      fputs(recvline, stdout);
      printf(" >> ");      
    }
    if (FD_ISSET(stdinfd, &rset)) {
      if (fgets(sendline, MAXLINE, stdin) != NULL) {
        send(sockfd, sendline, strlen(sendline), 0);
      }
    }
  }

  return 0;
}
