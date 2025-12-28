#ifndef HIGH_LOAD_C_SERVER_BASE64_H
#define HIGH_LOAD_C_SERVER_BASE64_H

#include <stddef.h>

char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);

#endif //HIGH_LOAD_C_SERVER_BASE64_H
