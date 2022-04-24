#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/ip_icmp.h>

#include "ft_ping/utils/print.h"
#include "ft_ping/initialize.h"

int main(int argc, char *argv[])
{
	if (argc == 1) {
		print_error(argv[0], "usage error", "Destination address required");
		return 1;
	}
	int socket_fd;
	if (initialize_socket(&socket_fd, argv[0]) == -1) {
		return 2;
	}
	struct sockaddr_in sockaddr_in;
	if (initialize_sockaddr_in(&sockaddr_in, argv[1], argv[0]) == -1) {
		close(socket_fd);
		return 2;
	}
	size_t packet_size = 56;
	uint8_t *icmp_echo;
	if (initialize_icmp_echo(&icmp_echo, packet_size, argv[0]) == -1) {
		close(socket_fd);
		return 2;
	}

	if (sendto(socket_fd, icmp_echo, sizeof(struct icmphdr) + packet_size, 0,
			(struct sockaddr *)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
		print_error(argv[0], "sendto", strerror(errno));
		free(icmp_echo);
		close(socket_fd);
		return 2;
	}
	printf("ICMP ECHO REQUEST sent\n");

	free(icmp_echo);
	close(socket_fd);
	return 0;
}
