#ifndef FT_PING_CHECKSUM_H
#define FT_PING_CHECKSUM_H

#include <stddef.h>
#include <stdint.h>

uint16_t get_checksum(void *addr, size_t size);

#endif
