#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int BUFFER_SIZE = 100;

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
    return -1;
  }

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	inet_pton(AF_INET, argv[1], &server_address.sin_addr);

	server_address.sin_port = htons(atoi(argv[2]));

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

	int n = 0;
	int len = 0;
  char buffer[BUFFER_SIZE];
  
	while (1) {
    printf("Send >");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strlen(buffer) - 1] = '\0';  // remove \n
    send(sock, buffer, strlen(buffer), 0);

    n = recv(sock, buffer, BUFFER_SIZE, 0);
    if (n <= 0) {
      break;
    }
		printf("received: '%s'\n", buffer);
	}

	close(sock);
	return 0;
}