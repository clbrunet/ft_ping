#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "ft_ping/main.h"
#include "ft_ping/utils/inet.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/initialize.h"
#include "ft_ping/icmp.h"
#include "ft_ping/terminate.h"
#include "ft_ping/recv_loop.h"
#include "ft_ping/utils/string.h"

variables_t g_vars = {0};

static void interrupt_handler(int signum)
{
	(void)signum;
	int exit_status = 0;

	print_string("\n--- ");
	print_string(g_vars.destination.name);
	print_string(" ping statistics ---\n");

	print_number(g_vars.transmitted_packets_count);
	print_string(" packets transmitted, ");
	print_number(g_vars.received_packets_count);
	print_string(" received, ");
	double packet_loss_percent = 100;
	if (g_vars.transmitted_packets_count != 0) {
		packet_loss_percent = 100 - 100 * ((double)g_vars.received_packets_count
				/ (double)g_vars.transmitted_packets_count);
	}
	print_number(packet_loss_percent);
	print_string("% packet loss, time ");
	print_number(g_vars.ms_from_first_sending_time);
	print_string("ms\n");
	if (g_vars.received_packets_count == 0) {
		print_char('\n');
		exit_status = 1;
	} else {
		print_string("rtt min/agv/max/mdev = ");
		// TODO: print correct values
		print_string("VALUES");
		print_string(" ms\n");
	}
	terminate();
	exit(exit_status);
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

	struct timeval current_tv;
	if (gettimeofday(&current_tv, NULL) == -1) {
		print_error("gettimeofday", ft_strerror(errno));
		terminate();
		exit(2);
	}
	if (ft_ntohs(((struct icmphdr *)g_vars.icmp_request)->un.echo.sequence) == 1) {
		g_vars.first_sending_time = current_tv;
	} else {
		struct timeval diff_from_first_recetion_tv = {
			.tv_sec = current_tv.tv_sec - g_vars.first_sending_time.tv_sec,
			.tv_usec = current_tv.tv_usec - g_vars.first_sending_time.tv_usec,
		};
		g_vars.ms_from_first_sending_time = diff_from_first_recetion_tv.tv_sec * 1000
			+ (double)diff_from_first_recetion_tv.tv_usec / 1000;
	}
	g_vars.transmitted_packets_count++;
}

int main(int argc, char *argv[])
{
	g_vars.program_name = argv[0];
	if (argc < 2) {
		print_error("usage error", ft_strerror(EDESTADDRREQ));
		return 1;
	}

	if (initialize(argv[1]) == -1) {
		return 2;
	}

	signal(SIGINT, &interrupt_handler);
	signal(SIGALRM, &alarm_handler);

	printf("PING %s (%s) %zu(%zu) bytes of data.\n", g_vars.destination.name,
			g_vars.destination.ip, g_vars.icmp_request_payload_size,
			IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_vars.icmp_request_payload_size)));

	alarm_handler(SIGALRM);

	if (recv_loop() == -1) {
		terminate();
		return 2;
	}
}
