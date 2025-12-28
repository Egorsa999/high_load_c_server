#ifndef TEST_USER_H
#define TEST_USER_H

#include <sqlite3.h>

#include "config.h"

typedef enum {
    PROTO_UNKNOWN,
    PROTO_TCP,
    PROTO_WS_HANDSHAKE,
    PROTO_WS_CONNECTED
} UserState;

struct User {
    int logged;
    int id;
    char name[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    // input buffer
    char buffer[RECEIVE_SIZE + 1];
    int buffer_size;
    int buffer_checked;
    // output buffer
    char obuffer[SEND_SIZE];
    int obuffer_size;
    int obuffer_sent;
    // connection type
    UserState state;
};

/**
 * database init
 * @param db database
 * @return execution result
 */
int init_db(sqlite3 **db);
/**
 * save user in database
 * @param database database
 * @param user users array
 * @return execution result
 */
int user_save(sqlite3 *database, struct User *user);
/**
 * get user from database
 * @param database database
 * @param user user
 * @return execution result
 */
int user_get(sqlite3 *database, struct User *user);

#endif //TEST_USER_H
