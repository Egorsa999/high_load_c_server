#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "websocket.h"
#include "network.h"
#include "sha1.h"
#include "base64.h"

// magic constant
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

int text_to_frame(char *message, int *size) {
    int header_length = 2;
    if (*size >= 126) {
        if (*size > 65535) {
            return -1; // too large size
        }
        header_length = 4;
    }
    if (*size + header_length > SEND_SIZE) return -1; // too large size;
    //left shift our message to header_length
    memmove(message + header_length, message, *size);
    message[0] = 0x81;
    if (header_length == 2) {
        message[1] = *size;
    } else {
        message[1] = 126;
        message[2] = (*size >> 8) & 0xFF;
        message[3] = *size & 0xFF;
    }
    *size += header_length;
    return *size;
}

int frame_to_text(struct Client *client) {
    if (client -> buffer_size - client -> buffer_checked < 2) return 0; // not enough bytes

    unsigned char *data = (unsigned char *)client -> buffer + client -> buffer_checked;

    int opcode = data[0] & 0x0F;
    if (opcode == 0x8) return -1; // close connection

    int is_masked = data[1] & 0x80;
    int size = data[1] & 0x7F;
    int header_length = 2;

    if (size == 126) {
        if (client -> buffer_size - client -> buffer_checked < 4) return 0; // not enough bytes;
        size = (data[2] << 8) | data[3];
        header_length = 4;
    } else {
        if (size == 127) {
            return -1; // too large frame
        }
    }

    if (!is_masked) return -1; // protocol error

    int total_size = header_length + 4 + size; // + 4 mask byte
    if (client -> buffer_size - client -> buffer_checked < total_size) return 0; // not enough bytes

    // decode
    unsigned char mask[4];
    memcpy(mask, data + header_length, 4);
    unsigned char *payload = data + header_length + 4;
    char *start = client -> buffer + client -> buffer_checked;

    for (int i = 0; i < size; i++) {
        start[i] = payload[i] ^ mask[i % 4];
    }
    start[size] = '\0';

    client -> buffer_checked += total_size;

    return size;
}

int ws_handshake(struct Server *server, struct Client *client) {
    char *buffer = client -> buffer;

    char *key_start = strstr(buffer, "Sec-WebSocket-Key: ");
    if (!key_start) {
        return -1; // bad websocket request
    }

    key_start += 19; // shift throught "Sec-WebSocket-Key: "

    char *key_end = strstr(key_start, "\r\n");
    if (!key_end) {
        return -1; // bad websocket request
    }

    char key[128];
    int key_length = key_end - key_start;

    if (key_length > 64) {
        return -1; // bad websocket request;
    }

    strncpy(key, key_start, key_length);
    key[key_length] = '\0';
    // add magic constant
    strcat(key, WS_GUID);

    // calc SHA-1 hash
    unsigned char hash[20];
    SHA1_CTX sha;
    SHA1Init(&sha);
    SHA1Update(&sha, (unsigned char *)key, strlen(key));
    SHA1Final(hash, &sha);

    // encode hash to base-63
    size_t out_len;
    char *base64_hash = base64_encode(hash, 20, &out_len);
    if (!base64_hash) {
        return -1; // error
    }

    // make answer to handshake
    char response[512];
    int response_len = snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "\r\n",
        base64_hash
    );

    free(base64_hash);
    return send_message(server, client, response, response_len);
}
