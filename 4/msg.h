#ifndef MSG_H
#define MSG_H

#define USERNAME_LENGTH 30;
#define MAX_LINE 1024;
#define LOGIN_MESSAGE ''

enum MESSAGE_TYPE {
    LOGIN_MESSAGE,
    TEXT_MESSAGE,
    QUIT_MESSAGE,
};

typedef struct {
    short type;
} Message;

typedef struct {
    Message msg;
    char username[USERNAME_LENGTH];
} LoginMessage;

typedef struct {
    Message msg;
    size_t length;
    char from[USERNAME_LENGTH];
    char *content;
} TextMessage;

#endif