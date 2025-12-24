#ifndef TEST_USER_H
#define TEST_USER_H

#include <sqlite3.h>

struct User {
    int logged;
    int id;
    char name[51];
    char password[31];
};

int init_db(sqlite3 **db);
int user_save(sqlite3 *database, struct User *user);
int user_get(sqlite3 *database, struct User *user);

#endif //TEST_USER_H
