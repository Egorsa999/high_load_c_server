#ifndef TEST_REQUESTS_H
#define TEST_REQUESTS_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "user.h"
#include "registration.h"
#include "config.h"

/**
 * process users commands
 * @param server server struct
 * @param client client struct
 * @param buffer command from user
 * @param amount_bytes size of command
 */
void user_request(struct Server *server, struct Client *client, char *buffer, int amount_bytes);

#endif //TEST_REQUESTS_H
