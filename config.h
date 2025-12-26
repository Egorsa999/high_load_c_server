#ifndef HIGH_LOAD_C_SERVER_CONFIG_H
#define HIGH_LOAD_C_SERVER_CONFIG_H

#define PORT "3490" // port for listening socket
#define BACKLOG 32768 // max amount of simultaneous connections

#define RECEIVE_SIZE 4096 // max size of receive
#define SEND_SIZE 4096 // max size of send

#define USERNAME_SIZE 51 // max size of username
#define PASSWORD_SIZE 31 // max size of password
#define USERNAME_SIZES "50" // max size of username for parsing
#define PASSWORD_SIZES "30" // max size of password for parsing

#define CMD_SEP '\n' // command separator

#endif //HIGH_LOAD_C_SERVER_CONFIG_H
