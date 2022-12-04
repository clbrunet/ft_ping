#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <linux/icmp.h>

#include "ft_ping/icmp.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/inet.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/checksum.h"

uint8_t *create_icmp_request(uint16_t id, uint16_t sequence, size_t payload_size)
{
	uint8_t *icmp_request = malloc(ICMP_PACKET_SIZE(payload_size));
	if (icmp_request == NULL) {
		print_error("malloc", ft_strerror(errno));
		return NULL;
	}
	struct icmphdr *icmphdr = (struct icmphdr *)icmp_request;
	icmphdr->type = ICMP_ECHO;
	icmphdr->code = 0;
	icmphdr->checksum = 0;
	icmphdr->un.echo.id = ft_htons(id);
	icmphdr->un.echo.sequence = ft_htons(sequence);

	uint8_t *payload = (uint8_t *)(icmphdr + 1);
	int payload_pad_bytes_begin_index = 0;
	if (payload_size >= sizeof(struct timeval)) {
		if (gettimeofday((struct timeval *)payload, NULL) == -1) {
			print_error("gettimeofday", ft_strerror(errno));
			free(icmp_request);
			return NULL;
		}
		payload_pad_bytes_begin_index += sizeof(struct timeval);
	}
	for (size_t i = payload_pad_bytes_begin_index; i < payload_size; i++) {
		payload[i] = (uint8_t)i;
	}

	icmphdr->checksum = get_checksum(icmphdr, ICMP_PACKET_SIZE(payload_size));
	return icmp_request;
}

int update_icmp_request(void)
{
	struct icmphdr *icmphdr = (struct icmphdr *)g_ping.icmp_request;
	assert(icmphdr->type == ICMP_ECHO);
	assert(icmphdr->code == 0);
	icmphdr->un.echo.sequence = ft_htons(ft_ntohs(icmphdr->un.echo.sequence) + 1);

	uint8_t *payload = (uint8_t *)(icmphdr + 1);
	if (g_ping.icmp_payload_size >= sizeof(struct timeval)) {
		if (gettimeofday((struct timeval *)payload, NULL) == -1) {
			print_error("gettimeofday", ft_strerror(errno));
			return -1;
		}
	}

	icmphdr->checksum = 0;
	icmphdr->checksum = get_checksum(icmphdr, ICMP_PACKET_SIZE(g_ping.icmp_payload_size));
	return 0;
}

int send_icmp_request(void)
{
	if (sendto(g_ping.socket_fd, g_ping.icmp_request,
			ICMP_PACKET_SIZE(g_ping.icmp_payload_size), 0,
			(const struct sockaddr *)&g_ping.destination.sockaddr_in,
			sizeof(struct sockaddr_in)) == -1) {
		print_error("sendto", ft_strerror(errno));
		return -1;
	}
	return 0;
}

bool is_icmphdr_valid(struct icmphdr *icmphdr, size_t size,
		uint8_t expected_type, uint16_t expected_id)
{
	if (icmphdr->type != expected_type) {
		return false;
	}
	if (icmphdr->code != 0) {
		return false;
	}
	if (is_checksum_valid(icmphdr, size, &icmphdr->checksum) != true) {
		return false;
	}
	if (ft_ntohs(icmphdr->un.echo.id) != expected_id) {
		return false;
	}
	return true;
}
