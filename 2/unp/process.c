#include "timelib.h"
#include "unp.h"

void sig_chld(int signo) {
  pid_t pid;
  int stat;

  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    printf("child %d terminated\n", pid);
  }
  return;
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

  Signal(SIGCHLD, sig_chld);

  for (;;) {
    clilen = sizeof(cliaddr);
    if ((connfd = accept(listenfd, (SA *)&cliaddr, &clilen)) < 0) {
      if (errno == EINTR)
        continue;
      else
        err_sys("accept error");
    }

    if ((childpid = Fork()) == 0) {
      Close(listenfd);
      print_time(connfd);
      exit(0);
    }
    Close(connfd);
  }
}
