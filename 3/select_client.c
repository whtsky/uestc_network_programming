#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 9999
#define BACKLOG 5
#define MAX_BUF_SIZE 100

/* MACROS */
#define MAX(a, b) (a > b ? a : b)

void str_cli(FILE *fp, int sockfd) {

  int n;
  int maxfdp1;
  int stdineof = 0;
  fd_set fdset;
  char sendline[MAX_BUF_SIZE], recvline[MAX_BUF_SIZE];

  FD_ZERO(&fdset);

  for (;;) {
    if (stdineof == 0) {
      FD_SET(fileno(fp), &fdset);
    }
    FD_SET(sockfd, &fdset);
    maxfdp1 = MAX(fileno(fp), sockfd) + 1;
    if (select(maxfdp1, &fdset, NULL, NULL, NULL) < 0) {
      perror("Error select");
    }
    /* socket is readable */
    if (FD_ISSET(sockfd, &fdset)) {
      if ((n = recv(sockfd, recvline, MAX_BUF_SIZE, 0)) == 0) {
        if (stdineof == 1) {
          printf("server closed connection\n");
          return; /* normal termination */
        } else {
          printf("server terminated prematurely\n");
          exit(1);
        }
      }
      recvline[n] = '\0';
      fputs(recvline, stdout);
    }
    /* input is readable */
    if (FD_ISSET(fileno(fp), &fdset)) {
      if (fgets(sendline, MAX_BUF_SIZE, fp) == NULL) {
        stdineof = 1;
        /* send FIN on write half, client closed connection */
        if (shutdown(sockfd, SHUT_WR) < 0) {
          perror("Error shutdown");
        }
        FD_CLR(fileno(fp), &fdset);
        continue;
      }
      send(sockfd, sendline, strlen(sendline), 0);
    }
  }
}

int main(int argc, char **argv) {

  int sock_fd, cli_fd;
  int yes = 1;
  int num_bytes;
  char buf[MAX_BUF_SIZE];
  unsigned int cli_len;
  struct sockaddr_in cli_addr, serv_addr;

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error Socket");
    exit(1);
  }

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, "192.168.1.77", &serv_addr.sin_addr);

  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
    perror("Error Setsockopt");
    exit(1);
  }

  if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Error Connect");
    exit(1);
  }

  str_cli(stdin, sock_fd);

  return 0;
}