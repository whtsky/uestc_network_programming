#include <stdlib.h>
#include <stdio.h>

void error(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}