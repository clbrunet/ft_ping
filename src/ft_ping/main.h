#ifndef FT_PING_MAIN_H
#define FT_PING_MAIN_H

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct
{
	struct sockaddr_in sockaddr_in;
	char name[NI_MAXHOST];
	char ip[INET_ADDRSTRLEN];
} destination_t;

typedef struct
{
	const char *program_name;
	int socket_fd;
	destination_t destination;
	size_t icmp_request_payload_size;
	uint16_t icmp_request_id;
	uint8_t *icmp_request;
	size_t icmp_reply_buf_size;
	uint8_t *icmp_reply_buf;
} variables_t;

extern variables_t g_vars;

#endif
