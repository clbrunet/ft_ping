#ifndef FT_PING_MAIN_H
#define FT_PING_MAIN_H

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>

typedef struct variables_s
{
	const char *program_name;
	int socket_fd;
	struct sockaddr_in destination_sockaddr_in;
	size_t icmp_request_payload_size;
	size_t icmp_request_id;
	uint8_t *icmp_request;
} variables_t;

extern variables_t g_vars;

#endif
