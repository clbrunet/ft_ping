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

int initialize_icmp_echo(uint8_t **icmp_echo_ptr, size_t packet_size,
		uint16_t id, uint16_t sequence, const char *executable)
{
	assert(icmp_echo_ptr != NULL);
	assert(executable != NULL);

	*icmp_echo_ptr = malloc(sizeof(struct icmphdr) + packet_size);
	if (*icmp_echo_ptr == NULL) {
		print_error(executable, "malloc", strerror(errno));
		return 2;
	}
	struct icmphdr *icmp_header = (struct icmphdr *)*icmp_echo_ptr;
	icmp_header->type = ICMP_ECHO;
	icmp_header->code = 0;
	icmp_header->un.echo.id = id;
	icmp_header->un.echo.sequence = sequence;

	uint8_t *packet = (uint8_t *)(icmp_header + 1);
	int packet_pad_bytes_begin_index = 0;
	if (packet_size >= sizeof(struct timeval)) {
		if (gettimeofday((struct timeval *)(packet), NULL) == -1) {
			print_error(executable, "gettimeofday", strerror(errno));
			free(*icmp_echo_ptr);
			return 2;
		}
		packet_pad_bytes_begin_index += sizeof(struct timeval);
	}
	for (size_t i = packet_pad_bytes_begin_index; i < packet_size; i++) {
		packet[i] = (uint8_t)i;
	}

	icmp_header->checksum = get_checksum(*icmp_echo_ptr, sizeof(struct icmphdr) + packet_size);
	return 0;
}
