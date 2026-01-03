#ifndef HIGH_LOAD_C_SERVER_WEBSOCKET_H
#define HIGH_LOAD_C_SERVER_WEBSOCKET_H

#include <sys/epoll.h>

#include "user.h"
#include "network.h"


/**
 * make handshake
 * @param server server struct
 * @param client client struct
 * @return execution code
 */
int ws_handshake(struct Server *server, struct Client *client);
/**
 * frame to text
 * @param client client struct
 * @return execution code
 */
int frame_to_text(struct Client *client);
/**
 * text to frame
 * @param message message
 * @param size message size
 * @return execution code
 */
int text_to_frame(char *message, int *size);

#endif //HIGH_LOAD_C_SERVER_WEBSOCKET_H
