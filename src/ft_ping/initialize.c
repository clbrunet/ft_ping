#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include "ft_ping/initialize.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/checksum.h"

int initialize_socket(int *socket_fd, const char *executable)
{
	assert(socket_fd != NULL);
	assert(executable != NULL);

	*socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (*socket_fd == -1) {
		print_error(executable, "socket", strerror(errno));
		return -1;
	}
	return 0;
}

int initialize_sockaddr_in(struct sockaddr_in *sockaddr_in, const char *destination,
		const char *executable)
{
	assert(sockaddr_in != NULL);
	assert(destination != NULL);
	assert(executable != NULL);

	*sockaddr_in = (struct sockaddr_in){0};
	sockaddr_in->sin_family = AF_INET;
	switch (inet_pton(AF_INET, destination, &sockaddr_in->sin_addr)) {
		case 0:
			// TODO: inet_pton src problem
			fprintf(stderr, "inet_pton src problem\n");
			return -1;
		case -1:
			print_error(executable, "inet_pton", strerror(errno));
			return -1;
	}
	return 0;
}

int initialize_icmp_echo_packet(uint8_t *icmp_echo_packet, size_t payload_size,
		uint16_t id, uint16_t sequence, const char *executable)
{
	assert(icmp_echo_packet != NULL);
	assert(executable != NULL);

	struct icmphdr *icmphdr = (struct icmphdr *)icmp_echo_packet;
	icmphdr->type = ICMP_ECHO;
	icmphdr->code = 0;
	icmphdr->checksum = 0;
	icmphdr->un.echo.id = id;
	icmphdr->un.echo.sequence = sequence;

	uint8_t *payload = (uint8_t *)(icmphdr + 1);
	int payload_pad_bytes_begin_index = 0;
	if (payload_size >= sizeof(struct timeval)) {
		if (gettimeofday((struct timeval *)(payload), NULL) == -1) {
			print_error(executable, "gettimeofday", strerror(errno));
			return 2;
		}
		payload_pad_bytes_begin_index += sizeof(struct timeval);
	}
	for (size_t i = payload_pad_bytes_begin_index; i < payload_size; i++) {
		payload[i] = (uint8_t)i;
	}

	icmphdr->checksum = get_checksum(icmphdr, sizeof(struct icmphdr) + payload_size);
	return 0;
}
