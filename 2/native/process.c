#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "timelib.h"

void sig_chld(int signo) {
  pid_t pid;
  int stat;

  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    printf("child %d terminated\n", pid);
  }
  return;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return -1;
  }
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	int listen_sock;
	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "could not create listen socket\n");
		return -1;
	}

	if ((bind(listen_sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		fprintf(stderr, "could not bind socket\n");
		return 1;
	}

	int wait_size = 16;
	if (listen(listen_sock, wait_size) < 0) {
		printf("could not open socket for listening\n");
		return -1;
	}

	struct sockaddr_in client_address;
	socklen_t client_address_len = 0;

	while (1) {
    int childpid;
		int sock = accept(listen_sock, (struct sockaddr *)&client_address, &client_address_len);
		if (sock < 0) {
			fprintf(stderr, "could not open a socket to accept data\n");
			return -1;
		}
    if ((childpid = fork()) == 0) {
      close(listen_sock);
      print_time(sock);
      exit(0);
    }
    close(sock);
	}

	close(listen_sock);
  return 0;
}
