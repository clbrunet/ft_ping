#ifndef FT_PING_UTILS_PRINT_H
#define FT_PING_UTILS_PRINT_H

#include <stddef.h>
#include <stdint.h>

void print_error(const char *error_title, const char *error_description);
void print_memory(const uint8_t *address, size_t size);

#endif
