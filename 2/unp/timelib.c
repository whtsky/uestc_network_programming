#include "unp.h"
#include <time.h>

void print_time(int sockfd) {
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  char *timestring = asctime(timeinfo);
  Writen(sockfd, timestring, strlen(timestring));
}