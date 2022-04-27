#include <assert.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include "ft_ping/checksum.h"

static uint16_t get_data_sum(uint16_t *addr, size_t size)
{
	assert(addr != NULL);

	uint32_t sum = 0;
	size_t remaining_bytes_count = size;

	while (remaining_bytes_count >= 2) {
		sum += *addr;
		if (sum > UINT16_MAX) {
			sum = (sum & UINT16_MAX) + (sum >> 16);
		}
		addr++;
		remaining_bytes_count -= 2;
	}

	if (remaining_bytes_count == 1) {
		sum += *(uint8_t *)addr;
		if (sum > UINT16_MAX) {
			sum = (sum & UINT16_MAX) + (sum >> 16);
		}
	}
	return sum;
}

uint16_t get_checksum(void *addr, size_t size)
{
	assert(addr != NULL);

	return ~get_data_sum((uint16_t *)addr, size);
}
