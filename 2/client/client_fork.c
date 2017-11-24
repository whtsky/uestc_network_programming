#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int BUFFER_SIZE = 100;

void sig_chld(int signo) {
  pid_t pid;
  int stat;

  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    printf("child %d terminated\n", pid);
  }
  return;
}

int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];  
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <server> <port> <request_number>\n", argv[0]);
    return -1;
  }

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	inet_pton(AF_INET, argv[1], &server_address.sin_addr);

	server_address.sin_port = htons(atoi(argv[2]));
  int request_number = atoi(argv[3]);

  for (int i = 0; i < request_number; i++) {
    int childpid;
    if ((childpid = fork()) == 0) {
      int sock;
      if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "could not create socket\n");
        return -1;
      }
      if (connect(sock, (struct sockaddr*)&server_address,
                  sizeof(server_address)) < 0) {
        fprintf(stderr, "could not connect to server\n");
        return -1;
      }
      
      int n = recv(sock, buffer, BUFFER_SIZE, 0);
      if (n > 0) {
        printf("%s", buffer);
      }
      close(sock);
      exit(EXIT_SUCCESS);
    } else {
      printf("child: %d\n", childpid);
    }
  }
	return 0;
}