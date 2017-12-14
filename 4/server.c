
#include "msg.h"

struct sockaddr_in server_addr;
struct sockaddr_in self_addr;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf("Usage: %s <server_port>", argv[0]);
    return 0;
  }

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("creating socket");
    exit(EXIT_FAILURE);
  }


}