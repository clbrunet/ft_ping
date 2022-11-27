#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"
#include "ft_ping/utils/string.h"

void print_error(const char *error_title, const char *error_description)
{
	assert(g_ping.program_name != NULL);
	assert(error_title != NULL);
	assert(error_description != NULL);

	write(STDERR_FILENO, g_ping.program_name, ft_strlen(g_ping.program_name));
	write(STDERR_FILENO, ": ", 2);
	write(STDERR_FILENO, error_title, ft_strlen(error_title));
	write(STDERR_FILENO, ": ", 2);
	write(STDERR_FILENO, error_description, ft_strlen(error_description));
	write(STDERR_FILENO, "\n ", 1);
}

void print_memory(const uint8_t *address, size_t size)
{
	assert(address != NULL);

	if (size == 0) {
		printf("%07x\n", 0);
		return;
	}
	printf("%07x ", 0);
	printf("%02x", *address);
	address++;
	size_t i = 1;
	while (i < size) {
		if (i % 16 == 0) {
			printf("\n%07zx ", i);
		} else if (i % 2 == 0) {
			printf(" ");
		}
		printf("%02x", *address);
		address++;
		i++;
	}
	printf("\n%07zx\n", i);
}

void print_char(char c)
{
	write(1, &c, 1);
}

void print_string(const char *str)
{
	assert(str != NULL);

	write(1, str, ft_strlen(str));
}

void print_number(long long nbr)
{
	unsigned long long u_nbr;
	if (nbr == LLONG_MIN) {
		write(1, "-", 1);
		u_nbr = (unsigned long long)LLONG_MAX + 1;
	} else if (nbr < 0) {
		write(1, "-", 1);
		u_nbr = -nbr;
	} else {
		u_nbr = nbr;
	}

	if (u_nbr >= 10) {
		print_number(u_nbr / 10);
	}
	print_char(u_nbr % 10 + '0');
}

void print_double(double nbr, size_t precision)
{
	print_number((long long)nbr);
	print_char('.');
	for (size_t i = 0; i < precision; i++) {
		nbr -= (long long)nbr;
		nbr *= 10;
		print_number((long long)nbr);
	}
}
