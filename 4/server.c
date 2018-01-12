#include "msg.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>


struct sockaddr_in server_addr;
socklen_t addrlen = sizeof(struct sockaddr_in);

typedef struct client {
  char username[USERNAME_LENGTH];
  struct sockaddr addr;
  time_t last_active;
  struct client *next;
} clientnode, *client;

client head = NULL;
int sock;

bool same_node(struct sockaddr *a, struct sockaddr *b) {
  return memcmp(a->sa_data, b->sa_data, sizeof(a->sa_data)) == 0;
}

void server_loop(void) {
  fd_set rset, allset;
  Message message, response;
  struct sockaddr client_addr;
  ssize_t status;
  FD_ZERO(&allset);
  FD_SET(sock, &allset);
  FD_SET(STDIN_FILENO, &allset);
  int maxfd = (STDIN_FILENO > sock) ? STDIN_FILENO : sock;
  time_t now;
  client c;
  struct timeval timeout = {0, 0};
  while (1) {
    rset = allset;
    timeout.tv_sec = HEARTBEAT_TIMEOUT;
    int nready = select(maxfd + 1, &rset, NULL, NULL, &timeout);
    if (nready < 0) {
      perror("select");
      exit(EXIT_FAILURE);
    }
    time(&now);
    if (FD_ISSET(sock, &rset)) {
      status =
          recvfrom(sock, &message, sizeof(message), 0, &client_addr, &addrlen);
      if (status == -1) {
        perror("error receiving message");
      }
      switch (message.type) {
      case LOGIN_MESSAGE: {
        response.type = LOGINRESPONSE_MESSAGE;
        response.payload.login_response.success = true;
        for (c = head; c != NULL; c = c->next) {
          if (strcmp(c->username, message.payload.login_message.username) ==
              0) {
            response.payload.login_response.success = false;
            strcpy(response.payload.login_response.reason,
                   "Duplicated username");
            break;
          }
        }
        if (c == NULL) {
          client newclient = malloc(sizeof(clientnode));
          newclient->last_active = now;
          strcpy(newclient->username, message.payload.login_message.username);
          newclient->addr = client_addr;
          newclient->next = head;
          for (client n = head; n != NULL; n = n->next) {
            sendto(sock, &message, sizeof(message), 0, &n->addr, addrlen);
          }
          head = newclient;
        }
        sendto(sock, &response, sizeof(response), 0, &client_addr, addrlen);
        printf("New user: %s\n", message.payload.login_message.username);
      } break;
      case PING_MESSAGE: {
        for (c = head; c != NULL; c = c->next) {
          if (same_node(&c->addr, &client_addr)) {
            printf("Receive ping from %s\n", c->username);
            c->last_active = now;
            break;
          }
        }
      } break;
      case TEXT_MESSAGE: {
        client source_node = NULL, target_node = NULL;
        for (c = head; c != NULL; c = c->next) {
          if (source_node != NULL && target_node != NULL) {
            break;
          }
          puts(c->username);
          puts(message.payload.text_message.user);
          printf("compare: %d\n",
                 strcmp(c->username, message.payload.text_message.user));
          if (strcmp(c->username, message.payload.text_message.user) == 0) {
            target_node = c;
          }
          if (same_node(&c->addr, &client_addr)) {
            source_node = c;
          }
        }
        if (source_node != NULL && target_node != NULL) {
          source_node->last_active = now;
          strcpy(message.payload.text_message.user, source_node->username);
          sendto(sock, &message, sizeof(message), 0, &target_node->addr,
                 addrlen);
        } else {
          puts("source or target node not found");
        }
      } break;
      case BOARDCAST_MESSAGE: {
        response.type = TEXT_MESSAGE;
        strcpy(response.payload.text_message.content,
               message.payload.boardcast_message.content);
        for (c = head; c != NULL; c = c->next) {
          if (same_node(&c->addr, &client_addr)) {
            c->last_active = now;
            strcpy(response.payload.text_message.user, c->username);
            break;
          }
        }
        for (c = head; c != NULL; c = c->next) {
          if (!same_node(&c->addr, &client_addr)) {
            sendto(sock, &response, sizeof(response), 0, &c->addr, addrlen);
          }
        }
      } break;
      }
    }
    if (FD_ISSET(STDIN_FILENO, &rset)) {
      // handles as server boardcast
      message.type = TEXT_MESSAGE;
      scanf("%s ", message.payload.text_message.user);
      strcpy(message.payload.text_message.user, "server");
      fgets(message.payload.text_message.content, MAX_LINE, stdin);
      sendto(sock, &message, sizeof(message), 0,
             (struct sockaddr *)&server_addr, addrlen);
    }
    c = head;
    client prev = NULL;
    while (c != NULL) {
      double diff = difftime(now, c->last_active);
      printf("client: %s, heartbeat diff: %f\n", c->username, diff);
      if (diff > HEARTBEAT_TIMEOUT) {
        printf("Client timeout: %s\n", c->username);
        if (prev != NULL) {
          prev->next = c->next;
        }
        free(c);
        if (prev != NULL) {
          c = prev->next;
        } else {
          // c is head
          head = NULL;
          c = NULL;
        }
        continue;
      }
      prev = c;
      c = c->next;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <server_port>", argv[0]);
    return 0;
  }

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("creating socket");
    exit(EXIT_FAILURE);
  }

  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(atoi(argv[1]));
  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind error");
    exit(EXIT_FAILURE);
  }
  server_loop();
}