#ifndef FT_PING_CHECKSUM_H
#define FT_PING_CHECKSUM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint16_t get_checksum(void *addr, size_t size);
bool is_checksum_valid(void *addr, size_t size, uint16_t *checksum_ptr);

#endif
