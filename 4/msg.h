#include <stdbool.h>

#ifndef MSG_H
#define MSG_H

#define USERNAME_LENGTH 30
#define MAX_LINE 1024
#define HEARTBEAT_TIMEOUT 6

enum MESSAGE_TYPE {
  LOGIN_MESSAGE,
  LOGOUT_MESSAGE,
  LOGINRESPONSE_MESSAGE,
  TEXT_MESSAGE,
  BOARDCAST_MESSAGE,
  PING_MESSAGE
};

typedef struct {
  char username[USERNAME_LENGTH];
} LoginMessage;

typedef struct {
  bool success;
  char reason[MAX_LINE];
} LoginResponse;

typedef struct {
  char user[USERNAME_LENGTH];
  char content[MAX_LINE];
} TextMessage;

typedef struct {
  char content[MAX_LINE];
} BroadcastMessage;

typedef struct {
  short type;
  union {
    LoginMessage login_message;
    LoginResponse login_response;
    TextMessage text_message;
    BroadcastMessage boardcast_message;
  } payload;
} Message;

#endif