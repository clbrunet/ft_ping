#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdint.h>
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
#include <linux/filter.h>
#include <limits.h>

#include "ft_ping/initialize.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/ctype.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/icmp.h"
#include "ft_ping/utils/string.h"
#include "ft_ping/parsing.h"

static int initialize_socket()
{
	g_ping.socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_ping.socket_fd == -1) {
		print_error("socket", ft_strerror(errno));
		return -1;
	}
	int recverr = 1;
	if (setsockopt(g_ping.socket_fd, SOL_IP, IP_RECVERR, &recverr, sizeof(int)) == -1) {
		print_error("setsockopt", ft_strerror(errno));
		return -1;
	}
	if (g_ping.is_ttl_specified) {
		if (setsockopt(g_ping.socket_fd, SOL_IP, IP_TTL, &g_ping.ttl, sizeof(int)) == -1) {
			print_error("cannot set unicast time-to-live", ft_strerror(errno));
			return -1;
		}
	}
	struct icmp_filter filter = { .data = ~(1 << ICMP_ECHOREPLY) };
	if (setsockopt(g_ping.socket_fd, SOL_RAW, ICMP_FILTER, &filter, sizeof(struct icmp_filter)) == -1) {
		print_error("setsockopt", ft_strerror(errno));
		return -1;
	}
	return 0;
}

int initialize(const char *const argv[])
{
	assert(argv != NULL);

	g_ping.program_name = argv[0];
	if (parse_args(argv + 1) == -1) {
		return -1;
	}
	if (initialize_socket() == -1) {
		return -1;
	}
	g_ping.icmp_request_id = getpid() & UINT16_MAX;
	g_ping.icmp_request = create_icmp_request(g_ping.icmp_request_id, 0,
			g_ping.icmp_payload_size);
	if (g_ping.icmp_request == NULL) {
		close(g_ping.socket_fd);
		return -1;
	}
	g_ping.icmp_reply_buf_size
		= IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_ping.icmp_payload_size)) + 1;
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
