#ifndef TEST_REGISTRATION_H
#define TEST_REGISTRATION_H

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "user.h"

void registration(int fd, char *buffer, int amount_bytes, struct User *users, sqlite3 *database);

#endif //TEST_REGISTRATION_H
