#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void print_error(const char *executable, const char *error_title, const char *error_description)
{
	fprintf(stderr, "%s: %s: %s\n", executable, error_title, error_description);
}

static uint16_t get_checksum(void *addr, size_t size)
{
  uint32_t sum = 0;
  size_t remaining_bytes_count = size;
  uint16_t *iter = addr;

  while (remaining_bytes_count >= 2) {
    sum += *iter;
    if (sum > 0xffff) {
      sum -= 0xffff;
    }
    iter++;
    remaining_bytes_count -= 2;
  }

  if (remaining_bytes_count == 1) {
    sum += *(uint8_t *)iter;
    if (sum > 0xffff) {
      sum -= 0xffff;
    }
  }
  return ~sum;
}

int main (int argc, char *argv[])
{
	if (argc == 1) {
		print_error(argv[0], "usage error", "Destination address required");
		return 1;
	}

	int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (socket_fd == -1) {
		print_error(argv[0], "socket", strerror(errno));
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
			print_error(argv[0], "inet_pton", strerror(errno));
			close(socket_fd);
			return 2;
	}

	unsigned char buf[sizeof(struct icmphdr) + 56] = {0};
	struct icmphdr *icmp_header = (struct icmphdr *)buf;
	icmp_header->type = ICMP_ECHO;
	icmp_header->code = 0;

	if (56 > sizeof(struct timeval)) {
		if (gettimeofday((struct timeval *)(buf + sizeof(struct icmphdr)), NULL) == -1) {
			print_error(argv[0], "gettimeofday", strerror(errno));
			close(socket_fd);
			return 2;
		}
	}

	icmp_header->checksum = get_checksum(buf, sizeof(struct icmphdr) + 56);

	if (sendto(socket_fd, buf, sizeof(buf), 0,
			(struct sockaddr *)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
		print_error(argv[0], "sendto", strerror(errno));
		close(socket_fd);
		return 2;
	}
	printf("ICMP ECHO REQUEST sent\n");

	close(socket_fd);
	return 0;
}
