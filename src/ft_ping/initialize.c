#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
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

static void usage(void)
{
	printf(
			"\n"
			"Usage\n"
			"  ft_ping [options] <destination>\n"
			"\n"
			"Options:\n"
			"  <destination>   dns name or ip adress\n"
			"  -h              print help and exit\n"
			"  -v              verbose output\n"
		  );
	exit(2);
}

static void invalid_option(char c)
{
	printf("ft_ping: invalid option -- '%c'\n", c);
	usage();
}

static int parse_args(const char *const args[])
{
	bool is_destination_initialized = false;
	while (*args != NULL) {
		const char *arg = *args;
		if (arg[0] == '-' && arg[1] != '\0') {
			arg++;
			while (*arg != '\0') {
				switch (*arg) {
					case 'h':
						usage();
						break;
					case 'v':
						g_ping.is_verbose = true;
						break;
					default:
						invalid_option(*arg);
				}
				arg++;
			}
		} else {
			if (is_destination_initialized) {
				print_error("usage error", "Too many destination addresses");
				return -1;
			}
			if (initialize_destination(&g_ping.destination, arg) == -1) {
				return -1;
			}
			is_destination_initialized = true;
		}
		args++;
	}
	if (is_destination_initialized == false) {
		print_error("usage error", ft_strerror(EDESTADDRREQ));
		exit(1);
	}
	return 0;
}

int initialize(const char *const argv[])
{
	assert(argv != NULL);

	g_ping.program_name = argv[0];
	g_ping.icmp_request_payload_size = 56;
	g_ping.is_verbose = false;
	if (parse_args(argv + 1) == -1) {
		return -1;
	}

	if (initialize_socket(&g_ping.socket_fd) == -1) {
		return -1;
	}
	g_ping.icmp_request_id = getpid() & UINT16_MAX;
	g_ping.icmp_request = create_icmp_request(g_ping.icmp_request_id, 0,
			g_ping.icmp_request_payload_size);
	if (g_ping.icmp_request == NULL) {
		close(g_ping.socket_fd);
		return -1;
	}
	g_ping.icmp_reply_buf_size
		= IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_ping.icmp_request_payload_size)) + 1;
	g_ping.icmp_reply_buf = malloc(g_ping.icmp_reply_buf_size);
	if (g_ping.icmp_reply_buf == NULL) {
		free(g_ping.icmp_request);
		close(g_ping.socket_fd);
		return -1;
	}

	g_ping.transmitted_packets_count = 0;
	g_ping.received_packets_count = 0;
	g_ping.ms_from_first_sending_time = 0;
	g_ping.min_rtt = DBL_MAX;
	g_ping.rtt_sum = 0.0;
	g_ping.max_rtt = 0.0;
	g_ping.squared_rtt_sum = 0.0;
	g_ping.ewma = 0.0;
	return 0;
}
