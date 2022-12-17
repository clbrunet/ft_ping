#include <linux/icmp.h>
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
#include "ft_ping/utils/math.h"

ping_t g_ping = {0};

static void interrupt_handler(int signum)
{
	(void)signum;
	int exit_status = 0;

	print_string("\n--- ");
	print_string(g_ping.destination.name);
	print_string(" ping statistics ---\n");

	print_number(g_ping.transmitted_packets_count);
	print_string(" packets transmitted, ");
	print_number(g_ping.received_packets_count);
	print_string(" received, ");
	if (g_ping.error_packets_count != 0) {
		print_char('+');
		print_number(g_ping.error_packets_count);
		print_string(" errors, ");
	}
	double packet_loss_percent = 100;
	if (g_ping.transmitted_packets_count != 0) {
		packet_loss_percent = 100 - 100 * ((double)g_ping.received_packets_count
				/ (double)g_ping.transmitted_packets_count);
	}
	print_number(packet_loss_percent);
	print_string("% packet loss, time ");
	print_number(g_ping.ms_from_first_sending_time);
	print_string("ms\n");
	if (g_ping.received_packets_count == 0) {
		print_char('\n');
		exit_status = 1;
	} else {
		print_string("rtt min/avg/max/mdev = ");
		print_double(g_ping.min_rtt, 3);
		print_char('/');
		double avg = g_ping.rtt_sum / g_ping.received_packets_count;
		print_double(avg, 3);
		print_char('/');
		print_double(g_ping.max_rtt, 3);
		print_char('/');
		// Standard deviation formula : https://www.brainkart.com/article/Calculation-of-Standard-Deviation_39437/
		double mdev = ft_sqrt(g_ping.squared_rtt_sum / g_ping.received_packets_count - (avg * avg));
		print_double(mdev, 3);
		print_string(" ms\n");
	}
	terminate();
	exit(exit_status);
}

static void quit_handler(int signum)
{
	(void)signum;

	print_string("\b\b");
	print_number(g_ping.received_packets_count);
	print_char('/');
	print_number(g_ping.transmitted_packets_count);
	print_string(" packets, ");
	double packet_loss_percent = 100;
	if (g_ping.transmitted_packets_count != 0) {
		packet_loss_percent = 100 - 100 * ((double)g_ping.received_packets_count
				/ (double)g_ping.transmitted_packets_count);
	}
	print_number(packet_loss_percent);
	print_string("% loss");
	if (g_ping.received_packets_count == 0) {
		print_char('\n');
		return;
	}
	print_string(", min/avg/ewma/max = ");
	print_double(g_ping.min_rtt, 3);
	print_char('/');
	double avg = g_ping.rtt_sum / g_ping.received_packets_count;
	print_double(avg, 3);
	print_char('/');
	print_double(g_ping.ewma / EWMA_FACTOR, 3);
	print_char('/');
	print_double(g_ping.max_rtt, 3);
	print_string(" ms\n");
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
	if (ft_ntohs(((struct icmphdr *)g_ping.icmp_request)->un.echo.sequence) == 1) {
		g_ping.first_sending_time = current_tv;
	} else {
		struct timeval diff_from_first_reception_tv = {
			.tv_sec = current_tv.tv_sec - g_ping.first_sending_time.tv_sec,
			.tv_usec = current_tv.tv_usec - g_ping.first_sending_time.tv_usec,
		};
		g_ping.ms_from_first_sending_time = diff_from_first_reception_tv.tv_sec * 1000
			+ (double)diff_from_first_reception_tv.tv_usec / 1000;
	}
	g_ping.transmitted_packets_count++;
}

int main(int argc, char *argv[])
{
	(void)argc;
	if (initialize((const char *const *)argv) == -1) {
		return 2;
	}

	signal(SIGINT, &interrupt_handler);
	signal(SIGQUIT, &quit_handler);
	signal(SIGALRM, &alarm_handler);

	printf("PING %s (%s) %zu(%zu) bytes of data.\n", g_ping.destination.name,
			g_ping.destination.ip, g_ping.icmp_payload_size,
			IPV4_PACKET_SIZE(ICMP_PACKET_SIZE(g_ping.icmp_payload_size)));

	alarm_handler(SIGALRM);

	if (recv_loop() == -1) {
		terminate();
		return 2;
	}
}
