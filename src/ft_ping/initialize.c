#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ft_ping/initialize.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/icmp.h"

static int initialize_socket(int *socket_fd)
{
	assert(socket_fd != NULL);

	*socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (*socket_fd == -1) {
		print_error("socket", ft_strerror(errno));
		return -1;
	}
	return 0;
}

static int initialize_sockaddr_in(struct sockaddr_in *sockaddr_in, const char *destination)
{
	assert(sockaddr_in != NULL);
	assert(destination != NULL);

	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP,
	};
	struct addrinfo *res;
	int error_code = getaddrinfo(destination, NULL, &hints, &res);
	if (error_code != 0) {
		print_error("getaddrinfo", ft_gai_strerror(error_code));
		return -1;
    }
	*sockaddr_in = *(struct sockaddr_in *)res->ai_addr;
	freeaddrinfo(res);
	return 0;
}

int initialize(const char *destination)
{
	assert(destination != NULL);

	if (initialize_socket(&g_vars.socket_fd) == -1) {
		return -1;
	}
	if (initialize_sockaddr_in(&g_vars.destination_sockaddr_in, destination) == -1) {
		close(g_vars.socket_fd);
		return -1;
	}
	g_vars.icmp_request_id = getpid() & UINT16_MAX;
	g_vars.icmp_request_payload_size = 56;
	g_vars.icmp_request = create_icmp_request(g_vars.icmp_request_id, 0,
			g_vars.icmp_request_payload_size);
	if (g_vars.icmp_request == NULL) {
		close(g_vars.socket_fd);
		return -1;
	}
	g_vars.icmp_reply_buf_size = sizeof(struct icmphdr) + sizeof(struct icmphdr)
		+ g_vars.icmp_request_payload_size + 1;
	g_vars.icmp_reply_buf = malloc(g_vars.icmp_reply_buf_size);
	if (g_vars.icmp_reply_buf == NULL) {
		free(g_vars.icmp_request);
		close(g_vars.socket_fd);
		return -1;
	}
	return 0;
}
