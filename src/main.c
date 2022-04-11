#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main (int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "ft_ping: usage error: Destination address required\n");
		return 1;
	}

	int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (socket_fd == -1) {
		perror("socket");
		return 2;
	}

	struct sockaddr_in sockaddr_in = {0};
	sockaddr_in.sin_family = AF_INET;
	switch (inet_pton(AF_INET, argv[1], &sockaddr_in.sin_addr)) {
		case 0:
			// TODO: inet_pton src problem
			fprintf(stderr, "inet_pton src problem\n");
			close(socket_fd);
			return 2;
		case -1:
			perror("inet_pton");
			close(socket_fd);
			return 2;
	}

	unsigned char buf[sizeof(struct icmphdr) + 56] = {0};
	struct icmphdr *icmp_header = (struct icmphdr *)buf;
	icmp_header->type = ICMP_ECHO;
	icmp_header->code = 0;
	// TODO: icmp_header->checksum
	// TODO: data

	if (sendto(socket_fd, buf, sizeof(buf), 0,
			(struct sockaddr *)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror("sendto");
		close(socket_fd);
		return 2;
	}
	printf("ICMP ECHO REQUEST sent\n");

	close(socket_fd);
	return 0;
}
