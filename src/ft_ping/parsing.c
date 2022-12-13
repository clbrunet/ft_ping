#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <errno.h>

#include "main.h"
#include "utils/ctype.h"
#include "utils/print.h"
#include "utils/error.h"
#include "utils/string.h"

static void usage(void)
{
	printf(
			"\n"
			"Usage\n"
			"  ft_ping [options] <destination>\n"
			"\n"
			"Options:\n"
			"  <destination>   dns name or ip adress\n"
			"  -h              print help and exit\n"
			"  -v              verbose output\n"
			"  -t <ttl>        define time to live\n"
		  );
	exit(2);
}

static void invalid_option(char c)
{
	printf("%s: invalid option -- '%c'\n", g_ping.program_name, c);
	usage();
}

static void invalid_argument(const char *arg, const char* description)
{
	assert(arg != NULL);

	printf("%s: invalid argument: '%s'", g_ping.program_name, arg);
	if (description != NULL) {
		printf(": %s", description);
	}
	printf("\n");
	exit(1);
}

static int64_t parse_int64(const char **arg)
{
	assert(arg != NULL && *arg != NULL);

	uint64_t res = 0;
	const char *iter = *arg;
	bool is_negative = false;
	if (*iter == '+') {
		iter++;
	} else if (*iter == '-') {
		is_negative = true;
		iter++;
	}
	while (*iter != '\0') {
		if (!ft_isdigit(*iter)) {
			invalid_argument(*arg, NULL);
		}
		res *= 10;
		res += *iter - '0';
		if ((is_negative == false && res > (uint64_t)INT64_MAX)
			|| (is_negative == true && res > (uint64_t)INT64_MAX + 1)) {
			invalid_argument(*arg, "Numerical result out of range");
		}
		iter++;
	}
	*arg = iter;
	if (is_negative) {
		return -res;
	}
	return res;
}

static uint8_t parse_uint8(const char **arg)
{
	const char *backup = *arg;
	int64_t res = parse_int64(arg);
	if (!(0 <= res && res <= UINT8_MAX)) {
		invalid_argument(backup, "out of range: 0 <= value <= 255");
	}
	return (uint8_t)res;
}

static int parse_destination(destination_t *destination, const char *destination_arg)
{
	assert(destination != NULL);
	assert(destination_arg != NULL);

	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP,
		.ai_flags = AI_CANONNAME,
	};
	struct addrinfo *res;
	int error_code = getaddrinfo(destination_arg, NULL, &hints, &res);
	if (error_code != 0) {
		print_error(destination_arg, ft_gai_strerror(error_code));
		return -1;
    }
	destination->sockaddr_in = *(struct sockaddr_in *)res->ai_addr;
	ft_strlcpy(destination->name, res->ai_canonname, NI_MAXHOST);
	inet_ntop(AF_INET, (const void *)&destination->sockaddr_in.sin_addr,
			destination->ip, INET_ADDRSTRLEN);
	freeaddrinfo(res);
	return 0;
}

int parse_args(const char *const args[])
{
	g_ping.icmp_payload_size = 56;
	g_ping.is_verbose = false;
	g_ping.ttl = 0;
	const char *destination_arg = NULL;
	while (*args != NULL) {
		const char *arg = *args;
		if (!(arg[0] == '-' && arg[1] != '\0')) {
			if (destination_arg != NULL) {
				print_error("usage error", "Too many destination addresses");
				return -1;
			}
			destination_arg = arg;
			args++;
			continue;
		}
		arg++;
		while (*arg != '\0') {
			switch (*arg) {
				case 'h':
					usage();
					break;
				case 'v':
					g_ping.is_verbose = true;
					break;
				case 't':
					arg++;
					if (*arg == '\0') {
						args++;
						arg = *args;
					}
					g_ping.ttl = parse_uint8(&arg);
					arg--;
					break;
				default:
					invalid_option(*arg);
			}
			arg++;
		}
		args++;
	}
	if (destination_arg == NULL) {
		print_error("usage error", ft_strerror(EDESTADDRREQ));
		exit(1);
	}
	if (parse_destination(&g_ping.destination, destination_arg) == -1) {
		return -1;
	}
	return 0;
}
