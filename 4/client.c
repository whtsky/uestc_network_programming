#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#include "msg.h"

char *help_message = "/help: show this message\n\
/send <username> <message>: send user a message\n\
/sendall <message>: send everyone a message\n\
/quit: exit\n";

struct sockaddr_in self_addr, server_addr;
int sock;
char *nickname;
socklen_t addrlen = sizeof(struct sockaddr_in);

Message ping_message = {.type = PING_MESSAGE};

void heartbeat(void) {
  sendto(sock, &ping_message, sizeof(ping_message.type), 0,
         (struct sockaddr *)&server_addr, addrlen);
}

void do_login(void) {
  Message message;
  message.type = LOGIN_MESSAGE;
  strcpy(message.payload.login_message.username, nickname);
  sendto(sock, &message, sizeof(message), 0, (struct sockaddr *)&server_addr,
         addrlen);
  ssize_t status = recvfrom(sock, &message, sizeof(message), 0,
                            (struct sockaddr *)&server_addr, &addrlen);
  if (status == -1) {
    perror("error receiving login response");
    exit(EXIT_FAILURE);
  }
  if (message.type != LOGINRESPONSE_MESSAGE) {
    perror("Not receiving login response");
    exit(EXIT_FAILURE);
  }
  if (!message.payload.login_response.success) {
    printf("Login error: %s\n", message.payload.login_response.reason);
    exit(EXIT_FAILURE);
  }
}

void receive_packet_loop(void) {
  // use select to ensure we send message at least HEARTBEAT_TIMEOUT/2 seconds
  Message message;
  ssize_t status;
  struct timeval timeout = {0, 0};
  fd_set allset, rset;
  FD_ZERO(&allset);
  FD_SET(STDIN_FILENO, &allset);
  FD_SET(sock, &allset);
  int maxfd = (STDIN_FILENO > sock) ? STDIN_FILENO : sock;
  puts(help_message);
  while (1) {
    rset = allset;
    timeout.tv_sec = HEARTBEAT_TIMEOUT / 2;
    int nready = select(maxfd + 1, &rset, NULL, NULL, &timeout);
    if (nready < 0) {
      perror("select");
    }
    if (FD_ISSET(STDIN_FILENO, &rset)) {
      char command[10];
      if (scanf(" /%s", command)) {
        if (strcmp(command, "quit") == 0) {
          exit(EXIT_SUCCESS);
        } else if (strcmp(command, "help") == 0) {
          puts(help_message);
          heartbeat();
        } else if (strcmp(command, "send") == 0) {
          message.type = TEXT_MESSAGE;
          scanf(" %s", message.payload.text_message.user);
          fgets(message.payload.text_message.content, MAX_LINE, stdin);
          sendto(sock, &message, sizeof(message), 0,
                 (struct sockaddr *)&server_addr, addrlen);
        } else if (strcmp(command, "sendall") == 0) {
          message.type = BOARDCAST_MESSAGE;
          fgets(message.payload.boardcast_message.content, MAX_LINE, stdin);
          sendto(sock, &message, sizeof(message), 0,
                 (struct sockaddr *)&server_addr, addrlen);
        } else {
          puts(help_message);
        }
      }
    } else {
      heartbeat();
    }
    if (FD_ISSET(sock, &rset)) {
      status = recvfrom(sock, &message, sizeof(message), 0,
                        (struct sockaddr *)&server_addr, &addrlen);
      if (status == -1) {
        perror("error receiving message");
      }
      switch (message.type) {
      case LOGIN_MESSAGE:
        printf("Welcome new user: %s\n",
               message.payload.login_message.username);
        break;
      case LOGOUT_MESSAGE:
        printf("%s has logged out.\n", message.payload.login_message.username);
        break;
      case TEXT_MESSAGE:
        printf("[%s]: %s", message.payload.text_message.user,
               message.payload.text_message.content);
        break;
      default:
        printf("Unexpected receive: %d", message.type);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr,
            "Usage: %s <nickname> <server_ip> <server_port> <local_port>",
            argv[0]);
    return 0;
  }

  nickname = argv[1];
  char *server_ip = argv[2];
  int server_port = atoi(argv[3]);
  int local_port = atoi(argv[4]);

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("create socket");
    exit(EXIT_FAILURE);
  }

  bzero(&self_addr, sizeof(self_addr));
  self_addr.sin_family = AF_INET;
  self_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  self_addr.sin_port = htons(local_port);

  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  if (inet_aton(server_ip, &server_addr.sin_addr) == 0) {
    perror("parsing server ip");
    exit(EXIT_FAILURE);
  }
  server_addr.sin_port = htons(server_port);
  if (bind(sock, (struct sockaddr *)&self_addr, addrlen) < 0) {
    perror("bind error");
    exit(EXIT_FAILURE);
  }
  do_login();
  puts("Login success");

  receive_packet_loop();
}