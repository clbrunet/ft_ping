#ifndef FT_PING_INITIALIZE_H
#define FT_PING_INITIALIZE_H

#include <stdint.h>
#include <netinet/in.h>

int initialize_socket(int *socket_fd, const char *executable);
int initialize_sockaddr_in(struct sockaddr_in *sockaddr_in, const char *destination,
		const char *executable);
int initialize_icmp_echo(uint8_t **icmp_echo_ptr, size_t packet_size,
		uint16_t id, uint16_t sequence, const char *executable);

#endif
