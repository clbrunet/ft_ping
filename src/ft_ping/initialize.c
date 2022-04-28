#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "ft_ping/initialize.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/icmp.h"

static int initialize_socket(int *socket_fd)
{
	assert(socket_fd != NULL);

	*socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (*socket_fd == -1) {
		print_error("socket", strerror(errno));
		return -1;
	}
	return 0;
}

static int initialize_sockaddr_in(struct sockaddr_in *sockaddr_in, const char *destination)
{
	assert(sockaddr_in != NULL);
	assert(destination != NULL);

	*sockaddr_in = (struct sockaddr_in){0};
	sockaddr_in->sin_family = AF_INET;
	switch (inet_pton(AF_INET, destination, &sockaddr_in->sin_addr)) {
		case 0:
			// TODO: inet_pton src problem
			fprintf(stderr, "inet_pton src problem\n");
			return -1;
		case -1:
			print_error("inet_pton", strerror(errno));
			return -1;
	}
	return 0;
}

int initialize(const char *destination)
{
	assert(destination != NULL);

	if (initialize_socket(&g_vars.socket_fd) == -1) {
		return 2;
	}
	if (initialize_sockaddr_in(&g_vars.destination_sockaddr_in, destination) == -1) {
		close(g_vars.socket_fd);
		return 2;
	}
	g_vars.icmp_request_id = getpid() & UINT16_MAX;
	g_vars.icmp_request_payload_size = 56;
	g_vars.icmp_request = create_icmp_request(g_vars.icmp_request_id, 1,
			g_vars.icmp_request_payload_size);
	if (g_vars.icmp_request == NULL) {
		close(g_vars.socket_fd);
		return 2;
	}
	return 0;
}
