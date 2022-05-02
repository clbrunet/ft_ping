#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/utils/error.h"
#include "ft_ping/initialize.h"
#include "ft_ping/icmp.h"
#include "ft_ping/terminate.h"
#include "ft_ping/recv_loop.h"

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

int main(int argc, char *argv[])
{
	if (argc < 1) {
		fprintf(stderr, "argv must be filled\n");
		return 1;
	}
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
	return 0;
}
