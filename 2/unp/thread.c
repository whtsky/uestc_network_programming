#include "timelib.h"
#include "unpthread.h"

void thread_runner(int connfd) {
  Pthread_detach(pthread_self());
  print_time(connfd);
  Close(connfd);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>", argv[0]);
    exit(EXIT_FAILURE);
  }
  int listenfd, connfd;
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));

  Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

  Listen(listenfd, LISTENQ);

  while (1) {
    pthread_t tid;
    clilen = sizeof(cliaddr);
    connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    Pthread_create(&tid, NULL, (void *(*)(void *))thread_runner, (void *)connfd);
  }
  return 0;
}
