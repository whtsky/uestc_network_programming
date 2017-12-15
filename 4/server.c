#include "msg.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/select.h>
#include <unistd.h>


int TIMEOUT = 60;
int PING_TIMEOUT = TIMEOUT / 2;
int SELECT_TIMEOUT = TIMEOUT / 2;

typedef struct client {
  char username[USERNAME_LENGTH];
  struct sockaddr_in addr;
  time_t last_active;
  struct client *next;
} clientnode, *client;

client head = NULL;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf("Usage: %s <server_port>", argv[0]);
    return 0;
  }

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("creating socket");
    exit(EXIT_FAILURE);
  }

  struct socksaddr_in serveraddr;

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(atoi(argv[2]));
  if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    perror("bind error");
    exit(EXIT_FAILURE);
  }

  fd_set rset, allset;
  FD_ZERO(&allset);
  FD_SET(sockfd, &allset);
  for (;;) {
    rset = allset;
    nready = select(sockfd + 1, &rset, NULL, NULL, SELECT_TIMEOUT);
    if (nready < 0) {
      perror("select");
    }
    time_t now;
    time(&rawtime);

    if (FD_ISSET(sockfd, &rset)) {

      // todo: interactes w/ client
    }

    client c = head, prev = NULL;
    while (c != NULL) {
      uintmax_t delta = now - c->last_active;
      if (delta > TIMEOUT) {
        prev->next = c->next;
        free(c);
        c = prev->next;
        continue
      } else if (delta >= PING_TIMEOUT) {
        // ping client
      }
      prev = c;
      c = c->next;
    }
  }
}