#ifndef FT_PING_ICMP_H
#define FT_PING_ICMP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <netinet/ip.h>
#include <linux/icmp.h>

#define IPV4_PACKET_SIZE(data_size) (sizeof(struct iphdr) + data_size)
#define ICMP_PACKET_SIZE(payload_size) (sizeof(struct icmphdr) + payload_size)

uint8_t *create_icmp_request(uint16_t id, uint16_t sequence, size_t payload_size);
int update_icmp_request(void);
int send_icmp_request(void);
bool is_icmphdr_valid(struct icmphdr *icmphdr, size_t size,
		uint8_t expected_type, uint16_t expected_id);

#endif
