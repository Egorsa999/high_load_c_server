#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "user.h"
#include "registration.h"
#include "config.h"
#include "network.h"

void user_request(struct Server *server, struct Client *client, char *buffer, int amount_bytes) {
    if (client -> state == PROTO_WS_CONNECTED) {
        printf("Receive from WebSocket %d: %s\n", client -> fd, buffer);
    } else {
        printf("Receive from TCP %d: %s\n", client -> fd, buffer);
    }
    if (client -> user.logged == 0) {
        registration(server, client, buffer, amount_bytes);
    } else {
        char message[SEND_SIZE];
        unsigned int message_length;
        // send message to each connected user
        message_length = snprintf(message, sizeof(message), "Message from %s: %s\n", client -> user.name, buffer);
        if (message_length >= sizeof(message)) {
            message_length = sizeof(message) - 1;
        }
        for (int i = 0; i < server -> amount_connections; i++) {
            if (client -> fd != server -> connections[i]) {
                if (send_message(server, &server -> clients[server -> connections[i]], message, message_length) == -1) {
                    perror("send");
                }
            }
        }

        message_length = snprintf(message, sizeof(message), "You are logged as %s, wait next updates!\nNow your message was sent to every logged user!\n", client -> user.name);
        if (message_length >= sizeof(message)) {
            message_length = sizeof(message) - 1;
        }
        if (send_message(server, client, message, message_length) == -1) {
            perror("send");
        }
    }
}
