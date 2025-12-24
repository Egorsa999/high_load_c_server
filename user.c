#include "user.h"
#include <string.h>
#include <stdio.h>

int init_db(sqlite3 **database) {
    int return_error = sqlite3_open("server.db", database);
    if (return_error != SQLITE_OK) return return_error;

    char *sql = "CREATE TABLE IF NOT EXISTS Users ("
                "Id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "Name TEXT UNIQUE, "
                "Password TEXT);";

    char *err_msg = 0;
    return_error = sqlite3_exec(*database, sql, 0, 0, &err_msg);
    if (return_error != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return return_error;
    }
    return SQLITE_OK;
}

int user_save(sqlite3 *database, struct User *user) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Users (Name, Password) VALUES (?, ?);";
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, user -> name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user -> password, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        user -> id = (int)sqlite3_last_insert_rowid(database);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return -1;
}

int user_get(sqlite3 *database, struct User *user) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT Id, Password FROM Users WHERE Name = ?;";
    int status = -1;

    if (sqlite3_prepare_v2(database, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, user->name, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *db_pass = (const char *)sqlite3_column_text(stmt, 1);
        if (db_pass && strcmp(user -> password, db_pass) == 0) {
            user -> id = sqlite3_column_int(stmt, 0);
            user -> logged = 1;
            status = 0;
        } else {
            status = -2;
        }
    } else {
        status = -3;
    }

    sqlite3_finalize(stmt);
    return status;
}