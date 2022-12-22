#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <linux/icmp.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdio.h>
#include <linux/errqueue.h>
#include <arpa/inet.h>

#include "ft_ping/recv_loop.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/utils/inet.h"
#include "ft_ping/icmp.h"
#include "ft_ping/ip.h"

int recv_error(void)
{
	struct sockaddr_in name;
	uint8_t buf[32768];
	struct iovec msg_iov[1] = {
		[0] = {
			.iov_base = buf,
			.iov_len = sizeof(buf),
		},
	};
	uint8_t control_buf[32768];
	struct msghdr msg = {
		.msg_name = &name,
		.msg_namelen = sizeof(struct sockaddr_in),
		.msg_iov = msg_iov,
		.msg_iovlen = 1,
		.msg_control = control_buf,
		.msg_controllen = sizeof(control_buf),
	};
	ssize_t ret = recvmsg(g_ping.socket_fd, &msg, MSG_ERRQUEUE);
	if (ret == -1) {
		print_error("recvmsg", ft_strerror(errno));
		return -1;
	}
	g_ping.error_packets_count++;
	if (!g_ping.is_verbose) {
		return 0;
	}
	struct cmsghdr *cmsg;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level != SOL_IP || cmsg->cmsg_type != IP_RECVERR) {
			continue;
		}
		struct sock_extended_err *ee = (struct sock_extended_err *)CMSG_DATA(cmsg);
		if (ee->ee_origin == SO_EE_ORIGIN_ICMP) {
			char ip[INET_ADDRSTRLEN] = {0};
			struct sockaddr_in *addr_in = (struct sockaddr_in *)SO_EE_OFFENDER(ee);
			if (addr_in->sin_family == AF_INET) {
				inet_ntop(AF_INET, (const void *)&addr_in->sin_addr, ip, INET_ADDRSTRLEN);
			} else {
				inet_ntop(AF_INET, (const void *)&name.sin_addr, ip, INET_ADDRSTRLEN);
			}
			struct icmphdr *icmphdr = (struct icmphdr *)msg_iov->iov_base;
			printf("From %s: icmp_seq=%u type=%u code=%u\n", ip,
					ft_ntohs(icmphdr->un.echo.sequence), ee->ee_type, ee->ee_code);
			break;
		}
	}
	return 0;
}

int recv_icmp_reply(void)
{
	struct sockaddr_in name;
	struct iovec msg_iov[1] = {
		[0] = {
			.iov_base = g_ping.icmp_reply_buf,
			.iov_len = g_ping.icmp_reply_buf_size,
		},
	};
	struct msghdr msg = {
		.msg_name = &name,
		.msg_namelen = sizeof(struct sockaddr_in),
		.msg_iov = msg_iov,
		.msg_iovlen = 1,
	};
	ssize_t ret = recvmsg(g_ping.socket_fd, &msg, 0);
	if (ret == -1) {
		if (recv_error() == -1) {
			return -1;
		}
		return 0;
	}
	struct timeval current_tv;
	if (gettimeofday(&current_tv, NULL) == -1) {
		print_error("gettimeofday", ft_strerror(errno));
		return -1;
	}
	if ((size_t)ret != IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_ping.icmp_payload_size))) {
		return 0;
	}
	struct iphdr *response_iphdr = (struct iphdr *)g_ping.icmp_reply_buf;
	if (is_iphdr_valid(response_iphdr, ret) == false) {
		return 0;
	}
	struct icmphdr *response_icmphdr = (struct icmphdr *)(response_iphdr + 1);
	if (is_icmphdr_valid(response_icmphdr, ICMP_PACKET_SIZE(g_ping.icmp_payload_size),
			ICMP_ECHOREPLY, g_ping.icmp_request_id) == false) {
		return 0;
	}
	g_ping.received_packets_count++;
	char timestamp_prefix[30] = {0};
	if (g_ping.should_print_timestamp) {
		snprintf(timestamp_prefix, sizeof(timestamp_prefix), "[%ld.%06ld] ",
				current_tv.tv_sec, current_tv.tv_usec);
	}
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, (const void *)&name.sin_addr, ip, INET_ADDRSTRLEN);
	char time_suffix[30] = {0};
	if (g_ping.icmp_payload_size >= sizeof(struct timeval)) {
		struct timeval *sending_tv = (struct timeval *)(response_icmphdr + 1);
		struct timeval diff_tv = {
			.tv_sec = current_tv.tv_sec - sending_tv->tv_sec,
			.tv_usec = current_tv.tv_usec - sending_tv->tv_usec,
		};
		double ms = diff_tv.tv_sec * 1000 + (double)diff_tv.tv_usec / 1000;
		if (ms < g_ping.min_rtt) {
			g_ping.min_rtt = ms;
		}
		g_ping.rtt_sum += ms;
		if (g_ping.max_rtt < ms) {
			g_ping.max_rtt = ms;
		}
		g_ping.squared_rtt_sum += ms * ms;
		if (g_ping.ewma == 0.0) {
			g_ping.ewma = ms * EWMA_FACTOR;
		} else {
			g_ping.ewma += ms - g_ping.ewma / EWMA_FACTOR;
		}
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
		snprintf(time_suffix, sizeof(time_suffix), " time=%.*f ms", ms_precision, ms);
	}
	printf("%s%lu bytes from %s: icmp_seq=%hu ttl=%hhu%s\n", timestamp_prefix,
			ret - sizeof(struct iphdr), ip, ft_ntohs(response_icmphdr->un.echo.sequence),
			response_iphdr->ttl, time_suffix);
	return 0;
}

int recv_loop(void)
{
	while (true) {
		recv_icmp_reply();
	}
}
