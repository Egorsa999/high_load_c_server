#ifndef TEST_REGISTRATION_H
#define TEST_REGISTRATION_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "user.h"
#include "config.h"
#include "network.h"

/**
 * registration/login logic
 * @param server server struct
 * @param client client struct
 * @param buffer command from user
 * @param amount_bytes size of command
 */
void registration(struct Server *server, struct Client *client, char *buffer, int amount_bytes);

#endif //TEST_REGISTRATION_H
