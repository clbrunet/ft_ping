#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <signal.h>

#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/initialize.h"
#include "ft_ping/icmp.h"
#include "ft_ping/terminate.h"

variables_t g_vars = {0};

static void interrupt_handler(int signum)
{
	(void)signum;

	write(STDOUT_FILENO, "\nEND\n", 5);

	terminate();
	exit(0);
}

static void alarm_handler(int signum)
{
	(void)signum;

	update_icmp_request();
	if (send_icmp_request() == -1) {
		terminate();
		exit(2);
	}
	alarm(1);
}

static int recv_loop(void)
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
			print_error("recvmsg", strerror(errno));
			return -1;
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
		printf("UTC time : %ld:%ld.%ld\n", hour, min, sec);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 1) {
		fprintf(stderr, "argv must be filled\n");
		return 1;
	}
	g_vars.program_name = argv[0];

	if (argc < 2) {
		print_error("usage error", "Destination address required");
		return 1;
	}

	if (initialize(argv[1]) == -1) {
		return 2;
	}

	signal(SIGINT, &interrupt_handler);
	signal(SIGALRM, &alarm_handler);

	alarm_handler(SIGALRM);

	if (recv_loop() == -1) {
		terminate();
		return 2;
	}
	return 0;
}
