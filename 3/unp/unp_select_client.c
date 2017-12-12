#include "unp.h"

int main(int argc, char **argv) {
  setbuf(stdout, NULL);

  int sockfd;
  struct sockaddr_in servaddr;

  if (argc != 3) {
    printf("Usage: %s <ip> <port>", argv[0]);
    exit(EXIT_FAILURE);
  }

  sockfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));
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
    int nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
    if (nready < 0) {
      printf("select failed\n ");
      return -1;
    }
    if (FD_ISSET(sockfd, &rset)) {
      // echo back from server, write into
      if (Readline(sockfd, recvline, MAXLINE) == 0)
        err_quit("server terminated prematurely");
      Fputs(recvline, stdout);
      printf(" >> ");      
    }
    if (FD_ISSET(stdinfd, &rset)) {
      if (Fgets(sendline, MAXLINE, stdin) != NULL) {
        Writen(sockfd, sendline, strlen(sendline));
      }
    }
  }

  exit(0);
}
