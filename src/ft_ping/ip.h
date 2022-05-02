#ifndef FT_PING_IP_H
#define FT_PING_IP_H

#include <stdbool.h>
#include <stdint.h>
#include <netinet/ip.h>

bool is_iphdr_valid(struct iphdr *iphdr, uint16_t expected_total_length);

#endif
