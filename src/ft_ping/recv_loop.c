#include <errno.h>
#include <stdio.h>

#include "ft_ping/recv_loop.h"
#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/utils/error.h"
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
		if ((size_t)ret != IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_vars.icmp_request_payload_size))) {
			continue;
		}
		if (is_iphdr_valid((struct iphdr *)g_vars.icmp_reply_buf,
				IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_vars.icmp_request_payload_size))) == false) {
			continue;
		}
		printf("\nMessage received\n");
		print_memory(g_vars.icmp_reply_buf, ret);
		struct iphdr *response_iphdr = (void *)g_vars.icmp_reply_buf;
		printf("IP header\n");
		printf("saddr %u\n", response_iphdr->saddr);
		printf("daddr %u\n", response_iphdr->daddr);
		struct icmphdr *response_icmphdr = (void *)g_vars.icmp_reply_buf + 20;
		printf("ICMP header\n");
		printf("type %hhu\n", response_icmphdr->type);
		printf("code %hhu\n", response_icmphdr->code);
		printf("checksum %hu\n", response_icmphdr->checksum);
		printf("id %hu or %hu\n", response_icmphdr->un.echo.id, ntohs(response_icmphdr->un.echo.id));
		printf("seq %hu or %hu\n", response_icmphdr->un.echo.sequence, ntohs(response_icmphdr->un.echo.sequence));
		struct timeval *tm = (struct timeval *)(response_icmphdr + 1);
		long sec = tm->tv_sec % 60;
		long min = tm->tv_sec / 60 % 60;
		long hour = tm->tv_sec / 60 / 60 % 24;
		printf("UTC time : %02ld:%02ld.%02ld\n", hour, min, sec);
	}
}
