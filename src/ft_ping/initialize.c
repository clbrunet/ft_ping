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
#include <float.h>

#include "ft_ping/initialize.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/icmp.h"
#include "ft_ping/utils/string.h"

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

static int initialize_destination(destination_t *destination, const char *destination_arg)
{
	assert(destination != NULL);
	assert(destination_arg != NULL);

	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP,
		.ai_flags = AI_CANONNAME,
	};
	struct addrinfo *res;
	int error_code = getaddrinfo(destination_arg, NULL, &hints, &res);
	if (error_code != 0) {
		print_error(destination_arg, ft_gai_strerror(error_code));
		return -1;
    }
	destination->sockaddr_in = *(struct sockaddr_in *)res->ai_addr;
	ft_strlcpy(destination->name, res->ai_canonname, NI_MAXHOST);
	inet_ntop(AF_INET, (const void *)&destination->sockaddr_in.sin_addr,
			destination->ip, INET_ADDRSTRLEN);
	freeaddrinfo(res);
	return 0;
}

int initialize(const char *destination_arg)
{
	assert(destination_arg != NULL);

	if (initialize_socket(&g_vars.socket_fd) == -1) {
		return -1;
	}
	if (initialize_destination(&g_vars.destination, destination_arg) == -1) {
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
	g_vars.icmp_reply_buf_size
		= IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_vars.icmp_request_payload_size)) + 1;
	g_vars.icmp_reply_buf = malloc(g_vars.icmp_reply_buf_size);
	if (g_vars.icmp_reply_buf == NULL) {
		free(g_vars.icmp_request);
		close(g_vars.socket_fd);
		return -1;
	}
	g_vars.transmitted_packets_count = 0;
	g_vars.received_packets_count = 0;
	g_vars.ms_from_first_sending_time = 0;
	g_vars.min_rtt = DBL_MAX;
	g_vars.rtt_sum = 0.0;
	g_vars.max_rtt = 0.0;
	g_vars.squared_rtt_sum = 0.0;
	return 0;
}
