#ifndef FT_PING_ICMP_H
#define FT_PING_ICMP_H

#include <stddef.h>
#include <stdint.h>

uint8_t *create_icmp_request(uint16_t id, uint16_t sequence, size_t payload_size);
int send_icmp_request(void);

#endif
