#include <sys/socket.h>
#include <time.h>

void print_time(int sockfd) {
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  timeinfo = localtime(&rawtime);
  char *timestring = asctime(timeinfo);
  send(sockfd, timestring, strlen(timestring), 0);
}
