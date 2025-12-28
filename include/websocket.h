#ifndef HIGH_LOAD_C_SERVER_WEBSOCKET_H
#define HIGH_LOAD_C_SERVER_WEBSOCKET_H

#include <poll.h>

#include "user.h"


/**
 * make handshake
 * @param fd socket fd
 * @param user user struct
 * @param poll_struct poll struct
 * @return execution code
 */
int ws_handshake(int fd, struct User *user, struct pollfd *poll_struct);
/**
 * frame to text
 * @param user user struct
 * @return execution code
 */
int frame_to_text(struct User *user);
/**
 * text to frame
 * @param message message
 * @param size message size
 * @return execution code
 */
int text_to_frame(char *message, int *size);

#endif //HIGH_LOAD_C_SERVER_WEBSOCKET_H
