#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <stdio.h>

#include "ft_ping/recv_loop.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/utils/inet.h"
#include "ft_ping/icmp.h"
#include "ft_ping/ip.h"

int recv_loop(void)
{
	struct iovec msg_iov[1] = {
		[0].iov_base = g_vars.icmp_reply_buf,
		[0].iov_len = g_vars.icmp_reply_buf_size,
	};
	struct msghdr msg = {
		.msg_iov = msg_iov,
		.msg_iovlen = 1,
	};
	while (true) {
		ssize_t ret = recvmsg(g_vars.socket_fd, &msg, 0);
		if (ret == -1) {
			print_error("recvmsg", ft_strerror(errno));
			return -1;
		}
		struct timeval current_tv;
		if (g_vars.icmp_request_payload_size >= sizeof(struct timeval)) {
			if (gettimeofday(&current_tv, NULL) == -1) {
				print_error("gettimeofday", ft_strerror(errno));
				return -1;
			}
		}
		if ((size_t)ret != IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_vars.icmp_request_payload_size))) {
			continue;
		}
		struct iphdr *response_iphdr = (struct iphdr *)g_vars.icmp_reply_buf;
		if (is_iphdr_valid(response_iphdr,
				IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_vars.icmp_request_payload_size))) == false) {
			continue;
		}
		struct icmphdr *response_icmphdr = (struct icmphdr *)(response_iphdr + 1);
		if (is_icmphdr_valid(response_icmphdr, g_vars.icmp_request_payload_size,
				ICMP_ECHOREPLY, g_vars.icmp_request_id) == false) {
			continue;
		}
		printf("%lu bytes from ", ret - sizeof(struct iphdr));
		if (has_ip_format(g_vars.destination.name)) {
			printf("%s: ", g_vars.destination.ip);
		} else {
			printf("%s (%s): ", g_vars.destination.name, g_vars.destination.ip);
		}
		printf("icmp_seq=%hu ttl=%hhu", ft_ntohs(response_icmphdr->un.echo.sequence),
				response_iphdr->ttl);
		if (g_vars.icmp_request_payload_size >= sizeof(struct timeval)) {
			struct timeval *sending_tv = (struct timeval *)(response_icmphdr + 1);
			struct timeval diff_tv = {
				.tv_sec = current_tv.tv_sec - sending_tv->tv_sec,
				.tv_usec = current_tv.tv_usec - sending_tv->tv_usec,
			};
			double ms = diff_tv.tv_sec * 1000 + (double)diff_tv.tv_usec / 1000;
			int ms_precision;
			if (ms < 1) {
				ms_precision = 3;
			} else if (ms < 10) {
				ms_precision = 2;
			} else if (ms < 100) {
				ms_precision = 1;
			} else {
				ms_precision = 0;
			}
			printf(" time=%.*f", ms_precision, ms);
		}
		printf("\n");
	}
}
