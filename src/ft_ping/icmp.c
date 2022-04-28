#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <netinet/ip_icmp.h>

#include "ft_ping/utils/print.h"
#include "ft_ping/checksum.h"

uint8_t *create_icmp_request(uint16_t id, uint16_t sequence, size_t payload_size)
{
	uint8_t *icmp_request = malloc(sizeof(struct icmphdr) + payload_size);
	if (icmp_request == NULL) {
		print_error("malloc", strerror(errno));
		return NULL;
	}
	struct icmphdr *icmphdr = (struct icmphdr *)icmp_request;
	icmphdr->type = ICMP_ECHO;
	icmphdr->code = 0;
	icmphdr->checksum = 0;
	icmphdr->un.echo.id = id;
	icmphdr->un.echo.sequence = sequence;

	uint8_t *payload = (uint8_t *)(icmphdr + 1);
	int payload_pad_bytes_begin_index = 0;
	if (payload_size >= sizeof(struct timeval)) {
		if (gettimeofday((struct timeval *)(payload), NULL) == -1) {
			print_error("gettimeofday", strerror(errno));
			free(icmp_request);
			return NULL;
		}
		payload_pad_bytes_begin_index += sizeof(struct timeval);
	}
	for (size_t i = payload_pad_bytes_begin_index; i < payload_size; i++) {
		payload[i] = (uint8_t)i;
	}

	icmphdr->checksum = get_checksum(icmphdr, sizeof(struct icmphdr) + payload_size);
	return icmp_request;
}
