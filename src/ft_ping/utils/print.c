#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "ft_ping/main.h"
#include "ft_ping/utils/print.h"

void print_error(const char *error_title, const char *error_description)
{
	assert(g_vars.program_name != NULL);
	assert(error_title != NULL);
	assert(error_description != NULL);

	fprintf(stderr, "%s: %s: %s\n", g_vars.program_name, error_title, error_description);
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
