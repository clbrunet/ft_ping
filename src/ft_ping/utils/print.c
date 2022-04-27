#include <stdint.h>
#include <stdio.h>

#include "ft_ping/utils/print.h"

void print_error(const char *executable, const char *error_title, const char *error_description)
{
	fprintf(stderr, "%s: %s: %s\n", executable, error_title, error_description);
}

void print_memory(const uint8_t *address, size_t size)
{
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
